//
// Temperaure.ino
//
// Abfrage eines fischertechnik Temperatur-Widerstands an Eingang I1
//
// (c) 2018 by Till Harbaum <till@harbaum.org>
//

#define USE_OLED

#include <Ftduino.h>
#include <math.h>          // Für Fliesspunkt-Arithmetik

#define K2C 273.15         // Offset Kelvin nach Grad Celsius
#define B 3900.0           // sog. B-Wert des Sensors
#define R_N 1500.0         // Widerstand bei 25°C Referenztemperatur
#define T_N (K2C + 25.0)   // Referenztemperatur in Kelvin

#ifdef USE_OLED
#include <Adafruit_GFX.h>
#include "FtduinoDisplay.h"
FtduinoDisplay display;
#endif

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

#ifdef USE_OLED
  display.setTextColor(WHITE);
  display.setTextSize(1);

  display.setCursor(64-(13*3),0);
  display.print(">> ftDuino <<");

  display.display();
#else 
  Serial.begin(115200);
  while(!Serial);
#endif

  ftduino.init();  

  // Die Temperaturmessung erfolgt mit einem 
  ftduino.input_set_mode(Ftduino::I1, Ftduino::RESISTANCE);
  ftduino.output_set(Ftduino::O1, Ftduino::LO, Ftduino::MAX);
}

void loop() {
  uint16_t r = ftduino.input_get(Ftduino::I1);

#ifdef USE_OLED
  static uint16_t r_avg[10];
  static uint8_t used = sizeof(r_avg) / sizeof(uint16_t);

  memmove(r_avg+1, r_avg, sizeof(r_avg) - sizeof(uint16_t));
  r_avg[0] = r;
  if(used) used--;
  else { 
    char i; 
    uint32_t r_sum = 0;
    for(i=0;i<used;i++) r_sum += r_avg[i];
    r_sum /= used;
    
    String tmp_str = String(r2deg(r)) + "^C";

    display.setTextSize(2);
    display.fillRect(0, 16, 128, 16, BLACK);
    display.setCursor(64-(tmp_str.length()*12)/2,16);
    display.print(tmp_str);
    display.display();

    if(r2deg(r) >= 30.0)
      ftduino.output_set(Ftduino::O1, Ftduino::HI, Ftduino::MAX);
    else
      ftduino.output_set(Ftduino::O1, Ftduino::LO, Ftduino::MAX);
  }
  delay(100);
#else
  Serial.print("I1: ");
  Serial.print(r2deg(r));
  Serial.println(" °C");
  delay(1000);
#endif

}
