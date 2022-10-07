#ifndef _FTDUINO_DISPLAY_H_
#define _FTDIUNO_DISPLAY_H_

#include <Adafruit_GFX.h>

#define FTDDSP_WIDTH 128
#define FTDDSP_HEIGHT 32

#define BLACK 0
#define WHITE 1
#define INVERSE 2

class FtduinoDisplay : public Adafruit_GFX {
  public:
    FtduinoDisplay();
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void display();
 
  private:  
    uint8_t i2c_send_byte(uint8_t data);
    uint8_t i2c_start(void);
    void i2c_stop(void);
    void i2c_send(uint16_t num, uint8_t *p);
    void i2c_send(uint16_t num, const uint8_t *p);
    
    uint8_t buffer[FTDDSP_HEIGHT * FTDDSP_WIDTH / 8];
};


#endif // _FTDIUNO_DISPLAY_H_
