// USB/Bluetooth bridge example for use with HM10 connected via I2C-UART bridge SC16IS750

#include "SC16IS750.h"

SC16IS750 i2cuart = SC16IS750(SC16IS750_PROTOCOL_I2C,SC16IS750_ADDRESS_BB);

void setup() 
{
    Serial.begin(9600);
    while(!Serial);
    i2cuart.begin(9600);               //baudrate setting
};

void loop() 
{
    while(i2cuart.available())
      Serial.write(i2cuart.read());
      
    if(Serial.available())
      i2cuart.write(Serial.read());
};