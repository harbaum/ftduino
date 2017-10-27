/*
  BounceVisu

  visualisiert Tastenprellen
*/

#include <FtduinoSimple.h>

#define EVENT_TIME  480    // 480us
uint8_t event[EVENT_TIME];

// die setup-Funktion wird einmal beim Start aufgerufen
void setup() {
  Serial.begin(9600);
  while(!Serial);      // warte auf USB-Verbindung
}

uint8_t letzter_zustand = false;
uint8_t wechselzaehler = 0;

// die loop-Funktion wird immer wieder aufgerufen
void loop() {

  // Warte bis Taster gedrückt
  if(ftduino.input_get(Ftduino::I1)) {

    // hole 480 Mikrosekunden lang im MiKrosekundentakt je einen Eingangswert
    for(uint16_t i=0;i<EVENT_TIME;i++) {
      event[i] = ftduino.input_get(Ftduino::I1);
      _delay_us(1);
    }

    // gib zunächst 20 Nullen aus
    for(uint16_t i=0;i<20;i++)
      Serial.println(0);

    // gib die eingelesenen 480 Werte aus
    for(uint16_t i=0;i<EVENT_TIME;i++) 
      Serial.println(event[i]);

    // Warte eine Sekunde
    delay(1000);
  }
}

