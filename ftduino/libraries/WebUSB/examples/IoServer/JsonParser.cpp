// this parser is highly specialized for two reasons:
// a) this saves memory and computing power
// b) it can directly react while parsing

// https://www.json.org/json-de.html

#ifdef ARDUINO
#include <Ftduino.h>
#else
#include <stdio.h>
#include <stdlib.h>
#endif

#include <string.h>
#include "JsonParser.h"

#if USB_VERSION == 0x210
#include <WebUSB.h>
extern WebUSB WebUSBSerial;
#define Serial WebUSBSerial
#endif

// generic JSON error codes
#define ERR_UNEXP_STATE 1  // unexpected state
#define ERR_EXP_OBJ     2  // expecting object
#define ERR_EXP_STR     3  // expecting string
#define ERR_EXP_SEP     4  // expecting ':'
#define ERR_UNEXP_SUB   5  // unexpected substate
#define ERR_UNEXP_ESTR  6  // unexpected state at end of string
#define ERR_UNEXP_CONT  7  // unexpected continuation
#define ERR_UNEXP_VAL   8  // unexpected value
#define ERR_UNEXP_ACONT 9  // unexpected array continuation

// ftDuino specific errors
#define ERR_UNK_CMD     10  // unknown command
#define ERR_UNK_PARM    11  // unknown parameter
#define ERR_WRONG_VTYPE 12  // wrong value type
#define ERR_ILL_PORT    13  // illegal port specification
#define ERR_ILL_MODE    14  // illegal mode specification
#define ERR_INV_MODE    15  // mode invalid for port
#define ERR_INT         16  // internal error
#define ERR_ILL_REQ     17  // illegal get request

void JsonParser::reply_error(char id) {
#ifdef ARDUINO
  Serial.print("{ \"error\": ");
  Serial.print(id, DEC);
  Serial.print(" }");
  Serial.flush();
#else
  printf("error: %d\n", id);
  exit(-1);
#endif
}

#ifdef ARDUINO
void JsonParser::reply_value(char *port, bool b, uint16_t v) {
  Serial.print("{ \"port\": \"");
  Serial.print(port);
  Serial.print("\", \"value\": ");
  if(b) {
    if(v) Serial.print("true");
    else  Serial.print("false");
  } else {
    Serial.print("\"");
    Serial.print(v, DEC);
    Serial.print("\"");
  }
  Serial.print("}");
  Serial.flush();
}
#endif

JsonParser::JsonParser() {
  reset();
}

void JsonParser::cmd_reset(void) {
  cmd = CMD_NONE;
  mode = MODE_NONE;
  parm = PARM_NONE;
  req = REQ_NONE;
  port.type = port::PORT_NONE;
  value.valid = false;
}

void JsonParser::reset(void) {
  // JSON parser state
  state = IDLE;
  substate = NONE;
  depth = 0;

  // ftduino specific state
  cmd_reset();

#ifdef ARDUINO
  // reset all ftduino ports
  digitalWrite(LED_BUILTIN, LOW);
  for(uint8_t i=0;i<8;i++)
    ftduino.output_set(Ftduino::O1+i, Ftduino::OFF, 0);
#endif  
}

bool JsonParser::isWhite(char c) {
  return(c == ' ' || c == '\n' || c == '\r' || c == '\t'); 
}

bool JsonParser::isDigit(char c) {
  return(c >= '0' && c <= '9'); 
}

bool JsonParser::isChar(char c) {
  return((c >= 'A' && c <= 'Z') ||
	 (c >= 'a' && c <= 'z'));
}

