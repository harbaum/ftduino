/* Simples Beispiel, um den Mini-I²C-Servo-Adapter zu verwenden */

#include <Wire.h>

#define ADDRESS 0x11  // Standardadresse des Adapters

void setup() {
  Wire.begin();  // I2C-Bus öffnen

  // Servo 1 auf Mittenposition und eine Sekunde warten
  Wire.beginTransmission(ADDRESS);
  Wire.write(0x00);
  Wire.write(92);
  Wire.endTransmission();

  delay(1000);  
}

// nutzbarer Wertebreeich 62 bis 125. Entspricht Pulsweite 1 bis 2ms
// bzw minimaler und maximaler Servoauslenkung
byte x = 62;     
byte y = 125;

void loop() {
  Wire.beginTransmission(ADDRESS);
  Wire.write(0x00);          // Register 0: PWM-Wert Servo 1 setzen
  Wire.write(x);
  Wire.endTransmission(); 

  Wire.beginTransmission(ADDRESS);
  Wire.write(0x01);          // Register 1: PWM-Wert Servo 2 setzen
  Wire.write(y);
  Wire.endTransmission();

  // beide Servos gegenläufig bewegen
  x++;
  if(x>125) x=62;
  
  y--;
  if(y<62) y=125;
  
  delay(50);
}
