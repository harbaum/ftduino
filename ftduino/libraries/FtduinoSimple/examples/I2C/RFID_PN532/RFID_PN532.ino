/**************************************************************************/

/**************************************************************************/

#include <Wire.h>
#include "Adafruit_PN532.h"

#include <FtduinoSimple.h>

#define PN532_IRQ   (0)
#define PN532_RESET (0)  // Not connected

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10); // for Leonardo/Micro/Zero

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  
  Serial.println("Waiting for an ISO14443A Card ...");

  // switch internal led on
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop(void) {
  static uint8_t registered_uid[4];
  static bool registered = false;
  
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100)) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("  UID Value: ");  nfc.PrintHex(uid, uidLength);
    Serial.println("");
    
    if (uidLength == 4) {
      Serial.println("Seems to be a Mifare Classic card (4 byte UID)");

      // "accept" the first card we see
      if(!registered) {
        Serial.println("Card registered");
        memcpy(registered_uid, uid, uidLength);
        digitalWrite(LED_BUILTIN, LOW);
        registered = true;
      } else {
        if(memcmp(registered_uid, uid, uidLength) == 0) {
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
  } else {
    // no card -> both outputs off
    ftduino.output_set(Ftduino::O1, Ftduino::LO);
    ftduino.output_set(Ftduino::O2, Ftduino::LO);
  }
  
}
