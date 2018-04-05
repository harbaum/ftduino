// I2cSlave.ino
// ftDuino als passiver Teilnehmer am I2C-Bus
// Version 1.1: 
// - alle Ausgangsfähigkeiten unterstützt

#include <Ftduino.h>
#include <Wire.h>

// Puffer, um den Zustand der Ausgänge zu speichern
unsigned char output_state[4][2];

void setup() {
  Wire.begin(42);               // tritt I2C-Bus an Adresse #42 als "Slave" bei
  Wire.onReceive(receiveEvent); // Auf Schreib-Ereignisse registrieren
  Wire.onRequest(requestEvent); // Auf Lese-Ereignisse registrieren

  ftduino.init();

  // alle Ausgänge sind hochohmig
  memset(output_state, 0, sizeof(output_state));
}

void loop() {
  delay(100);
}

int addr = 0;

// Funktion, die immer dann aufgerufen wird, wenn vom Master
// Daten üner I2C gesehndet werden. Diese Funktion wurde in
// setup() registriert.
void receiveEvent(int num) {
  // es muss mindestens ein Byte empfangen worden sein
  if(Wire.available()) {
    // erstes Byte ist die Register-Adresse
    addr = Wire.read();

    // alle weiteren Bytes sind Datenbytes
    while(Wire.available()) {
      unsigned char value = Wire.read();

      // Adressen 0x00 bis 0x0f schalten und kontrollieren
      // die Ausgänge
      if((addr >= 0)&&(addr <= 0x0f)) {
        unsigned char port = addr >> 1;
        unsigned char reg = addr & 1;

        // Wert in internem Puffer speichern
        output_state[port][reg] = value;

        // Hardware entsprechend konfigurieren, wenn ein Ausgangswert-
        // Register geschrieben wird
        if(reg == 1) {
          // Ausgänge O2, O4, O6 und O8 werden nnur gesetzt, wenn 
          // die zugehörigen Ausgänge O1, O3, O5 oder O7 nicht als
          // Motorausgang konfiguriert sind
          if(((port&1) == 0) || 
            ((port&1) == 1) && ((output_state[port-1][0] & 1) != 1)) {
              // skaliere 0...255 nach 0..Ftduino::MAX
              int pwm = (value * Ftduino::MAX)/255;
              int mode;

              // Modus des Ausgangs
              switch( output_state[port][0] ) {
                case 0x00:  mode = Ftduino::OFF;   break;
                case 0x01:  mode = Ftduino::HI;    break;
                case 0x02:  mode = Ftduino::LO;    break;
                case 0x10:  mode = Ftduino::OFF;   break;
                case 0x11:  mode = Ftduino::BRAKE; break;
                case 0x12:  mode = Ftduino::LEFT;  break;
                case 0x13:  mode = Ftduino::RIGHT; break;
                default:    mode = Ftduino::OFF;
              }
        
              if( output_state[port][0] & 0x10)   
                ftduino.motor_set(Ftduino::M1+port, mode, pwm);
              else
                ftduino.output_set(Ftduino::O1+port, mode, pwm);
          }
        }
      }
      
      // Die geraden Adressen 0x10 bis 0x1f konfigurireen die Eingänge
      if((addr >= 0x10)&&(addr <= 0x1f) && (!(addr & 1))) {
        unsigned char port = (addr - 0x10) >> 1;
        int mode;

        switch(value) {
          case 0x00: mode = Ftduino::VOLTAGE;    break;
          case 0x01: mode = Ftduino::RESISTANCE; break;
          case 0x02: mode = Ftduino::SWITCH;     break;
          default:   mode = Ftduino::VOLTAGE;          
        }
        
        ftduino.input_set_mode(Ftduino::I1+port, mode);
      }
      
      addr++;  // nächste Adresse
    }
  }
}

void requestEvent() {
  // Eingangswerte lesen. Man kann maximal einen Eingang zur Zeit
  // lesen
  if((addr >= 0x10) && (addr <= 0x1f)) {
    unsigned short val = ftduino.input_get((addr - 0x10)>>1);
    if(!(addr & 1)) Wire.write(val & 0xff);
    Wire.write(val >> 8);
  }
}

