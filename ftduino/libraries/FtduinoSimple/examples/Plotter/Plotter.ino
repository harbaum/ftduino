// Plotter.ino
// a very simple HP/GL plotter using two stepper motors

// enable support for the v1.2 with internal display
#define ENABLE_DISPLAY

// run even without hw connected
// #define NOHW

#define NONE         0
#define SERVOSHIELD  1
#define I2CCLIENT    2  // this gas never been fully implemented and my need some work

#define PEN  SERVOSHIELD   // can be NONE, SERVOSHIELD, I2CCLIENT, ...

#if PEN == SERVOSHIELD
#include <Adafruit_PWMServoDriver.h>
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
#define SERVO_NUM    0    // leftmost connector
#define SERVO_UP   350
#define SERVO_DOWN 410
#endif

// enable xon/xoff signalling
// #define ENABLE_XONOFF

#include <Wire.h>

#ifdef ENABLE_DISPLAY
#include "FtduinoDisplay.h"
FtduinoDisplay display;
#endif

#include <FtduinoSimple.h>

#define MAX_CMD 16

// 
#define PLT_STEP   0.025    // steps per mm in plt file


#if 0  
// -------------- slow but precise and flexible float calculation
// motor steps per full rotation
#define MOTOR_STEPS       200
#define GEAR_MM_PER_ROT   4.7   // head movement on full motor rotation 
#define HW_MM_PER_STEP    (GEAR_MM_PER_ROT / MOTOR_STEPS)

// conversion to plotter units
#define SCALE        1.0
#define PLT2HW(a)    (SCALE*((a)*PLT_STEP/HW_MM_PER_STEP))
#else
// --------------- faster integer calculation 
// #define PLT2HW(a)    (25*((uint32_t)a)/24)   // 100% size, columbia = 35x24cm
#define PLT2HW(a)    (25*((uint32_t)a)/48)   // 50% size, columbia = 17.5 * 12cm
//#define PLT2HW(a)    (25*((uint32_t)a)/92)   // 25% size, columbia = 9 * 6cm
#endif

// HP/GL command buffer
char buffer[MAX_CMD+1];
unsigned char bcnt;

// current pen position and state (up or down)
bool pd;
int px, py;

// motors run up to 400 steps per second -> 120 rpm
#define STEPS_PER_SEC  400
#define STEP_DELAY_US  (1000000/STEPS_PER_SEC)
#define STEP_DELAY     _delay_us(STEP_DELAY_US)

#define X 0
#define Y 1

// timer indicating when the plotter head was home'd last time. Used
// to display plot time
uint32_t timer;

void step(uint8_t m, int8_t value) {
  static char stepper_state[2];  
  // stepper motor state (one of four possible coil states)
  static const uint8_t stepper_coil_state[4][2] = {
    { Ftduino::LEFT,  Ftduino::RIGHT },
    { Ftduino::LEFT,  Ftduino::LEFT  },
    { Ftduino::RIGHT, Ftduino::LEFT  },
    { Ftduino::RIGHT, Ftduino::RIGHT }
  };

  if(!value) stepper_state[m] = 0;
  else       stepper_state[m] = (stepper_state[m] + value)&3;
  
  uint8_t port = (m==X)?Ftduino::M1:Ftduino::M3;
  ftduino.motor_set(port,   stepper_coil_state[stepper_state[m]][0]);
  ftduino.motor_set(port+1, stepper_coil_state[stepper_state[m]][1]);  
}

