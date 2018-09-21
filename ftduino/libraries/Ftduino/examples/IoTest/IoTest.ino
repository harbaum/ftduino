//
// IoTest.ino
//
// (c) 2018 Till Harbaum <till@harbaum.org>
//

#include <Ftduino.h>
#include <Wire.h>

boolean ultrasonic_enabled = false;

char num_parms(char *p) {
  char parms = 0;

  if(!p) return 0;
  
  while(*p) {
    // skip all whitespace
    while(*p && isWhitespace(*p)) p++;
    if(*p) {
      parms++;
      while(*p && !isWhitespace(*p)) p++;      
    }
  }
  return parms;
}

boolean parms_check(char *p, char min, char max) {
  char num = num_parms(p);
  
  if((num < min) || (num > max)) {
    Serial.print("Error: expected ");
    Serial.print(min, DEC);
    if(min != max) {
      Serial.print(" to ");
      Serial.print(max, DEC);
    }    
    Serial.print(" parameters, got ");
    Serial.print(num, DEC);
    Serial.println("!");
    return false;
  }
  return true;
}

char *parm_get_p(char *p, char idx) {
  static char lbuf[16];
  char *lptr = lbuf;

  while(*p) {
    // skip whitespace
    while(*p && isWhitespace(*p)) p++;
    // skip parameter
    lptr = lbuf;
    while(*p && !isWhitespace(*p)) {
      if((lptr - lbuf) < sizeof(lbuf)-1) *lptr++ = *p++;
      else                               p++;
    }
    *lptr = '\0';
    
    // matching index -> return 
    if(!idx) return lbuf;
    idx--;
  }
  return NULL;  
}

// check if the string str starts with the string pre
bool startswith(const char *pre, const char *str) {
  size_t lenpre = strlen(pre), lenstr = strlen(str);
  return lenstr < lenpre ? false : strncasecmp(pre, str, lenpre) == 0;
}

boolean parm_get_bool(char *p, char idx) {
  p = parm_get_p(p, idx);
  return
    (startswith(p, "on")) ||
    (startswith(p, "true")) ||
    (startswith(p, "1"));
}

int16_t parm_get_int(char *p, char idx) {
  return String(parm_get_p(p, idx)).toInt();
}

uint8_t parm_get_mmode(char *p, char idx) {
  p = parm_get_p(p, idx);
  if(startswith(p, "left"))  return Ftduino::LEFT;
  if(startswith(p, "right")) return Ftduino::RIGHT;
  if(startswith(p, "brake")) return Ftduino::BRAKE;
  return Ftduino::OFF;
}

uint8_t parm_get_imode(char *p, char idx) {
  p = parm_get_p(p, idx);
  if(startswith(p, "resistance")) return Ftduino::RESISTANCE;
  if(startswith(p, "voltage")) return Ftduino::VOLTAGE;
  return Ftduino::SWITCH;
}

uint8_t parm_get_omode(char *p, char idx) {
  p = parm_get_p(p, idx);
  if(startswith(p, "high")) return Ftduino::HI;
  if(startswith(p, "low")) return Ftduino::LO;
  return Ftduino::OFF;
}

uint8_t parm_get_cmode(char *p, char idx) {
  p = parm_get_p(p, idx);
  if(startswith(p, "any")) return Ftduino::C_EDGE_ANY;
  if(startswith(p, "rising")) return Ftduino::C_EDGE_RISING;
  if(startswith(p, "falling")) return Ftduino::C_EDGE_FALLING;
  return Ftduino::OFF;
}

void parm_check_failed(char *p) {
  Serial.print("Error, illegal parameter ");
  Serial.print(p);
  Serial.println("!"); 
}

boolean parm_check_bool(char *p, char idx) {
  p = parm_get_p(p, idx);  
  boolean ok = 
    (startswith(p, "on"))    || (startswith(p, "off")) ||
    (startswith(p, "true"))  || (startswith(p, "false")) ||
    (startswith(p, "1"))     || (startswith(p, "0"));

  if(!ok) parm_check_failed(p);    

  return ok;
}

