#include <Wire.h>
#include "I2cSerialBt.h"
I2cSerialBt btSerial;

#include <Ftduino.h>

void setup() {
  Serial.begin(115200);
//  while(!Serial);   // wait for USB
  
  // wait max 1 sec for adapter to appear on bus. This is not
  // needed as begin() will wait for the device. But this way
  // we can use the led as an inidictaor for problems with 
  // the i2c uart adapater
  if(!btSerial.check(1000)) {
    // fast blink with led on failure
    pinMode(LED_BUILTIN, OUTPUT); 
    while(true) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
  }
  
  btSerial.begin(9600);

  ftduino.init();

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  if(btSerial.available()) {
    char chr = btSerial.read();
    Serial.print("RX: "); Serial.print(String(chr)); 
    Serial.print(" - "); Serial.println(chr, HEX);

    switch(chr) {
      case 'u':  // forward
        digitalWrite(LED_BUILTIN, HIGH); 
        ftduino.motor_set(Ftduino::M1, Ftduino::RIGHT, Ftduino::MAX);
        ftduino.motor_set(Ftduino::M2, Ftduino::RIGHT, Ftduino::MAX);
      	break;
	
      case 'd':  // backward
        digitalWrite(LED_BUILTIN, HIGH); 
        ftduino.motor_set(Ftduino::M1, Ftduino::LEFT, Ftduino::MAX);
        ftduino.motor_set(Ftduino::M2, Ftduino::LEFT, Ftduino::MAX);
      	break;

      case 'r':  // turn right
        digitalWrite(LED_BUILTIN, HIGH); 
        ftduino.motor_set(Ftduino::M1, Ftduino::RIGHT, Ftduino::MAX);
        ftduino.motor_set(Ftduino::M2, Ftduino::BRAKE, Ftduino::MAX);
        break;
	
      case 'l':  // turn left
        digitalWrite(LED_BUILTIN, HIGH); 
        ftduino.motor_set(Ftduino::M1, Ftduino::BRAKE, Ftduino::MAX);
        ftduino.motor_set(Ftduino::M2, Ftduino::RIGHT, Ftduino::MAX);
        break;

      case 'R':  // rotate right
        digitalWrite(LED_BUILTIN, HIGH); 
        ftduino.motor_set(Ftduino::M1, Ftduino::RIGHT, Ftduino::MAX);
        ftduino.motor_set(Ftduino::M2, Ftduino::LEFT, Ftduino::MAX);
        break;
	
      case 'L':  // rotate left
        digitalWrite(LED_BUILTIN, HIGH); 
        ftduino.motor_set(Ftduino::M1, Ftduino::LEFT, Ftduino::MAX);
        ftduino.motor_set(Ftduino::M2, Ftduino::RIGHT, Ftduino::MAX);
        break;

	    // 'e', 't' and 'x' are sent when the program ends
      default:   // any other character stops the motors
        digitalWrite(LED_BUILTIN, LOW); 
        ftduino.motor_set(Ftduino::M1, Ftduino::OFF, Ftduino::MAX);
        ftduino.motor_set(Ftduino::M2, Ftduino::OFF, Ftduino::MAX);
        break;
    }
  }
}
