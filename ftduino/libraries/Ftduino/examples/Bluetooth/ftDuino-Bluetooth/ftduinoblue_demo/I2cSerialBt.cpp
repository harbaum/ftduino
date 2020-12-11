#include "I2cSerialBt.h"
#include <Wire.h>
#include <Arduino.h>
#define ADDR 5  // make configurable

I2cSerialBt::I2cSerialBt() { }

bool I2cSerialBt::check(int timeout_msec) {
  Wire.begin();
  
  uint32_t timeout = millis();
  do {
    Wire.beginTransmission(ADDR);
    if(!Wire.endTransmission())
      return true;
  }
  while(millis() - timeout < timeout_msec);

  return false;
}

uint8_t I2cSerialBt::registerRead(char r) {
  Wire.beginTransmission(ADDR);
  Wire.write(r);
  Wire.endTransmission(false); 
  Wire.requestFrom(ADDR,1);
  return Wire.read();
}

void I2cSerialBt::registerWrite(char r, uint8_t d) {
  Wire.beginTransmission(ADDR);
  Wire.write(r);
  Wire.write(d);
  Wire.endTransmission();
}

void I2cSerialBt::begin(uint32_t baud) {
  Wire.begin();

  // wait for i2c adapter to show up on bus
  do
    Wire.beginTransmission(ADDR);
  while(Wire.endTransmission());  

  // set baud rate
  registerWrite(0, baud/1200);
}

void I2cSerialBt::key(bool on) {
  registerWrite(1, on?0:1);
}
  
void I2cSerialBt::end() { }

int I2cSerialBt::available() {
  // check i2c client for data
  return registerRead(0);
}

int I2cSerialBt::availableForWrite() {
  // check for space in clients tx buffer
  return registerRead(1);
}

int I2cSerialBt::read() { 
  return registerRead(2);
}

// peek does not work (yet) ...
int I2cSerialBt::peek() { return 0; }

void I2cSerialBt::flush() { }

size_t I2cSerialBt::write(uint8_t byte) { 
  registerWrite(2, byte);
  return 1; 
}

// using Print::write;
