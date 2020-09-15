/*
 *  ftduinoblue_demo.ino
 *  
 *  This is a demo sketch for ftDuinoBlue. It make  
 */

#include <avr/pgmspace.h>

#define USE_I2C_BT   // use i2c bluetooth adapter, otherwise serial1 

#ifdef USE_I2C_BT
// use i2c bluetooth adapter
#include "I2cSerialBt.h"
I2cSerialBt btSerial;
#else
#define btSerial Serial1
#endif

#define LED LED_BUILTIN

// A very simple wrapper class that appends a checksum to
// every message sent.
class ftDuinoBlueSerial: public Stream {
public:
  ftDuinoBlueSerial(Stream &s) { this->mStream = &s; }
  int available() { return this->mStream->available(); }
  int read() { return this->mStream->read(); }
  size_t write(uint8_t b) { 
    if(b>=32) mOutSum += b;  // ignore all control characters (especially the \r and \n at the end of the line)
    // according to
    // https://www.arduino.cc/reference/en/language/functions/communication/serial/println/
    // println always sends \r\n. So we insert the checksum before the \r
    if(b == '\r') {
      this->mStream->write(':');
      this->mStream->print(mOutSum, HEX);
      mOutSum = 0;
    }
    return this->mStream->write(b);
  } 
  int peek() { return this->mStream->peek(); }
private:
  Stream *mStream;
  uint8_t mOutSum = 0;
};

ftDuinoBlueSerial ftdbSerial(btSerial);

// the layout to be sent to the ftDuinoBlue app
#define LAYOUT \
    "<layout orientation='portrait' name='ftDuinoBlue Led Demo'>" \
    "<switch id='1' size='20' width='parent' place='hcenter;top'>LED on/off</switch>" \
    "<label id='2' size='20' place='left;below:1'>LED brightness</label>" \
    "<slider id='3' width='parent' max='255' place='hcenter;below:2'/>" \
    "<label id='4' size='20' place='left;below:3'>Blink speed</label>" \
    "<slider id='5' width='parent' place='hcenter;below:4'/>" \
    "</layout>"

void setup() {
  Serial.begin(9600);     // some debug output is done on Serial (USB)
  btSerial.begin(9600);

// optionally set the bluetooth name.
//    delay(10);
//    btSerial.println("AT+NAMEftDuinoBlue");
//    delay(10);

  // prepare led
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);   
};

char parseHexDigit(char a) {
  if(a >= '0' && a <= '9') return a-'0';  
  if(a >= 'a' && a <= 'f') return 10+a-'a';  
  if(a >= 'A' && a <= 'F') return 10+a-'A';  
  return 0;
}

// skip current part (word) in string and any whitespace behind
// and return the position in front of the next part
int parseParameter(char **idx) {
  while(**idx && **idx != ' ') (*idx)++;  // skip non-whites
  while(**idx == ' ')          (*idx)++;  // skip whites
  return atoi(*idx);                      // parse number
}

// led config. This has a startup default and can be changed
// by the user via bluetooth. This is also reported back to
// the user app to provide the correct initial values should
// the user reconnect with the app
static char ledChanged = true;
static char ledState = true;         // LED is on
static uint8_t ledBlinkSpeed = 50;   // ~ 2 Hz
static uint8_t ledBrightness = 128;  // 50% brightness


