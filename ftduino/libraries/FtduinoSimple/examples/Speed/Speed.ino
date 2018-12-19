//
// Speed.ino
//
// measure speed with two light barriers
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

  display.setCursor(64-(11*3),0);
  display.print(">> Speed <<");

  display.display();
#else 
  Serial.begin(115200);
  while(!Serial);
  Serial.println(">> Speed measurement <<");

  if(!ftduino.counter_get_state(Ftduino::C1))
    Serial.println("Warning: Input C1 not low");
  
  if(!ftduino.counter_get_state(Ftduino::C2))
    Serial.println("Warning: Input C2 not low");
#endif

}

#define DIST      30              // 30mm
#define DIST_P_M  (1000/DIST)

void loop() {
  uint8_t c1 = ftduino.counter_get_state(Ftduino::C1);
  uint8_t c2 = ftduino.counter_get_state(Ftduino::C2);

  //
  static uint32_t start;
  if(!c1 && c2) start = micros();
  else if(!c2) {
    uint32_t t = micros() - start;
    char str[32], n[10];

#ifdef USE_OLED
    display.setTextColor(WHITE);
    display.fillRect(0, 8, 128, 24, BLACK);
    sprintf(str, "Time: %ld us", t);
    display.setCursor(64-(strlen(str)*6)/2, 8);
    display.print(str);
    dtostrf(1000000.0/((float)t*DIST_P_M), 4, 2, n);
    sprintf(str, "%s m/s", n);
    display.setCursor(64-(strlen(str)*6)/2, 16);
    display.print(str);
    dtostrf(3600000.0/((float)t*DIST_P_M), 4, 2, n);
    sprintf(str, "%s km/h", n);
    display.setCursor(64-(strlen(str)*6)/2, 24);
    display.print(str);
    display.display();
#else
    Serial.print("Time: ");
    Serial.print(t, DEC);
    Serial.print("µs -> ");
    dtostrf(1000000.0/((float)t*DIST_P_M), 4, 2, str);
    Serial.print(str);
    Serial.print("m/s -> ");
    dtostrf(3600000.0/((float)t*DIST_P_M), 4, 2, str);
    Serial.print(str);
    Serial.println("km/h");
#endif

    // wait until obstacle removed
    while(!ftduino.counter_get_state(Ftduino::C1) ||
          !ftduino.counter_get_state(Ftduino::C2));
    
    delay(1000);
  }
}
