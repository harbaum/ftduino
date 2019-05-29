#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "gfx.h"
#include "joystick.h"
#include "highscores.h"
#include "textutils.h"
#include "starfield.h"
#include "game_objects.h"
#include "game_state.h"
#include "game.h"

#include <FtduinoSimple.h>

const uint8_t I2C_ADDRESS_DISPLAY = 0x3C;
const int8_t OLED_RESET = -1;

Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);
Game shootduino;

void fire_bullet() {
  if (shootduino.ticks - shootduino.bullet_fired < BULLET_DELAY) {
    return;
  }
  shootduino.bullet_fired = shootduino.ticks;
  
  for (uint8_t i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].is_active) {
      bullets[i].type = BULLET;
      bullets[i].is_active = true;
      bullets[i].x = player.x + PLAYER_W;
      bullets[i].y = player.y + (PLAYER_H / 2);
      bullets[i].vx = 3;
      break;
    }
  }
}

void start_asteroid() {
  if (shootduino.ticks - shootduino.asteroid_started < ASTEROID_DELAY) {
    return;
  }
  shootduino.asteroid_started = shootduino.ticks;
  
  for (uint8_t i = 0; i < MAX_ASTEROIDS; i++) {
    if (!asteroids[i].is_active) {
      asteroids[i].type = ASTEROID;
      asteroids[i].is_active = true;
      asteroids[i].x = display.width();
      asteroids[i].y = random(display.height() - ASTEROID_H) + 1;
      asteroids[i].vx = -1 * random(2) - 2;
      asteroids[i].anim_frame = random(NUM_ASTEROID_FRAMES);
      asteroids[i].frame_count = 0;
      break;
    }
  }
}

void move_player() {
  if (joystick.left) {
    player.x -= player.vx;
    if (player.x < 0) {
      player.x = 0;
    }
  }
  if (joystick.right) {
    player.x += player.vx;
    if (player.x + PLAYER_W > display.width()) {
      player.x = display.width() - PLAYER_W;
    }
  }
  if (joystick.up) {
    player.y -= player.vy;
    if (player.y < 0) {
      player.y = 0;
    }
  }
  if (joystick.down) {
    player.y += player.vy;
    if (player.y + PLAYER_W > display.height()) {
      player.y = display.height() - PLAYER_H;
    }
  }
}

void draw_player() {
  if (!player.is_active)
    return;
  if (player.type == PLAYER) {
    display.drawBitmap(player.x, player.y, spaceship_bmp, PLAYER_W, PLAYER_H, WHITE);
  } else {
    const uint8_t* bmp = explosion_anim + (2 * EXPLOSION_H) * player.anim_frame;
    display.drawBitmap(player.x, player.y, bmp, EXPLOSION_W, EXPLOSION_H, WHITE);
    player.frame_count++;
    if (player.frame_count == ANIM_FRAME_DELAY) {
      player.frame_count = 0;            
      player.anim_frame++;
      if (player.anim_frame == NUM_EXPLOSION_FRAMES)
        player.is_active = false;
    }
  }
}

void check_collisions() {
  for (uint8_t i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].is_active) {
      for (uint8_t j = 0; j < MAX_ASTEROIDS; j++) {
        if (asteroids[j].is_active && asteroids[j].type == ASTEROID) {
          if (bullets[i].x < asteroids[j].x + ASTEROID_W && bullets[i].x + BULLET_W > asteroids[j].x &&
              bullets[i].y < asteroids[j].y + ASTEROID_H && bullets[i].y + BULLET_H > asteroids[j].y)
          { 
            ftduino.output_set(Ftduino::O1, Ftduino::HI);

            asteroids[j].type = EXPLOSION;
            asteroids[j].vx = 0;
            bullets[i].is_active = false;
            shootduino.score += 100;
            break;
          }
        }        
      }
    }
  }
  
  for (uint8_t i = 0; i < MAX_ASTEROIDS; i++) {          
    if (asteroids[i].is_active && asteroids[i].type == ASTEROID) {
      if (player.x < asteroids[i].x + ASTEROID_W && player.x + PLAYER_W > asteroids[i].x &&
          player.y < asteroids[i].y + ASTEROID_H && player.y + PLAYER_H > asteroids[i].y)
      {
        shootduino.player_hit = true;
        player.type = EXPLOSION;
        player.vx = player.vy = 0;
        asteroids[i].type = EXPLOSION;
        asteroids[i].vx = 0;
        break;
      }
    }
  }
}