boolean parm_check_int(char *p, char idx, int16_t min, int16_t max) {
  char *q = parm_get_p(p, idx);  
  for(char i=0;q[i];i++) {
    if(!isDigit(q[i])) {
      parm_check_failed(q);
      return false;
    }
  } 
  
  int16_t val = parm_get_int(p, idx);
  if((val < min) || (val > max)) {
    parm_check_failed(q);
    return false;
  }
    
  return true;
}

boolean parm_check_mmode(char *p, char idx) {
  p = parm_get_p(p, idx);  
  boolean ok = 
    (startswith(p, "off")) ||
    (startswith(p, "left")) ||
    (startswith(p, "right")) ||
    (startswith(p, "brake"));

  if(!ok) parm_check_failed(p);    

  return ok;
}

boolean parm_check_omode(char *p, char idx) {
  p = parm_get_p(p, idx);  
  boolean ok = 
    (startswith(p, "high")) ||
    (startswith(p, "low")) ||
    (startswith(p, "off"));

  if(!ok) parm_check_failed(p);    

  return ok;
}

boolean parm_check_cmode(char *p, char idx) {
  p = parm_get_p(p, idx);  
  boolean ok = 
    (startswith(p, "off")) ||
    (startswith(p, "rising")) ||
    (startswith(p, "falling")) ||
    (startswith(p, "any"));

  if(!ok) parm_check_failed(p);    

  return ok;
}

void cmd_display_help(char *) {
  Serial.println("help                   - display this help");
  Serial.println("led state              - switch internal led to state on/off");
  Serial.println("motor port mode speed  - set motor port 1..4 left/right/off/brake with speed in %");
  Serial.println("output port mode value - set output port 1..8 high/low/off with value in %");
  Serial.println("input port [mode]      - read input port 1..4 and configure it as resistance/voltage/switch");
  Serial.println("ultrasonic             - read ultrasonic sensor on counter input C1");
  Serial.println("counter port [mode]    - read counter port 1..4 and configure off/rising/falling/any");
  Serial.println("i2c                    - scan i2c bus");
  Serial.println("<empty command>        - the empty command (just return) switches all outputs off");
}

void cmd_led_set(char *p) {
  // led function is supposed to have only one parameter
  if(!parms_check(p, 1, 1)) return;
  if(!parm_check_bool(p, 0)) return;  // parm 0 is of type bool

  boolean state = parm_get_bool(p, 0);
  digitalWrite(LED_BUILTIN, state?HIGH:LOW);
  Serial.println("ok");  
}

void cmd_motor_set(char *p) {
  // motor function is supposed to have three parameters
  if(!parms_check(p, 2, 3)) return;
  if(!parm_check_int(p, 0, 1, 4))   return;  // parm 0 is of type int 1..4
  int16_t m = parm_get_int(p, 0);
  if(!parm_check_mmode(p, 1))       return;  // parm 1 is of type motor mode
  uint8_t mode = parm_get_mmode(p, 1);
  int16_t value = 0;
  // only mode "off" may have no third parameter
  if((mode != Ftduino::OFF) || (num_parms(p) == 3)) {
    if(!parms_check(p, 3, 3)) return;    
    if(!parm_check_int(p, 2, 0, 100)) return;  // parm 2 is of type int 0..100
    value = parm_get_int(p, 2);
  }
  ftduino.motor_set(Ftduino::M1-1+m, mode, Ftduino::MAX * value / 100);
  Serial.println("ok");  
}

void cmd_output_set(char *p) {
  // output function is supposed to have three parameters
  if(!parms_check(p, 2, 3)) return;
  if(!parm_check_int(p, 0, 1, 8))   return;  // parm 0 is of type int 1..4
  int16_t m = parm_get_int(p, 0);
  if(!parm_check_omode(p, 1))       return;  // parm 1 is of type output mode
  uint8_t mode = parm_get_omode(p, 1);
  int16_t value = 0;
  // only mode "off" may have no third parameter
  if((mode != Ftduino::OFF) || (num_parms(p) == 3)) {
    if(!parms_check(p, 3, 3)) return;    
    if(!parm_check_int(p, 2, 0, 100)) return;  // parm 2 is of type int 0..100
    value = parm_get_int(p, 2);
  }
  ftduino.output_set(Ftduino::O1-1+m, mode, Ftduino::MAX * value / 100);
  Serial.println("ok");  
}

