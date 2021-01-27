#include <Wire.h>
#include "I2cSerialBt.h"
I2cSerialBt btSerial;

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

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  if(btSerial.available()) {
    char chr = btSerial.read();
    Serial.print("RX: ");
    Serial.println(chr, HEX);
    switch(chr) {
      case 'r':
        digitalWrite(LED_BUILTIN, HIGH); 
        break;
      
      case 'l':
        digitalWrite(LED_BUILTIN, LOW); 
        break;
    }
  }
}
