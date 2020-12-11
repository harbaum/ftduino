/*
 * bluetooth_config.ino
 * 
 * Bluetooth configuration tool for ftDuino bluetooth modules
 */

#include <Wire.h>
#include <avr/wdt.h>

#include "I2cSerialBt.h"
I2cSerialBt btSerial;

uint32_t cmd_timer = 0;
enum cmd_e { CMD_NONE=0, CMD_CHECK, CMD_RESET, CMD_BAUD, CMD_NAME, CMD_VERSION } cmd = CMD_NONE;
enum module_type_e { MODULE_NONE, MODULE_HM11, MODULE_HM17, MODULE_UNKNOWN } module_type = MODULE_NONE;

static const long baud_vals_hm11[] = { 9600, 19200, 38400, 57600, 115200,  4800,  2400,   1200, 230400, -1 };
static const long baud_vals_hm17[] = { 1200,  2400,  4800,  9600,  19200, 38400, 57600, 115200, 230400, -1 };

void halt() {
  Serial.println(F("Press a key to retry ..."));
  while(!Serial.available());
  wdt_enable(WDTO_15MS);
  for(;;);
}

void setup() {
  
  // setup connection to host and wait for host
  Serial.begin(115200);
  while(!Serial);

  Serial.println(F("\n\n\n-=- ftDuino bluetooth configuration -=-"));
  Serial.println(F("For more information see http://ftduino.de"));

  if(!btSerial.check(1000)) {
    Serial.println(F("ERROR: No ftDuino bluetooth adapter found on i²c address 5"));
    Serial.println(F("Please check that the module is properly connected and restart your ftDuino."));
    halt();
  }

  Serial.println(F("I²C uart bridge found."));

  baud_detect();

  Serial.println(F("Type 'HELP' for more info."));
  show_prompt();
}

// flush anything that might be in the queue from the bt modules
void bt_flush() {
  delay(50);
  while(btSerial.available()) { btSerial.read(); delay(50); }  
}

bool check_baud_value(long baud) {
  char buffer[16] = "";
  char fill = 0;
  
  Serial.print(F("Checking for baud value ... "));

  bt_flush();  
  btSerial.println(F("AT+BAUD?"));

  // wait for reply (with timeout)
  uint32_t timeout = millis();
  while(millis() - timeout < 1000) {
    if(btSerial.available()) {
      char c = btSerial.read();
      if(c > 31) {
        if(fill < sizeof(buffer)-1)
          buffer[fill++] = c;        
      } else if(c == '\n') {
        buffer[fill++] = 0;
        bt_flush();

        if(strlen(buffer) < 8) {
          Serial.print(F("FAILED (reply too short '"));
          Serial.print(buffer);
          Serial.println(F("')"));
          return false;
        }

        if(strncasecmp(buffer, "OK+Get:", 7) != 0) {
          Serial.print(F("FAILED (unexpected reply '"));
          Serial.print(buffer);
          Serial.println(F("')"));
          return false;          
        }

        if(buffer[7] < '0' || buffer[7] > '8') {
          Serial.print(F("FAILED (unexpected value '"));
          Serial.print(buffer[7]);
          Serial.println(F("')"));
          return false;                    
        }

        // check if this is the expected value
        char value = buffer[7] - '0';
        if((baud_vals_hm11[value] != baud) && (baud_vals_hm17[value] != baud)) {
          Serial.print(F("FAILED (values don't match '"));
          Serial.print(value, DEC);
          Serial.print(F(" != "));
          Serial.print(baud, DEC);
          Serial.println(F("')"));
          Serial.println(F("ERROR: Detected module is not a HM-11, HM-17 or HM-19."));
          Serial.println(F("       Baud rate settings disabled."));
          module_type = MODULE_UNKNOWN;
          return true;
        }
        
        if(baud_vals_hm11[value] == baud) {
          Serial.println(F("OK, found HM-11"));
          module_type = MODULE_HM11;
        } else {
          Serial.println(F("OK, found HM-17/HM-19"));
          module_type = MODULE_HM17;          
        }

        if(baud == 230400) {
          Serial.println(F("Warning: The detected baud rate is valid for HM-11,"));
          Serial.println(F("         HM-17 and HM-19. BAUD rate setting may be wrong"));
        }
        
        return true;
      }
    }
  }
  
  Serial.println(F("FAILED (reply timeout)"));
  bt_flush();  
  return false;
}

