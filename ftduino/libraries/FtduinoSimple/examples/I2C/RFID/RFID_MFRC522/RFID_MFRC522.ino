#include <Wire.h>

#include "MFRC522_I2C.h"
#include <FtduinoSimple.h>

MFRC522 mfrc522(0x28, 0);  // Create MFRC522 instance.

void ShowReaderDetails();

void setup() {
  Serial.begin(115200); // Initialize serial communications with the PC

  // wait up to 3000ms for usb
  // for Leonardo/Micro/Zero
  uint32_t to = millis();
  while((!Serial) && ((millis()-to) < 3000))
    delay(10);

  Wire.begin(); // Initialize I2C
  mfrc522.PCD_Init();   // Init MFRC522
  ShowReaderDetails();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, type, and data blocks..."));

  // switch internal led on
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  static uint8_t registered_uid[4];
  static bool registered = false;
 
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    // no card -> both outputs off
    ftduino.output_set(Ftduino::O1, Ftduino::LO);
    ftduino.output_set(Ftduino::O2, Ftduino::LO);
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Dump debug info about the card; PICC_HaltA() is automatically called
//  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

  if (mfrc522.uid.size == 4) {
    // "accept" the first card we see
    if(!registered) {
      Serial.println("Card registered");
      memcpy(registered_uid, mfrc522.uid.uidByte, mfrc522.uid.size);
      digitalWrite(LED_BUILTIN, LOW);
      registered = true;
    } else {
      if(memcmp(registered_uid, mfrc522.uid.uidByte, mfrc522.uid.size) == 0) {
        Serial.println("Card accepted");
        ftduino.output_set(Ftduino::O1, Ftduino::HI);
        ftduino.output_set(Ftduino::O2, Ftduino::LO);          
      } else {          
        Serial.println("Card rejected");
        ftduino.output_set(Ftduino::O1, Ftduino::LO);
        ftduino.output_set(Ftduino::O2, Ftduino::HI);          
      }
    }
  }
  // next PICC_IsNewCardPresent() will fail even with card in range. Thus
  // call it now
  mfrc522.PICC_IsNewCardPresent();
}

void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown)"));
  Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
  }
}