bool JsonParser::startsWith(const char *pre, const char *str) {
  size_t lenpre = strlen(pre), lenstr = strlen(str);
  return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

void JsonParser::check_name(void) {
  lowercase_str();  // all names are case insensitive
  
  // on depth 0 only commands are received
  if(depth == 0) {
    // printf("COMMAND: '%s'\n", substate_value.value.str);
    if(strcmp(substate_value.value.str, "set") == 0)
      cmd = CMD_SET;
    else if(strcmp(substate_value.value.str, "get") == 0)
      cmd = CMD_GET;
    else 
      reply_error(ERR_UNK_CMD);
  }

  // parse parameters on level 1
  if(depth == 1) {
    // printf("PARAMETER: '%s'\n", substate_value.value.str);
    
    if(strcmp(substate_value.value.str, "port") == 0)
      parm = PARM_PORT;
    else if(strcmp(substate_value.value.str, "value") == 0)
      parm = PARM_VALUE;
    else if(strcmp(substate_value.value.str, "mode") == 0)
      parm = PARM_MODE;
    else
      reply_error(ERR_UNK_PARM);   
  }
}

void JsonParser::lowercase_str(void) {
  if(substate_value.type == value::TYPE_STR)
    for(char i=0;i<sizeof(substate_value.value.str);i++)
      if((substate_value.value.str[i] >= 'A') &&
	 (substate_value.value.str[i] <= 'Z'))
	substate_value.value.str[i] =
	  substate_value.value.str[i] - ('A' - 'a');   
}

void JsonParser::check_value(void) {
  lowercase_str();  // value string are all case insensitive

  if(cmd == CMD_GET) {
    if((depth == 1) && (parm == PARM_PORT)) {
      port.type = port::PORT_NONE;
      if(substate_value.type == value::TYPE_STR) {
	
	// check for inputs 1-8 or counter 1-4
	if(strlen(substate_value.value.str) == 2) {
	  if((substate_value.value.str[0] == 'i') &&
	     (port.index >= 0) && (port.index <= 7)) 
	    port.type = port::PORT_I;
	  else if((substate_value.value.str[0] == 'c') &&
		  (port.index >= 0) && (port.index <= 3)) 
	    port.type = port::PORT_C;
	  else
	    reply_error(ERR_ILL_PORT);
	} else
	  reply_error(ERR_WRONG_VTYPE);
      } else
	reply_error(ERR_WRONG_VTYPE);     
    }
      
    // the request for "devices" comes as a value
    if(depth == 0) {
      // printf("GET %s\n", substate_value.value.str);
      if(substate_value.type == value::TYPE_STR) {
      	if(startsWith(substate_value.value.str, "devices"))
	        req = REQ_DEVS;
        else if(startsWith(substate_value.value.str, "version"))
          req = REQ_VER;
      	else
	        reply_error(ERR_ILL_REQ);
      } else
      	reply_error(ERR_WRONG_VTYPE);
    }
  }
    
  if(cmd == CMD_SET) {
    // parse with respect to the parameter name
    // all set parameters are within sub-objects at depth 1
    if(depth == 1) {
      if(parm == PARM_PORT) {
      	port.type = port::PORT_NONE;
	
      	// port is a string begining with i, o, c or m
      	if(substate_value.type == value::TYPE_STR) {
          if(startsWith(substate_value.value.str, "led"))
            port.type = port::PORT_LED;
          else {      
	          port.index = substate_value.value.str[1] -'0' - 1;
	          // check for inputs or outputs 1-8, counters ...
	          if(strlen(substate_value.value.str) == 2) {
	            if((substate_value.value.str[0] == 'i') &&
	              (port.index >= 0) && (port.index <= 7)) 
	              port.type = port::PORT_I;
	            else if((substate_value.value.str[0] == 'o') &&
		            (port.index >= 0) && (port.index <= 7)) 
	              port.type = port::PORT_O;
	            else if((substate_value.value.str[0] == 'c') &&
		            (port.index >= 0) && (port.index <= 3)) 
	              port.type = port::PORT_C;
	            else if((substate_value.value.str[0] == 'm') &&
		            (port.index >= 0) && (port.index <= 3)) 
	              port.type = port::PORT_M;
	            else
      	        reply_error(ERR_ILL_PORT);
  	        } else
	            reply_error(ERR_ILL_PORT);
          }
	      } else
	        reply_error(ERR_WRONG_VTYPE);
	
      } else if(parm == PARM_MODE) {
      	if(substate_value.type == value::TYPE_STR) {
	  
	        if(startsWith(substate_value.value.str, "high"))
	          mode = MODE_HI;
      	  else if(startsWith(substate_value.value.str, "low"))
	          mode = MODE_LO;
	        else if(startsWith(substate_value.value.str, "open") ||
                  startsWith(substate_value.value.str, "off"))
	          mode = MODE_OPEN;
	        else if(startsWith(substate_value.value.str, "voltage"))
	          mode = MODE_U;
	        else if(startsWith(substate_value.value.str, "resistance"))
	          mode = MODE_R;
	        else if(startsWith(substate_value.value.str, "switch"))
	          mode = MODE_SW;
	        else if(startsWith(substate_value.value.str, "left"))
	          mode = MODE_LEFT;
	        else if(startsWith(substate_value.value.str, "right"))
	          mode = MODE_RIGHT;
	        else if(startsWith(substate_value.value.str, "brake"))
	          mode = MODE_BRAKE;
	        else
	          reply_error(ERR_ILL_MODE);
      	} else
	        reply_error(ERR_WRONG_VTYPE);

	    } else if(parm == PARM_VALUE) {
        if(substate_value.type == value::TYPE_NUM) {
          value.valid = true;
          value.type = VALUE_TYPE_NUM;
	        value.v = substate_value.value.num;
        } else if(substate_value.type == value::TYPE_BOOL) {
	        value.valid = true;
                value.type = VALUE_TYPE_BOOL;
                value.b = substate_value.value.bin;
	      } else {
          value.type = VALUE_TYPE_UNKNOWN;
	        reply_error(ERR_WRONG_VTYPE);
	      }
      }
    }
  }
}

const struct JsonParser::mode_map_S JsonParser::o_mode_map[] = {
#ifdef ARDUINO
  { MODE_HI, Ftduino::HI }, { MODE_LO, Ftduino::LO },
  { MODE_OPEN, Ftduino::OFF }, { MODE_NONE, 0 }
#else
  { MODE_HI, 12 }, { MODE_LO, 13 }, { MODE_OPEN, 14 }, { MODE_NONE, 0 }
#endif
};

const struct JsonParser::mode_map_S JsonParser::i_mode_map[] = {
#ifdef ARDUINO
  { MODE_U, Ftduino::VOLTAGE }, { MODE_R, Ftduino::RESISTANCE },
  { MODE_SW, Ftduino::SWITCH }, { MODE_NONE, 0 }
#else
  { MODE_U, 12 }, { MODE_R, 13 }, { MODE_SW, 14 }, { MODE_NONE, 0 }
#endif
};

const struct JsonParser::mode_map_S JsonParser::m_mode_map[] = {
#ifdef ARDUINO
  { MODE_LEFT, Ftduino::LEFT }, { MODE_RIGHT, Ftduino::RIGHT },
  { MODE_BRAKE, Ftduino::BRAKE }, { MODE_OPEN, Ftduino::OFF }, { MODE_NONE, 0 }
#else
  { MODE_LEFT, 1 }, { MODE_RIGHT, 2 }, { MODE_BRAKE, 3 },
  { MODE_OPEN, 4 }, { MODE_NONE, 0 }
#endif
};

uint8_t JsonParser::getFtdMode(mode_e mode, const struct mode_map_S *m) {
  for(char i=0;m[i].mode != MODE_NONE;i++)
    if(m[i].mode == mode)
      return m[i].ftd_mode;

  return 255;
}

void JsonParser::cmd_complete(void) {
#ifdef ARDUINO
  // keep track of output modes ...
  static uint8_t output_mode[8] = {
    Ftduino::OFF, Ftduino::OFF, Ftduino::OFF, Ftduino::OFF, 
    Ftduino::OFF, Ftduino::OFF, Ftduino::OFF, Ftduino::OFF
  };
  // ... and motor modes ...
  static uint8_t motor_mode[4] = {
    Ftduino::OFF, Ftduino::OFF, Ftduino::OFF, Ftduino::OFF
  };
  // ... and input modes
  static uint8_t input_mode[8] = {
    Ftduino::RESISTANCE, Ftduino::RESISTANCE, Ftduino::RESISTANCE, Ftduino::RESISTANCE,
    Ftduino::RESISTANCE, Ftduino::RESISTANCE, Ftduino::RESISTANCE, Ftduino::RESISTANCE
  };
#endif
  
  switch(cmd) {
  case CMD_GET:
    // in some cases a single string is enough to specify
    // what's to be returned. In some cases an object is
    // required to describe exactly what's requested
    if(port.type == port::PORT_I) {
#ifdef ARDUINO
      uint16_t v = ftduino.input_get(Ftduino::I1 + port.index);
      char port_name[3] = { 'I', (char)('1' + port.index), 0 };
      reply_value(port_name, input_mode[port.index] == Ftduino::SWITCH, v);
#else
      printf("get input port %d\n", port.index);
#endif      
    }
    
    if(req == REQ_DEVS) {
#ifdef ARDUINO
      // currently only one master is supported
      Serial.print("{ \"devices\": "
       "{ \"name\": \"ftDuino\", \"id\": 0, \"io\": true }"
       "}" );
      Serial.flush();
#else
      // currently only one master is supported
      printf("{ \"devices\": "
	     "{ \"name\": \"ftDuino\", \"id\": 0, \"io\": true }"
	     "}" );
#endif
    }
    
    if(req == REQ_VER) {
#ifdef ARDUINO
      // currently only one master is supported
      Serial.print("{ \"version\": \"0.9.0\" }");
      Serial.flush();
#else
      printf("Version request\n");
#endif
    }
    
    break;
    
  case CMD_SET:
    switch(port.type) {

      case port::PORT_LED:
        if(value.valid) {
	        // a truth value translates into 0 and 1
	        if(value.type == VALUE_TYPE_BOOL)
	          value.v = value.b?1:0;
	  
#ifdef ARDUINO
          digitalWrite(LED_BUILTIN, value.v?HIGH:LOW);
#else
      	  printf("LED: %s\n", value.v?"on":"off");
#endif
      	}
        break;
    
      // act according to output set commands
    case port::PORT_O:
      if(mode != MODE_NONE) {
      	uint8_t ftdm = getFtdMode(mode, o_mode_map);
	      if(ftdm != 255) {
#ifdef ARDUINO
          output_mode[port.index] = ftdm;

	  // this invalidates the motor mode of the combined port
	  motor_mode[port.index>>1] = Ftduino::OFF;
	  
          // the user may not have specified a value when
          // asking the port to be switched off. Thus this case
          // needs to be handled here
          if(!value.valid && (ftdm == Ftduino::OFF))
            ftduino.output_set(Ftduino::O1+port.index, ftdm, 0);
#else
          printf("set O%d mode %d\n", port.index, ftdm);
#endif
	      } else
	        reply_error(ERR_INV_MODE);
      }
	
      if(value.valid) {
#ifdef ARDUINO
	      if(value.type == VALUE_TYPE_BOOL)
	        value.v = value.b?Ftduino::MAX:0;
         
	      ftduino.output_set(Ftduino::O1+port.index, output_mode[port.index], value.v);
#else
	      printf("set O%d = %d\n", port.index, value.v);
#endif
      }
      break;

      // act according to input set commands
    case port::PORT_I:
      if(mode != MODE_NONE) {
      	uint8_t ftdm = getFtdMode(mode, i_mode_map);
	      if(ftdm != 255) {
#ifdef ARDUINO
		input_mode[port.index] = ftdm;
		ftduino.input_set_mode(Ftduino::I1+port.index, ftdm);
#else
	        printf("set I%d mode %d\n", port.index, ftdm);
#endif
	      } else
	        reply_error(ERR_INV_MODE);
      }
      break;

      // act according to output set commands
    case port::PORT_M:
      if(mode != MODE_NONE) {
      	uint8_t ftdm = getFtdMode(mode, m_mode_map);
	      if(ftdm != 255) {
#ifdef ARDUINO
          motor_mode[port.index] = ftdm;

	  // this invalidates the output mode of the related ports
	  output_mode[2*port.index+0] = Ftduino::OFF;
	  output_mode[2*port.index+1] = Ftduino::OFF;
	  
          // the user may not have specified a value when
          // asking the port to be switched off. Thus this case
          // needs to be handled here
          if(!value.valid && (ftdm == Ftduino::OFF))
            ftduino.motor_set(Ftduino::M1+port.index, ftdm, 0);
#else
          printf("set M%d mode %d\n", port.index, ftdm);
#endif
	      } else
	        reply_error(ERR_INV_MODE);
      }
	
      if(value.valid) {
#ifdef ARDUINO
	      if(value.type == VALUE_TYPE_BOOL)
	        value.v = value.b?Ftduino::MAX:0;
         
	      ftduino.motor_set(Ftduino::M1+port.index, motor_mode[port.index], value.v);
#else
	      printf("set M%d = %d\n", port.index, value.v);
#endif
      }
      break;
      
    default:
      break;
    }
    
    break;

  default:
    reply_error(ERR_INT);
    break;
  }

  cmd_reset();
}
  
int JsonParser::parse(char c) {
#ifdef ARDUINOx
  digitalWrite(LED_BUILTIN, HI);
  delay(10);
  digitalWrite(LED_BUILTIN, LOW);
#endif
  
  // unless we are parsing a string, white spaces are ignored
  if((substate != STRING) && isWhite(c))
    return 0;

  // printf("(%d:%d,%d) RX: %c (%d)\n", depth, state, substate, c, c);

  // a null-byte or ESC resets the parser
  if(!c || (c == 0x1b)) {
    reset();
    return 0;
  }
  
  if(substate != NONE) {
    switch(substate) {

    case NUM:
      if(!isDigit(c))    // done parsing numeric
	substate = ENDNUM;
      else
	substate_value.value.num =
	  10 * substate_value.value.num + (c-'0');
      break;
      
    case BOOL:
      // first char must have been 't' or 'f'. We just
      // ignore the rest ...
      if(!isChar(c))
	substate = ENDBOOL;
      break;
      
    case STRING:
      // parse the entire string until the "
      
      // TODO: handle escaped chars
      
      if(c == '"') {
	substate = ENDSTR;  // done parsing string
	c = 0;              // " doesn't need further processing
      } else {
	// search end of current string
	int i=0;
	while(substate_value.value.str[i]) i++;
	if(i < sizeof(substate_value.value.str)-1) {
	  substate_value.value.str[i] = c;
	  substate_value.value.str[i+1] = 0;
	}
      }
      break;
      
    default:
      reply_error(ERR_UNEXP_SUB);     
    }

    // substate has ended, advance main state
    if((substate == ENDSTR) || (substate == ENDNUM) || (substate == ENDBOOL)) {
      if(substate == ENDSTR)      substate_value.type = value::TYPE_STR;
      else if(substate == ENDNUM) substate_value.type = value::TYPE_NUM;
      else                        substate_value.type = value::TYPE_BOOL;
      
      // value parsing has ended, call processing
      if(state == NAME)  check_name();
      if(state == VALUE) check_value();
      
      // end of string: main state advances
      switch(state) {
      case NAME:    state = SEP;   break;
      case VALUE:   state = CONT;  break;
      case ARRAY:   state = ACONT; break;
      default:      reply_error(ERR_UNEXP_ESTR);
      }      

      substate = NONE;      
    }
  }

  if(substate == NONE && c) {
    switch(state) {
    case IDLE:
      // in IDLE state only the { is allowed
      if(c == '{')
	state = NAME;          // search for object name
      else reply_error(ERR_EXP_OBJ);
      break;

    case NAME:
      if(c == '}') { // an object may be empty
	cmd_reset();
	state = IDLE;
      } else if(c == '"') {
	substate = STRING;
	substate_value.value.str[0] = 0;
      } else reply_error(ERR_EXP_STR);
      break;
      
    case SEP:
      // object name and value are sepeated by ':'
      if(c == ':') state = VALUE;
      else         reply_error(ERR_EXP_SEP);
      break;
      
    case VALUE:
    case ARRAY:
      // value can be
      // a numerical value
      // a string
      // or an array
      // null, true and false are not suppoted (yet)
      if(c == '"') {
	substate = STRING;
	substate_value.value.str[0] = 0;
      } else if(isDigit(c)) {
	substate = NUM;
	substate_value.value.num = c-'0';
      } else if((c == 't') || (c == 'T') || (c == 'f') || (c == 'F')) {
	substate = BOOL;
	substate_value.value.bin = (c == 't') || (c == 'T');
      } else if((c == '[') && (state == VALUE)) {
	// an array must not contain further arrays in this
	// implementation
	state = ARRAY;
      } else if(c == '{') {
	depth++;
	state = NAME;
      } else reply_error(ERR_UNEXP_VAL);
      break;

    case CONT:
      // an object either ends with a } or continues
      // with a ,
      if(c == '}') {
	if(depth) {
	  // object ended, but not on root level,
	  // continue parsing one level below
	  depth--;
	  state = CONT;
	} else {
	  // parsing ended on root level. Try to
	  // act according to the parsed command
	  cmd_complete();
	  state = IDLE;
	}
      } else if(c == ',')
	state = NAME;             // search for object name
      else
	reply_error(ERR_UNEXP_CONT);
      break;
      
    case ACONT:
      // an array either ends with a } or continues
      // with a ,
      if(c == ']') state = CONT;
      else if(c == ',')
	state = ARRAY;
      else
	reply_error(ERR_UNEXP_ACONT);
      break;
      
    default:
      reply_error(ERR_UNEXP_STATE);
    }
  }
  
  return 0;
}
