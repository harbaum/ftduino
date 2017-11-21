/*
*/

#include "HidGamePad.h"

#define REPORT_ID   3
#define REPORT_SIZE 3

static const uint8_t _hidReportDescriptor[] PROGMEM = {
  0x05, 0x01,                // USAGE_PAGE (Generic Desktop)
  0x09, 0x05,                // USAGE (Gamepad)
  0x85, REPORT_ID,           // REPORT_ID(3)
  0xa1, 0x01,                // COLLECTION (Application)
  0x09, 0x01,                //   USAGE (Pointer)
  0xa1, 0x00,                //   COLLECTION (Physical)
  0x09, 0x30,                //     USAGE (X)
  0x09, 0x31,                //     USAGE (Y)
  0x15, 0x00,                //     LOGICAL_MINIMUM(0)
  0x26, 0xff, 0x00,          //     LOGICAL_MAXIMUM(255)
  0x35, 0x00,                //     PHYSICAL_MINIMUM(0)
  0x46, 0xff, 0x00,          //     PHYSICAL_MAXIMUM(255)
  0x75, 0x08,                //     REPORT_SIZE(8)
  0x95, 0x02,                //     REPORT_COUNT(2)
  0x81, 0x02,                //     INPUT (Data,Var,Abs)
  0xc0,                      //   END_COLLECTION
  0x05, 0x09,                //   USAGE_PAGE (Button)
  0x19, 0x01,                //   USAGE_MINIMUM (Button 1)
  0x29, 0x02,                //   USAGE_MAXIMUM (Button 2)
  0x15, 0x00,                //   LOGICAL_MINIMUM(0)
  0x25, 0x01,                //   LOGICAL_MAXIMUM(1)
  0x95, 0x02,                //   REPORT_COUNT(2)
  0x75, 0x01,                //   REPORT_SIZE(1)
  0x81, 0x02,                //   INPUT (Data,Var,Abs)
  0x95, 0x06,                //   REPORT_COUNT(6)
  0x81, 0x03,                //   INPUT (Const,Var,Abs)
  0xc0                       // END_COLLECTION
};

HidGamePad_::HidGamePad_() {
	// Setup HID report structure
	static HIDSubDescriptor node(_hidReportDescriptor, sizeof(_hidReportDescriptor));
	HID().AppendDescriptor(&node);
	
	// Initalize State
	xAxis = 0;
	yAxis = 0;
	buttons = 0;
}

void HidGamePad_::begin() {
	sendState();
}

void HidGamePad_::end() {
}

void HidGamePad_::setButton(uint8_t button, bool value) {
  if(value) bitSet(buttons, button);
  else      bitClear(buttons, button);
  
  sendState();
}

void HidGamePad_::setXAxis(int8_t value) {
	xAxis = value;
	sendState();
}

void HidGamePad_::setYAxis(int8_t value) {
	yAxis = value;
	sendState();
}

void HidGamePad_::sendState() {
	uint8_t report[REPORT_SIZE];

  // joystick rests in the center with no button pressed
  report[0] = xAxis + 127;
  report[1] = yAxis + 127;
	report[2] = buttons;		

	// HID().SendReport(Report number, array of values in same order as HID descriptor, length)
	HID().SendReport(REPORT_ID, report, REPORT_SIZE);
}

HidGamePad_ HidGamePad;

