#include <Wire.h>
#include "I2cSerialBt.h"
I2cSerialBt btSerial;

void setup() {
  Serial.begin(115200);
  while(!Serial);
  btSerial.begin(9600);
  Wire.setClock(400000);
  btSerial.key(0);

#if 0
  delay(1000);
  btSerial.key(1);         // force 9600 baud
  delay(1500);
  btSerial.key(0);
#endif
}

void loop() {

  // send if there's data to be sent and if the receiver is able to
  // accept data
  while(Serial.available() && btSerial.availableForWrite())
    btSerial.write(Serial.read());

  if(btSerial.available() && Serial.availableForWrite())
    Serial.write(btSerial.read());

}
