// FtduinoDisplay.cpp 
// Support for the internal display of ftDuino V1.2
//
// This version uses the wire lib and is supposed to
// be used in conjunction with other Wire based I2C
// clients

#include "FtduinoDisplay.h"

// LCD boot sequence
static const uint8_t PROGMEM lcdBootProgram[] = {
  // Display Off
  1, 0xAE,

  // Set Display Clock Divisor v = 0xF0
  // default is 0x80
  2, 0xD5, 0xF0,

  // Set Multiplex Ratio v = HEIGHT-1
  2, 0xA8, FTDDSP_HEIGHT-1,

  // Set Display Offset v = 0
  2, 0xD3, 0x00,

  // Set Start Line (0)
  1, 0x40,

  // Charge Pump Setting v = enable (0x14)
  // default is disabled
  2, 0x8D, 0x14,

  // Set Segment Re-map (A0) | (b0001)
  // default is (b0000)
  1, 0xA1,

  // Set COM Output Scan Direction
  1, 0xC8,

  // Set COM Pins v
  2, 0xDA, 0x02,

  // Set Contrast v = 0x8F
  2, 0x81, 0x8F,

  // Set Precharge = 0xF1
  2, 0xD9, 0xF1,

  // Set VCom Detect
  2, 0xDB, 0x40,

  // Entire Display ON
  1, 0xA4,

  // Set normal/inverse display
  1, 0xA6,

  // Display On
  1, 0xAF,

  // set display mode = horizontal addressing mode (0x00)
  2, 0x20, 0x00,

  // set col address range
  3, 0x21, 0x00, FTDDSP_WIDTH-1,

  // set page address range
  3, 0x22, 0x00, 0x03,

  0
};

FtduinoDisplay::FtduinoDisplay(void) :
Adafruit_GFX(FTDDSP_WIDTH, FTDDSP_HEIGHT) {
}

void FtduinoDisplay::begin() {
  Wire.setClock(400000);

  // split into single commands and send one by one
  uint8_t *p = lcdBootProgram;
  uint8_t l = pgm_read_byte(p++);
  while(l) {
    Wire.beginTransmission(0x3C);
    Wire.write(0);  // command
    for(char i=0;i<l;i++)
      Wire.write(pgm_read_byte(p++));
    Wire.endTransmission();
    
    l = pgm_read_byte(p++);
  }
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
  // wire lib uses irq buffers and can transfer about 32 bytes. Since we
  // need to add the control byte it's the next smaller power of two
  // (16) for the data bytes
  for(int c=0;c<512/16;c++) {
    Wire.beginTransmission(0x3C);
    Wire.write(0x40);  // data 
    Wire.write(buffer+c*16, 16);
    Wire.endTransmission();
  }
}
