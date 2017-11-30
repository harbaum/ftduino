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

void setup() {
  pinMode(11, OUTPUT);
}

void loop() {
  // alle Noten der Melodie durchgehen
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // die Länge der Note entspricht einer Sekunde geteilt durch 
    // die in der Musik übliche Längenangabe. Eine viertel Note
    // dauert also z.B. 1/4 Sekunde bzw. 1000/4 Millisekunden
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(11, melody[thisNote], noteDuration);

    // um die Noten akkustisch zu trennen fügen wir eine
    // Pause von 30% ein
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
  }
  delay(1000);
}


