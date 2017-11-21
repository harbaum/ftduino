/*
*/

#ifndef HIDGAMEPAD_h
#define HIDGAMEPAD_h

#include <HID.h>

class HidGamePad_ {
public:
	HidGamePad_();

	void begin();
	void end();

	void setXAxis(int8_t value);
	void setYAxis(int8_t value);
	void setButton(uint8_t button, bool value);
	void sendState();
 
private:
  int8_t   xAxis;
  int8_t   yAxis;
  uint8_t  buttons;
};

extern HidGamePad_ HidGamePad;
#endif
