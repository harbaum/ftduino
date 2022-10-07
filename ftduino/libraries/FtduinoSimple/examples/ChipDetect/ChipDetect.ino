// ChipDetect.ino
//
// Detect the output driver chip installed on the ftDuino

#include <SPI.h>
#include <Adafruit_GFX.h>
#include "FtduinoDisplay.h"

FtduinoDisplay display;

#define CHIP_UNKNOWN  0
#define CHIP_MC33879  1
#define CHIP_TLE94108 2
#define CHIP_DRV8908  3

int chip = CHIP_UNKNOWN;

const String names[] = {
   "UNKNOWN", "MC33879", "TLE94108", "DRV8908"
};

void print_centered(char size, char y, String str) {
  display.setTextSize(size);
  display.setCursor(64-(str.length()*3*size), 8*y);
  display.print(str);
}

void driver_prepare() {
  // start the SPI library:
  SPI.begin();
  SPI.beginTransaction(SPISettings(250000, MSBFIRST, SPI_MODE1));

  // /CS is PB.0
  DDRB |= (1<<0);
  
  // For MC33879 PB.7 is the PWM input of the IN6 of O2. We don't use that
  // feature in this driver, so it's just pulled down. This is also ok with
  // the TLE94108 which has this pin grounded. For the DRV8908 this is the
  // fault output which is an open collector output. Grounding that is also
  // not a problem.  
  DDRB |= (1<<7);
  PORTB &= ~(1<<7);  // no PWM on IN6

  // get chip out of sleep mode
  DDRE  |= (1<<6);
  PORTE |= (1<<6);  // not in sleep mode
}

uint8_t transfer(uint8_t val) {
  delay(1);
  PORTB &= ~(1<<0);  
  uint8_t r = SPI.transfer(val);
  PORTB |=  (1<<0);
  
  return r;
}

int probe() {
  transfer(0x00);  // first transfer will cause a SPI error on TLE94108
  uint8_t r = transfer(0x00);
  
  // test for MC33879: This simply returns 0x00
  if(!r) return CHIP_MC33879;

  // test for TLE94108: ignore undervoltage bit 4, all other bits should be
  // zero except bit 0 which reports a SPI error as 0x00 is not a valid request
  if((r & ~4) == 0x01) return CHIP_TLE94108;

  // test for drv8908: The two upper bits are always 1, ignore undervoltage
  // bit 4, all other bits should be zero
  if((r & ~4) == 0xc0) return CHIP_DRV8908;
  
  return CHIP_UNKNOWN;
}

void setup() {
  Serial.begin(115200);

  driver_prepare();
  
  // wait a second for USB
  uint32_t to = millis();
  while(!Serial && millis()-to < 2000);

  Serial.println("ftDuino output driver chip detector");
  
  // LED initialisieren
  pinMode(LED_BUILTIN, OUTPUT);
  
  display.setTextColor(WHITE);

  print_centered(1, 0, "Driver Detection");

  chip = probe();
  Serial.print("Chip: ");
  Serial.println(names[chip]);

  print_centered(2, 2, names[chip]);  

  display.display();
}

void blink(int len) {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(len);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}

void loop() {
  blink(250);   // long
  
  // blink pattern if no display is connected
  // n * short
  for(char i=0;i<chip;i++)
    blink(100);
    
  delay(1000);
}
