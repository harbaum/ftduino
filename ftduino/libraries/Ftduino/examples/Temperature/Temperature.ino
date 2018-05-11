//
// Temperaure.ino
//
// Abfrage eines fischertechnik Temperatur-Widerstands an Eingang I1
//
// (c) 2018 by Till Harbaum <till@harbaum.org>
//

#include <Ftduino.h>
#include <math.h>          // Für Fliesspunkt-Arithmetik

#define K2C 273.15         // Offset Kelvin nach Grad Celsius
#define B 3900.0           // sog. B-Wert des Sensors
#define R_N 1500.0         // Widerstand bei 25°C Referenztemperatur
#define T_N (K2C + 25.0)   // Referenztemperatur in Kelvin

float r2deg(uint16_t r) {
  if(r == 0) return NAN;   // ein Widerstand von 0 Ohm ergibt keine sinnvolle Temperatur
        
  // Widerstand in Kelvin umrechnen
  float t = T_N * B / (B + T_N * log(r / R_N));

  // Kelvin in °C umrechnen
  return t - K2C;

  // alternativ: Kelvin in °F umrechnen
//  return t * 9 / 5 - 459.67;
}
        
void setup() {
  // LED initialisieren
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.begin(115200);
  while(!Serial);

  ftduino.init();  

  // Die Temperaturmessung erfolgt mit einem 
  ftduino.input_set_mode(Ftduino::I1, Ftduino::RESISTANCE);
}

void loop() {
  uint16_t r = ftduino.input_get(Ftduino::I1);

  Serial.print("I1: ");
  Serial.print(r2deg(r));
  Serial.println(" °C");

  delay(1000);
}
