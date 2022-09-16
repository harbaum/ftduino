/*
  Ftduino.cpp - Library for ftDuino

  (c) 2017 by Till Harbaum <till@harbaum.org>
*/

#include <Arduino.h>

#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#define IN_FTDUINO_LIB
#include "Ftduino.h"
#include "adc_table.h"

// decreasing the filter time is possible, increasing it to more
// than 255 requires some variable sizes to be expanded

#ifndef COUNTER_FILTER
#define COUNTER_FILTER  250   // in 1/250000 sec -> 250 == 1 msec
#endif

// the library itself contains a static instance
Ftduino ftduino;

void Ftduino::cd4051_init() {
  // PC.6 = INH, PF.0 = A, PF.1 = B, PD.5 = C
  
  // make all 4051 control signals outputs
  DDRC |= (1<<6);   // INHIBIT
  PORTC |= (1<<6);  // pull it high by default, disabling the 4051

  DDRF |= (1<<0);   // A
  PORTF &= ~(1<<0); // = low
  
  DDRF |= (1<<1);   // B
  PORTF &= ~(1<<1); // = low
  
  DDRD |= (1<<5);   // C
  PORTD &= ~(1<<5); // = low
}

void Ftduino::cd4051_set(char mode) {
  // enable/disable pullup
  // 0..7 = on for I1..I8, 8 = off
  if(mode & 8)     
    PORTC |= (1<<6);   // activate INH pin
  else {
    PORTC &= ~(1<<6);  // de-activate INH pin
    if(mode & 1)   PORTF |=  (1<<0);  // A=1
    else           PORTF &= ~(1<<0);  // A=0
    if(mode & 2)   PORTF |=  (1<<1);  // B=1
    else           PORTF &= ~(1<<1);  // B=0
    if(mode & 4)   PORTD |=  (1<<5);  // C=1
    else           PORTD &= ~(1<<5);  // C=0    
  }
}

#define ADC_REF  ((0 << REFS1) | (1 << REFS0))   // AVcc with CAP on AREF

/* adc interrupt service routine */
/* At 16Mhz with a adc prescaler of 128, the conversion rate is at most 8900/sec. */
/* Since there are 8 channels and we scan every channel twice the total */
/* conversion rate is roughly 550Hz. */

// non-static function called by static IRQ handler
void Ftduino::adc_interrupt_exec() {

  // second conversion successful
  if(adc_state & 1) {
    // read ADC value and store it
    adc_val[adc_state>>1] = ADC;
    input_valid |= (1<<(adc_state>>1));
  }

  // next channel, lowest channel bit is state toggle
  adc_state = (adc_state+1)&15;
  
  // We abuse the adc irq to frequently check for events
  // in the counter inputs. There must be a nicer solution
  if(!(adc_state & 3))
    counter_check_pending(adc_state >> 2);
 
  // switch to next channel and restart conversion
  if(!(adc_state & 1)) 
    adc_prepare(adc_state>>1);
  else
    ADCSRA |= (1<<ADSC);
}

// the IRQ handler itself is static
void Ftduino::adc_interrupt() {
  ftduino.adc_interrupt_exec();
}

// prepare an ad conversion, setup the pullup and the channel
// this actually also triggers one first conversion which is then
// ignored and restarted to give the result some time to settle
void Ftduino::adc_prepare(uint8_t ch) {

  // enable pullup for next channel to evaluated
  if(ftduino.input_pullup & (1<<(ftduino.adc_state>>1)))
    ftduino.cd4051_set(ftduino.adc_state>>1);
  else
    ftduino.cd4051_set(0xff);
    
  if(!(ftduino.adc_state & 8)) {
    // ADC 4..7
    ADMUX = (1 << MUX2) | ADC_REF | ( (ftduino.adc_state>>1) & 0x03);
    ADCSRB = 0;  // MUX5 = 0     -"-
  } else {
    // ADC 8..11
    ADMUX = ADC_REF | ( (ftduino.adc_state>>1) & 0x03);
    ADCSRB =  (1<<MUX5);  // MUX5 = 1     -"-
  }
  
  ADCSRA |= (1<<ADSC);
}

