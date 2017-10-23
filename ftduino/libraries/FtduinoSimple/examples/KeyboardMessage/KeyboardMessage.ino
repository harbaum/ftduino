/*
  KeyboardMessage - USB-Tastatur

  Der ftDuino gibt sich als USB-Tastatur aus und "tippt" eine Nachricht, sobald
  ein Taster an Eingang I1 für mindestens 10 Millisekunden gedrückt wird.

  Basierend auf dem Sketch:
  http://www.arduino.cc/en/Tutorial/KeyboardMessage

  Dieser Beispielcode ist Public-Domain.
*/

#include <FtduinoSimple.h>
#include <Keyboard.h>

unsigned long lastButtonEvent = 0;
uint16_t previousButtonState = Ftduino::OFF;    // for checking the state of a pushButton

void setup() {
  // initialize control over the keyboard:
  Keyboard.begin();
}

void loop() {
  // Taste an Eingang I1 auslesen
  uint16_t buttonState = ftduino.input_get(Ftduino::I1);
  
  // Hat sich der Zustand der Taste geändert?
  if(buttonState != previousButtonState) {
    // ja, Zeit des Wechsels merken
    lastButtonEvent = millis();
    // und den neuen Zustand merken, damit wir weitere
    // Änderungen erkennen können
    previousButtonState = buttonState;
  }

  // Gibt es ein unbearbeitetes Eregnis und hat sich der Zustand der Taste seitdem
  // für mehr als 10 Millisekunden nicht geändert?
  if(lastButtonEvent && ((millis() - lastButtonEvent) > 10)) {
    // Zeit dieses Ereignisses vergessen
    lastButtonEvent = 0;

    // Taste wurde gedrückt
    if(buttonState) {
      // Nachricht "tippen"
      Keyboard.println("Hallo vom ftDuino!");
    }
  }
}

