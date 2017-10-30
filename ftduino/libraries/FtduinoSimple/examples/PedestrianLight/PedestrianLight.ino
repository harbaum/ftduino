/* 
 *  PedestrianLight - Idas Ampel
 *  
 *  Kein Gerät aus dem Hause Harbaum ist komplett ohne Idas
 *  Schleich-Tier-Ampel. Die Ampel implementriert eine 
 *  klassische Bedarfsampel mit fünf Lampen
 */

#include "FtduinoSimple.h"

// ---------------------------------------------------------

// Konstanten für die diversen Lampen und die
// verwendeten Ausgänge
#define CARS_RED   Ftduino::O1
#define CARS_YLW   Ftduino::O2
#define CARS_GRN   Ftduino::O3
#define PED_RED    Ftduino::O4
#define PED_GRN    Ftduino::O5

// Konstanten für Licht-An und Licht-Aus
#define LIGHT_ON   Ftduino::HI
#define LIGHT_OFF  Ftduino::LO

// Der für den Fußgänger-Knopf verwendete Eingang
#define BUTTON     Ftduino::I1

// schalte die Auto-Ampel auf rot
void cars_red() {
  ftduino.output_set(CARS_RED, LIGHT_ON);
  ftduino.output_set(CARS_YLW, LIGHT_OFF);
  ftduino.output_set(CARS_GRN, LIGHT_OFF);
}

// schalte die Auto-Ampel auf gelb
void cars_yellow() {
  ftduino.output_set(CARS_RED, LIGHT_OFF);
  ftduino.output_set(CARS_YLW, LIGHT_ON);
  ftduino.output_set(CARS_GRN, LIGHT_OFF);
}

// schalte die Auto-Ampel auf grün
void cars_green() {
  ftduino.output_set(CARS_RED, LIGHT_OFF);
  ftduino.output_set(CARS_YLW, LIGHT_OFF);
  ftduino.output_set(CARS_GRN, LIGHT_ON);
}

// schalte die Auto-Ampel auf rot/gelb
void cars_red_yellow() {
  ftduino.output_set(CARS_RED, LIGHT_ON);
  ftduino.output_set(CARS_YLW, LIGHT_ON);
  ftduino.output_set(CARS_GRN, LIGHT_OFF);
}

// schalte die Fußgänger-Ampel auf rot
void pedestrians_red() {
  ftduino.output_set(PED_RED, LIGHT_ON);
  ftduino.output_set(PED_GRN, LIGHT_OFF);
}

// schalte die Fußgänger-Ampel auf grün
void pedestrians_green() {
  ftduino.output_set(PED_RED, LIGHT_OFF);
  ftduino.output_set(PED_GRN, LIGHT_ON);
}

// Schalte alle Ampeln aus
void off(void) {
  ftduino.output_set(CARS_RED, LIGHT_OFF);
  ftduino.output_set(CARS_YLW, LIGHT_OFF);
  ftduino.output_set(CARS_GRN, LIGHT_OFF);
  ftduino.output_set(PED_RED, LIGHT_OFF);
  ftduino.output_set(PED_GRN, LIGHT_OFF);
}

// die setup-Funktion wird einmal beim Start aufgerufen
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);     // LED

  // initially all lights are off
  off();
}

// Konstanten für die Länge der Ampelphasen in Millisekunden
#define CARS_GREEN_PHASE         (5000)    // Grünpahse bevor und nachdem die Ampel sich abschaltet
#define YELLOW_PHASE             (2000)
#define BOTH_RED_PHASE           (2000)
#define PEDESTRIANS_GREEN_PHASE  (15000)
#define CARS_MAIN_GREEN_PHASE    (30000)

// die loop-Funktion wird immer wieder aufgerufen
void loop() {
  // Zeitpunkt des nächsten Lichtwechsel-Ereignisses
  static unsigned long next_event = 0;
  // Aktueller Zustand der Amepl
  static char state = 0;

  // Die interne Leuchtdiode soll einmal pro Sekunde blinken
  static unsigned long flash_timer = 0;
  if(millis() > flash_timer + 10)
    digitalWrite(LED_BUILTIN, LOW);
  if(millis() > flash_timer + 1000) {
    digitalWrite(LED_BUILTIN, HIGH);
    flash_timer = millis();
  }

  // Teste ob ein Fussgänger im Zustand 0 (Ampel aus) den
  // Knopf gedrückt hat
  if((state == 0) && (ftduino.input_get(BUTTON)))
    state = 1;   // ja -> wechsel in Zustand 1

  // Zustandswechsel auf Zeitbasis passieren in allen Zuständen > 0, denn
  // Zustand 0 ist die abgeschaltete Ampel, die auf einen Knopfdruck wartet
  if(state > 0) {
  
    // Teste, ob die eingestellte Zeit vergangen ist
    if(millis() > next_event) {
      switch(state) {

        // Ampel wechselt in Zustand 1: Autos haben grün, Fussgänger haben rot
        case 1: {
          // schalte Lampen
          cars_green();
          pedestrians_red();
          // setze Zeitpunkt für nächstes Ereignis
          next_event = millis() + CARS_GREEN_PHASE;
          // setze Zustand für nächstes Ereignis
          state++;   // Kurzschreibweise für "state = state + 1"
          break;
        }
        
        // Ampel wechselt in Zustand 2: Autos haben gelb, Fussgänger haben rot
        case 2: {
          cars_yellow();
          next_event = millis() + YELLOW_PHASE;
          state++;
          break;
        }

        // Ampel wechselt in Zustand 3: Autos haben rot, Fussgänger haben rot
        case 3: {
          cars_red();
          next_event = millis() + BOTH_RED_PHASE;
          state++;
          break;
        }
        
        // Ampel wechselt in Zustand 4: Autos haben rot, Fussgänger haben grün
        case 4: {
          pedestrians_green();
          next_event = millis() + PEDESTRIANS_GREEN_PHASE;
          state++;
          break;
        }

        // Ampel wechselt in Zustand 5: Autos haben rot, Fussgänger haben rot
        case 5: {
          pedestrians_red();
          next_event = millis() + BOTH_RED_PHASE;
          state++;
          break;
        }
        
        // Ampel wechselt in Zustand 6: Autos haben gelb, Fussgänger haben rot
        case 6: {
          cars_red_yellow();
          next_event = millis() + YELLOW_PHASE;
          state++;
          break;
        }
        
        // Ampel wechselt in Zustand 7: Autos haben grün, Fussgänger haben rot
        case 7: {
          cars_green();
          next_event = millis() + CARS_GREEN_PHASE;
          state++;
          break;
        }
        
        // Ampel wechselt in Zustand 8: Ampel ist aus
        case 8: {
          off();
          // Rückkehr in Zustand 0
          state = 0;
          break;
        }
      }
    }
  }
}

