/*
  Debounce

  Demonstriert Tastenprellen
*/

#include <FtduinoSimple.h>

// die setup-Funktion wird einmal beim Start aufgerufen
void setup() {
  Serial.begin(9600);

  while(!Serial);      // warte auf USB-Verbindung

  Serial.println("ftDuino Tastenprell-Beispiel");
}

uint8_t letzter_zustand = false;
uint8_t wechselzaehler = 0;

// die loop-Funktion wird immer wieder aufgerufen
void loop() {
  uint8_t zustand = ftduino.input_get(Ftduino::I1);  // Taster auslesen
  
  if(zustand != letzter_zustand) {         // Hat der Zustand sich geändert?
    wechselzaehler = wechselzaehler + 1;   // Ja, Zähler rum eins erhöhen
    
    Serial.print("I1 ");                   // und eine Meldung ausgeben
    Serial.print(wechselzaehler);
    Serial.println(" mal geändert");
    letzter_zustand = zustand;             // neuen Zustand als letzten merken
  }
}

