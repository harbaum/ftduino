/***************************************************************************
 *                                                                         *
 ***************************************************************************/

/* use from Linux host:
   i2cdetect -l
   -> e.g. bus 9 (i2cdetect 9)
   i2cset -y 9 0x11 0x00 0x00   -> channel 0 off
   i2cset -y 9 0x11 0x00 0xff   -> channel 0 max (limited to 125/2ms by default)
   i2cset -y 9 0x11 0x01 0x00   -> channel 1 off
   i2cset -y 9 0x11 0x01 0x5e   -> channel 1 94/1.5ms, servo centered

   i2cget -y 9 0x11 0x00        -> read version register (0x10 for 1.0)

   i2cset -y 9 0x11 0x08 0x13   -> change i2c address temporarily
   i2cset -y 9 0x13 0x09 0xa5   -> save settings in eeprom (using new address!!)
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#ifndef TWI_ADDRESS
#define TWI_ADDRESS		0x11
#endif

#define TWI_STATE_IDLE			0
#define TWI_STATE_RX_ACK		1
#define TWI_STATE_BYTE_RX		2
#define TWI_STATE_ACK_PR_TX             3
#define TWI_STATE_PR_ACK_TX		4
#define TWI_STATE_BYTE_TX		5

volatile uint8_t twi_state = TWI_STATE_IDLE;

volatile uint8_t pulse_width[2] = { 0x00, 0x00 };
volatile uint8_t led_timeout = 0;

#define MAGIC 0xdeadbeef

// config as stored in eeprom
typedef struct {
  uint8_t twi_address;

  struct {  
    uint8_t offset;
    uint8_t min, max;
  } servo[2]; 
} ee_config_t;

ee_config_t config;

#define SDA  (1<<0)
#define PWM2 (1<<1)  // OC1A
#define SCL  (1<<2)
#define LED  (1<<3)
#define PWM1 (1<<4)  // OC1B

// The servo has a full cycle of 20ms, a 1-2ms high phase within this
// is indicating a min to max angle.

// This cannot be handled using one 8 bit timer. The timer would run
// at 7812.5 Hz from 0 to 155. The 1ms to 2ms range would be from 8 to
// 15 resulting in only 8 positions/angles the servo could be set to.

// Using two timers:
// One timer running at 7812.5 is used to trigger a second timer which
// is put in a kind of one shot mode to generate the 1-2ms signal

void timer_init(void) {

  // setup timer 0 in CTC mode at F_CPU/256
  // timer 0 has no /128 divider, we thus run with a /256 divider
  // at sligtly lower precision
  TCNT0 = 0;
  TCCR0A = (1 << WGM01);   // CTC, OCx not connected
#if F_CPU == 1000000
  TCCR0B = (1 << CS02);    // CTC, prescaler /256
  OCR0A = F_CPU/256/50;    // 78 -> frequency = 50.08 Hz
#elif F_CPU == 8000000
  TCCR0B = (1 << CS02) | (1 << CS00);    // CTC, prescaler /1024
  OCR0A = F_CPU/1024/50;    // 156 -> frequency = 50.08 Hz
#else
#error "Unsupported CPU clock"
#endif
  TIMSK |= (1<< OCIE0A);   // generate output compare interrupt

#if F_CPU == 1000000
  // timer 1: F_CPU / 16 (prescaler) -> 62500 Hz
  TCCR1 = (1 << CS12) | (1 << CS10);
#elif F_CPU == 8000000
  // timer 1: F_CPU / 128 (prescaler) -> 62500 Hz
  TCCR1 = (1 << CS13);
#endif
  
  // drive LED on port PB.3, LED on for 1 second
  DDRB |= LED;
  PORTB |= LED;
  led_timeout = 50;

  // drive PWM outputs low
  DDRB |= (PWM1 | PWM2);
  PORTB &= ~(PWM1 | PWM2);
}

// timer firing at 50Hz/20ms
ISR(TIMER0_COMPA_vect) {
  // run down led timer and switch LED off after timeout
  if(led_timeout > 0) {
    led_timeout--;
    if(!led_timeout)    
      PORTB &= ~LED;       // switch off LED
  }

  // setup timer 1
  // max pulse length = 255 * 16us = 4,08ms
  
  if(pulse_width[0] != 0) {
    // add limit range and add offset
    uint8_t val = pulse_width[0];
    if(val < config.servo[0].min) val = config.servo[0].min;
    if(val > config.servo[0].max) val = config.servo[0].max;
    val += config.servo[0].offset;

    // use hw OC1B
    GTCCR |= (1 << COM1B0) | (1 << COM1B1) | (1<<FOC1B);  // set
    GTCCR &= ~(1 << COM1B0); // let timer clear
    
    OCR1B = val;               // @62500/sec
  } else {
    GTCCR &= ~((1 << COM1B0) | (1 << COM1B1));
    PORTB &= ~PWM1;            // set output low
  }
    
  if(pulse_width[1] != 0) {
    // add limit range and add offset
    uint8_t val = pulse_width[1];
    if(val < config.servo[1].min) val = config.servo[1].min;
    if(val > config.servo[1].max) val = config.servo[1].max;
    val += config.servo[1].offset;
    
    // use hw OC1A
    TCCR1 |= (1 << COM1A0) | (1 << COM1A1);
    GTCCR |= (1<<FOC1A);     // set OC1A
    TCCR1 &= ~(1 << COM1A0); // let timer clear OC1A

    OCR1A = val;               // @62500/sec
  } else {
    TCCR1 &= ~((1 << COM1A0) | (1 << COM1A1));
    PORTB &= ~PWM2;            // set output low
  }
    
  // start timer1
  TCNT1 = 0;
}

void save_config(void) {
  eeprom_write_dword((void*)0, MAGIC);
  eeprom_write_block(&config, (void*)4, sizeof(ee_config_t));
}

void USI_init(void) {
  if(eeprom_read_dword((void*)0) != MAGIC) {
    // load default values
    config.twi_address = TWI_ADDRESS;

    // allowed range = 1ms ... 2ms, no offset
    config.servo[0].min = config.servo[1].min = 63;
    config.servo[0].max = config.servo[1].max = 125;
    config.servo[0].offset = config.servo[1].offset = 0;
    
  } else {
    // load config from eeprom
    eeprom_read_block(&config, (void*)4, sizeof(ee_config_t));
    
    // make sure address is within sane bounds
    if((config.twi_address < 1) || (config.twi_address > 126))
      config.twi_address = TWI_ADDRESS;
  }
  
  DDRB  &= ~SDA;    // PB.0 = SDA
  PORTB &= ~SDA;   
  DDRB  |=  SCL;    // PB.2 = SCL
  PORTB |=  SCL;

  // 2-wire mode; Hold SCL on start and overflow; ext. clock
  // startcondition interrupt enable
  USICR = (1 << USISIE) | (0 << USIOIE) | (1 << USIWM1) |
    (0 << USIWM0) | (1 << USICS1) | (0 << USICS0) | (0 << USICLK) | (0 << USITC);
  // write 1 to clear flags, clear counter
  USISR = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC);
}

int main(void) {
#if 0
  DDRB |= PWM1 | PWM2;
  TCNT1 = 0;
  OCR1B = 94;
  // F_CPU / 16 (prescaler) -> 62500 Hz
  TCCR1  = (1 << CS12) | (1 << CS10);
  GTCCR  = (1 << COM1B1);    // clear on match
  //  GTCCR |= (1<<FOC1A) | (1<<FOC1B);
  
  //  sei();
  for(;;) {
    _delay_ms(100);
    TCNT1 = 0;
    GTCCR  = (1 << COM1B0) | (1 << COM1B1) | (1<<FOC1B);  // set
    GTCCR  = (1 << COM1B1); // let timer clear
  }
#else  
  
  timer_init();
  USI_init();

  sei();

  for(;;) { }
#endif
}

// http://www.aca-vogel.de/TINYUSII2C_AVR312/I2C_corrected.c
SIGNAL(SIG_USI_START) {
  uint8_t tmpUSISR __attribute__((unused)) = USISR;
  twi_state = TWI_STATE_IDLE;

  // Wait for SCL to go low to ensure the "Start Condition" has completed.
  // otherwise the counter will count the transition
#if 0
  // TO
  uint8_t to = 255;
  while ( (PINB & SCL) && to)
    to--;
  
  if(PINB & SCL) return;   // timeout
#else
  // SDW
  while ( (PINB & SCL) && ( !(PINB & SDA) ) );
#endif
  
  USISR = 0xf0; // write 1 to clear flags; clear counter

  // enable USI interrupt on overflow; SCL goes low on overflow
  USICR |= (1<<USIOIE) | (1<<USIWM0);
}

SIGNAL(SIG_USI_OVERFLOW) {
  static uint8_t twi_cnt, twi_reg;
  uint8_t twi_rx_byte = USIDR;

  switch(twi_state) {

  case TWI_STATE_IDLE:
    if (((twi_rx_byte & 0xfe) >> 1) != config.twi_address) {	// if not receiving my address
      // disable USI interrupt on overflow; disable SCL low on overflow
      USICR &= ~((1<<USIOIE) | (1<<USIWM0));
    } else { // else address is mine
      PORTB |= LED;       // switch on LED
      led_timeout = 5;    // keep LED on for 100ms

      DDRB |= SDA;  // drive SDA
      USISR = 0x0e; // reload counter for ACK, (SCL) high and back low
      
      if (twi_rx_byte & 0x01)
	twi_state = TWI_STATE_ACK_PR_TX;   // master writes
      else 
	twi_state = TWI_STATE_RX_ACK;   // master reads

      twi_cnt = 0;
    }
    break;
    
  case TWI_STATE_RX_ACK:
    DDRB  &= ~SDA;  // release SDA
    twi_state = TWI_STATE_BYTE_RX;
    break;
    
  case TWI_STATE_BYTE_RX:
    // write pulse_width registers
    if(twi_cnt == 0) twi_reg = USIDR;
    else if(twi_cnt == 1) {
      if((twi_reg & 0xfe) == 0x00)     // reg 0 / 1
	pulse_width[twi_reg&1] = USIDR;      
      if((twi_reg & 0xfe) == 0x02)     // reg 2 / 3  (servo 0/1 min)  
	config.servo[twi_reg&1].min = USIDR;      
      if((twi_reg & 0xfe) == 0x04)     // reg 4 / 5  (servo 0/1 max)  
	config.servo[twi_reg&1].max = USIDR;      
      if((twi_reg & 0xfe) == 0x06)     // reg 6 / 7  (servo 0/1 offset)  
	config.servo[twi_reg&1].min = USIDR;      
      if(twi_reg == 0x08) {            // reg 8
	uint8_t val = USIDR;
	// limit address to 0x03 - 0x77 range except 0x3c (maybe internal display)
	if((val >= 0x03) && (val <= 0x77) && (val != 0x3c)) 
	  config.twi_address = val;
      }
      if((twi_reg == 0x09) && (USIDR == 0xa5)) // reg 9/0xa5
	save_config();
    }
    twi_cnt++;
    
    DDRB  |=  SDA;    // drive SDA
    USISR = 0x0e;     // reload counter for ACK, (SCL) high and back low
    twi_state = TWI_STATE_RX_ACK;
    break;
    
  case TWI_STATE_ACK_PR_TX:
    if(twi_reg == 0x00)
      USIDR = VERSION;   // return chip revision for register 0
    else
      USIDR = 0x5a;      // some kind of magic byte allowing to identify the chip
    
    PORTB |=  SDA; // transparent for shifting data out
    twi_state = TWI_STATE_BYTE_TX;
    break;
    
  case TWI_STATE_PR_ACK_TX:
    if(twi_rx_byte & 0x01) {
      twi_state = TWI_STATE_IDLE; // no ACK from master --> no more bytes to send
      PORTB &= ~LED; // switch off LED
    } else {
      USIDR = 0x00;  // all further bytes read 0
      
      PORTB |=  SDA; // transparent for shifting data out
      DDRB  |=  SDA;
      twi_state = TWI_STATE_BYTE_TX;
    }
    break;
    
  case TWI_STATE_BYTE_TX:
    DDRB  &= ~SDA;
    PORTB &= ~SDA;
    USISR = 0x0e;	// reload counter for ACK, (SCL) high and back low
    twi_state = TWI_STATE_PR_ACK_TX;
    break;
  }
  
  USISR |= (1<<USIOIF); // clear overflowinterruptflag, this also releases SCL
}





