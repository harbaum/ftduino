/*
  OnOffTristate - der dritte Zustand
*/

#include <FtduinoSimple.h>

void setup() { }

// Die Loop-Funktion wird endlos immer und immer wieder ausgeführt
void loop() {
  // Ausgang O1 auf 9V schalten
  ftduino.output_set(Ftduino::O1, Ftduino::HI);
  delay(1000);
  // Ausgang O1 auf Masse schalten
  ftduino.output_set(Ftduino::O1, Ftduino::LO);
  delay(1000);
  // Ausgang O1 unbeschaltet lassen
  ftduino.output_set(Ftduino::O1, Ftduino::OFF);
  delay(1000);
}

