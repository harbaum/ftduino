// I2cMaster.ino
// ftDuino als aktiver Teilnehmer am I2C-Bus
// Zur Verwendung mit mehreren weiteren ftDuinos, auf denen der
// Ftduino/I2C/I2cSlave.ino-Sketch mit ggf. angepasster Adresse läuft

#include <Ftduino.h>
#include <Wire.h>

// Liste der anzusteuernden I2c-Clients, beginnend mit 0 (der Master selbst)
// und -1 als Endemarkierung. In diesem Fall sind drei Clients unter den Adressen
// 43, 44 und 45 angeschlossen
static const int8_t clients[] = { 0, 43, 44, 45, -1 };

#define OUTPUT_PORT  Ftduino::O1

void setup() {
  ftduino.init();

  // Taster an Eingang I1
  ftduino.input_set_mode(Ftduino::I1, Ftduino::SWITCH);

  Wire.begin();                              // tritt dem I2C-Bus als Master bei

  delay(100);                                // Verzögerung, damit Clients fertig gebootet sind

  // an allen clients den Ausgang O1 als Enzelausgang gegen 9V schalten
  for(uint8_t i=0; clients[i] >= 0; i++) {
    // Ausgang O1 als Einzelausgang gegen 9V schalten
    Wire.beginTransmission(clients[i]);        // Sende Daten an I2C-Gerät der angegebenen Adresse 
    Wire.write((OUTPUT_PORT-Ftduino::O1)<<1);  // Sende Modus-Registernummer des Ausgangs 
    Wire.write(0x01);                          // Modus 0x01
    Wire.endTransmission();                    // Ende der Übertragung
  }
}

void set_output(uint8_t client, unsigned char o, unsigned char v) {
  // client 0 ist der Master selbst
  if(client == 0) {
    ftduino.output_set(Ftduino::O1+o, Ftduino::HI, (v * Ftduino::MAX)/255);
  } else {
    Wire.beginTransmission(client);            // Sende Daten an I2C-Gerät mit der angegebenen Adresse
    Wire.write(((o-Ftduino::O1)<<1)+1);        // Sende Ausgangswert-Registersnummer des Ausgangs 
    Wire.write(v);                             // Sende Ausgangswert
    Wire.endTransmission();                    // Ende der Übertragung
  }
}

void loop() {
  static uint8_t client = 0; 

  set_output(clients[client], OUTPUT_PORT, 0);

  // Lauflicht vorwärts oder rückwärts
  if(ftduino.input_get(Ftduino::I1)) {
    client++;
    if(clients[client] == -1) client = 0;
  } else {
    if(client > 0)
      client--;
    else
      while(clients[client+1] != -1) client++; 
  }
  
  set_output(clients[client], OUTPUT_PORT, 255);

  delay(250);
}
