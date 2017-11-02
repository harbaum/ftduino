#ifndef __TEXTUTILS_H_
#define __TEXTUTILS_H_
const uint8_t BASE_FONT_W = 6;

uint8_t center_pos(const char* str, const uint8_t text_size);
void pmem_print(uint8_t x, uint8_t y, uint8_t size, const char* str, uint16_t color = WHITE);
void pmem_print_center(uint8_t y, uint8_t size, const char* str, uint16_t color = WHITE);
#endif

