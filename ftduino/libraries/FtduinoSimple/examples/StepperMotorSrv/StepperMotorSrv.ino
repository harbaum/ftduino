//
// StepperMotorSrv.ino
//
// (c) 2018 by Till Harbaum <till@harbaum.org>
//
// Interrupt-getriebener Schrittmotorserver
//

#include <FtduinoSimple.h>

static volatile bool direction = true;

ISR(TIMER1_COMPA_vect) { 
  // schrittzaehler 0..3
  static uint8_t state = 0;

  // Zustand auswerten und in nächsten Zustand wechseln
  uint8_t m = (state&1)?Ftduino::M1:Ftduino::M2;
  uint8_t d = (state&2)?Ftduino::LEFT:Ftduino::RIGHT;
  if(direction) state = (state+1)&3;
  else          state = (state-1)&3;
  
  ftduino.motor_set(m, d);
}
    
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // -------- Initialisierung des 16-Bit-Timer 1 ------------

  // 16 Bit Timer 1, CTC with OCR1A
  TCCR1A = 0;
  TCCR1B = (1<<WGM12);
  TCCR1C = 0;

  // 62500/(step/s) = timer clocks per step
  TCNT1 = 0;
  OCR1A = 62500/400;

  // generate interrupt when timer passes OCR1A
  TIMSK1 = (1<<OCIE1A);
}

void stepper_run(bool on) {
  if(on) {
    TCCR1B |= (1<<CS12);  // start timer at 1/256 F_CPU -> 62.5kHz 
  } else {
    ftduino.motor_set(Ftduino::M1, Ftduino::OFF);
    ftduino.motor_set(Ftduino::M2, Ftduino::OFF); 
    TCCR1B &= ~(1<<CS12); 
  }
}

void stepper_direction(bool d) {
   direction = d;
}

void loop() {
  static uint32_t last_led_event = 0;
  // flash led once a second  
  if((millis() - last_led_event) >= 1000) {
    digitalWrite(LED_BUILTIN, HIGH);    // turn the LED on
    last_led_event = millis();
  } else if((millis() - last_led_event) > 50) 
    digitalWrite(LED_BUILTIN, LOW);     // turn the LED off

  static int8_t stepper = 0;  // aus
  delay(10);

  // Taste an I1 gedrückt
  if(ftduino.input_get(Ftduino::I1)) {
    if(stepper == 0) {
      stepper_direction(true);
      stepper_run(true);
      stepper = 1;
    }
  }
  
  // Taste an I2 gedrückt
  if(ftduino.input_get(Ftduino::I2)) {
    if(stepper == 0) {
      stepper_direction(false);
      stepper_run(true);
      stepper = -1;
    }
  }

  if(!ftduino.input_get(Ftduino::I1) && !ftduino.input_get(Ftduino::I2)) {
    if(stepper) {
      stepper_run(false);
      stepper = 0;
    }
  }
  

}



