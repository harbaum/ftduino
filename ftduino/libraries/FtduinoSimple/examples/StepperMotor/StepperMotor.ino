//
// StepperMotor.ino
//
// (c) 2018 by Till Harbaum <till@harbaum.org>
//

#include <FtduinoSimple.h>

// 200 Schritte/Sek -> 1 Umdrehung pro Sekunde
// 400 Schritte/Sek -> 2 Umdrehungen pro Sekunde
// ...
#define STEPS_PER_SEC  400
#define US_PER_STEP (1000000/STEPS_PER_SEC)

void setup() {
  ftduino.motor_set(Ftduino::M1, Ftduino::LEFT);
  ftduino.motor_set(Ftduino::M2, Ftduino::LEFT); 
}

void loop() {

  // Taste an I1 gedrückt
  if(ftduino.input_get(Ftduino::I1)) {
    // Motor dreht
    ftduino.motor_set(Ftduino::M1, Ftduino::LEFT);
    _delay_us(US_PER_STEP);
    ftduino.motor_set(Ftduino::M2, Ftduino::LEFT); 
    _delay_us(US_PER_STEP);
    ftduino.motor_set(Ftduino::M1, Ftduino::RIGHT);
    _delay_us(US_PER_STEP);
    ftduino.motor_set(Ftduino::M2, Ftduino::RIGHT); 
    _delay_us(US_PER_STEP);
    
  // Taste an I2 gedrückt
  } else if(ftduino.input_get(Ftduino::I2)) {
    // Motor dreht in Gegenrichtung
    ftduino.motor_set(Ftduino::M1, Ftduino::LEFT);
    _delay_us(US_PER_STEP);
    ftduino.motor_set(Ftduino::M2, Ftduino::RIGHT); 
    _delay_us(US_PER_STEP);   
    ftduino.motor_set(Ftduino::M1, Ftduino::RIGHT);
    _delay_us(US_PER_STEP);
    ftduino.motor_set(Ftduino::M2, Ftduino::LEFT); 
    _delay_us(US_PER_STEP);
  }
}
