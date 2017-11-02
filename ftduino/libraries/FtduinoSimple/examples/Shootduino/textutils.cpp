#include <Adafruit_SSD1306.h>
#include "textutils.h"

extern Adafruit_SSD1306 display;

uint8_t center_pos(const char* str, const uint8_t text_size) {
  return (display.width() - (strlen_P(str) * text_size * BASE_FONT_W)) / 2;
}

void pmem_print(uint8_t x, uint8_t y, uint8_t size, const char* str, uint16_t color) {
  uint8_t font_width = size * BASE_FONT_W;
  char c;
  uint8_t i = 0;
  display.setTextSize(size);
  display.setTextColor(color);
  while (c = pgm_read_byte(str++)) {
    display.setCursor(x + i++ * font_width, y);
    display.print(c);
  }
}

void pmem_print_center(uint8_t y, uint8_t size, const char* str, uint16_t color) {
  pmem_print(center_pos(str, size), y, size, str, color);
}

