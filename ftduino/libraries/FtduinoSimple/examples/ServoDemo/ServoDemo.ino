//
// Servo.ino
//
// (c) 2018 by Till Harbaum <till@harbaum.org>
//

#include "Servo.h"

void setup() {
  // LED initialisieren
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  servo.begin();
}

void loop() {
  static uint8_t value;
  
  if(value < Servo::VALUE_MAX) value++;
  else                         value = 0;
  servo.set(value);

  delay(10);   
}
