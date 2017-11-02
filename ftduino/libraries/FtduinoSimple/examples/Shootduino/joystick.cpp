#include "joystick.h"
#include "FtduinoSimple.h"

JoystickState joystick;

void init_joystick() {
}

void update_joystick() {
  joystick.right_button = ftduino.input_get(Ftduino::I1);
  joystick.up = ftduino.input_get(Ftduino::I2);
  joystick.down = ftduino.input_get(Ftduino::I3);
}