void parseCommand(char *buffer) {       
  if(strcmp(buffer, "VERSION") == 0) {
    Serial.println("version cmd");
    ftdbSerial.println("VERSION 1.0.0");
  } else if(strcmp(buffer, "LAYOUT") == 0) {
    Serial.println("layout cmd");
    ftdbSerial.print("LAYOUT ");
    ftdbSerial.println(F(LAYOUT));
  } else if(strcmp(buffer, "STATE") == 0) {
    // app requests state. Send state of the LED switch
    Serial.println("state cmd");
    ftdbSerial.print("SWITCH 1 ");
    ftdbSerial.println(ledState?"ON":"OFF");
    ftdbSerial.print("SLIDER 3 ");
    ftdbSerial.println(ledBrightness);
    ftdbSerial.print("SLIDER 5 ");
    ftdbSerial.println(ledBlinkSpeed);
  } else if(strncmp(buffer, "BUTTON ", 7) == 0) {
    char *idx = buffer;
    char id = parseParameter(&idx);
    parseParameter(&idx);
    char down = (strncmp(idx, "DOWN", 4) == 0);

    Serial.print("BUTTON ");
    Serial.print(id, DEC);
    Serial.print(" ");
    Serial.println(down?"DOWN":"UP");
    
  } else if(strncmp(buffer, "SLIDER ", 7) == 0) {
    char *idx = buffer;
    char id = parseParameter(&idx);
    int value = parseParameter(&idx);
    
    Serial.print("SLIDER ");
    Serial.print(id, DEC);
    Serial.print(" ");
    Serial.println(value, DEC);

    if(id == 3) ledBrightness = value;
    if(id == 5) ledBlinkSpeed = value;
    ledChanged = true;   // user has changed something -> led needs to be updated

  } else if(strncmp(buffer, "SWITCH ", 7) == 0) {
    char *idx = buffer;
    char id = parseParameter(&idx);
    parseParameter(&idx);   // return value is ignored as we parse the string outselves
    char on = (strncmp(idx, "ON", 2) == 0);

    Serial.print("SWITCH ");
    Serial.print(id, DEC);
    Serial.print(" ");
    Serial.println(on?"ON":"OFF");
          
    if(id == 1) ledState = on;
    ledChanged = true;   // user has changed something -> led needs to be updated

  } else {
    Serial.print("unknown cmd ");
    Serial.println(buffer);
    ftdbSerial.print("ERROR:unknown command ");
    ftdbSerial.println(buffer);
  }
}

// the i2c_bluetooth adapter has a on-board led which can
// be used to show that the communication is fine.
void adapter_led() {
#ifdef USE_I2C_BT
  // flash led on adapter
  static uint32_t led_tick = 0;
  static bool led_state = false;
  if((!led_state && (millis() - led_tick) > 100) ||
     ( led_state && (millis() - led_tick) > 900)) {
    btSerial.led(led_state);
    led_tick = millis();
    led_state = !led_state;
  }  
#endif
}

void loop() {
  static char buffer[32];
  static uint8_t buffer_fill = 0;

  
  adapter_led();

  // led blink timer
  static uint32_t led_tick = 0;
  static bool led_state = false;

  // led is updated on timer or by user change
  if(ledChanged || (millis() - led_tick) > 50+10*(100-ledBlinkSpeed)) {
    // check if led update was triggered by user
    if(!ledChanged) {
      // no. So it was the timer -> handle timer
      led_state = !led_state;
      led_tick = millis();
    } else
      ledChanged = false;

    // update state of physical led. Set analog brightness if
    // a) led is enabled by user and b) led is actually in the on state
    // of the blinking. Switch it off otherwise
    if(led_state && ledState) analogWrite(LED, ledBrightness);
    else                      digitalWrite(LED, LOW);      
  }

  // parse everything coming from Serial1 (bluetooth side)
  while(ftdbSerial.available()) {
    uint8_t c = ftdbSerial.read();

    // buffer all characters but the control characters 
    if(c >= 32) {
      if(buffer_fill < sizeof(buffer))
        buffer[buffer_fill++] = c;
    } else if((c == '\n' || c == '\r')) {
      // verify checksum if present
      if(buffer_fill > 3 && buffer[buffer_fill-3] == ':') {
         // extract checksum
         uint8_t csum_in = 
            16 * parseHexDigit(buffer[buffer_fill-2]) +
                 parseHexDigit(buffer[buffer_fill-1]);

         // calculate checksum
         uint8_t csum_calc = 0;
         for(char i=0;i<buffer_fill-3;i++)
           csum_calc += buffer[i];

         if(csum_calc == csum_in) {
           // cut off checksum for further processing
           buffer[buffer_fill-3] = 0;
           parseCommand(buffer);
         }
       }
       // whatever happened, the buffer contents have been consumed now
       buffer_fill = 0;
    }
  }
};
