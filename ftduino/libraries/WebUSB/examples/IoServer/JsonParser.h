#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <stdint.h>

class JsonParser
{
  public:
    JsonParser();
    int parse(char c);
    void reset(void);
      
 private:
    void reply_error(char id);
    void reply_value(char *port, bool b, uint16_t v);

    bool isWhite(char c);
    bool isDigit(char c);
    bool isChar(char c);
    void check_name(void);
    void check_value(void);
    void cmd_reset(void);
    void cmd_complete(void);
    void lowercase_str(void);
    bool startsWith(const char *pre, const char *str);

    // the "state" is a high level state
    enum { IDLE, NAME, SEP, VALUE, ARRAY, CONT, ACONT} state;
    enum { NONE, ENDSTR, ENDNUM, ENDBOOL, STRING, NUM, BOOL } substate;
    struct value {
      enum { TYPE_STR, TYPE_NUM, TYPE_BOOL } type;
      union { char str[8]; uint16_t num; char bin; } value;
    } substate_value; 
    uint8_t depth; 

    // ftduino specific state information
    enum { CMD_NONE, CMD_SET, CMD_GET } cmd;
    enum mode_e { MODE_NONE,
	   MODE_U, MODE_R, MODE_SW,            // I1-I8 input modes
	   MODE_HI, MODE_LO, MODE_OPEN,        // O1-O8 output modes
	   MODE_LEFT, MODE_RIGHT, MODE_BRAKE,  // M1-M4 motor modes
    } mode;
    enum { PARM_NONE,
	   PARM_PORT, PARM_VALUE, PARM_MODE, PARM_TYPE,
    } parm;
    enum { REQ_NONE,
	   REQ_DEVS, REQ_VER
    } req;
    enum { TYPE_NONE, TYPE_STATE, TYPE_COUNTER,
    } type;
    struct port {
      enum { PORT_NONE, PORT_I, PORT_C, PORT_O, PORT_M, PORT_LED } type:3;
      uint8_t index:5;
    } port;
    enum value_type_e { VALUE_TYPE_UNKNOWN, VALUE_TYPE_NUM, VALUE_TYPE_BOOL };
    struct {
      bool valid;
      value_type_e type;
      union {
	char b;
	uint16_t v;
      };
    } value;

    struct mode_map_S { mode_e mode; uint8_t ftd_mode; };
    static const struct mode_map_S o_mode_map[], i_mode_map[], m_mode_map[];
    uint8_t getFtdMode(mode_e mode, const struct mode_map_S *m);
};
    

#endif // JSONPARSER_H
