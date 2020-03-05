/* Eggduino-Firmware by Joachim Cerny, 2014

   Thanks for the nice libs ACCELSTEPPER and SERIALCOMMAND, which made this project much easier.
   Thanks to the Eggbot-Team for such a funny and enjoable concept!
   Thanks to my wife and my daughter for their patience. :-)

 */

// implemented Eggbot-Protocol-Version v13
// EBB-Command-Reference, I sourced from: http://www.schmalzhaus.com/EBB/EBBCommands.html
// no homing sequence, switch-on position of pen will be taken as reference point.
// No collision-detection!!
// Supported Servos: I do not know, I use Arduino Servo Lib with TG9e- standard servo.
// Note: Maximum-Speed in Inkscape is 1000 Steps/s. You could enter more, but then Pythonscript sends nonsense.
// EBB-Coordinates are coming in for 16th-Microstepmode. The Coordinate-Transforms are done in weired integer-math. Be careful, when you diecide to modify settings.

/* TODOs:
   1	collision control via penMin/penMax
   2	implement homing sequence via microswitch or optical device
 */

#include <Wire.h>  // ftDuino uses MiniServo adapter
#include <FtduinoSimple.h>

#define ENABLE_DISPLAY

#ifdef ENABLE_DISPLAY
#include "FtduinoDisplay.h"
FtduinoDisplay display;
#endif

#include "AccelStepper.h" // nice lib from http://www.airspayce.com/mikem/arduino/AccelStepper/
#include "SerialCommand.h" //nice lib from Stefan Rado, https://github.com/kroimon/Arduino-SerialCommand
#include <avr/eeprom.h>
#include "button.h"

#define initSting "EBBv13_and_above Protocol emulated by Eggduino-Firmware V1.6a for ftDuino"
//Rotational Stepper:
#define rotMicrostep 2  //MicrostepMode, only 1,2,4,8,16 allowed, because of Integer-Math in this Sketch
//Pen Stepper:
#define penMicrostep 8 //MicrostepMode, only 1,2,4,8,16 allowed, because of Integer-Math in this Sketch
 
#define servoAddr 0x11 //Servo

// EXTRAFEATURES - UNCOMMENT TO USE THEM -------------------------------------------------------------------

#define prgButton Ftduino::I2 // PRG button
#define penToggleButton Ftduino::I3 // pen up/down button
#define motorsButton Ftduino::I4 // motors enable button

//-----------------------------------------------------------------------------------------------------------

#define penUpPosEEAddress ((uint16_t *)0)
#define penDownPosEEAddress ((uint16_t *)2)

//make Objects
AccelStepper penMotor(AccelStepper::FTDUINOSTEP, Ftduino::M1, Ftduino::M2);
AccelStepper rotMotor(AccelStepper::FTDUINOSTEP, Ftduino::M3, Ftduino::M4);
SerialCommand SCmd;
//create Buttons
#ifdef prgButton
void setprgButtonState();
  Button prgButtonToggle(prgButton, setprgButtonState);
#endif
#ifdef penToggleButton
void doTogglePen();
	Button penToggle(penToggleButton, doTogglePen);
#endif
#ifdef motorsButton
void toggleMotors();
	Button motorsToggle(motorsButton, toggleMotors);
#endif
// Variables... be careful, by messing around here, everything has a reason and crossrelations...
int penMin=0;
int penMax=0;
int penUpPos=5;  //can be overwritten from EBB-Command SC
int penDownPos=20; //can be overwritten from EBB-Command SC
int servoRateUp=0; //from EBB-Protocol not implemented on machine-side
int servoRateDown=0; //from EBB-Protocol not implemented on machine-side
long rotStepError=0;
long penStepError=0;
int penState=penUpPos;
uint32_t nodeCount=0;
unsigned int layer=0;
boolean prgButtonState=0;
uint8_t rotStepCorrection = 16/rotMicrostep ; //devide EBB-Coordinates by this factor to get EGGduino-Steps
uint8_t penStepCorrection = 16/penMicrostep ; //devide EBB-Coordinates by this factor to get EGGduino-Steps
float rotSpeed=0;
float penSpeed=0; // these are local variables for Function SteppermotorMove-Command, but for performance-reasons it will be initialized here
boolean motorsEnabled = 0;
uint32_t lastCmd = 0;

void displayPen(int penPos) {
#ifdef ENABLE_DISPLAY
  char buf[16];
  
  display.fillRect(0, 16, 64, 8, BLACK);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  sprintf(buf, "Pen: %d", penPos);
  display.setCursor(0,  16);
  display.print(buf);

  display.display();
#endif
}

void displayPos(int x, int y) {
#ifdef ENABLE_DISPLAY
  static int cx = 0, cy = 0;
  char buf[16];

  cx += x; cy += y;
  
  display.fillRect(0, 24, 128, 8, BLACK);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  sprintf(buf, "X: %d", cx);  display.setCursor(0,  24);
  display.print(buf);
  sprintf(buf, "Y: %d", cy);  display.setCursor(64,  24);
  display.print(buf);

  display.display();
#endif
}

void setup() {   
	Serial.begin(9600);

#ifdef ENABLE_DISPLAY
  display.begin();
  display.fillRect(0, 0, 128, 32, BLACK);
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(64-6*9,  0);
  display.print("ft-EggBot");  
  display.display();
#endif

	makeComInterface();
	initHardware();

  displayPen(penState);
}

void loop() {
	moveOneStep();

  // check for commands
	if(SCmd.readSerial())
    lastCmd = millis();

  // no new command for 5 seconds?
  if(lastCmd && (millis() - lastCmd > 5000)) {
    lastCmd = 0;
    motorsOff();
  }

#ifdef penToggleButton
	penToggle.check();
#endif

#ifdef motorsButton
	motorsToggle.check();
#endif

#ifdef prgButton
	prgButtonToggle.check();
#endif
}