void cmd_input(char *p) {
  // input can have one or two parameters
  if(!parms_check(p, 1, 2))
    return;
    
  if(!parm_check_int(p, 0, 1, 4))   return;  // parm 0 is of type int 1..4
  int16_t i = parm_get_int(p, 0);
  if(num_parms(p) == 2) {
    uint8_t mode = parm_get_imode(p, 1);
    ftduino.input_set_mode(Ftduino::I1-1+i, mode);  
  } 

  uint16_t val = ftduino.input_get(Ftduino::I1-1+i);  
  Serial.println(val, DEC);  
}

void cmd_counter(char *p) {
  // counter can have one or two parameters
  if(!parms_check(p, 1, 2))
    return;
    
  if(!parm_check_int(p, 0, 1, 4))   return;  // parm 0 is of type int 1..4
  int16_t c = parm_get_int(p, 0);
  if(num_parms(p) == 2) {
    uint8_t mode = parm_get_cmode(p, 1);
    ftduino.counter_set_mode(Ftduino::C1-1+c, mode);  
    ftduino.counter_clear(Ftduino::C1-1+c);  
  } 

  uint16_t state = ftduino.counter_get_state(Ftduino::C1-1+c);  
  Serial.print(state, DEC);  
  Serial.print(" ");  
  uint16_t val = ftduino.counter_get(Ftduino::C1-1+c);  
  Serial.println(val, DEC);  
}

void cmd_ultrasonic(char *p) {
  if(!ultrasonic_enabled) {
    ftduino.ultrasonic_enable(Ftduino::ON);
    ultrasonic_enabled = true;
    delay(1000);
  }
  
  int16_t distance = ftduino.ultrasonic_get();
  if(distance < 0) Serial.println("no sensor connected");
  else             Serial.println(distance, DEC);
}

void cmd_i2c(char *p) {
  Serial.print("00: XX ");
  for(uint8_t address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    if(!Wire.endTransmission()) 
      Serial.print(address, HEX);
    else
      Serial.print("--");

    if((address+1) & 15)
      Serial.print(" ");
    else {
      Serial.println();
      Serial.print(address+1, HEX);
      Serial.print(": ");
    }
  }
  Serial.println("XX");
}

struct {
  char *name;
  void (*func)(char *parms);
} commands[] = {
  { "help",        cmd_display_help },
  { "led",         cmd_led_set      },
  { "motor",       cmd_motor_set    },
  { "output",      cmd_output_set   },
  { "input",       cmd_input        },
  { "counter",     cmd_counter      },
  { "ultrasonic",  cmd_ultrasonic   },
  { "i2c",         cmd_i2c          },
  { NULL, NULL }
};

void do_cmd(char *cmd, char *parms) {
  // check for empty command: "all off"
  if(!cmd[0]) {
    Serial.println("Switching all outputs off!");

    for(char i=0;i<8;i++)
      ftduino.output_set(Ftduino::O1+i, Ftduino::OFF, 0);

    return;
  }
  
  for(char i=0;commands[i].name;i++) {
    if(startswith(cmd, commands[i].name)) {      
      commands[i].func(parms);
      return;
    }
  }
  Serial.print("Unknown command: ");
  Serial.println(cmd);
  Serial.println("Enter 'help' for more information");
}

void prompt() {
  Serial.print("> ");  
  Serial.flush();
}

void setup() {
  Serial.begin(115200);
  while(!Serial);

  Wire.begin();

  ftduino.init();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.println("--- ftDuino IoTest ---");
  Serial.println("Enter 'help' for more information");
  prompt();
}

void process_input() {
  static char buffer[32], buffer_used = 0;

  if(Serial.available()) {
    char chr = Serial.read();

    // ignore leading whitespace
    if(!buffer_used && isWhitespace(chr))
      return;

    if(chr == '\n') {
      Serial.println("");      
      
      // terminate buffered string
      buffer[buffer_used++] = '\0';

      // get command
      char *p = buffer;
      while(!isWhitespace(*p) && *p) p++;
      if(*p) { *p = '\0'; p++; }
      else   p = NULL;      
      
      do_cmd(buffer, p);            
      buffer_used = 0;
      
      prompt();
    } else if(buffer_used < sizeof(buffer)-1) {
      Serial.print(chr);
      buffer[buffer_used++] = chr;
    }
  }
}

void loop() {
  process_input();
}
