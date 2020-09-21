#include <Arduino.h>
#include <inttypes.h>

// A very simple wrapper class that appends a checksum to
// every message sent.
class FtduinoBlue: public Stream {
public:
  FtduinoBlue(Stream &s, const char *l);
  int available();
  int read();
  size_t write(uint8_t b);
  int peek();
  void handle(void);
  void setStateCb(void (*)());
  void setButtonCb(void (*)(char, bool));
  void setSwitchCb(void (*)(char, bool));
  void setSliderCb(void (*)(char, int));
  void setJoystickCb(void (*)(char, char, char));
private:
  char parseHexDigit(char a);
  void parseCommand(char *buffer);     
  int parseParameter(char **idx);

  void (*m_state_cb)();
  void (*m_button_cb)(char, bool);
  void (*m_switch_cb)(char, bool);
  void (*m_slider_cb)(char, int);
  void (*m_joystick_cb)(char, char, char);

  Stream *mStream;
  uint8_t mOutSum = 0;
  char buffer[32];
  uint8_t buffer_fill = 0;
  const char *layout;
};