long baud_detect_rate() {
  // try to figure baud rate out
  long baud = -1;
  for(char i = 0; (baud < 0) && baud_vals_hm11[i] > 0;i++) {
    Serial.print(F("Checking for bluetooth module at "));
    Serial.print(baud_vals_hm11[i], DEC);
    Serial.print(F(" baud ..."));    
    
    if(baud_check(baud_vals_hm11[i])) {
      Serial.println(F(" OK"));
      baud = baud_vals_hm11[i];
    } else
      Serial.println(F(" FAILED"));
  }

  if(baud >= 0) {
    // check baud value of module and compare it against detected rate.
    if(!check_baud_value(baud))
      halt();
  }

  return baud;
}

void baud_detect() {
  if(baud_detect_rate() < 0) {  
    Serial.println(F("No module in AT mode found. Trying to force by sys_key ..."));

    btSerial.key(true);
    delay(1500);
    btSerial.key(false);
    delay(1000);
    
    if(baud_detect_rate() < 0) {  
      Serial.println(F("ERROR: No bluetooth module found!"));
      halt();
    }
  }
}

char baud_check(long rate) {
  char fill = 0, buffer[4]="";
  
  btSerial.begin(rate);
  Wire.setClock(400000);

  // flush buffer
  bt_flush();

  // send "AT"
  btSerial.println("AT");
  uint32_t timeout = millis();

  // wait for "OK"
  while((strncasecmp(buffer, "OK", 2) != 0 && (millis() - timeout) < 250))
    if(fill < 3 && btSerial.available())
      buffer[fill++] = btSerial.read();
      
  // flush buffer
  bt_flush();

  // return true if "OK" was found in buffer
  return(strncasecmp(buffer, "OK", 2) == 0);
}

void cmd_done() {
  cmd = CMD_NONE;
  cmd_timer = 0;
  show_prompt();
}

void send_reset() {
  send_at_cmd(CMD_RESET, (char*)"");
}

void send_baud(char *parms) {
  if(module_type != MODULE_HM11 && module_type != MODULE_HM17)
    return;
  
  if(!parms) {
     send_at_cmd(CMD_BAUD, NULL);
     return;
  }
  
  long b = atol(parms);

  char req_baud = -1;
  if(module_type == MODULE_HM11) {
    for(char i = 0; baud_vals_hm11[i] > 0;i++)
      if(baud_vals_hm11[i] == b)
        req_baud = i;
  } else {
    for(char i = 0; baud_vals_hm17[i] > 0;i++)
      if(baud_vals_hm17[i] == b)
        req_baud = i;    
  }

  if(req_baud < 0) {
    Serial.println(F("ERROR: Unsupported baud rate!"));
    cmd_done();
    return;
  }

  parms[0] = '0' + req_baud;
  parms[1] = 0;
  send_at_cmd(CMD_BAUD, parms);
}

void send_version() {
  send_at_cmd(CMD_VERSION, NULL);
}

void send_name(char *parms) {
  if(!parms) {
     send_at_cmd(CMD_NAME, NULL);
     return;
  }

  if(strlen(parms) > 13) {
    Serial.println(F("ERROR: New name too long. The name must have at most 13 characters."));
    cmd_done();
    return;
  }

  send_at_cmd(CMD_NAME, parms);
}

void send_check() {
  btSerial.println("AT");
  cmd = CMD_CHECK;
  cmd_timer = millis();
}

