// I2cMaster.ino
// ftDuino als aktiver Teilnehmer am I2C-Bus

#include <FtduinoSimple.h>
#include <Wire.h>

void setup() {
  Wire.begin();                       // tritt dem I2C-Bus als Mas6ter bei
}

void set_output(int o, int v) {
  Wire.beginTransmission(42);         // Sende Daten an I2C-Gerät mit Adresse 42
  Wire.write(0+o-Ftduino::O1);        // Sende Nummer des zu schaltenden Ausgangs 
  Wire.write((v==Ftduino::HI)?1:0);   // Sende 1 für "an" oder 0 für "aus"
  Wire.endTransmission();             // Ende der Übertragung
}

void loop() {
  // Taster an Eingang I1 abfragen
  if(ftduino.input_get(Ftduino::I1))
    set_output(Ftduino::O1, Ftduino::HI);
  else
    set_output(Ftduino::O1, Ftduino::LO);
  delay(10);
}
