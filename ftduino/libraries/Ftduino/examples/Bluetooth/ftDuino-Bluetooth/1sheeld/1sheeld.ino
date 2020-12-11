//
// 1sheeld.ino
//
// The bluetooth modules name needs to start with 1Sheeld for this to work


#define CUSTOM_SETTINGS
#define INCLUDE_SEVEN_SEGMENT_SHIELD
#define INCLUDE_KEYPAD_SHIELD

#include "Ftduino.h"
#include <OneSheeld.h>

#include "I2cSerialBt.h"
I2cSerialBt btSerial;

#include "FirmataSerial.h"
FirmataSerial firmataSerial(btSerial);

void setup() {
  Serial.begin(115200);

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

  OneSheeld.begin(firmataSerial);

  ftduino.init();
}

void loop() {
  int keys[][3] = {
    { 0,0,Ftduino::O1 },  // 1
    { 0,1,Ftduino::O2 },  // 2
    { 0,2,Ftduino::O3 },  // 3
    { 1,0,Ftduino::O4 },  // 4
    { 1,1,Ftduino::O5 },  // 5
    { 1,2,Ftduino::O6 },  // 6
    { 2,0,Ftduino::O7 },  // 7
    { 2,1,Ftduino::O8 }   // 8
  };

  for(char i=0;i<8;i++) {  
    // request onesheeld keypad state
    if(Keypad.isRowPressed(keys[i][0]) && Keypad.isColumnPressed(keys[i][1]))
      ftduino.output_set(keys[i][2], Ftduino::HI, Ftduino::MAX);
    else
      ftduino.output_set(keys[i][2], 0, 0);
  }

  // set onesheeld seven segment number
  static byte number = 0;
  SevenSegment.setNumber(number++);
  if(number == 10) number = 0;

  OneSheeld.delay(100);
}
