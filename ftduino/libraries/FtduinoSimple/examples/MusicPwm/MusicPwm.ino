// MusicPwm.ino
// Music-Sketch für den ftDuino auf Basis der Hardware-PWM

#include <FtduinoSimple.h>

#include "pitches.h"

// Noten der Melodie
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// Tonlängen der Melodie: 4 = eine Viertelnote, 8 = eine Achtelnote, ...
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

// Frequenz in Timer-Werte umrechnen
uint16_t freq(int hz) {
  return (uint16_t) (((F_CPU / 8 / 2) + (hz / 2)) / hz) - 1;
}

void hw_pwm_tone(int hz) {
  if(hz == 0)
    TCCR1A = 0;           // PWM-Erzeugung abschalten
  else {
    TCCR1A = bit(COM1C0); // "toggle on compare mode" einschalten für pin OC1C
    OCR1A = freq(hz);     // 16 bit-Reload-Wert entsprechend der Frequenz laden
  }  
}
  
void setup() {
  pinMode(11, OUTPUT);

  // der verwendete Pin 12 des Atmega32u4 ist mit Timer 1 verbunden
  TCCR1A = 0;
  TCCR1B = (bit(WGM12) | bit(CS11)); // CTC mode. Divide by 8 clock prescale
}

void loop() {
  // alle Noten der Melodie durchgehen
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // die Länge der Note entspricht einer Sekunde geteilt durch 
    // die in der Musik übliche Längenangabe. Eine viertel Note
    // dauert also z.B. 1/4 Sekunde bzw. 1000/4 Millisekunden
    int noteDuration = 1000 / noteDurations[thisNote];
    hw_pwm_tone(melody[thisNote]);
    delay(noteDuration);
    hw_pwm_tone(0);

    // um die Noten akkustisch zu trennen fügen wir eine
    // Pause von 30% ein
    int pauseBetweenNotes = noteDuration * 0.30;
    delay(pauseBetweenNotes);
  }
  delay(1000);
}


