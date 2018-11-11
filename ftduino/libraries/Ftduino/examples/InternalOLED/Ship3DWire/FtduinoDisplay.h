#ifndef _FTDUINO_DISPLAY_H_
#define _FTDIUNO_DISPLAY_H_

#include <Wire.h>
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
    void begin();
 
  private:  
    uint8_t buffer[FTDDSP_HEIGHT * FTDDSP_WIDTH / 8];
};


#endif // _FTDIUNO_DISPLAY_H_