void send_at_cmd(cmd_e c, char *parms) {
  btSerial.print(F("AT+"));
  if(c == CMD_BAUD)  
    btSerial.print(F("BAUD"));
  else if(c == CMD_RESET)  
    btSerial.print(F("RESET"));
  else if(c == CMD_NAME)  
    btSerial.print(F("NAME"));
  else if(c == CMD_VERSION)  
    btSerial.print(F("VERS"));
  else {
    Serial.println(F("ERROR: Unknown command code"));
    cmd_done();
    return;
  }

  if(!parms) btSerial.println(F("?"));
  else       btSerial.println(parms);

  cmd = c;
  cmd_timer = millis();
}

void show_prompt() {
  Serial.print(F("CMD> "));
}

void print_help() {
  Serial.println(F("Available commands:"));
  Serial.println(F("HELP        - show this help"));  
  Serial.println(F("CHECK       - check communication"));  
  if(module_type == MODULE_HM11 || module_type == MODULE_HM17)
    Serial.println(F("BAUD <rate> - show/set baud rate"));  
  Serial.println(F("NAME <name> - show/set device name"));  
  Serial.println(F("VERSION     - show firmware version"));  
  Serial.println(F("RESET       - reset the module"));  
  Serial.println(F("KEY         - assert SYS KEY for 1.5 sec"));  
  if(module_type != MODULE_HM11 && module_type != MODULE_HM17)
    Serial.println(F("Baud rate setting are disabled for this unknown device!"));  
}

void parse_user(char c) {
  static char buffer[32+1];  // max 32 bytes + term char
  static char fill = 0;

  if(c >= 32 && fill < sizeof(buffer)-1) {
    buffer[fill++] = c;
    Serial.write(c);
  } else if(c == '\r') {
    Serial.println("");
    buffer[fill++] = 0;   // terminate command
    fill = 0;

    char *cmd_begin = buffer;
    while(*cmd_begin && *cmd_begin == ' ') cmd_begin++;
    char *cmd_end = cmd_begin;
    while(*cmd_end && *cmd_end != ' ') cmd_end++;

    char *parms = NULL;
    if(*cmd_end != 0) {
      parms = cmd_end;
      while(*parms && *parms == ' ') parms++;
      if(!*parms)
        parms = NULL;  // there were only white spaces after the command

      // remove all trailing spaces
      char *pend = parms;
      while(*pend) pend++;
      while(pend > parms && (*pend == 0 || *pend == ' '))
        *pend-- = 0;
    }

    // terminate command
    *cmd_end = 0;

    if(strcasecmp(cmd_begin, "HELP") == 0) {
      print_help();
      show_prompt();
    } else if(strcasecmp(cmd_begin, "CHECK") == 0) {
      send_check();
    } else if(strcasecmp(cmd_begin, "RESET") == 0) {
      send_reset();
    } else if(strcasecmp(cmd_begin, "KEY") == 0) {
      Serial.println(F("Activating key!"));
      btSerial.key(true);
      delay(1500);
      Serial.println(F("Releasing key!"));
      btSerial.key(false);
      show_prompt();
    } else if(strcasecmp(cmd_begin, "NAME") == 0) {
      send_name(parms);
    } else if(strcasecmp(cmd_begin, "VERSION") == 0) {
      send_version();
    } else if(((module_type == MODULE_HM11) || 
               (module_type == MODULE_HM17)) &&
               strcasecmp(cmd_begin, "BAUD") == 0) {
      send_baud(parms);
    } else {
      Serial.print(F("ERROR: Unknown command '"));
      Serial.print(cmd_begin);
      Serial.println(F("'!"));
      show_prompt();
    }
  }
}

