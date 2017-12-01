/*
 * Author: Paulo Costa
 * Modified for ftDuino by Till Harbaum
 * 
 * Benutzng unter Linux:
 * $ aplaymidi -l
 *   Port    Client name                      Port name
 *   14:0    Midi Through                     Midi Through Port-0
 *   24:0    ftDuino                          ftDuino MIDI 1
 * 
 * $ aplaymidi -p 24:0 silent_night.mid
 */ 

// Das Einbinden der Bibliothek sorgt dafür, dass die Ausgänge initialisiert werden.
// Damit wird Ausgang O2 über Arduino Pin 11 nutzbar und kann zum Anschluss eines 
// Lautsprechers genutzt werden
#include <FtduinoSimple.h>

#include <MIDIUSB.h>
#include "pitchToFrequency.h"

#define BUZZ_PIN 11

const char* pitch_name(byte pitch) {
  static const char* names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  return names[pitch % 12];
}

int pitch_octave(byte pitch) {
  return (pitch / 12) - 1;
}

void noteOn(byte channel, byte pitch, byte velocity) {
  tone(BUZZ_PIN, pitchFrequency[pitch]);

  Serial.print("Note On: ");
  Serial.print(pitch_name(pitch));
  Serial.print(pitch_octave(pitch));
  Serial.print(", channel=");
  Serial.print(channel);
  Serial.print(", velocity=");
  Serial.println(velocity);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  noTone(BUZZ_PIN);

  Serial.print("Note Off: ");
  Serial.print(pitch_name(pitch));
  Serial.print(pitch_octave(pitch));
  Serial.print(", channel=");
  Serial.print(channel);
  Serial.print(", velocity=");
  Serial.println(velocity);
}

void controlChange(byte channel, byte control, byte value) {
  Serial.print("Control change: control=");
  Serial.print(control);
  Serial.print(", value=");
  Serial.print(value);
  Serial.print(", channel=");
  Serial.println(channel);
}

void setup() {
  Serial.begin(115200);
}

void loop() {
  midiEventPacket_t rx = MidiUSB.read();
  switch (rx.header) {
    case 0:
      break; //No pending events
      
    case 0x9:
      noteOn(
        rx.byte1 & 0xF,  //channel
        rx.byte2,        //pitch
        rx.byte3         //velocity
      );
      break;
      
    case 0x8:
      noteOff(
        rx.byte1 & 0xF,  //channel
        rx.byte2,        //pitch
        rx.byte3         //velocity
      );
      break;
      
    case 0xB:
      controlChange(
        rx.byte1 & 0xF,  //channel
        rx.byte2,        //control
        rx.byte3         //value
      );
      break;
      
    default:
      Serial.print("Unhandled MIDI message: ");
      Serial.print(rx.header, HEX);
      Serial.print("-");
      Serial.print(rx.byte1, HEX);
      Serial.print("-");
      Serial.print(rx.byte2, HEX);
      Serial.print("-");
      Serial.println(rx.byte3, HEX);
  }
}

