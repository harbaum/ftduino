// Pinball.ino
//
// (c) 2017 by Till Harbaum <till@harbaum.org>
//

#include <Ftduino.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(128, 64, &Wire, -1);

#define SCORE_Y  16
#define BALLS_Y  34
#define BALLS_R   8

enum { IDLE=0, BUTTONRELEASE, WAIT4BALL, RUN } state;
uint8_t balls = 0;
uint32_t score = 0;

uint32_t gate_timer = 0;
uint32_t loose_timer = 0;
uint32_t color_timer = 0;

void display_text(String a, String b) {
  display.setTextSize(2);
  
  if(a[0]) {
    display.fillRect(0, SCORE_Y, 128, 14, BLACK);
    display.setCursor(64-(a.length()*12)/2,SCORE_Y);
    display.print(a);
  }

  if(b[0]) {
    display.fillRect(0, BALLS_Y, 128, 2*BALLS_R+1, BLACK);
    display.setCursor(64-(b.length()*12)/2,BALLS_Y);
    display.print(b);
  }
  
  display.display();
}

void balls_display(int b) {
  display.fillRect(0, BALLS_Y, 128, 2*BALLS_R+1, BLACK);

  for (uint8_t i=0;i<b;i++)
    display.fillCircle(64-((int)(2.5*BALLS_R)*b)/2+(int)(2.5*BALLS_R)*i+BALLS_R, BALLS_Y+BALLS_R, BALLS_R, WHITE);

  display.display();
}

void score_display(int i) {
  String score_str = String(i);
  
  display.setTextSize(2);
  display.fillRect(0, SCORE_Y, 128, 14, BLACK);
  display.setCursor(64-(score_str.length()*12)/2,SCORE_Y);
  display.print(score_str);
  display.display();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // initialize the OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.fillRect(0, 0, 128, 12, WHITE);
  display.setTextColor(BLACK);
  display.setCursor(64-(19*6)/2,2);
  display.println("- ftDuino pinball -");
  display.setTextColor(WHITE);
  display.setCursor(64-(19*6)/2,56);
  display.println("ftduino@harbaum.org");
  
  // start all timers, IRQs and the like
  ftduino.init();

  // I1 and I2 are switch inputs
  ftduino.input_set_mode(Ftduino::I1, Ftduino::SWITCH);
  ftduino.input_set_mode(Ftduino::I2, Ftduino::SWITCH);
  ftduino.input_set_mode(Ftduino::I3, Ftduino::SWITCH);     // top left
  ftduino.input_set_mode(Ftduino::I4, Ftduino::SWITCH);     // ball lost
  ftduino.input_set_mode(Ftduino::I8, Ftduino::VOLTAGE);    // color sensor

  // state
  state = IDLE;
  display_text("Press both","buttons");
}

void loop() {
  static uint32_t last_led_event = 0;

  // flash led once a second  
  if((millis() - last_led_event) >= 1000) {
    digitalWrite(LED_BUILTIN, HIGH);    // turn the LED on
    last_led_event = millis();
  } else if((millis() - last_led_event) > 50) 
    digitalWrite(LED_BUILTIN, LOW);     // turn the LED off

  if(state == IDLE) {
    if(ftduino.input_get(Ftduino::I1) &&
       ftduino.input_get(Ftduino::I2)) {

      // light lamps and start compressor
      ftduino.output_set(Ftduino::O3, Ftduino::HI, Ftduino::MAX);
      ftduino.output_set(Ftduino::O6, Ftduino::HI, Ftduino::MAX);
      ftduino.output_set(Ftduino::O7, Ftduino::HI, Ftduino::MAX);

      display_text(">>> GO <<<","");

      balls = 3;
      score = 0;

      // initialize the event timers
      gate_timer = millis();
      loose_timer = millis();
      color_timer = millis();
      
      balls_display(balls);

      state = BUTTONRELEASE;
    }
  } else if(state == BUTTONRELEASE) {

    // wait for player to release both buttons before controlling the bumpers
    if(!ftduino.input_get(Ftduino::I1) &&
       !ftduino.input_get(Ftduino::I2)) 
      state = WAIT4BALL;
      
  } else {   
    // control valves of bumpers
    ftduino.output_set(Ftduino::O4, Ftduino::HI, ftduino.input_get(Ftduino::I1)?Ftduino::MAX:Ftduino::OFF);
    ftduino.output_set(Ftduino::O5, Ftduino::HI, ftduino.input_get(Ftduino::I2)?Ftduino::MAX:Ftduino::OFF);

    // ball passed top left light barrier
    if(!ftduino.input_get(Ftduino::I3)) {
      if(state == WAIT4BALL) {
         // waiting for ball pass the light gate the first time
         score += 100;
         score_display(score);
         state = RUN;
         gate_timer = millis();   // remember of this event
      } else {
        // light gate has been passed previously

        // ignore anything within 500ms after last event
        if(millis() - gate_timer > 500) {
          if(millis() - gate_timer < 5000)
            score += 500;    // 500 points for hitting again within 5 seconds
          else
            score += 100;    // 100 points else

          score_display(score);
          gate_timer = millis();   // remember of this event
        }
      }
    }

    // ball passed loose barrier
    if(!ftduino.input_get(Ftduino::I4)) {
      if(millis() - loose_timer > 1000) {
        if(state == WAIT4BALL) 
          display_text("LUCKY","");
        else {
          balls--;
          if(balls > 0) {  
            display_text("ONCE MORE","");
            balls_display(balls);
            state = WAIT4BALL;
          } else {        
            display_text("","GAME OVER");
            
            // switch offlamps and compressor
            ftduino.output_set(Ftduino::O3, Ftduino::OFF, 0);
            ftduino.output_set(Ftduino::O6, Ftduino::OFF, 0);
            ftduino.output_set(Ftduino::O7, Ftduino::OFF, 0);

            state = IDLE;
          }
        }
      }
      loose_timer = millis();
    }

    // ball passed color sensor
    if(ftduino.input_get(Ftduino::I8) < 1000) {
      if(millis() - color_timer > 100) {
        score += 1000;    // 1000 points
        score_display(score);        
      }
      color_timer = millis();
    } 
  }
}