void parse_bt(char c) {
  static char buffer[32+1];  // max 32 bytes + term char
  static char fill = 0;

  if(c >= 32 && fill < sizeof(buffer)-1) {
    buffer[fill++] = c;
  } else if(c == '\r') {
    buffer[fill++] = 0;   // terminate command
    fill = 0;

    // check if reply begins with "OK+"
    if(strncasecmp(buffer, "OK", 2) == 0) {
      // if the pending command is CMD_CHECK, then the OK is all we need
      if(cmd == CMD_CHECK)
        Serial.println(F("Module is ok"));
      else {      
        // check if this is a 'Set:' reply
        if(strncasecmp(buffer+2, "+Set:", 5) == 0) {
          if(cmd == CMD_BAUD) {
            char val = buffer[7]-'0';
            if(val < 0 || val > 8) {
              Serial.print(F("ERROR: Unexpected baud value: "));
              Serial.println(val, DEC);
            } else {            
              Serial.print(F("BAUD rate changed to "));
              Serial.print(val, DEC);
              Serial.print(F(": "));
              if(module_type == MODULE_HM11)
                Serial.print(baud_vals_hm11[val], DEC);
              else if(module_type == MODULE_HM17)
                Serial.print(baud_vals_hm17[val], DEC);
              else {
                Serial.print("value");
                Serial.print(val, DEC);
              }
              Serial.println(F(". A reset is needed to activate it."));
            }
          } else if(cmd == CMD_NAME) {
            char *name = buffer+7;
            
            Serial.print(F("Name changed to: '"));
            Serial.print(name);
            Serial.println(F("'"));
          } else {
            Serial.print(F("Set value to: "));
            Serial.println(buffer+7);                      
          }
        }
        
        // check if this is a 'Get:' reply
        else if(strncasecmp(buffer+2, "+Get:", 5) == 0) {
          if(cmd == CMD_BAUD) {
            char val = buffer[7]-'0';
            if(val < 0 || val > 8) {
              Serial.print(F("ERROR: Unexpected baud value: "));
              Serial.println(val, DEC);
            } else {            
              Serial.print(F("BAUD value is "));
              Serial.print(val, DEC);
              Serial.print(F(": "));
              if(module_type == MODULE_HM11)
                Serial.print(baud_vals_hm11[val], DEC);
              else if(module_type == MODULE_HM17)
                Serial.print(baud_vals_hm17[val], DEC);
              else {
                Serial.print("value");
                Serial.print(val, DEC);
              }
                
              Serial.println(F(" baud."));
            }
          } else if(cmd == CMD_VERSION) {
            // hm-17 and hm-19 report with "Get:"
            Serial.print(F("Firmware version is '"));
            Serial.print(buffer+7);
            Serial.println(F("'."));            
          } else {
            Serial.print(F("Got a value back: "));
            Serial.println(buffer+7);
          }
        }

        else if(strncasecmp(buffer+2, "+reset", 6) == 0) {
          Serial.println(F("Reset successful. Re-checking for module"));
          delay(1000);
          baud_detect();
        }
                
        else if(strncasecmp(buffer+2, "+name:", 6) == 0) {
          char *name = buffer+8;
          while(*name == ' ') name++;
          
          Serial.print(F("Name is: '"));
          Serial.print(name);
          Serial.println("'");
        }
        
        else {
          Serial.print(F("Reply code: "));
          Serial.println(buffer+2);        
        }
      }
    } else if(cmd == CMD_VERSION) {
      // hm-11 doesn't even send 'ok'
      Serial.print(F("Firmware version is '"));
      Serial.print(buffer);
      Serial.println(F("'."));            
    } else {
      Serial.print(F("ERROR: Unexpected reply: '"));
      Serial.print(buffer);
      Serial.println(F("'"));
    }
    cmd_done();
  }
}

void loop() {
  while(Serial.available() && !cmd_timer)
    parse_user(Serial.read());

  while(btSerial.available())
    parse_bt(btSerial.read());

  // check for 2 seconds command timeout
  if(cmd_timer && (millis()- cmd_timer > 2000)) {
    Serial.println(F("ERROR: Timeout while waiting for reply from module"));
    cmd_done();
  }
}
