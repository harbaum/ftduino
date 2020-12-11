#include "FirmataSerial.h"

// #define DEBUG_SYSEX
// #define DEBUG_BASIC

#define VERSION_MAJOR 1
#define VERSION_MINOR 6

bool FirmataSerial::parse_sysex(byte st, byte b) {
  static uint8_t uart_flag, uart_byte;   // buffer to assemble uart bytes 

  // end of sysex message
  if(b == 0xf7)
    return true;

  // start of a new message?
  if(st) {
    uart_flag = 0;

    switch(b) {
#ifdef DEBUG_SYSEX    
    case 0x5c:
      Serial.print(" QUERY_UART_BAUD_RATE");
      break;
    case 0x5f:
      Serial.print(" REPORT_INPUT_PINS");
      break;
    case 0x62:
      Serial.print(" IS_ALIVE");
      break;
    case 0x64:
      Serial.print(" FIRMATA_MUTE");
      break;
#endif
    case 0x66:
#ifdef DEBUG_SYSEX    
      Serial.print(" UART_DATA");
#endif
      uart_flag = 1;
      break;
    default:  
#ifdef DEBUG_SYSEX    
      Serial.print(" CMD");
      Serial.print(b, HEX);
#endif
      break;
    }
  } else {        
    // with a sysex command   
    if(uart_flag) {
      // uart parsing in progress?
      if(uart_flag & 1) {
        uart_flag = 2;
        uart_byte = b & 0x7f;   // actually bit 7 must never be set, anyway
      } else {
        uart_flag = 1;
        uart_byte |= b << 7;

        this->uart_buffer_valid = true;
        this->uart_buffer = uart_byte;
      }
    }
#ifdef DEBUG_SYSEX    
    else {        
      Serial.print("(");
      Serial.print(b, HEX);
      Serial.print(")");
    }
#endif
  }
  return false;
}

void FirmataSerial::parse(byte b) {
  static int8_t expect = 0;
  
  if(expect) {
    if(expect > 0) {    
#ifdef DEBUG_BASIC
      Serial.print(" ");
      Serial.print(b, HEX);
#endif
      expect--;

#ifdef DEBUG_BASIC
      if(expect == 0)
        Serial.println("");
#endif
    } else {
      if(parse_sysex(expect==-2, b)) {
#ifdef DEBUG_BASIC
        Serial.println(" <");
#endif
        expect = 0;   // done parsing sysex
      } else
        expect = -1;  // continue parsing sysex
    }
    return;
  }

  if((b & 0xf0) == 0xe0) {
#ifdef DEBUG_BASIC
    Serial.print("analog port message");
    Serial.print(b&0x0f, DEC);
    Serial.print(" ");
#endif
    expect = 2;
  } else if((b & 0xf0) == 0x90) {
#ifdef DEBUG_BASIC
    Serial.print("digital port message");
    Serial.print(b&0x0f, DEC);
    Serial.print(" ");
#endif
    expect = 2;
  } else if((b & 0xf0) == 0xc0) {
#ifdef DEBUG_BASIC
    Serial.print("report analog port ");
    Serial.print(b&0x0f, DEC);
#endif
    expect = 1;
  } else if((b & 0xf0) == 0xd0) {
#ifdef DEBUG_BASIC
    Serial.print("report digital port ");
    Serial.print(b&0x0f, DEC);
#endif
    expect = 1;
  } else if(b == 0xf0) {
#ifdef DEBUG_BASIC
    Serial.print("sysex");
#endif
    expect = -2;
  } else if(b == 0xf4) {
#ifdef DEBUG_BASIC
    Serial.print("set pin mode");
#endif
    expect = 2;
  } else if(b == 0xf5) {
#ifdef DEBUG_BASIC
    Serial.print("set digital pin value");
#endif
    expect = 2;
  } else if(b == 0xf9) {
#ifdef DEBUG_BASIC
    Serial.println("version");
#endif
#if defined(VERSION_MAJOR) && defined(VERSION_MINOR)
    this->m_s->write(0xf9);
    this->m_s->write(VERSION_MINOR);
    this->m_s->write(VERSION_MAJOR);
#endif
    expect = 0;
  } else if(b == 0xff) {
#ifdef DEBUG_BASIC
    Serial.println("system reset");
#endif
    expect = 0;
  }
  
#ifdef DEBUG_BASIC
  else {
    Serial.print("Unknown cmd ");
    Serial.println(b, HEX);
  }
#endif
}

FirmataSerial::FirmataSerial(Stream &s) {
  this->uart_buffer_valid = false;
  this->m_s = &s;
}

void FirmataSerial::begin() { }
  
void FirmataSerial::end() { }

void FirmataSerial::poll(void) { 
  // if there is a byte in the buffer don't read more as
  // we couldn't store any result
  if(this->uart_buffer_valid)
    return;
  
  if(this->m_s->available())
    parse(this->m_s->read());   
}

int FirmataSerial::available() {
  poll();
  return this->uart_buffer_valid?1:0;
}

int FirmataSerial::availableForWrite() {
  return 1;
}

int FirmataSerial::read() { 
  poll();
  this->uart_buffer_valid = false;  
  return this->uart_buffer;
}

int FirmataSerial::peek() { 
  return this->uart_buffer; 
}

void FirmataSerial::flush() { }

size_t FirmataSerial::write(uint8_t byte) { 
  // write as serial
  this->m_s->write(0xf0);  // sysex start
  this->m_s->write(0x66);  // uart data
  this->m_s->write(byte & 0x7f); // LSB
  this->m_s->write((byte >> 7) & 1); // MSB  
  this->m_s->write(0xf7);  // sysex end
  
  return 1; 
}