void Ftduino::adc_init() {
  // setup 4051 mux driving the pullups
  cd4051_init();

  // disable digitial function of analog pins
  DIDR0 = (1 << ADC7D) | (1 << ADC6D) | (1 << ADC5D) | (1 << ADC4D);
  DIDR2 = (1 << ADC11D) | (1 << ADC10D) | (1 << ADC9D) | (1 << ADC8D);
  
  /* 10 bit, AVcc reference */
  ADMUX = (1 << MUX2) | ADC_REF;

  /* conversion rate prescaler /128 */
  ADCSRA = (1<<ADEN) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
  
  /* free running */
  ADCSRB = 0;

  // start first conversion
  adc_state = 0;
  input_valid = 0;
  adc_prepare(0);
}

void Ftduino::input_set_mode(uint8_t ch, uint8_t mode) {
  uint8_t mask = (1<<(ch-Ftduino::I1));

  // check if mode changed at all and don't do anything
  // if it didn't change
  if(mode == Ftduino::RESISTANCE) {
    if((input_analog & mask) && (input_pullup & mask))
      return;
  } else if(mode == Ftduino::SWITCH) {
    if(!(input_analog & mask) && (input_pullup & mask))
      return;
  } else {
    // voltage is always analog
    if(!(input_pullup & mask))   return;
  }

  // mode actually changes
  if((mode == Ftduino::RESISTANCE) || (mode == Ftduino::SWITCH))
    input_pullup |=  mask;
  else
    input_pullup &= ~mask;

  if((mode == Ftduino::RESISTANCE) || (mode == Ftduino::VOLTAGE))
    input_analog |=  mask;
  else
    input_analog &= ~mask;
    
  // and set result to invalid to make sure values are only delivered 
  // once a new value in new mode is available
  input_valid &= ~mask;
}

uint16_t Ftduino::adc_get(uint8_t ch) {
  if((ch >= Ftduino::I1) && (ch <= Ftduino::I8)) {
    // wait for valid data if necessary
    while(!(input_valid & (1<<(ch-Ftduino::I1))))
      _delay_us(100);
    
    return adc_val[ch-Ftduino::I1];
  }
  
  return 0;  // sane value, but should never happen
}

uint16_t Ftduino::adc_get_r(uint8_t ch) {
  return pgm_read_word_near(adc2r + adc_get(ch));
}

/* a switch is considered closed if the resistance is < 500R */
uint16_t Ftduino::adc_get_s(uint8_t ch) {
  return( adc_get_r(ch) < 500 );
}

uint16_t Ftduino::adc_get_v(uint8_t ch) {
  return 10000l * adc_get(ch) / 1023;
}

uint16_t Ftduino::input_get(uint8_t ch) {
  // pullup enabled -> resistance or switch
  if(input_pullup & (1<<(ch-Ftduino::I1))) {
    if(input_analog & (1<<(ch-Ftduino::I1))) 
      return adc_get_r(ch);
    else
      return adc_get_s(ch);
  }
  
  return adc_get_v(ch);
}

// interrupt driven 32 bit transfers
// an empty spi tx irq does roughly 110000 4 byte transfers/sec at SPI clk = 1Mhz
// or 27500 4 byte transfers/sec at SPI clk = 250khz
// 

// an output O1-O8 can be in one of three states:
// - floating: no MOSFET enabled
// - low: only lowside MOSFET enabled
// - high: only highside MOSFET enabled
// (- both MOSFETs should never be enabled at the same time)
// PWM toggles between floating and low or floating and high

void Ftduino::spi_interrupt_exec() {
  if(!(spi_state & 3)) {
    // set /SS high and low again
    PORTB |=  (1<<0);
    PORTB &= ~(1<<0);
    
    // fetch next data word
    spi_tx_data.w = spi_tx_in[(spi_state>>2)&(SPI_PWM_CYCLE_LEN-1)];
  }
  
  if(spi_state & 1) SPDR = spi_tx_data.b[(spi_state>>1) & 1];
  else              SPDR = 0;
  spi_state++;
}

void Ftduino::spi_interrupt() {
  ftduino.spi_interrupt_exec();
}

