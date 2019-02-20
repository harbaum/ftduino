#include <Wire.h>

void setup() {
  Wire.begin(); // join i2c bus (address optional for master)
  
  Wire.beginTransmission(0x11);
  Wire.write(0x00);
  Wire.write(92);
  Wire.endTransmission();    // stop transmitting

  delay(1000);  
}

byte x = 62;
byte y = 125;

void loop() {
  Wire.beginTransmission(0x11);
  Wire.write(0x00);
  Wire.write(x);    // 92 = 1.5ms
  Wire.endTransmission();    // stop transmitting

  Wire.beginTransmission(0x11);
  Wire.write(0x01);
  Wire.write(y);    // 92 = 1.5ms
  Wire.endTransmission();    // stop transmitting

  x++;
  if(x>125) x=62;
  
  y--;
  if(y<62) y=125;
  
  delay(50);
}
