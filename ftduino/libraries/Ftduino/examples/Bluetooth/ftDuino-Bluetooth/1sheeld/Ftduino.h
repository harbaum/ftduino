/*
  Ftduino.h - Library for ftduino

  (c) 2017 by Till Harbaum <till@harbaum.org>
*/

#ifndef ftduino_h
#define ftduino_h

#include "Arduino.h"

#define STRINGIFY(name) #name
#define CLASS_IRQ(name, vector) \
    static void name(void) asm(STRINGIFY(vector)) \
    __attribute__ ((signal, __INTR_ATTRS))

extern const PROGMEM unsigned short adc2r[];

// the PWM rate is roughly SPI-CLK/36/CYCLE_LEN
// if spi_state is 8 bit, SPI_PWM_CYCLE_LEN must be at most 256
#define SPI_PWM_CYCLE_LEN 64    // must be a power of 2

class Ftduino {
  public:
    Ftduino();

    void init();

    // constant names for the counter inputs
    static const uint8_t C1 = 0, C2 = 1, C3 = 2, C4 = 3;

    // constant names for the inputs
    static const uint8_t I1 = 0, I2 = 1, I3 = 2, I4 = 3;
    static const uint8_t I5 = 4, I6 = 5, I7 = 6, I8 = 7;

    // input modes
    static const uint8_t RESISTANCE = 0, VOLTAGE = 1, SWITCH = 2;
    
    // constant names for the outputs
    static const uint8_t O1 = 0, O2 = 1, O3 = 2, O4 = 3;
    static const uint8_t O5 = 4, O6 = 5, O7 = 6, O8 = 7;
    static const uint8_t M1 = 0, M2 = 1, M3 = 2, M4 = 3;

    // input counter modes
    static const uint8_t C_EDGE_NONE = 0, C_EDGE_RISING = 1, C_EDGE_FALLING = 2, C_EDGE_ANY = 3;
    
    // max/ḿin PWM values and level namesVOLTAGE
    static const uint8_t MAX = SPI_PWM_CYCLE_LEN, ON = SPI_PWM_CYCLE_LEN, OFF = 0;   // name for extreme PWM values
    static const uint8_t HI = 1, LO = 2;               // OFF = 0 also applies

    // motor modes
    static const uint8_t LEFT = 1, RIGHT = 2, BRAKE = 3;

    void input_set_mode(uint8_t ch, uint8_t mode);
    uint16_t input_get(uint8_t ch);

    void output_set(uint8_t port, uint8_t mode, uint8_t pwm);
    void motor_set(uint8_t port, uint8_t mode, uint8_t pwm);
    void motor_counter(uint8_t port, uint8_t mode, uint8_t pwm, uint16_t counter);
    bool motor_counter_active(uint8_t port);
    void motor_counter_set_brake(uint8_t port, bool on);

    void counter_set_mode(uint8_t ch, uint8_t mode);
    uint16_t counter_get(uint8_t ch);
    void counter_clear(uint8_t ch);
    bool counter_get_state(uint8_t ch);

  private:

    // --------------- ADC -------------
    void adc_init();
    
    void adc_prepare(uint8_t ch);
    uint16_t adc_get(uint8_t ch);
    uint16_t adc_get_r(uint8_t ch);
    uint16_t adc_get_v(uint8_t ch);
    uint16_t adc_get_s(uint8_t ch);
   
    // ADC related variables
    uint8_t adc_state;
    volatile uint16_t adc_val[8];

    // ADC irq handler
    void adc_interrupt_exec();
    CLASS_IRQ(adc_interrupt, ADC_vect);

    // --------------- 4051 -------------   
    void cd4051_init();
    void cd4051_set(char mode);

    uint8_t input_pullup = 0xff;      // all pullups enabled by default, a pullup enabled means "resistance mode"
    uint8_t input_analog = 0x00;      // flag whether input is digital or analog. All inouts are digital by default
    uint8_t input_valid = 0x00;       // a valid value has been received for this channel
    
    // ---------- outputs ----------------    
    void output_init();

    void spi_interrupt_exec();
    CLASS_IRQ(spi_interrupt, SPI_STC_vect);
    
    uint8_t spi_state = 0;                  // byte/word counter for spi transmission
    uint16_t spi_tx_in[SPI_PWM_CYCLE_LEN];  // set outside interrupt

    // repeatedly sent during interupt:
    union {
      uint16_t w;    // repeated during interupt
      uint8_t b[2];
    } spi_tx_data;

    void pulldown_c1_init();

    // ------- counter inputs ------------
    void counter_init(void);
    void counter_timer_exceeded(uint8_t c);
    bool counter_get_pin_state(uint8_t c);

    // time when last event has been seen
    uint32_t counter_event_time[4] = { 0,0,0,0 };
    void counter_check_pending(uint8_t ch);

    uint8_t counter_in_state;
    
    void ext_interrupt_exec(uint8_t c);
    CLASS_IRQ(ext_interrupt2, INT2_vect);
    CLASS_IRQ(ext_interrupt3, INT3_vect);
    CLASS_IRQ(pc_interrupt, PCINT0_vect);

    volatile uint8_t counter4motor;
    volatile uint16_t counter_val[4];
    uint8_t counter_modes;
};

extern Ftduino ftduino;

#endif
