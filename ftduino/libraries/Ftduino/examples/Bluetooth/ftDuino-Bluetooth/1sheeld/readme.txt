1Sheeld for ftDuino
-------------------

This is a 1Sheeld demo sketch for the ftDuino and the ftDuino
bluetooth adapter. It's supposed to be used in conjunction with the
1Sheeld smartphone app. This app is available for Android and
IOS. Only the Android version has been tested with this sketch.

This sketch requires the OneSheeld library to be installed in the
Arduino IDE. You can install it using the library manager.

This sketch makes use of the "Keypad" and "7 Segement" features if 1Sheeld.
The sketch allows to control the outputs O1 to O8 of the ftDuino using
the keypad keys 1 to 8. And the 7 segement is used as a counter by the
ftDuino.

1Sheeld will only detect the ftDuino if the Bluetooth name starts with
"1Sheeld". Use e.g. the ftDuino bluetooth_config sketch to set the name
to 1Sheeld like so:

-=- ftDuino bluetooth configuration -=-
For more information see http://ftduino.de
I²C uart bridge found.
Checking for bluetooth module at 9600 baud ... OK
Checking for baud value ... OK, found HM-11
Type 'HELP' for more info.
CMD> name 1Sheeld
Name changed to: '1Sheeld'
CMD>

Afterwards the module will be detected by the 1Sheeld app.
