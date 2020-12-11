//
// ArduinoBlue
//
// ftDuino example for ArduinoBlue https://sites.google.com/stonybrook.edu/arduinoble
// This demo needs the ArduinoBlue library. Please install it using the library manager.
//

#include <Ftduino.h>
#include <ArduinoBlue.h>
#include "I2cSerialBt.h"
I2cSerialBt btSerial;

ArduinoBlue phone(btSerial);

int prevThrottle = 49;
int prevSteering = 49;
int throttle, steering, sliderVal, button, sliderId;

void setup() 
{
    ftduino.init();  
    Serial.begin(9600);

    // wait max 1 sec for adapter to appear on bus. This is not
    // needed as begin() will wait for the device. But this way
    // we can use the led as an inidictaor for problems with 
    // the i2c uart adapater
    if(!btSerial.check(1000)) {
      // fast blink with led on failure
      pinMode(LED_BUILTIN, OUTPUT); 
      while(true) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
      }
    }

    btSerial.begin(9600);
    btSerial.key(0);
};

void loop() 
{
    // ID of the button pressed pressed.
    button = phone.getButton();

    // Returns the text data sent from the phone.
    // After it returns the latest data, empty string "" is sent in subsequent.
    // calls until text data is sent again.
    String str = phone.getText();

    // Throttle and steering values go from 0 to 99.
    // When throttle and steering values are at 99/2 = 49, the joystick is at center.
    throttle = phone.getThrottle();
    steering = phone.getSteering();

    // ID of the slider moved.
    sliderId = phone.getSliderId();

    // Slider value goes from 0 to 200.
    sliderVal = phone.getSliderVal();

    // Display button data whenever its pressed.
    if (button != -1) {
        Serial.print("Button: ");
        Serial.println(button);
    }

    // Display slider data when slider moves
    if (sliderId != -1) {
        Serial.print("Slider ID: ");
        Serial.print(sliderId);
        Serial.print("\tValue: ");
        Serial.println(sliderVal);
    }

    // Display throttle and steering data if steering or throttle value is changed
    if (prevThrottle != throttle || prevSteering != steering) {
        Serial.print("Throttle: "); Serial.print(throttle); Serial.print("\tSteering: "); Serial.println(steering);
        prevThrottle = throttle;
        prevSteering = steering;

        // use throttle to control M1
        if(throttle >= 50)  // 50 ... 99
          ftduino.motor_set(Ftduino::M1, Ftduino::LEFT, Ftduino::MAX * (throttle-50) / 49);
        else if(throttle <= 48) // 0 ... 48
          ftduino.motor_set(Ftduino::M1, Ftduino::RIGHT, Ftduino::MAX * (48-throttle) / 48);
        else // 49
          ftduino.motor_set(Ftduino::M1, Ftduino::OFF, Ftduino::MAX);
    }

    // If a text from the phone was sent print it to the serial monitor
    if (str != "") {
        Serial.println(str);
    }

    // Send string from serial command line to the phone. This will alert the user.
    if (Serial.available()) {
        Serial.write("send: ");
        String str = Serial.readString();
        phone.sendMessage(str); // phone.sendMessage(str) sends the text to the phone.
        Serial.print(str);
        Serial.write('\n');
    }
};
