/*
  FtduinoSimple.cpp - Library for ftduino

  (c) 2017 by Till Harbaum <till@harbaum.org>

  A very simple io library for the ftduino only supporting
  diginal IO but without and interrupt handlers and with a
  small footprint.

  If you need analog io, ultrasonic IO and fast counters use
  the full feature version.
*/

#include "Arduino.h"

#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "FtduinoSimple.h"

Ftduino ftduino;

void Ftduino::cd4051_init() {
  // PC.6 = ING, PF.0 = A, PF1 = B, PD.5 = C
  
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

void Ftduino::input_init() {
  cd4051_init();

  // the digital counterparts of ADC4-11 are distributed all over the chip

  // ADC4-7 are PF.4-PF.7
  DDRF &= ~((1<<7)|(1<<6)|(1<<5)|(1<<4));
  // ADC8,9,10 are PD.4,PD.6 and PD.7
  DDRD &= ~((1<<7)|(1<<6)|(1<<4));
  // ADC11 is PB.4
  DDRB &= ~(1<<4);
}

uint8_t Ftduino::input_get(uint8_t ch) {
  uint8_t rval = 0;
  
  // this enables the pullup for the given channel and returns
  // its digital state

  // enable pullup
  cd4051_set(ch);
  _delay_us(100);

  switch(ch) {
    case Ftduino::I1:
      rval = PINF & (1<<4);
      break;
    case Ftduino::I2:
      rval = PINF & (1<<5);
      break;
    case Ftduino::I3:
      rval = PINF & (1<<6);
      break;
    case Ftduino::I4:
      rval = PINF & (1<<7);
      break;
    case Ftduino::I5:
      rval = PIND & (1<<4);
      break;
    case Ftduino::I6:
      rval = PIND & (1<<6);
      break;
    case Ftduino::I7:
      rval = PIND & (1<<7);
      break;
    case Ftduino::I8:
      rval = PINB & (1<<4);
      break;
  }

  // return true if pin is grounded
  return !rval;
}

void Ftduino::output_init() {
  uint8_t i;
  
  // configure /SS, SCK and MOSI as output
  // drive /SS it high
  DDRB |= (1<<0) | (1<<1) | (1<<2);
  PORTB |= (1<<0);

  // enable SPI, no interrupts, MSB first, Master mode,
  // mode 1 (SCK low when idle, data valid on falling edge)
  // and clock = FCPU/16 = 1Mhz
  SPCR = (1<<SPE) | (0<<DORD) | (1<<MSTR) |
    (0<<CPOL) | (1<<CPHA) | (0<<SPR1) | (1<<SPR0);

  SPSR &= ~(1<<SPI2X);   // single speed

  // PE.6 is EN. Chip is in sleep then EN == 0. EN is held low
  // during and directly after reset by pull-down r31
  DDRE  |= (1<<6);
  PORTE |= (1<<6);  // not in sleep mode

  // PB.7 is the PWM input of the IN6 of O2. We don't use that feature by
  // now, so it's just pulled down
  DDRB |= (1<<7);
  PORTB &= ~(1<<7);  // no PWM on IN6

  spi_tx = 0;        // all outputs off
  output_spi_tx();
}

void Ftduino::output_spi_tx(void) {
  uint8_t i;
  uint32_t data = spi_tx;
  
  // set /SS low
  PORTB &= ~(1<<0);

  // send four bytes (32 bit) via SPI
  for(i=0;i<4;i++) {
    SPDR = data & 0xff;
    data >>= 8;
    while(!(SPSR & (1<<SPIF)));
  }
  
  // set /SS high
  PORTB |=  (1<<0);
}

#define MC33879_PORT(a)   (1<<(a-1))

static const uint8_t mc33879_highside_map[4] = { 
      MC33879_PORT(2), MC33879_PORT(6), MC33879_PORT(4), MC33879_PORT(5) };

static const uint8_t mc33879_lowside_map[4] = { 
      MC33879_PORT(8), MC33879_PORT(1), MC33879_PORT(7), MC33879_PORT(3) };

// any SPI transfer transfers 16 bits per mc33879 and 32 bits in total
void Ftduino::output_set(uint8_t port, uint8_t mode) {
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
  if(port & 4) {
    set_mask <<= 8;
    clr_mask <<= 8;
  } else {
    set_mask <<= 24;
    clr_mask <<= 24;
  }

  spi_tx |=  set_mask;
  spi_tx &= ~clr_mask;

  output_spi_tx();
}

void Ftduino::motor_set(uint8_t port, uint8_t mode) {
  // map from motor to output port
  uint8_t o = (2 * (port - Ftduino::M1)) + Ftduino::O1;

  switch(mode) {
    case Ftduino::OFF:
      // both outputs open
      output_set(o  , Ftduino::OFF);
      output_set(o+1, Ftduino::OFF);
      break;
      
    case Ftduino::LEFT:
      // both outputs driven low/hi
      output_set(o  , Ftduino::LO);
      output_set(o+1, Ftduino::HI);
      break;
      
    case Ftduino::RIGHT:
      // both outputs driven low/hi
      output_set(o  , Ftduino::HI);
      output_set(o+1, Ftduino::LO);
      break;
      
    case Ftduino::BRAKE:
      // both outputs driven low
      output_set(o  , Ftduino::LO);
      output_set(o+1, Ftduino::LO);
      break;
  }
}

// control pull-down on c1 to trigger distance sensor
void Ftduino::pulldown_c1_init() {
  DDRE |= (1<<2);      // pulldown is controlled by PE.2 (#HWB)
  PORTE &= ~(1<<2);    // de-activate by default
}

void Ftduino::pulldown_c1_enable(char on) {
  if(on) PORTE |=  (1<<2);
  else   PORTE &= ~(1<<2);
}

uint8_t Ftduino::counter_get_state(uint8_t ch) {
  if(ch == Ftduino::C1) return !(PIND & (1<<2));
  if(ch == Ftduino::C2) return !(PIND & (1<<3));
  if(ch == Ftduino::C3) return !(PINB & (1<<5));
  if(ch == Ftduino::C4) return !(PINB & (1<<6));

  return 0;
}

void Ftduino::counter_init(void) {
  // C1/C2 are PD.2/PD.3 and C3/C4 are on PB.5/PB.6
  DDRD &= ~((1 << 2) | (1 << 3));      // counter ports C1 and C2 are inputs
  PORTD &= ~((1 << 2) | (1 << 3));     // disable internal pullup, we use external
  DDRB &= ~((1 << 5) | (1 << 6));      // counter ports C3 and C4 are inputs
  PORTB &= ~((1 << 5) | (1 << 6));     // disable internal pullup, we use external
}

Ftduino::Ftduino() {
  // ---------------------------------------------------------
  
  // prepare digital input system fot I1..I8
  input_init();

  // init the pulldown for the ultrasonic. However, no
  // support for the sensor in the simple lib
  pulldown_c1_init();

  // prepare spi service for M1..M4/O1..O8
  // No PWM in the simple lib
  output_init();

  // the simple lib only uses the counters as digital inouts
  counter_init();
}

