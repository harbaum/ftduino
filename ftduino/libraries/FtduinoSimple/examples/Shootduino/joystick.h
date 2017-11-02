#ifndef __JOYSTICK_H_
#define __JOYSTICK_H_
#include <Arduino.h>

struct JoystickState {
  bool left : 1;
  bool right : 1;
  bool up : 1;
  bool down : 1;
  bool joy_button : 1;
  
  bool left_button : 1;
  bool right_button : 1;
};

extern JoystickState joystick;

static uint16_t resting_pos_x;
static uint16_t resting_pos_y;

void init_joystick();
void update_joystick();
static void init_joystick_button(const uint8_t pin);
#endif

