/*
  HighLevelRack - fischertechnik Hochregallager
*/

#include <Ftduino.h>

#include "Tower.h"
#include "Arm.h"

// Instanzen der Objekte des Modells
Arm arm;
Tower tower;

bool ok = true;

#define MAX_CMD 32
char rxCur = 0;
char rxBuf[MAX_CMD+1]; // an array to store the received data + Nullbyte

// die setup-Funktion wird einmal beim Start aufgerufen
void setup() {
  Serial.begin(115200);  // seriellen USB-Port öffnen

  // warte 5 Sekunden auf PC
  while(!Serial);        // warte auf PC

  Serial.println("ftDuino - Hochregallager");
  
  ftduino.init();        // initialisierte die ftDuino-Hardware 
  arm.init();
  tower.init();

  Serial.println("Befehle:");
  Serial.println("hol <X>        Holt das Objekt aus Fach X");
  Serial.println("bring <X>      Bringt das Objekt in Fach X");

  Serial.print("Initialisierung ...");
  
  // ziehe Arm ein
  if(!arm.moveHome()) {
    Serial.println(" Fehler: Arm");
    Serial.println("Programmausführung gestoppt");
    ok = false;
    return;
  }
    
  // fahre Turm auf Parkposition
  if(!tower.moveHome()) {
    Serial.println(" Fehler: Turm");
    Serial.println("Programmausführung gestoppt");
    ok = false;
    return;
  }
    
  Serial.println(" ok");
  Serial.print("> ");
}

// die loop-Funktion wird immer wieder aufgerufen
void loop() {
  // lies alle Bytes, die per USB/Serial empfangen wurden
  while(ok && Serial.available()) {
    char c = Serial.read();

    // "return" beendet ein Kommando
    if((c == '\n') || (c == '\r')) {
      if(rxCur > 0) {
        Serial.println("");
        rxBuf[rxCur] = '\0';

        // separiere bei erstem leerzeichen, wenn vorhanden
        char *parm = NULL;        
        if(strchr(rxBuf, ' ')) {
          parm = strchr(rxBuf, ' ')+1;
          *strchr(rxBuf, ' ') = '\0';
        }

        else if((strcasecmp_P(rxBuf, PSTR("hol")) == 0) && parm) {
          int c = atoi(parm);
          Serial.print("Hole aus Fach ");
          Serial.println(c, DEC);
          tower.moveTo(c, false);
          arm.moveOut();
          tower.moveTo(c, true);
          arm.moveIn();
          tower.moveTo(0, true);
          arm.moveOut();
          tower.moveTo(0, false);
          arm.moveIn();
          tower.moveHome();
        }

        else if((strcasecmp_P(rxBuf, PSTR("bring")) == 0) && parm) {
          int c = atoi(parm);
          Serial.print("Bring in Fach ");
          Serial.println(c, DEC);
          
          arm.moveOut();
          tower.moveTo(0, true);
          arm.moveIn();
          tower.moveTo(c, true);
          arm.moveOut();
          tower.moveTo(c, false);
          arm.moveIn();
          tower.moveTo(0, false);
          tower.moveHome();
        }

        rxCur = 0;
        Serial.print("> ");
      }
    } else if(rxCur < MAX_CMD) {
      Serial.print(c);
      rxBuf[rxCur++] = c;
    }
  }
}


