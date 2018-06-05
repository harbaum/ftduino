#include "Servo.h"

Servo servo;

// Timer 1 compare a interrupt service
void Servo::isr_exec() { 
  if(OCR1A > CYCLE/2) {
    PORTD |= (1<<1);
    OCR1A = CMIN+value;
  } else {
    PORTD &= ~(1<<1);
    OCR1A = CYCLE-CMIN-value; 
  }
}

// diese Interrupt-Routine wird von der Timer-Hardware automatisch
// aufgerufen.
void Servo::isr() {
  servo.isr_exec();
}

Servo::Servo() { }

// Initialisierte Timer 1, um im Hintergrund, das Servo-Signal zu erzeugen
void Servo::begin(void) {
  // 16 Bit Timer 1, CTC with OCR1A
  TCCR1A = 0;
  TCCR1B = (1<<WGM12);
  TCCR1C = 0;

  TCNT1 = 0;
  OCR1A = CYCLE; 
  TCCR1B |= (1<<CS11) | (1<<CS10); // start timer at 1/64 F_CPU -> 250 khz

  // Interrupt erzeugen, wenn der Zähler OCR1A passiert
  TIMSK1 = (1<<OCIE1A);

  // sda (port d.1) als Ausgang nutzen und low schalten
  // scl wäre port d.0
  DDRD |= (1<<1);
  PORTD &= ~(1<<1);
}

void Servo::set(uint8_t v) {
  value = v;
}

