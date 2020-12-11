/*
 *  txt_automation_3achs.ino
 *  
 *  Control sketch for the fischertechnik 3-axis robot driven by
 *  the ftDuino + ftDuino bluetooth module using ftDuinoBlue. 
 *  
 *  For more info see http://ftduino.de/blue  
 */

#include <Wire.h>
#include <avr/pgmspace.h>
#include "ftduinoblue.h"

// use i2c bluetooth adapter
#include "I2cSerialBt.h"
I2cSerialBt btSerial;
#include <Ftduino.h>

#define BUTTON_COLOR_1  "#009688"
#define BUTTON_COLOR_2  "#889600"
#define BUTTON_COLOR_E  "#800000"

#if 1  // vertical arrangement
// the layout to be sent to the ftDuinoBlue app
const char layout[] PROGMEM = 
"<layout orientation='portrait' name='fischertechnik 3-axis robot' bgcolor='#dddddd'>"
"  <space id='1' place='center'/>"
"  <button id='2' size='45' color='white' bgcolor='" BUTTON_COLOR_1 "' place='hcenter;below:1'>△</button>"
"  <button id='3' size='45' color='white' bgcolor='" BUTTON_COLOR_1 "' place='below:2;left_of:2'>◁</button>"
"  <button id='4' size='45' color='white' bgcolor='" BUTTON_COLOR_1 "' place='below:2;right_of:2'>▷</button>"
"  <button id='5' size='45' color='white' bgcolor='" BUTTON_COLOR_1 "' place='hcenter;below:4'>▽</button>"
"  <space id='6' width='10' height='40' place='hcenter;above:2'/>"
"  <button id='7' size='45' color='white' bgcolor='" BUTTON_COLOR_2 "' place='left_of:6;above:6'>|-◂</button>"
"  <button id='8' size='45' color='white' bgcolor='" BUTTON_COLOR_2 "' place='right_of:6;above:6'>|-▸</button>"
"  <space id='9' width='10' height='20' place='hcenter;above:7'/>"
"  <button id='10' size='45' color='white' bgcolor='" BUTTON_COLOR_2 "' place='left_of:9;above:9'>▸○◂</button>"
"  <button id='11' size='45' color='white' bgcolor='" BUTTON_COLOR_2 "' place='right_of:9;above:9'>◂○▸</button>"
"</layout>";
#endif

FtduinoBlue ftdblue(btSerial, layout);

void setup() {
  Serial.begin(9600);     // some debug output is done on Serial (USB)

  ftduino.init();
  
  // inputs I1-I4 are the endstop switches
  ftduino.input_set_mode(Ftduino::I1, Ftduino::SWITCH);   
  ftduino.input_set_mode(Ftduino::I2, Ftduino::SWITCH);    
  ftduino.input_set_mode(Ftduino::I3, Ftduino::SWITCH);
  ftduino.input_set_mode(Ftduino::I4, Ftduino::SWITCH);    

  // register callback for ftduinoblue
  ftdblue.setCallback(ftduinoblue_callback);

  // wait max 1 sec for adapter to appear on bus. This is not
  // needed as begin() will wait for the device. But this way
  // we can use the led as an inidictaor for problems with 
  // the i2c uart adapater
  if(!btSerial.check(1000)) {
    // fast blink with led on failure
    pinMode(LED_BUILTIN, OUTPUT); 
    while(true) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
  }

  // initialize i2c uart to 9600 baud
  btSerial.begin(9600);

  // prepare led
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);   
};

// current state of motors
static uint8_t motor_state[4] = { 0,0,0,0 };

// button ids per motor, e.g. buttons 3 and 4 control first motor M1
static const uint8_t id[4][2] = { { 3,4 }, { 7,8 }, { 2,5 }, { 11,10 } };
  
void process_motor(uint8_t i, FtduinoBlue::reply *r) {
 
  // check if the reported id belongs to one of the 
  // two buttons used for this motor
  if(r->id == id[i][0] || r->id == id[i][1]) {
    uint8_t m = motor_state[i];   // read current motor state
 
    if(r->id == id[i][0]) {       // motor left?
      if(r->state) m |=  1;
      else         m &= ~1;
    }
    if(r->id == id[i][1]) {       // motor right?
      if(r->state) m |=  2;
      else         m &= ~2;
    }

    // don't drive into active endstop
    if(m == 1 && ftduino.input_get(Ftduino::I1+i))
      m = 0;
      
    if(m != motor_state[i])  {
      if(m == 1)      ftduino.motor_set(Ftduino::M1+i, Ftduino::LEFT, Ftduino::MAX);
      else if(m == 2) ftduino.motor_set(Ftduino::M1+i, Ftduino::RIGHT, Ftduino::MAX);
      else            ftduino.motor_set(Ftduino::M1+i, Ftduino::BRAKE, Ftduino::MAX);
      motor_state[i] = m;
    }
  }
}

void process_motors(FtduinoBlue::reply *r) {
  for(char i=0;i<4;i++)
    process_motor(i, r);  
}

void ftduinoblue_callback(struct FtduinoBlue::reply *r) {
  switch(r->type) {
    case FtduinoBlue::FTDB_STATE:
      Serial.println("STATE");   
      // update state of endstop buttons if necessary
      for(char i=0;i<4;i++) {
        if(ftduino.input_get(Ftduino::I1+i)) {
          ftdblue.print("BGCOLOR "); ftdblue.print(id[i][0], DEC); ftdblue.println(" "BUTTON_COLOR_E);
          ftdblue.print("DISABLE "); ftdblue.println(id[i][0], DEC); 
        }
      }
      break;

    case FtduinoBlue::FTDB_BUTTON:
      process_motors(r);
      break;

    case FtduinoBlue::FTDB_SWITCH:
      Serial.print("SWITCH ");
      Serial.print(r->id, DEC);
      Serial.print(" ");
      Serial.println(r->state?"ON":"OFF");
      break;

    case FtduinoBlue::FTDB_SLIDER:
      Serial.print("SLIDER ");
      Serial.print(r->id, DEC);
      Serial.print(" ");
      Serial.println(r->slider, DEC);
      break;
  }
}

void loop() {
  // keep track of endstop states
  static uint8_t endstop_state[4] =  { 0,0,0,0};
 
  // permanently monitor the endstops
  for(char i=0;i<4;i++) {
    
    // check if endstop state has changed
    uint8_t endstop = ftduino.input_get(Ftduino::I1+i);
    if(endstop != endstop_state[i]) {
      // colorize buttons via endstop state
      ftdblue.print("BGCOLOR "); ftdblue.print(id[i][0], DEC); 
      ftdblue.println(endstop?" "BUTTON_COLOR_E:((id[i][0]<5)?" " BUTTON_COLOR_1:" " BUTTON_COLOR_2));

      // and enable/disable them
      if(endstop) ftdblue.print("DISABLE ");
      else        ftdblue.print("ENABLE ");
      ftdblue.println(id[i][0], DEC); 
      
      // if endstop active and motor running in direction of 
      // endstop, then stop the motor
      if(endstop && motor_state[i] == 1) {
        ftduino.motor_set(Ftduino::M1+i, Ftduino::BRAKE, Ftduino::MAX);
        motor_state[i] = 0;
      }

      // save new state
      endstop_state[i] = endstop;
    }    
  }
  
  ftdblue.handle();
};
