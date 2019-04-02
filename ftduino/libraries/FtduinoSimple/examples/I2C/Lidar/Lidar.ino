// Lidar.ino
// a very simple HP/GL plotter using two stepper motors

#include <Wire.h>

#include "FtduinoDisplay.h"
FtduinoDisplay display;

#include <FtduinoSimple.h>
#include "Adafruit_VL53L0X.h"
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  // I²C is being used for the display as well as the lidar sensor
  Wire.begin();
  
  display.begin();
  display.setTextColor(WHITE);

  if (!lox.begin()) {
    display.setTextSize(2);
    display.fillRect(0, 0, 128, 32, BLACK);
    display.setCursor(0,  0); display.print("Sensor Err");
    display.display();
    
    while(1);
  }
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure;

  digitalWrite(LED_BUILTIN, HIGH);
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
  digitalWrite(LED_BUILTIN, LOW);

  display.fillRect(0, 0, 128, 32, BLACK);
  display.setTextSize(4);
  display.setCursor(0,0);
    
  if((measure.RangeStatus != 4) &&
     (measure.RangeMilliMeter < 8191) &&
     (measure.RangeMilliMeter > 0)) { 
    display.print(measure.RangeMilliMeter);
    display.print("mm");
  } else
    display.print("RANGE");
  
  display.display();
  delay(50);
}