void Ftduino::output_init() {
  uint8_t i;
  
  // SPI clock considerations
  // - the higher the clock rate, the higher is the CPU load caused by the IRQ handler
  //   at 1Mhz SPI clock the IRQ handler consumes roughly 30% of the CPU time
  // - the lower the clock rate, the lower is the PWM clock cycle and PWM resolution
  //   that can be achieved.
  //   At 1Mhz we have roughly 100.000 bytes/sec or 25.000 MC33879 cycles/sec. At a
  //   PWM resolution of 0..64 which results in ~400 Hz PWM frequency
  
  // configure /SS, SCK and MOSI as output
  // drive /SS it high
  DDRB |= (1<<0) | (1<<1) | (1<<2);
  PORTB |= (1<<0);

  // enable SPI, enable interrupts, MSB first, Master mode,
  // mode 1 (SCK low when idle, data valid on falling edge)
  // and clock = FCPU/64 = 250khz
  SPCR = (1<<SPIE) | (1<<SPE) | (0<<DORD) | (1<<MSTR) |
    (0<<CPOL) | (1<<CPHA) | (1<<SPR1) | (0<<SPR0);

  SPSR &= ~(1<<SPI2X);   // single speed

  // PE.6 is EN. Chip is in sleep then EN == 0. EN is held low
  // during and directly after reset by pull-down r31
  DDRE  |= (1<<6);
  PORTE |= (1<<6);  // not in sleep mode

  // PB.7 is the PWM input of the IN6 of O2. We don't use that feature by
  // now, so it's just pulled down
  DDRB |= (1<<7);
  PORTB &= ~(1<<7);  // no PWM on IN6
  
  // clear SPI PWM table
  for(i=0;i<SPI_PWM_CYCLE_LEN;i++)
    spi_tx_in[i] = 0;
    
  // trigger first transfer
  spi_state = 0;
  SPDR = 0x00;
}

#define MC33879_PORT(a)   (1<<(a-1))

static const uint8_t mc33879_highside_map[4] = { 
      MC33879_PORT(2), MC33879_PORT(6), MC33879_PORT(4), MC33879_PORT(5) };

static const uint8_t mc33879_lowside_map[4] = { 
      MC33879_PORT(8), MC33879_PORT(1), MC33879_PORT(7), MC33879_PORT(3) };

// any SPI transfer transfers 16 bits per mc33879 and 32 bits in total
void Ftduino::output_set(uint8_t port, uint8_t mode, uint8_t pwm) {

  // mc33879 MSB = Open Load Detection Current On/OFF
  // mc33879 LSB = Output ON/OFF

  // MOSFET connections according to schematic
  // O1/O5 -> highside S2, lowside D8
  // O2/O6 -> highside S6, lowside D1
  // O3/O7 -> highside S4, lowside D7
  // O4/O8 -> highside S5, lowside D3

  // Never enable at the same time:
  // channel 2 and 8, 6 and 1, 4 and 7, 5 and 3

  // O1-O4 = first 33879 in chain
  // O5-O8 = second 33879 in chain

  uint32_t set_mask = 0, clr_mask = 0;

  // mode can be 0 (floating), 1 (high) or 2 (low)
  if(mode == 1) {
    set_mask = mc33879_highside_map[port&3];  // highside to be set
    clr_mask = mc33879_lowside_map[port&3];   // lowside to be cleared
  } else if(mode == 2) {
    set_mask = mc33879_lowside_map[port&3];   // lowside to be set
    clr_mask = mc33879_highside_map[port&3];  // highside to be cleared
  } else {
    set_mask = 0;                             // nothing to be set
    clr_mask = mc33879_highside_map[port&3] | // highside and lowside to be cleared
               mc33879_lowside_map[port&3];
  }

  // ports O1..O4 (0-3) are in MSB, ports O5-O8 (4-7) are in MSB of lower 16 bit
  if(!(port & 4)) {
    set_mask <<= 8;
    clr_mask <<= 8;
  }

  // set 0..32 entries in the PWN table according the requested PWM
  for(uint8_t i=0;i<SPI_PWM_CYCLE_LEN;i++) {
    if(i<pwm) {
       spi_tx_in[i] |=  set_mask;
       spi_tx_in[i] &= ~clr_mask;
    } else
       spi_tx_in[i] &= ~(set_mask | clr_mask);
  }
}

