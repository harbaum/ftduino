/*
  IoServer for ftDuino (http://ftduino.de)

  WebUSB client driver/sketch as required e.g. for brickly-lite
  https://harbaum.github.io/ftduino/webusb/brickly-lite

  (c) 2018 by Till Harbaum <till@harbaum.org>
*/

#if USB_VERSION != 0x210
#warning "Compiling for regular setup. For WebUSB select 'ftDuino (WebUSB)' board type"
#else
#warning "Compiling for WebUSB setup. For regular version select 'ftDuino' board type"

#include <WebUSB.h>
WebUSB WebUSBSerial(1 /* https:// */, "harbaum.github.io/ftduino/webusb/brickly-lite");
#define Serial WebUSBSerial
#endif

#include <Ftduino.h>

#include "FtduinoDisplay.h"
#include "JsonParser.h"

FtduinoDisplay display;
JsonParser parser;

// die setup-Funktion wird einmal beim Start aufgerufen
void setup() {
  Serial.begin(115200);

  ftduino.init();

  display.fillRect(0, 0, 128, 12, WHITE);
  display.setTextColor(BLACK);
  display.setTextSize(1);
#if USB_VERSION != 0x210
  display.setCursor(64-(20*3), 2);
  display.print("> ftDuino IoServer <");
#else
  display.setCursor(64-(21*3), 2);
  display.print("ftDuino WebUSB Server");
#endif

  display.display();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

// die loop-Funktion wird immer wieder aufgerufen
void loop() {
  // frequently trigger flush. This works around a problem in
  // the Arduino WebUSB stack that sometimes causes data not
  // to be sent even if flushed
  static uint32_t last_flush = 0;
  if((millis() - last_flush) > 100) {
    Serial.flush();
    last_flush = millis();
  }
  
  if(Serial.available())
    parser.parse(Serial.read());
}
