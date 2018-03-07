// BlinkO1.ino
//
// Blinken einer Lamoe an Ausgang O1
//
// (c) 2018 by Till Harbaum <till@harbaum.org>

#include <FtduinoSimple.h>

void setup() {
  // LED initialisieren
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // schalte die interne LED und den Ausgang O1 ein (HIGH bzw. HI)
  digitalWrite(LED_BUILTIN, HIGH);
  ftduino.output_set(Ftduino::O1, Ftduino::HI);

  delay(1000);                       // warte 1000 Millisekunden (eine Sekunde)

  // schalte die interne LED und den Ausgang O1 aus (LOW bzw. LO)
  digitalWrite(LED_BUILTIN, LOW);
  ftduino.output_set(Ftduino::O1, Ftduino::LO);
  
  delay(1000);                       // warte eine Sekunde
}
