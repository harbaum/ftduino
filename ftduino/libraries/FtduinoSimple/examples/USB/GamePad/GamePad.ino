/*
 * GamePad.ino - eine einfache Gamepad-Imlementierung für
 * den ftDuino. 
 */

#include <FtduinoSimple.h>
#include "HidGamePad.h"

void setup() {
  // initialisiere HidGamePad
  HidGamePad.begin();
}

void loop() {
  static int8_t x_axis = 0;
  static int8_t y_axis = 0;
  static uint8_t buttons = 0;

  // Taster I1 offen und Taster I1 offen: Gamepad horizontal zentriert
  if(!ftduino.input_get(Ftduino::I1) && !ftduino.input_get(Ftduino::I2) && x_axis != 0) {
    x_axis = 0;
    HidGamePad.setXAxis(x_axis);
  }

  // Taster I1 geschlossen und Taster I2 offen: Gamepad links
  if(ftduino.input_get(Ftduino::I1) && !ftduino.input_get(Ftduino::I2) && x_axis != -127) {
    x_axis = -127;
    HidGamePad.setXAxis(x_axis);
  }
    
  // Taster I1 offen und Taster I2 gechlossen: Gamepad rechts
  if(!ftduino.input_get(Ftduino::I1) && ftduino.input_get(Ftduino::I2) && x_axis != 127) {
    x_axis = 127;
    HidGamePad.setXAxis(x_axis);
  }
  
  // Taster I3 offen und Taster I4 offen: Gamepad vertikal zentriert
  if(!ftduino.input_get(Ftduino::I3) && !ftduino.input_get(Ftduino::I4) && y_axis != 0) {
    y_axis = 0;
    HidGamePad.setYAxis(y_axis);
  }
  
  // Taster I3 geschlossen und Taster I4 offen: Gamepad hoch
  if(ftduino.input_get(Ftduino::I3) && !ftduino.input_get(Ftduino::I4) && y_axis != -127) {
    y_axis = -127;
    HidGamePad.setYAxis(y_axis);
  }
    
  // Taster I3 offen und Taster I4 gechlossen: Gamepad runter
  if(!ftduino.input_get(Ftduino::I3) && ftduino.input_get(Ftduino::I4) && y_axis != 127) {
    y_axis = 127;
    HidGamePad.setYAxis(y_axis);
  }

  // Taster I5 offen: Button 0 nicht gedrückt
  if(!ftduino.input_get(Ftduino::I5) && (buttons&1)) {
    buttons &= ~1;
    HidGamePad.setButton(0, buttons&1);
  }
    
  // Taster I5 geschlossen: Button 0 gedrückt
  if(ftduino.input_get(Ftduino::I5) && !(buttons&1)) {
    buttons |= 1;
    HidGamePad.setButton(0, buttons&1);
  }
  
  // Taster I6 offen: Button 1 nicht gedrückt
  if(!ftduino.input_get(Ftduino::I6) && (buttons&2)) {
    buttons &= ~2;
    HidGamePad.setButton(1, buttons&2);
  }
    
  // Taster I6 geschlossen: Button 1 gedrückt
  if(ftduino.input_get(Ftduino::I6) && !(buttons&2)) {
    buttons |= 2;
    HidGamePad.setButton(1, buttons&2);
  }
}