bool home() {
#ifdef ENABLE_DISPLAY
  display.setTextSize(2);
  display.fillRect(0, 0, 128, 32, BLACK);
  display.setCursor(0,  0); 
  display_printF(PSTR("Homing ..."));
  display.display();
#endif

  // power up motors
  step(X, 0); step(Y, 0);
  
  // run both motors in home direction while switches are closed
  while(ftduino.input_get(Ftduino::I1) || ftduino.input_get(Ftduino::I2)) {
    if(ftduino.input_get(Ftduino::I1)) step(X, -1);    
    if(ftduino.input_get(Ftduino::I2)) step(Y, -1);
    STEP_DELAY;
  }

  // pause half a second
  delay(500);

  // plotter head should now be home, drive 1 second out of home position
  uint32_t t = millis();
  while(millis() - t < 1000) {
    step(X, +1);
    step(Y, +1);    
    STEP_DELAY;
  }

  // both switches _must_ now be closed  
  if(!ftduino.input_get(Ftduino::I1) || !ftduino.input_get(Ftduino::I2)) {
    ftduino.motor_set(Ftduino::M1, Ftduino::OFF);
    ftduino.motor_set(Ftduino::M2, Ftduino::OFF);
    ftduino.motor_set(Ftduino::M3, Ftduino::OFF);
    ftduino.motor_set(Ftduino::M4, Ftduino::OFF);
    return false;
  }

  // again drive both motors home
  while(ftduino.input_get(Ftduino::I1) || ftduino.input_get(Ftduino::I2)) {
    if(ftduino.input_get(Ftduino::I1)) step(X, -1);  
    if(ftduino.input_get(Ftduino::I2)) step(Y, -1);
    STEP_DELAY;
  }

  // pause half a second
  delay(500);

#if 0
  // optionally power down motors while not being used
  ftduino.motor_set(Ftduino::M1, Ftduino::OFF);
  ftduino.motor_set(Ftduino::M2, Ftduino::OFF);
  ftduino.motor_set(Ftduino::M3, Ftduino::OFF);
  ftduino.motor_set(Ftduino::M4, Ftduino::OFF);
#endif
    
  pd = false;
  px = 0;
  py = 0;

  return true;
}

#ifdef ENABLE_DISPLAY
void display_printF(const char *str) {
  char c = pgm_read_byte(str++);
  while(c) {  
    display.print(c);
    c = pgm_read_byte(str++);
  }    
}
#endif

void failure(const char *str, int code) {
#ifdef ENABLE_DISPLAY
  display.setTextSize(2);
  display.fillRect(0, 0, 128, 32, BLACK);
  display.setCursor(0,  0); 
  display_printF(PSTR("Error: "));
  display.print(code);
  display.setTextSize(1);
  display.setCursor(0, 24); 
  display_printF(str);
  display.display();
#endif

  while(1) {
    delay(1000);
    for(char i=0;i<code;i++) {
      digitalWrite(LED_BUILTIN, HIGH);    // turn the LED on
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);     // turn the LED off
      delay(100);
    }
  }
}

#if PEN == I2CCLIENT
// The pen may electrically be attached in various ways.
// This version uses a second ftDuino connected via I²C and
// running the Ftduino/I2C/I2cClient sketch

bool pen_get_i1_switch(void) {
  uint16_t sw;

  // read I1
  Wire.beginTransmission(43);
  Wire.write(0x10);               // select I1
  Wire.endTransmission( false );
  Wire.requestFrom(43, 2);        // request two bytes
  sw = Wire.read();               // read both bytes
  sw += 256 * Wire.read();
  Wire.endTransmission();

  return sw;
}

void pen_set_m1(uint8_t state) {
  // set O1
  Wire.beginTransmission(43);
  Wire.write(0x00);              // O1/M1
  switch(state) {
     case Ftduino::RIGHT: Wire.write(0x13); break;  // motor turn right
     case Ftduino::LEFT:  Wire.write(0x12); break;  // motor turn left
     default:             Wire.write(0x11); break;  // motor brake
  }  
  Wire.endTransmission();   
}
#endif

void pen(bool down) {
#if PEN == SERVOSHIELD
  if(down) pwm.setPWM(SERVO_NUM, 0, SERVO_DOWN);
  else     pwm.setPWM(SERVO_NUM, 0, SERVO_UP);
  delay(100);  // give pen some time to move
#endif
#if PEN == I2CCLIENT
  if(down) {
    pen_set_m1(Ftduino::LEFT);    
    // wait for switch on I1 to close
    while(!pen_get_i1_switch());
  } else {
    pen_set_m1(Ftduino::RIGHT);    
    delay(500);
  }
  pen_set_m1(Ftduino::BRAKE);  
#endif
}

