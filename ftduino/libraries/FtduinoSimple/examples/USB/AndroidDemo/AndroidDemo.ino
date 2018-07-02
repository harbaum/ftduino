// AndroidDemo.ino
// 
// Diese Demonstration nimmt Kommandos über USB entgegen und schaltet Ausgang O1
// entsprechend. Außerdem gibt er einmal pro Sekunde über USB eine Nachricht aus
//
// Dieser Sketch kann von der Android-App "ftDuino demo" bedient werden, wenn der
// ftDuino mit einem USB-Host-Kabel mit dem Smartphone verbunden ist.
//
// (c) 2018 by Till Harbaum <till@harbaum.org>

#include <FtduinoSimple.h>

// Abfrage, ob USB verbunden ist
#define USB_CONNECTED (USBSTA & (1<<VBUS))

void setup() {
  // Abfrage des USB-VBUS-Eingangs aktivieren
  USBCON |= (1<<OTGPADE);
  
  // LED initialisieren
  pinMode(LED_BUILTIN, OUTPUT);

  // USB/Seriell initialisieren
  Serial.begin(9600);
}

void loop() {
  // interne rote LED zeigt USB-Verbindung an
  if(USB_CONNECTED) 
    digitalWrite(LED_BUILTIN, HIGH);
  else
    digitalWrite(LED_BUILTIN, LOW);

  // handhabe USB-Kommunikation, wenn USB verbunden ist
  if(Serial && USB_CONNECTED) {
    static int a=0;
    static uint32_t t = 0;
    static char buffer[5] = "", bfill=0;

    // einmal pro Sekunde den Text "COUNTER: xxx" ausgeben
    if(millis() - t > 1000) {
      Serial.print("COUNTER: ");
      Serial.println(a++);    
      t = millis();
    }

    // Eingehden USB-Daten verarbeiten
    while(Serial.available()) {
      // Zeichen lesen
      char c = Serial.read();

      // wenn ein Zeilenende (Return) empfangen wurde kann das
      // empfangende Kommando ausgewertet werden
      if((c == '\n') || (c == '\r')) {

        // wurde das Kommando "ON" empfangen?
        if(!strcmp_P(buffer, PSTR("ON"))) {
          // schalte den Ausgang O1 ein (HIGH bzw. HI)
          ftduino.output_set(Ftduino::O1, Ftduino::HI);
        }

        // wurde das Kommando "OFF" empfangen?
        if(!strcmp_P(buffer, PSTR("OFF"))) {
          // schalte den Ausgang O1 aus (LOW bzw. LO)
          ftduino.output_set(Ftduino::O1, Ftduino::LO);
        }
      
        // reset Puffer
        buffer[0] = 0;
        bfill = 0;
      }

      // ankommende Zeichen in Puffer speichern
      if((c>=32) && (c < 127) && (bfill < 4)) {
        buffer[bfill++] = c;
        buffer[bfill] = 0;
      }
    }
  }
}