void init_game() {
  init_starfield();
  init_objects(bullets, MAX_BULLETS);
  init_objects(asteroids, MAX_ASTEROIDS);
  change_state(INTRO);
  player.x = 0;
  player.y = display.height() / 2;
  player.vx = player.vy = 2;
  player.type = PLAYER;
  player.is_active = true;
  player.anim_frame = 0;
  shootduino.player_hit = false;
  shootduino.lives = MAX_LIVES;
  shootduino.score = 0;
  shootduino.asteroids_missed = 0;
  init_highscore_entry(0);
}

void pause_game() {
  move_stars();
  draw_stars();
  draw_player();
  draw_objects(bullets, MAX_BULLETS);
  draw_objects(asteroids, MAX_ASTEROIDS);
  pmem_print_center(5, 2, PSTR("Pause"));
  pmem_print(0, 48, 1, PSTR("Lives: "));
  pmem_print(0, 56, 1, PSTR("Score: "));
  display.setCursor(42, 48);
  display.print(shootduino.lives);
  display.setCursor(42, 56);
  display.print(shootduino.score);
  if (joystick.left_button && (shootduino.ticks - shootduino.state_changed > MIN_DELAY_AFTER_STATE_CHANGE)) {
    change_state(RUNNING);
    joystick.left_button = false;
    delay(DEBOUNCE_DELAY);
  }
}

void lost_live() {
  static uint8_t flicker_cnt = 0;
  // ftDuino: flicker lamp
  if(flicker_cnt < 10) {
    ftduino.output_set(Ftduino::O1, (flicker_cnt&1)?Ftduino::LO:Ftduino::HI);
    flicker_cnt++;
  }
  
  move_stars();
  draw_stars();
  draw_objects(bullets, MAX_BULLETS);
  draw_objects(asteroids, MAX_ASTEROIDS);
  draw_player();
  pmem_print_center(5, 2, PSTR("Don't"));
  pmem_print_center(25, 2, PSTR("Give"));
  pmem_print_center(45, 2, PSTR("Up!"));
  if (joystick.right_button && (shootduino.ticks - shootduino.state_changed > MIN_DELAY_AFTER_STATE_CHANGE)) {
    change_state(RUNNING);
    flicker_cnt = 0;
    ftduino.output_set(Ftduino::O1, Ftduino::LO);
    player.vx = player.vy = 2;
    player.type = PLAYER;
    player.is_active = true;
    player.anim_frame = 0;
    joystick.right_button = false;
    delay(DEBOUNCE_DELAY);
  }
}

void print_score(const uint8_t y) {
  char tmp[MAX_SCORE_LEN];
  snprintf(tmp, MAX_SCORE_LEN, "%d", shootduino.score);
  const uint8_t score_txt_len = 7; // Length of "Score: ".
  const uint8_t score_len = strlen(tmp);
  const uint8_t x = (display.width() - (score_txt_len + score_len) * BASE_FONT_W) / 2;
  pmem_print(x, y, 1, PSTR("Score: "));
  display.setCursor(x + score_txt_len * BASE_FONT_W, y);
  display.print(shootduino.score);
}

void game_over() {
  move_stars();
  draw_stars();
  pmem_print_center(10, 2, PSTR("Game Over"));
  
  if (shootduino.lives == 0) {
    pmem_print_center(30, 1, PSTR("You have lost"));
    pmem_print_center(40, 1, PSTR("all your lives."));
  } else {
    pmem_print_center(30, 1, PSTR("You have missed."));
    pmem_print_center(40, 1, PSTR("too many asteroids."));
  }
  
  print_score(56);
  
  if (joystick.right_button && (shootduino.ticks - shootduino.state_changed > MIN_DELAY_AFTER_STATE_CHANGE)) {
    if (get_highscore_index(shootduino.score) != -1) {
      change_state(ENTER_HS);
    } else {
      init_game();
    }
    joystick.right_button = false;
    delay(DEBOUNCE_DELAY);
  }
}

