#include <Arduino.h>
#include <inttypes.h>

// A very simple wrapper class that appends a checksum to
// every message sent.
class FtduinoBlue: public Stream {
public:
  static const uint8_t FTDB_STATE    = 0;
  static const uint8_t FTDB_LABEL    = 1;
  static const uint8_t FTDB_BUTTON   = 2;
  static const uint8_t FTDB_SWITCH   = 3;
  static const uint8_t FTDB_JOYSTICK = 4;
  static const uint8_t FTDB_SLIDER   = 5;

  struct reply {
    uint8_t type, id;
    union {
      bool state;
      int slider;
      struct {
        int8_t x, y;
      } joystick;
    };
  } mReply;

  FtduinoBlue(Stream &s, const char *l);
  int available();
  int read();
  size_t write(uint8_t b);
  int peek();
  void handle(void);
  void setCallback(void (*)(struct reply *)); 
private:
  char parseHexDigit(char a);
  void parseCommand(char *buffer);     
  int parseParameter(char **idx);

  void (*m_callback)(struct reply *);

  Stream *mStream;
  uint8_t mOutSum = 0;
  char buffer[32];
  uint8_t buffer_fill = 0;
  const char *layout;
};
