#include "ftduinoblue.h"
#include <Stream.h>

FtduinoBlue::FtduinoBlue(Stream &s, const char *l) {
  this->mStream = &s; 
  this->layout = l;
  this->mOutSum = 0;
  this->buffer_fill = 0;
  this->m_state_cb = NULL;
  this->m_button_cb = NULL;
  this->m_switch_cb = NULL;
  this->m_slider_cb = NULL;
  this->m_joystick_cb = NULL;
}

void FtduinoBlue::setStateCb(void (*cb)()) {
  this->m_state_cb = cb;
}

void FtduinoBlue::setButtonCb(void (*cb)(char, bool)) {
  this->m_button_cb = cb;
}

void FtduinoBlue::setSwitchCb(void (*cb)(char, bool)) {
  this->m_switch_cb = cb;
}

void FtduinoBlue::setSliderCb(void (*cb)(char, int)) {
  this->m_slider_cb = cb;
}

void FtduinoBlue::setJoystickCb(void (*cb)(char, char, char)) {
  this->m_joystick_cb = cb;
}

int FtduinoBlue::available() {
  return this->mStream->available(); 
}

int FtduinoBlue::read() { 
  return this->mStream->read(); 
}

size_t FtduinoBlue::write(uint8_t b) { 
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

int FtduinoBlue::peek() {
  return this->mStream->peek(); 
}

char FtduinoBlue::parseHexDigit(char a) {
  if(a >= '0' && a <= '9') return a-'0';  
  if(a >= 'a' && a <= 'f') return 10+a-'a';  
  if(a >= 'A' && a <= 'F') return 10+a-'A';  
  return 0;
}

void FtduinoBlue::handle(void) {
  // parse everything coming from Serial1 (bluetooth side)
  while(available()) {
    uint8_t c = read();

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
}

// skip current part (word) in string and any whitespace behind
// and return the position in front of the next part
int FtduinoBlue::parseParameter(char **idx) {
  while(**idx && **idx != ' ') (*idx)++;  // skip non-whites
  while(**idx == ' ')          (*idx)++;  // skip whites
  return atoi(*idx);                      // parse number
}

void FtduinoBlue::parseCommand(char *buffer) {       
  if(strcmp(buffer, "VERSION") == 0) {
    println("VERSION 1.0.0");
  } else if(strcmp(buffer, "LAYOUT") == 0) {
    print("LAYOUT ");

    // the layout is stored in flash and needs to be read from there
    const char *c = layout;
    while(pgm_read_byte_near(c))
      write(pgm_read_byte_near(c++));
    println("");
  } else if(strcmp(buffer, "STATE") == 0) {
    // app requests state. Send state of the LED switch
    Serial.println("state cmd");

    // received the STATE command from the android app. Reply
    // with the current value for sliders and switches
    if(m_state_cb) m_state_cb();
    
  } else if(strncmp(buffer, "BUTTON ", 7) == 0) {
    char *idx = buffer;
    char id = parseParameter(&idx);
    parseParameter(&idx);
    char down = (strncmp(idx, "DOWN", 4) == 0);
    if(m_button_cb) m_button_cb(id, down);    
  } else if(strncmp(buffer, "SLIDER ", 7) == 0) {
    char *idx = buffer;
    char id = parseParameter(&idx);
    int value = parseParameter(&idx);
    if(m_slider_cb) m_slider_cb(id, value);    
  } else if(strncmp(buffer, "JOYSTICK ", 9) == 0) {
    char *idx = buffer;
    char id = parseParameter(&idx);
    int value1 = parseParameter(&idx);
    int value2 = parseParameter(&idx);
    if(m_joystick_cb) m_joystick_cb(id, value1, value2);    
  } else if(strncmp(buffer, "SWITCH ", 7) == 0) {
    char *idx = buffer;
    char id = parseParameter(&idx);
    parseParameter(&idx);   // return value is ignored as we parse the string outselves
    char on = (strncmp(idx, "ON", 2) == 0);
    if(m_switch_cb) m_switch_cb(id, on);    
  } else {
    Serial.print("unknown cmd ");
    Serial.println(buffer);
    print("ERROR:unknown command ");
    println(buffer);
  }
}
