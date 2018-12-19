//
// FtduinoDisplay.cpp - direct hardware version
//
// Very simple Adafruit_GFX driver for the internal ftDuino 128x32 OLED display.
//
// This version access the I2C bus directly bypassing e.g. the wire library.
// It's therefore incompatible with other i2c devices that do use the wire
// library. 
//
// (c) 2018 by Till Harbaum <till@harbaum.org>
//

#include "FtduinoDisplay.h"

static const uint8_t PROGMEM lcdBootProgram[] = {
  // Cmd mode
  0x00,

  // Display Off
  0xAE,

  // Set Display Clock Divisor v = 0xF0
  // default is 0x80
  0xD5, 0xF0,

  // Set Multiplex Ratio v = HEIGHT-1
  0xA8, FTDDSP_HEIGHT-1,

  // Set Display Offset v = 0
  0xD3, 0x00,

  // Set Start Line (0)
  0x40,

  // Charge Pump Setting v = enable (0x14)
  // default is disabled
  0x8D, 0x14,

  // Set Segment Re-map (A0) | (b0001)
  // default is (b0000)
  0xA1,

  // Set COM Output Scan Direction
  0xC8,

  // Set COM Pins v
  0xDA, 0x02,

  // Set Contrast v = 0x8F
  0x81, 0x8F,

  // Set Precharge = 0xF1
  0xD9, 0xF1,

  // Set VCom Detect
  0xDB, 0x40,

  // Entire Display ON
  0xA4,

  // Set normal/inverse display
  0xA6,

  // Display On
  0xAF,

  // set display mode = horizontal addressing mode (0x00)
  0x20, 0x00,

  // set col address range
  0x21, 0x00, FTDDSP_WIDTH-1,

  // set page address range
  0x22, 0x00, 0x03
};

uint8_t FtduinoDisplay::i2c_send_byte(uint8_t data) {
  uint8_t timeout = 0;
  TWDR = data;
  TWCR = _BV(TWINT)  |  _BV(TWEN);
  while(( !(TWCR & _BV(TWINT))) && ++timeout);
  return timeout;
}

uint8_t FtduinoDisplay::i2c_start(void) {
  TWCR = _BV(TWINT) | _BV(TWSTA)  | _BV(TWEN);
  while( !(TWCR & _BV(TWINT)));

  return i2c_send_byte(0x3C<<1);  // OLED I2C address
}

void FtduinoDisplay::i2c_stop(void) {
  TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);
  while( (TWCR & _BV(TWSTO)));
}

void FtduinoDisplay::i2c_send_buffer() {
  uint8_t *b = buffer;
  for(unsigned char i=0;i<FTDDSP_WIDTH*FTDDSP_HEIGHT/(8*8);i++) {
    i2c_send_byte(b[0]);
    i2c_send_byte(b[1]);
    i2c_send_byte(b[2]);
    i2c_send_byte(b[3]);
    i2c_send_byte(b[4]);
    i2c_send_byte(b[5]);
    i2c_send_byte(b[6]);
    i2c_send_byte(b[7]);
    b += 8;
  }
}

void FtduinoDisplay::i2c_send(uint16_t num, const uint8_t *p) {
  while(num--)
    i2c_send_byte(pgm_read_byte(p++));
}

FtduinoDisplay::FtduinoDisplay(void) :
Adafruit_GFX(FTDDSP_WIDTH, FTDDSP_HEIGHT) {
  // constructor
  TWSR = 0;
  TWBR = F_CPU/(2*400000)-8;

  if(!i2c_start()) return; 
  i2c_send(sizeof(lcdBootProgram), lcdBootProgram);
  i2c_stop();

  TWBR = 1;
  memset(buffer, 0, sizeof(buffer));
}

// the most basic function, set a single pixel
void FtduinoDisplay::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
    return;

  // x is which column
  switch (color) {
    case WHITE:   buffer[x+ (y/8)*FTDDSP_WIDTH] |=  (1 << (y&7)); break;
    case BLACK:   buffer[x+ (y/8)*FTDDSP_WIDTH] &= ~(1 << (y&7)); break;
    case INVERSE: buffer[x+ (y/8)*FTDDSP_WIDTH] ^=  (1 << (y&7)); break;
  }
}

void FtduinoDisplay::display(void) {
  i2c_start();
  i2c_send_byte(0x40);  // data
  i2c_send_buffer();
  i2c_stop();
}
