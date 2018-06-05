// einfache Servo-Bibliothek für den ftDuino

#ifndef SERVO_H
#define SERVO_H

#include <Arduino.h>

#define SERVO_STRINGIFY(name) #name
#define SERVO_CLASS_IRQ(name, vector) \
    static void name(void) asm(SERVO_STRINGIFY(vector)) \
    __attribute__ ((signal, __INTR_ATTRS))

// der Timer läuft mit 1/64 des 16MHz CPU-Takt, also 250kHz
// Umrechnung einer Zeit in Millisekunden in Timer-Schritte
#define MS2CYCLE(a)   (F_CPU/64/(1000/(a)))

// der Servo-Zyklus soll 20ms betragen
#define CYCLE MS2CYCLE(20)    // 20ms
#define CMIN  MS2CYCLE(1)     // 1ms
#define CMAX  MS2CYCLE(2)     // 2ms
#define CRANGE (CMAX-CMIN)

class Servo {
  public:
    Servo();
    void begin(void);
    void set(uint8_t v);

    static const uint8_t VALUE_MAX = CRANGE;

  private:  
    void isr_exec(void);
    SERVO_CLASS_IRQ(isr, TIMER1_COMPA_vect);
    volatile uint8_t value;
};

extern Servo servo;

#endif // SERVO_H
