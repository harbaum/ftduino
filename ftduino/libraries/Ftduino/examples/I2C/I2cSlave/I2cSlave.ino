// I2cSlave.ino
// ftDuino als passiver Teilnehmer am I2C-Bus

#include <Ftduino.h>
#include <Wire.h>

// Puffer, um den Zustand der Ausgänge zu speichern
unsigned char output_mode[8];
boolean c1_ultrasonic = false;
int addr = 0;
unsigned char led_counter = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // LED initialisieren

  Wire.begin(43);               // tritt I2C-Bus an Adresse #43 als "Slave" bei
  Wire.onReceive(receiveEvent); // Auf Schreib-Ereignisse registrieren
  Wire.onRequest(requestEvent); // Auf Lese-Ereignisse registrieren

  ftduino.init();

  // alle Ausgänge sind hochohmig
  memset(output_mode, 0, sizeof(output_mode));
}

void loop() {
  /* interne LED am Ende des Transfers abschalten */
  if(led_counter) 
    led_counter--;
  else
    digitalWrite(LED_BUILTIN, LOW);    
  
  delay(10);
}

// Funktion, die immer dann aufgerufen wird, wenn vom Master
// Daten üner I2C gesehndet werden. Diese Funktion wurde in
// setup() registriert.
void receiveEvent(int num) {
  /* interne LED zu Beginn des Transfers einschalten */
  digitalWrite(LED_BUILTIN, HIGH);
  led_counter = 10;

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

        // Modus speichern
        if(reg == 0) 
          output_mode[port] = value;

        // Hardware entsprechend konfigurieren, wenn ein Ausgangswert-
        // Register geschrieben wird
        if(reg == 1) {
          // Ausgänge O2, O4, O6 und O8 werden nnur gesetzt, wenn 
          // die zugehörigen Ausgänge O1, O3, O5 oder O7 nicht als
          // Motorausgang konfiguriert sind
          if(((port&1) == 0) || 
             ((port&1) == 1) && ((output_mode[port-1] & 0x10) != 0x10)) {
              // skaliere 0...255 nach 0..Ftduino::MAX
              int pwm = (value * Ftduino::MAX)/255;
              int mode;

              // Modus des Ausgangs
              switch( output_mode[port] ) {
                case 0x00:  mode = Ftduino::OFF;   break;
                case 0x01:  mode = Ftduino::HI;    break;
                case 0x02:  mode = Ftduino::LO;    break;
                case 0x10:  mode = Ftduino::OFF;   break;
                case 0x11:  mode = Ftduino::BRAKE; break;
                case 0x12:  mode = Ftduino::LEFT;  break;
                case 0x13:  mode = Ftduino::RIGHT; break;
                default:    mode = Ftduino::OFF;
              }
        
              if( output_mode[port] & 0x10)   
                ftduino.motor_set(Ftduino::M1+(port>>1), mode, pwm);
              else
                ftduino.output_set(Ftduino::O1+port, mode, pwm);
          }
        }
      }
      
      // Die geraden Adressen 0x10 bis 0x1f konfigurieren die Eingänge
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

      // Die Adressen 0x20 bis 0x2f konfigurieren die Zähler-Eingänge
      if((addr >= 0x20)&&(addr <= 0x2f)) {
        unsigned char port = (addr - 0x20) >> 2;
        unsigned char reg = addr & 3;

        if(reg == 0) {
          // einschalten des Ultraschallbetriebs an C1?
          if((port == 0) && (value & 0x04)) {
            if(!c1_ultrasonic) {
              ftduino.ultrasonic_enable(true);
              c1_ultrasonic = true;
            }
          } else {
            // ausschalten des Ultraschallbetriebs
            if((port == 0) && !(value & 0x04)) {
              if(c1_ultrasonic) {
                ftduino.ultrasonic_enable(false);
                c1_ultrasonic = false;              
              }
            }
            
            int mode;
            switch(value) {
              case 0x00: mode = Ftduino::C_EDGE_NONE;    break;
              case 0x01: mode = Ftduino::C_EDGE_RISING;  break;
              case 0x02: mode = Ftduino::C_EDGE_FALLING; break;
              case 0x03: mode = Ftduino::C_EDGE_ANY;     break;
              default:   mode = Ftduino::C_EDGE_NONE;          
            }
            ftduino.counter_set_mode(Ftduino::C1+port, mode);
          }
        }

        // schreiben des LSB mit Wert != 0 löscht den Zähler
        if((reg == 1) && (value != 0))
          ftduino.counter_clear(Ftduino::C1+port);  
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

  // Counterwerte lesen. Man kann maximal einen Counter zur Zeit
  // lesen
  if((addr >= 0x20) && (addr <= 0x2f)) {
    unsigned char port = (addr - 0x20) >> 2;
    unsigned char reg = addr & 3;

    // lesen des Modusregisters liefert den aktuellen Zustand des Eingangs
    if(reg == 0)
      Wire.write(ftduino.counter_get_state(Ftduino::C1+port));

    // Lesen eines der beiden Datenregister liefert den Zählerstand bzw. die Distanz
    if((reg == 1) || (reg == 2)) {
      unsigned short value = 0;
      
      // Ultraschallmessung?
      if((port == 0) && c1_ultrasonic) 
        value = ftduino.ultrasonic_get();
      else
        value = ftduino.counter_get(Ftduino::C1+port);
        
      if(reg == 1) Wire.write(value & 0xff);
      Wire.write(value >> 8);
    }
  }
}

