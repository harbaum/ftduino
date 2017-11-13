// I2cSlave.ino
// ftDuino als passiver Teilnehmer am I2C-Bus

#include <FtduinoSimple.h>
#include <Wire.h>

void setup() {
  Wire.begin(42);               // tritt I2C-Bus an Adresse #42 als "Slave" bei
  Wire.onReceive(receiveEvent); // Auf Eregnisse registrieren
}

void loop() {
  delay(100);
}

// Funktion, die immer dann aufgerufen wird, wenn vom Master
// Daten üner I2C gesehndet werden. Diese Funktion wurde in
// setup() registriert.
void receiveEvent(int num) {

  // es muss mindestens ein Byte empfangen worden sein
  if(Wire.available()) {
    // erstes Byte ist die Register-Adresse
    int addr = Wire.read();

    // Adressen 0-7 schalten die Ausgänge O1-O8
    if((addr >= 0) && (addr <= 7) && Wire.available()) {
      int value = Wire.read();
      ftduino.output_set(Ftduino::O1+addr, value?Ftduino::HI:Ftduino::LO);
    }
  }
}
