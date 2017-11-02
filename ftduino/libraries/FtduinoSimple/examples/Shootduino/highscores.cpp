#include <Adafruit_SSD1306.h>
#include "highscores.h"
#include "joystick.h"
#include "textutils.h"
#include "game.h"

const char INITIALS_LETTERS[] PROGMEM = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 .,-?!";

extern Adafruit_SSD1306 display;
extern JoystickState joystick;
extern HighScoreEntry highscore_entry;
extern Game shootduino;

uint32_t initials_control_hit = 0;
uint8_t letter_index[LEN_INITIALS] = { 0 };
uint8_t initials_index = 0;

void init_highscores() {
  uint16_t address = HIGHSCORE_ADDR;
  if (EEPROM.read(address) != HIGHSCORE_MARKER) {
    EEPROM.write(address, HIGHSCORE_MARKER);
    HighScoreEntry entry;
    address += 1;
    char initial = 'A';
    uint16_t score = 0;
    for (uint8_t i = 0; i < MAX_HIGHSCORES; i++) {
      for (uint8_t j = 0; j < LEN_INITIALS; j++) {
        entry.initials[j] = initial;
      }
      entry.score = score;
      EEPROM.put(address, entry);
      address += sizeof(entry);
      initial += 1;
    }
  }
}

static uint16_t entry_address(uint8_t index, const HighScoreEntry& entry) {
  return HIGHSCORE_ADDR + 1 + sizeof(entry) * index;
}

void get_entry(uint8_t index, HighScoreEntry& entry) {
  EEPROM.get(entry_address(index, entry), entry);
}

void set_entry(uint8_t index, const HighScoreEntry& entry) {
  EEPROM.put(entry_address(index, entry), entry);
}

void insert_entry(const HighScoreEntry& entry) {
  uint8_t index = get_highscore_index(entry.score);
  uint8_t current_index = MAX_HIGHSCORES - 1;
  while (current_index != index) {
    HighScoreEntry prev_entry;
    get_entry(current_index - 1, prev_entry);
    set_entry(current_index, prev_entry);
    current_index--;
  }
  set_entry(current_index, entry);
}

int8_t get_highscore_index(uint16_t score) {
  uint8_t index = -1;
  HighScoreEntry entry;
  for (uint8_t i = 0; i < MAX_HIGHSCORES; i++) {
    get_entry(i, entry);
    if (entry.score < score) {
      index = i;
      break;
    }
  }
  return index;
}

void show_highscore_entry(uint8_t y, HighScoreEntry entry) {
  char buf[LEN_HIGHSCORE_ENTRY + 1];
  for (uint8_t j = 0; j < LEN_INITIALS; j++) {
    buf[j] = entry.initials[j];
  }
  buf[LEN_INITIALS] = ' ';
  sprintf(buf + 4, "%05d", entry.score);
  buf[LEN_HIGHSCORE_ENTRY] = 0;
  display.setCursor(score_entry_xpos(), y);
  display.print(buf);
}

void init_highscore_entry(uint16_t score) {
  shootduino.highscore_entry.score = score;
  for (uint8_t i = 0; i < LEN_INITIALS; i++) {
    shootduino.highscore_entry.initials[i] = 'A';
  }
  for (uint8_t i = 0; i < LEN_INITIALS; i++)
    letter_index[i] = 0;
  initials_index = 0;
}

void handle_highscore_controls() {
  if (shootduino.ticks - initials_control_hit >= HS_CONTROL_DELAY) {
    initials_control_hit = shootduino.ticks;
    if (joystick.left_button) {
      initials_index++;
      initials_index %= LEN_INITIALS;
    } else if (joystick.up) {
      letter_index[initials_index]++;
      if (letter_index[initials_index] >= strlen_P(INITIALS_LETTERS) - 1)
        letter_index[initials_index] = 0;
    } else if (joystick.down) {
      if (letter_index[initials_index] == 0)
        letter_index[initials_index] = strlen_P(INITIALS_LETTERS) - 1;
      else
        letter_index[initials_index]--;
    }
  }
}

void copy_initials_letters() {
  for (uint8_t i = 0; i < LEN_INITIALS; i++) {
    shootduino.highscore_entry.initials[i] = pgm_read_byte(INITIALS_LETTERS + letter_index[i]);
  }
  shootduino.highscore_entry.score = shootduino.score;  
}

uint8_t score_entry_xpos() {
  return (display.width() - LEN_HIGHSCORE_ENTRY * BASE_FONT_W) / 2;
}

void show_highscore_display() {
  pmem_print_center(10, 1, PSTR("Enter Initials"));
  show_highscore_entry(30, shootduino.highscore_entry);
  display.setCursor(score_entry_xpos() + initials_index * BASE_FONT_W, 36);
  display.write(24); // Shows an arrow  
}

