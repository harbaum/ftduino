/*
  Pwm - Pulsweitenmodulation

  (c) 2017 by Till Harbaum <till@harbaum.org>
*/

#include <FtduinoSimple.h>

uint16_t schaltzeit = 8192;   // 8192 entspricht je 1/2 Sekunde an und aus

// Die Setup-Funktion wird einmal ausgeführt, wenn Reset gedrückt oder
// das Board gestartet wird.
void setup() { }

// warte die angegebene Zeit. Der "zeit"-Wert 8192 soll dabei einer halben Sekunde
// entsprechen. Es muss also "zeit" mal 1000000/8192/2 Mikrosekunden gewartet werden
void warte(uint16_t zeit) {
  while(zeit--)
    _delay_us(1000000/8192/2);
}

// Die Loop-Funktion wird endlos immer und immer wieder ausgeführt
void loop() {
  static uint8_t an_aus = false;      // der aktuelle Ausgang-an/aus-Zustand
  static uint8_t i1=false, i2=false;  // letzter Zustand der Tasten an I1 und I2

  // ist die Taste an I1 gedrückt?
  if(ftduino.input_get(Ftduino::I1)) {
    // und war die Taste vorher nicht gedrückt und ist die
    // aktuelle Schaltzeit kleiner 8192?
    if(!i1 && (schaltzeit < 8192)) {
      // dann verdopple die Schaltzeit
      schaltzeit *= 2;
      // warte eine Millisekunde, falls die Taste nachprellt
      _delay_ms(1);
    }
    // merke, dass die Taste an I1 zur Zeit gedrückt ist
    i1 = true;
  } else
    // merke, dass die Taste an I1 zur Zeit nicht gedrückt ist
    i1 = false;

  // ist die Taste an I2 gedrückt?
  if(ftduino.input_get(Ftduino::I2)) {
    // und war die Taste vorher nicht gedrückt und ist die
    // aktuelle Schaltzeit größer 1?
    if(!i2 && (schaltzeit > 1)) {
      // dann halbiere die Schaltzeit
      schaltzeit /= 2;
      // warte eine Millisekunde, falls die Taste nachprellt
      _delay_ms(1);
    }
    // merke, dass die Taste an I2 zur Zeit gedrückt ist
    i2 = true;
  } else
    // merke, dass die Taste an I2 zur Zeit nicht gedrückt ist
    i2 = false;

  // schalte den Ausgand O2 je nach Zustand der an_aus-Variable an oder aus
  if(an_aus) 
    // wenn der aktuelle an_aus-Zustand wahr ist, dann schalte den Ausgang ein
    ftduino.output_set(Ftduino::O1, Ftduino::HI);
  else
    // wenn der aktuelle an_aus-Zustand unwahr ist, dann schalte den Ausgang aus
    ftduino.output_set(Ftduino::O1, Ftduino::OFF);

  // warte die aktuelle Schaltzeit
  warte(schaltzeit);

  // wechsel den an_aus-Zustand
  an_aus = !an_aus;
}