void Ftduino::motor_set(uint8_t port, uint8_t mode, uint8_t pwm) {
  // map from motor to output port
  uint8_t o = (2 * (port - Ftduino::M1)) + Ftduino::O1;

  switch(mode) {
    case Ftduino::OFF:
      // both outputs open
      output_set(o  , Ftduino::OFF, Ftduino::OFF);
      output_set(o+1, Ftduino::OFF, Ftduino::OFF);
      break;
      
    case Ftduino::LEFT:
      // both outputs driven low/hi
      output_set(o  , Ftduino::LO, pwm);
      output_set(o+1, Ftduino::HI, pwm);
      break;
      
    case Ftduino::RIGHT:
      // both outputs driven low/hi
      output_set(o  , Ftduino::HI, pwm);
      output_set(o+1, Ftduino::LO, pwm);
      break;
      
    case Ftduino::BRAKE:
      // both outputs driven low
      output_set(o  , Ftduino::LO, pwm);
      output_set(o+1, Ftduino::LO, pwm);
      break;
  }
}

bool Ftduino::motor_counter_active(uint8_t port) {
  return(counter4motor & (1<< (port - Ftduino::M1)));
}

void Ftduino::motor_counter_set_brake(uint8_t port, bool on) {
  uint8_t brake_bit = 0x10 << (port - Ftduino::M1);
  if(on) counter4motor |=  brake_bit;
  else   counter4motor &= ~brake_bit;
}

void Ftduino::motor_counter(uint8_t port, uint8_t mode, uint8_t pwm, uint16_t counter) {
  // enable counter
  counter_set_mode(port - Ftduino::M1 + Ftduino::C1, Ftduino::C_EDGE_RISING);

  // load counter so that it passes 0 after "counter" events
  counter_val[port - Ftduino::M1] = 0 - counter;

  // set flag indicating that the counter is being used for a motor
  counter4motor |= (1<< (port - Ftduino::M1));

  // start motor using the given values
  motor_set(port, mode, pwm);
}

// control pull-down on c1 to trigger distance sensor
void Ftduino::pulldown_c1_init() {
  DDRE |= (1<<2);      // pulldown is controlled by PE.2 (#HWB)
  PORTE &= ~(1<<2);    // de-activate by default
}

#if ULTRASONIC_ENABLE
void Ftduino::pulldown_c1_enable(bool on) {
  if(on) PORTE |=  (1<<2);
  else   PORTE &= ~(1<<2);
}

void Ftduino::usart1_interrupt_exec() {
  uint8_t data = UDR1; 
  if(ultrasonic_state == 1) {
    // bits of first byte are
    // 1,valid,w/d#,ID1,ID0,L9,L8,L7
    // msb should be 10000
    if((data & 0xf8) == 0x80) {   
      ultrasonic_rx_data[0] = data & 0x07;
      ultrasonic_state++;
    } else
      ultrasonic_state = 2;  // prevent writing of second byte on error
  } else if(ultrasonic_state == 2) {
    if((data & 0x80) == 0x00)  
      ultrasonic_rx_data[1] = data;
      
    ultrasonic_state++;
    ultrasonic_timeout = 0;  // got a value -> reset timeout
  }
}

void Ftduino::usart1_interrupt() {
  ftduino.usart1_interrupt_exec();
}

// usart 1 at 115200 8n1 for utrasonic sensor
void Ftduino::usart_init() {
  UBRR1 = F_CPU / 8 / 115200 - 1;

  // usart disabled. will be enabled once the sensor has been triggered
  UCSR1A = (1<<U2X1);
  UCSR1B = (1<<RXCIE1);    
  UCSR1C = (1<<UCSZ11)|(1<<UCSZ10);   // 8 bit
}

void Ftduino::usart_enable(bool on) {
  if(on) UCSR1B |=  (1<<RXEN1);
  else   UCSR1B &= ~(1<<RXEN1);
}

// use timer 3 to control the ultrasonic sensor in the background, timer runs
// at twice the measurement rate (e.g. 4 Hz when two distance measurements
// per second are to be done)
void Ftduino::timer3_compa_interrupt_exec() {
  // send trigger signal every second IRQ -> 4Hz
  if(ultrasonic_state) {
    // handle timeout: 4 attempts without value -> error
    if(ultrasonic_timeout++ == 4)
      ultrasonic_rx_data[0] = ultrasonic_rx_data[1] = 0xff;
    
    // start a new measurement
    ultrasonic_state = 0;
    usart_enable(false);        // disable usart so we don't see our own trigger signal  
    pulldown_c1_enable(true);
    
    // make next IRQ happen after ~80us
    TCNT3 = (F_CPU/64/4)-1-20;
  } else {
    ultrasonic_state = 1;
    pulldown_c1_enable(false);
    usart_enable(true);
  }
}

