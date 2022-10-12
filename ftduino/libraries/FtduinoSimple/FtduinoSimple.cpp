/*
  FtduinoSimple.cpp - Library for ftduino

  (c) 2017-2021 by Till Harbaum <till@harbaum.org>

  A very simple io library for the ftduino only supporting
  diginal IO but without and interrupt handlers and with a
  small footprint.

  If you need analog io, ultrasonic IO and fast counters use
  the full feature version.
*/

#include "Arduino.h"

#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#define IN_FTDUINO_SIMPLE_LIB
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

bool Ftduino::input_get(uint8_t ch) {
  static char cd4051_mode = -1;
  bool rval = false;
  
  // this enables the pullup for the given channel and returns
  // its digital state

  // enable pullup if necessary
  if(cd4051_mode != ch) {
    cd4051_set(ch);
    _delay_us(100);
    cd4051_mode = ch;
  }

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

#if defined(OUTPUT_DRIVER_TLE94108EL) || defined(OUTPUT_DRIVER_AUTO)
#define HB_ACT_1_CTRL  0b10000011
#define HB_ACT_2_CTRL  0b11000011
#endif

#if defined(OUTPUT_DRIVER_DRV8908) || defined(OUTPUT_DRIVER_AUTO)
#define OP_CTRL_1  0x08
#define OP_CTRL_2  0x09
#define OLD_CTRL_1 0x1f
#define OLD_CTRL_2 0x20
#endif

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
  // now, so it's just pulled down. On V1.4 this is the RGB LED output
  DDRB |= (1<<7);
  PORTB &= ~(1<<7);  // no PWM on IN6

#if defined(OUTPUT_DRIVER_AUTO)
  _delay_ms(1);

  // SPI has been initialized MSB first (for MC33879A). Try to
  // detect chip

  // sending this 0x00 will trigger a SPI ERR on TLE94108EL, however
  // it will not trigger anything on MC33879A
  PORTB &= ~(1<<0);  // set /SS low
  SPDR = 0x00; while(!(SPSR & (1<<SPIF)));
  PORTB |=  (1<<0);  // set /SS high

  _delay_us(5);      // TLE94108EL needs this pause, MC33879A doesn't care

  // sending another 0x00 will return the previous SPI ERR
  PORTB &= ~(1<<0);  // set /SS low
  SPDR = 0x00; while(!(SPSR & (1<<SPIF))); uint8_t v0 = SPDR;
  PORTB |=  (1<<0);  // set /SS high

  if(!v0) {
    // mc33879a detected
    driver_chip = CHIP_MC33879;
#endif
#if defined(OUTPUT_DRIVER_AUTO) || defined(OUTPUT_DRIVER_MC33879)
    spi_tx = 0;        // all outputs of
    output_spi_tx();
#endif
  
#if defined(OUTPUT_DRIVER_AUTO)
  } else if((v0 & ~4) == 0x01) {
    // tle94108el detected
    driver_chip = CHIP_TLE94108;    
#endif  
#if defined(OUTPUT_DRIVER_AUTO) || defined(OUTPUT_DRIVER_TLE94108EL) 
    SPCR |= (1<<DORD);  // switch to LSB first

    state = 0;
    write_spi_reg(HB_ACT_1_CTRL, 0);
    write_spi_reg(HB_ACT_2_CTRL, 0); 
#endif

#if defined(OUTPUT_DRIVER_AUTO)
  } else if ((v0 & ~4) == 0xc0) {
    // drv8908 detected
    driver_chip = CHIP_DRV8908;    
#endif  
#if defined(OUTPUT_DRIVER_AUTO) || defined(OUTPUT_DRIVER_DRV8908) 
    state = 0;    
    write_spi_reg(OLD_CTRL_1, 0xff);   // disable open load detection        
    write_spi_reg(OLD_CTRL_2, 0xc0);   // ignore open load detection        
    write_spi_reg(OP_CTRL_1, 0);
    write_spi_reg(OP_CTRL_2, 0); 
#endif
  
#if defined(OUTPUT_DRIVER_AUTO)
  }
#endif
}

