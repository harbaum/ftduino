// LineSensor.ino
// a line following robot based on the ft line sensor 128598

#include <Ftduino.h>

void setup() {
  ftduino.init();
  
  ftduino.input_set_mode(Ftduino::I1, Ftduino::VOLTAGE); 
  ftduino.input_set_mode(Ftduino::I2, Ftduino::VOLTAGE); 
}

void loop() {
  // a voltage < 1 volt (1000 millivolts) means the sensor sees black
  uint16_t l = ftduino.input_get(Ftduino::I1);
  uint16_t r = ftduino.input_get(Ftduino::I2);

  if((l < 1000) && (r < 1000)) {
    // if both sensors see "black" then just drive ahead
    ftduino.motor_set(Ftduino::M1, Ftduino::RIGHT, Ftduino::MAX);
    ftduino.motor_set(Ftduino::M2, Ftduino::RIGHT, Ftduino::MAX);  
  } else if(r < 1000) {
    // if only right sensor sees "black" then turn left. M1 is the
    // right motor. Driving only M1 turns the robot left
    ftduino.motor_set(Ftduino::M1, Ftduino::RIGHT, Ftduino::MAX);
    ftduino.motor_set(Ftduino::M2, Ftduino::OFF,   Ftduino::MAX);  
  } else if(l < 1000) {
    // if only left sensor sees "black" then turn right. M2 is the
    // left motor. Driving only M2 turns the robot right
    ftduino.motor_set(Ftduino::M1, Ftduino::OFF,   Ftduino::MAX);
    ftduino.motor_set(Ftduino::M2, Ftduino::RIGHT, Ftduino::MAX);  
  } else {
    // otherwise turn in place
    ftduino.motor_set(Ftduino::M1, Ftduino::RIGHT, Ftduino::MAX);
    ftduino.motor_set(Ftduino::M2, Ftduino::LEFT,  Ftduino::MAX);  
  }
}