void Ftduino::timer3_compa_interrupt() {
  ftduino.timer3_compa_interrupt_exec();
}

void Ftduino::ultrasonic_enable(bool ena) {
  if(ena) {
    ultrasonic_state = 1;  // reset state machine
    ultrasonic_timeout = 0;
    ultrasonic_rx_data[0] = ultrasonic_rx_data[1] = 0xff;  // -> no value yet

    // make sure counter is disabled and uart events are not counted
    counter_set_mode(Ftduino::C1, Ftduino::C_EDGE_NONE);

    // run timer 3 in ctc mode with OCR3A as top
    TCCR3A = 0;
    TCCR3B = (1<<WGM32) | (1<<CS31) | (1<<CS30);  // 1/64
    // run isr at ~4Hz
    OCR3A = (uint16_t)(F_CPU/64/4)-1;
    TIMSK3 = (1<<OCIE3A);
  } else {
    // stop sensor reading
    TIMSK3 &= ~(1<<OCIE3A);

    // disable pullup and usart
    pulldown_c1_enable(false);
    usart_enable(false);    
  }
}

int16_t Ftduino::ultrasonic_get() {
  if(ultrasonic_rx_data[0] == 0xff)
    return -1;
  
  return ultrasonic_rx_data[0]*128 + ultrasonic_rx_data[1];
}
#endif

bool Ftduino::counter_get_pin_state(uint8_t c) {
  uint8_t state = 1;
  if(c == 0)      state = PIND & (1<<2);
  else if(c == 1) state = PIND & (1<<3);
  else if(c == 2) state = PINB & (1<<5);
  else            state = PINB & (1<<6);
  return(state != 0);
}

void Ftduino::counter_check_pending(uint8_t counter) {
  // check if the current counter is being used for a motor
  if(counter4motor & (1 << counter)) {
    // get current port state
    bool state = counter_get_pin_state(counter);
  
    // Something has happened. Check if it's a rising edge
    if(state && !(counter_in_state & (1<<counter))) {
      // check if this event (once processed after all debouncing) will
      // make the counter roll over to 0
      if(counter_val[counter] == 0xffff ) {
        // ok, counter will reach zero and a motor is active. Stop it!
        motor_set(counter + Ftduino::M1, 
          (counter4motor & (0x10 << counter))?Ftduino::BRAKE:Ftduino::OFF, 
          Ftduino::MAX);
        // unlink motor and counter
        counter4motor &= ~(1 << counter);
      }
    }
  }

  // check if there is a "unprocessed event for this counter
  if(counter_event_time[counter]) {
    // check if it's longer than the timeout time
    if((micros() - counter_event_time[counter]) > 4*COUNTER_FILTER)
      counter_timer_exceeded(counter);
  }
}

void Ftduino::counter_timer_exceeded(uint8_t c) {
  // get current port state
  bool state = counter_get_pin_state(c);

  // save last accepted state and only count if
  // the current state differs from the accepted one.
  // otherwise very short spikes may e.g. counted

  // check if state of counter input was high and has fallen
  if(counter_in_state & (1<<c)) {
    // pin state as not changed: Do nothing
    if(state) return;
    // pin state bas changed: Save new state
    else counter_in_state &= ~(1<<c);
  }
  
  // check if state of counter input was low and has risen
  else {
    // pin state as not changed: Do nothing
    if(!state) return;
    // pin state bas changed: Save new state
    else counter_in_state |= (1<<c);
  }

  // determine mode of current counter port
  uint8_t mode = (counter_modes >> (2*c)) & 3;

  // count event if it has the desired edge
  if((mode == Ftduino::C_EDGE_ANY) ||
     (!state && (mode == Ftduino::C_EDGE_FALLING)) ||
     ( state && (mode == Ftduino::C_EDGE_RISING)) )
    counter_val[c]++;
    
  // this counter timer has been processed
  counter_event_time[c] = 0;
}

uint16_t Ftduino::counter_get(uint8_t ch) {
  if((ch >= Ftduino::C1) && (ch <= Ftduino::C4)) {
    counter_check_pending(ch - Ftduino::C1);  
    return counter_val[ch-Ftduino::C1];
  }
  
  return 0;  // sane value, but should never happen
}

void Ftduino::counter_clear(uint8_t ch) {
  counter_val[ch-Ftduino::C1] = 0;
}