#if defined(OUTPUT_DRIVER_MC33879A) || defined(OUTPUT_DRIVER_AUTO)
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
#if defined(OUTPUT_DRIVER_MC33879A)
void Ftduino::output_set(uint8_t port, uint8_t mode)
#endif
#if defined(OUTPUT_DRIVER_AUTO)
void Ftduino::output_set_mc33879a(uint8_t port, uint8_t mode)
#endif
{
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
#endif

#if defined(OUTPUT_DRIVER_TLE94108EL) || defined(OUTPUT_DRIVER_DRV8908) || defined(OUTPUT_DRIVER_AUTO)
void Ftduino::write_spi_reg(uint8_t reg, uint8_t data) {
  PORTB &= ~(1<<0);  // set /SS low
  SPDR = reg;  while(!(SPSR & (1<<SPIF)));
  SPDR = data; while(!(SPSR & (1<<SPIF))); 
  PORTB |=  (1<<0);  // set /SS high

  // A pause of min 5us is required after setting SS high
  // before being allowed to drive it low again
  _delay_us(5);
}
#endif

#if defined(OUTPUT_DRIVER_TLE94108EL) || defined(OUTPUT_DRIVER_AUTO)
#if defined(OUTPUT_DRIVER_TLE94108EL)
void Ftduino::output_set(uint8_t port, uint8_t mode)
#endif
#if defined(OUTPUT_DRIVER_AUTO)
void Ftduino::output_set_tle94108el(uint8_t port, uint8_t mode)
#endif
{
  // TLE94108EL pins are mapped straight 0-7 to O1-O8
  // mode = 0 -> both drivers off,
  // mode = 1 -> highside on, mode = 2 -> lowside on
  
  if(mode == 1) {
    state &= ~(1 << (2*port));  // clear lowside driver
    state |=  (2 << (2*port));  // set highside driver  
  } else if(mode == 2) {
    state |=  (1 << (2*port));  // set lowside driver
    state &= ~(2 << (2*port));  // clear highside driver  
  } else {
    state &= ~(3 << (2*port));  // clear both drivers  
  }

  // and write state into registers
  write_spi_reg(HB_ACT_1_CTRL, state);
  write_spi_reg(HB_ACT_2_CTRL, state >> 8);
}
#endif

#if defined(OUTPUT_DRIVER_DRV8908) || defined(OUTPUT_DRIVER_AUTO)
#if defined(OUTPUT_DRIVER_DRV8908)
void Ftduino::output_set(uint8_t port, uint8_t mode)
#endif
#if defined(OUTPUT_DRIVER_AUTO)
void Ftduino::output_set_drv8908(uint8_t port, uint8_t mode)
#endif
{
  // DEV8908 pins are mapped straight 0-7 to O1-O8
  // mode = 0 -> both drivers off,
  // mode = 1 -> highside on, mode = 2 -> lowside on
  
  if(mode == 1) {
    state &= ~(1 << (2*port));  // clear lowside driver
    state |=  (2 << (2*port));  // set highside driver  
  } else if(mode == 2) {
    state |=  (1 << (2*port));  // set lowside driver
    state &= ~(2 << (2*port));  // clear highside driver  
  } else {
    state &= ~(3 << (2*port));  // clear both drivers  
  }

  // and write state into registers
  write_spi_reg(OP_CTRL_1, state);
  write_spi_reg(OP_CTRL_2, state >> 8);
}
#endif

#if defined(OUTPUT_DRIVER_AUTO)
void Ftduino::output_set(uint8_t port, uint8_t mode) {
  if(driver_chip == CHIP_MC33879)       output_set_mc33879a(port, mode);
  else if(driver_chip == CHIP_TLE94108) output_set_tle94108el(port, mode);
  else if(driver_chip == CHIP_DRV8908)  output_set_drv8908(port, mode);
}
#endif
  
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

bool Ftduino::counter_get_state(uint8_t ch) {
  if(ch == Ftduino::C1) return !(PIND & (1<<2));
  if(ch == Ftduino::C2) return !(PIND & (1<<3));
  if(ch == Ftduino::C3) return !(PINB & (1<<5));
  if(ch == Ftduino::C4) return !(PINB & (1<<6));

  return false;
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
