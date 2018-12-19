//
// Gravity.ino
//
// (c) 2018 by Till Harbaum <till@harbaum.org>
//

#define USE_OLED

#include <FtduinoSimple.h>

#ifdef USE_OLED
#include <Adafruit_GFX.h>
#include "FtduinoDisplay.h"
FtduinoDisplay display;
#endif

void setup() {
  // LED initialisieren
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

#ifdef USE_OLED
  display.fillRect(0, 0, 128, 8, WHITE);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.setCursor(64-(13*3),0);
  display.print(">> Gravity <<");

  display.setTextColor(WHITE);
  display.display();
#else 
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Gravity");

  if(!ftduino.counter_get_state(Ftduino::C1))
    Serial.println("Warning: Input C1 not low");
    
  if(ftduino.input_get(Ftduino::I1))
    Serial.println("Warning: Input I1 not high");
#endif

  ftduino.motor_set(Ftduino::M1, Ftduino::LEFT);
}

#define DIST  0.09   // 9 cm

void loop() {
  static uint32_t start = 0;
  uint8_t c1 = ftduino.counter_get_state(Ftduino::C1);
  uint8_t i1 = ftduino.input_get(Ftduino::I1);
  
  //
  if(!start && i1) {
    start = micros();
    ftduino.motor_set(Ftduino::M1, Ftduino::OFF);
  }

  if(start && !c1) {
    uint32_t t = micros() - start;
    float ts = (float)t/1000000;  // us time in seconds
    char str[32], n[10];
    
    // g = 2*s/t²
    dtostrf(2*0.09/(ts*ts), 4, 2, n);

#ifdef USE_OLED
    display.fillRect(0, 8, 128, 24, BLACK);
    sprintf(str, "%ld us", t);
    display.setCursor(64-(strlen(str)*6)/2,8);
    display.setTextSize(1);
    display.print(str);
    
    sprintf(str, "%s m/s^2", n);
    display.setCursor(64-(strlen(str)*12)/2,16);
    display.setTextSize(2);
    display.print(str);
    
    display.display();
#else
    Serial.print(t, DEC);
    Serial.print("µs -> ");
    Serial.print(n);
    Serial.println("m/s²");
#endif

    delay(1000);
    ftduino.motor_set(Ftduino::M1, Ftduino::LEFT);

    // wait until obstacle removed
    while(!ftduino.counter_get_state(Ftduino::C1));
    start = 0;
  }
}
