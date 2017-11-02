#include <Adafruit_SSD1306.h>
#include "starfield.h"

extern Adafruit_SSD1306 display;

Star starfield[MAX_STARS];

void init_starfield() {
  for (uint8_t i = 0; i < MAX_STARS; i++) {
    starfield[i].x = random(display.width());
    starfield[i].y = random(display.height());
    starfield[i].vx = random(3) + 1;
  }
}

void move_stars() {
  for (uint8_t i = 0; i < MAX_STARS; i++) {
    starfield[i].x -= starfield[i].vx;
    if (starfield[i].x < 0) {
      starfield[i].x = display.width();
      starfield[i].y = random(display.height());
      starfield[i].vx = random(3) + 1;
    }
  }
}

void draw_stars() {
  for (uint8_t i = 0; i < MAX_STARS; i++) {
    display.drawPixel(starfield[i].x, starfield[i].y, WHITE);
  }
}