void update_game() {
  if (joystick.left_button) {
    change_state(PAUSED);
    return;
  }
  if (shootduino.player_hit) {
    shootduino.player_hit = false;
    if (--shootduino.lives == 0) {
      change_state(DONE);
    } else {
      change_state(LOST_LIVE);
    }    
    return;
  }
  if (shootduino.asteroids_missed >= MAX_MISSES) {
      change_state(DONE);
      return;
  }
  if (joystick.right_button) {
    fire_bullet();
  }
  if (shootduino.ticks - shootduino.asteroid_started > ASTEROID_DELAY) {
    start_asteroid();
  }
  check_collisions();
  move_player();
  move_objects(bullets, MAX_BULLETS);
  move_objects(asteroids, MAX_ASTEROIDS);
  move_stars();
  draw_stars();
  draw_player();
  draw_objects(bullets, MAX_BULLETS);
  draw_objects(asteroids, MAX_ASTEROIDS);
}

void intro() {
  move_stars();
  draw_stars();
  pmem_print_center(10, 2, PSTR("Shootduino"));
  pmem_print_center(35, 1, PSTR("Press right button"));
  pmem_print_center(45, 1, PSTR("to start!"));

  if (shootduino.ticks - shootduino.state_changed > SHOW_HIGHSCORES_DELAY) {
    change_state(SHOW_HS);
  }
  
  if (joystick.right_button) {
    change_state(RUNNING);
    joystick.right_button = false;
    delay(DEBOUNCE_DELAY);
  }
}

void show_highscores() {
  move_stars();
  draw_stars();
  pmem_print_center(10, 1, PSTR("High Scores"));
  HighScoreEntry entry;
  uint8_t y = 25;
  for (uint8_t i = 0; i < MAX_HIGHSCORES; i++) {
    get_entry(i, entry);
    show_highscore_entry(y, entry);
    y += 10;
  }

  if (shootduino.ticks - shootduino.state_changed > SHOW_HIGHSCORES_DELAY) {
    change_state(INTRO);
  }

  if (joystick.right_button) {
    change_state(INTRO);
    joystick.right_button = false;
    delay(DEBOUNCE_DELAY);
  }
}

void enter_highscore() {
  move_stars();
  draw_stars();
  handle_highscore_controls();
  copy_initials_letters();
  show_highscore_display();
  
  if (joystick.right_button) {
    insert_entry(shootduino.highscore_entry);
    init_game();
    change_state(INTRO);
    joystick.right_button = false;
    delay(DEBOUNCE_DELAY);
  }
}

void init_display() {
  display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS_DISPLAY);
  display.display();
  delay(1000);
  display.clearDisplay();
}

void setup() {
  ftduino.motor_set(Ftduino::M1, Ftduino::LEFT);

  pinMode(LED_BUILTIN, OUTPUT);

  randomSeed(analogRead(A0));
  init_joystick();
  init_highscores();
  init_game();
  init_display();
}

void loop() {
  static uint32_t last_led_event = 0;

  // flash led once a second  
  if((millis() - last_led_event) >= 1000) {
    digitalWrite(LED_BUILTIN, HIGH);    // turn the LED on
    last_led_event = millis();
  } else if((millis() - last_led_event) > 50) 
    digitalWrite(LED_BUILTIN, LOW);     // turn the LED off

  shootduino.ticks = millis();
  update_joystick();
  display.clearDisplay();
  switch (shootduino.state) {
    case INTRO:     intro();           break;
    case PAUSED:    pause_game();      break;
    case RUNNING:   update_game();     break;
    case LOST_LIVE: lost_live();       break;
    case SHOW_HS:   show_highscores(); break;
    case ENTER_HS:  enter_highscore(); break;
    case DONE:      game_over();       break;
  }
  display.display();
}