bool pen_init(void) {
#if PEN == I2CCLIENT 
  // check for i2c client ftDuino
  delay(100);
  Wire.beginTransmission(43);
  if(Wire.endTransmission())
    return false;

  Wire.beginTransmission(43);
  Wire.write(0x10);   // config I1
  Wire.write(0x02);   // switch
  Wire.endTransmission();
#endif

#if PEN == SERVOSHIELD
  pwm.begin();  
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates

#if 0
  while(1) {    
    pwm.setPWM(SERVO_NUM, 0, SERVO_DOWN);
    delay(2000);
    pwm.setPWM(SERVO_NUM, 0, SERVO_UP);
    delay(500);
  }
#endif
#endif

  return true;
}

void display_state() {
#ifdef ENABLE_DISPLAY
  static uint16_t t;

  display.setTextSize(2);
  display.fillRect(0, 0, 128, 32, BLACK);
  display.setCursor(0,  0); display.print("X:" + String(px));
  display.setCursor(0, 16); display.print("Y:" + String(py));

  // pen being home resets timer
  if(!px && !py) timer = millis();
  else t = (millis() - timer)/1000;
    
  display.setTextSize(1);
  char buf[8];
  sprintf(buf, "%2d:%02d", t/60, t%60);
  display.setCursor(128-6*strlen(buf),  0);
  display.print(buf);
  display.display();
#endif
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_BUILTIN, OUTPUT);

  // I²C is being used for the display as well as the second "pen" ftduino
  Wire.begin();
  
#ifdef ENABLE_DISPLAY
  display.begin();
  display.setTextColor(WHITE);
#endif

  if(!pen_init())
     failure(PSTR("Pen init failed"), 2);

  pen(false); 
  
  // reset state
  buffer[0] = 0;
  bcnt = 0;  

#ifndef NOHW
  // recalibrate / home
  if(!home())
     failure(PSTR("Homing failed"), 3);
#endif

  delay(1000);
  display_state();
}


void loop() {
  if(Serial.available()) {
    // read char
    char c = Serial.read();
  
    if(c == ';') {
      if(!buffer[0]) {
        // ignore empty commands 
      } else if(buffer[0] == 0x1b) {
        // ignore escape commands
      } else if(buffer[0] == 'P') {
        // send xoff to stop data flow while moving pen
#ifdef ENABLE_XONOFF
        Serial.write(0x13);
#endif
        
        if(buffer[1] == 'U') {
          if(pd) pen(false);          
          pd = false;
        } else if(buffer[1] == 'D') {
          if(!pd) pen(true);          
          pd = true;
        }

        // PA/PU/PD can move the pen
        if((buffer[1] == 'A') || (buffer[1] == 'U') || (buffer[1] == 'D')) {
          // parse two coordinates
          char *b = buffer+2;
          while(*b && !isdigit(*b)) b++;
          if(*b) {
            int x = atoi(b);
            while(*b &&  isdigit(*b)) b++;
            while(*b && !isdigit(*b)) b++;
            if(*b) {
              int y = atoi(b);

              // convert x/y from PLT to plotter hw steps
              x = PLT2HW(x);
              y = PLT2HW(y);             
              
              // drive with bresenham
              int xdist = abs(x-px);
              int ydist = abs(y-py);
              int xstep = (px<x)?1:-1;  
              int ystep = (py<y)?1:-1;  

              // use longer run
              if(xdist > ydist) {
                int err = xdist/2;
                while(px != x) {
                  step(X, xstep);
                  px += xstep;
                  err -= ydist;
                  if(err < 0) {
                    step(Y, ystep);
                    py += ystep;
                    err += xdist;
                  }
                  STEP_DELAY;
                }
              } else {
                int err = ydist/2;
                while(py != y) {
                  step(Y, ystep);
                  py += ystep;
                  err -= xdist;
                  if(err < 0) {
                    step(X, xstep);
                    px += xstep;
                    err += ydist;
                  }
                  STEP_DELAY;
                }
              }

              display_state();
            }           
          }
        }
#ifdef ENABLE_XONOFF
        Serial.write(0x11);
#endif
      } 
      
      buffer[0] = 0;
      bcnt = 0;          
    } else if(bcnt < MAX_CMD) {
      buffer[bcnt++] = c;
      buffer[bcnt] = 0;
    }
  }
}