void Ftduino::counter_set_mode(uint8_t ch, uint8_t mode) {
  char ch_idx = (ch-Ftduino::C1)<<1; // CH1 = 0, CH2 = 2, ...
  
  counter_modes &= ~(3 << ch_idx);   // clear mode bits
  counter_modes |= mode << ch_idx;   // set new mode bits
}

bool Ftduino::counter_get_state(uint8_t ch) {
  return !counter_get_pin_state(ch-Ftduino::C1);
}
      
// this irq fires whenever anything on one of the four
// counter ports
void Ftduino::ext_interrupt_exec(uint8_t counter) {
  // implementation using the micros() function
  counter_check_pending(counter);  
  counter_event_time[counter] = micros();
}

void Ftduino::ext_interrupt2() {
  ftduino.ext_interrupt_exec(0);
}

void Ftduino::ext_interrupt3() {
  ftduino.ext_interrupt_exec(1);
}

void Ftduino::pc_interrupt() {
  // The hardware won't tell us which pin fired this interrupt.
  // so we need to keep track of that outselves
  static uint8_t portb_state = 0xff;  // assume both pins are pulled high
  uint8_t pinb = PINB;
    
  if((pinb ^ portb_state) & (1<<5))
    ftduino.ext_interrupt_exec(2);
    
  if((pinb ^ portb_state) & (1<<6))
    ftduino.ext_interrupt_exec(3);

  portb_state = pinb;
}

void Ftduino::counter_init(void) {
 
  // C1/C2 are PD.2/PD.3 and C3/C4 are on PB.5/PB.6
  DDRD  &= ~((1 << 2) | (1 << 3));     // counter ports C1 and C2 are inputs
  PORTD &= ~((1 << 2) | (1 << 3));     // disable internal pullup, we use external
  DDRB  &= ~((1 << 5) | (1 << 6));     // counter ports C3 and C4 are inputs
  PORTB &= ~((1 << 5) | (1 << 6));     // disable internal pullup, we use external

  // port change interrupts are not used as they cannot easily be made to cope with
  // input signal bouncing

  // no counter is currently being used to control an encoder motor and all four motor
  // brakes are on
  counter4motor = 0xf0;

  // default mode is to not count at all
  counter_modes = (Ftduino::C_EDGE_NONE << 6) | (Ftduino::C_EDGE_NONE << 4) |
                  (Ftduino::C_EDGE_NONE << 2) | (Ftduino::C_EDGE_NONE << 0);
                                    
  // generate irq on both edges for C1/INT2 and C1/INT3
  EICRA = (1<<ISC20) | (1<<ISC30);
  // enable INT2 and INT3 irqs
  EIMSK = (1<<INT2) | (1<<INT3);

  PCICR = 1<<PCIE0;
  PCMSK0 = (1<<PCINT5) | (1<<PCINT6);

  // run timer 1 in fast pwm mode with OCR1A as top. Since OCR1A is lower
  // than OCR1B, the counter will normally never pass OCR1B and the
  // interrupt will thus never fire unless TCNT is explicitely being
  // set to a value > OCR1A
  TCCR1B = (1<<WGM13) | (1<<WGM12);
  TCCR1A = (1<<WGM11) | (1<<WGM10);

  TCNT1 = 0;
  OCR1A = 0xffff-COUNTER_FILTER;  // 1 ms
  OCR1B = 0xffff;

  // ack any pending OCIE1B interrupt
  TIFR1 = (1<<OCF1B);
  // generate interrupt when timer passes OCR1B
  TIMSK1 = (1<<OCIE1B);

  TCCR1B |= (1<<CS11) | (1<<CS10);  // start timer at 1/64 F_CPU -> 250kHz

  // initially no counter input is assumed to be active, so we assmume they
  // are pulled high
  counter_in_state = 0xff;

  // clear all counters
  for(char i=0;i<4;i++)
    counter_val[i] = 0;
}

void Ftduino::init() {
  // initialization takes place in the init function and not in the
  // constructor since the arduino wiring.c itself changes hw
  // settings after the constructor has run
  
  // ---------------------------------------------------------
  
  // prepare ADC conversion for I1..I8
  adc_init();

  // prepare for ultrasonic distance sensor on C1
  pulldown_c1_init();
#if ULTRASONIC_ENABLE
  usart_init();
#endif

  // prepare spi service for M1..M4/O1..O8
  output_init();

  counter_init();
}

Ftduino::Ftduino() { }

