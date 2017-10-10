/*
  lamp_timer - Lampen-Zeitschaltuhr

  (c) 2017 by Till Harbaum <till@harbaum.org>

  Turns a lamp on output O1 on for 10 seconds when a push 
  button on input I1 is being pressed.

  Schaltet eine Lampe an Ausgang O1 für 10 Sekunden ein,
  sobald ein Taster an Eingang I1 gedrückt wird.
*/

#include <Ftduino_simple.h>

uint32_t start_time_zeit = 0;

// E: the setup function runs once when you press reset or power the board
// D: Die Setup-Funktion wird einmal ausgeführt, wenn Reset gedrückt oder
//    das Board gestartet wird.
void setup() { }

// E: the loop function runs over and over again forever
// D: Die Loop-Funktion wird endlos immer und immer wieder ausgeführt
void loop() {
  // E: check if push button at I1 is pressed
  // D: Teste, ob der Taster an I1 gedrückt ist
  if(ftduino.input_get(Ftduino::I1)) {
    // E: remember start time
    // D: merke Startzeit
    start_time_zeit = millis();

    // E: switch lamp on (output HI)
    // D: schalte Lampe ein (Ausgang HI)
    ftduino.output_set(Ftduino::O1, Ftduino::HI);
  }

  // E: valid start time and current time later than 10 sec 
  //    (10,000 msec) since then?
  // D: gültige Startzeit und seitdem mehr als 10 Sekunden
  //    (10.000 Millisekunden) verstrichen?
  if((start_time_zeit != 0) &&
     (millis() > start_time_zeit + 10000)) {
    // E: forget start time
    // D: vergiss Startzeit
    start_time_zeit = 0;
    // E: switch lamp off (output OFF)
    // D: schalte Lampe aus (Ausgang OFF)
    ftduino.output_set(Ftduino::O1, Ftduino::OFF);
  }
}

