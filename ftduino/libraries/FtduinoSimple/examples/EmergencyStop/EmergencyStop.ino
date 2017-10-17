/*
  EmergencyStop - Not-Aus

  (c) 2017 by Till Harbaum <till@harbaum.org>

  Schaltet einen Ventilator aus, sobald der Not-Aus-Taster
  betätigt wird.
*/

#include <FtduinoSimple.h>

// Die Setup-Funktion wird einmal bei Start des Systems ausgeführt
void setup() { 
  // Ventilator bei Start des Systems einschalten
  ftduino.motor_set(Ftduino::M1, Ftduino::LEFT);

  // Ausgang der internen roten LED aktivieren
  pinMode(LED_BUILTIN, OUTPUT);
  // und LED ausschalten
  digitalWrite(LED_BUILTIN, LOW);
}

// Die Loop-Funktion wird endlos immer und immer wieder ausgeführt
void loop() {
  // Teste, ob der Taster an I1 gedrückt ist
  if(ftduino.input_get(Ftduino::I1)) {
    // Motor bremsen
    ftduino.motor_set(Ftduino::M1, Ftduino::BRAKE);
    // interne rote LED einschalten    
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

