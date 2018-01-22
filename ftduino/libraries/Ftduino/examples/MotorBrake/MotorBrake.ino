//
// MotorBrake.ino
//
// ftDuino-Experiment zum Bremsverhalten von Elektromotoren
//

#include <Ftduino.h>

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // diee Bitrate ist im Falle des ftDuino bedeutungslos
  Serial.begin(115200);      // seriellen Port mit 115200 bps öffnen
  while(!Serial);

  // ftDuino-Timer, Interrupts etc starten
  ftduino.init();
}

void loop() {
  static uint32_t last_led_event = 0;

  // LED einmal pro Sekunden blitzen lassen
  if((millis() - last_led_event) >= 1000) {
    digitalWrite(LED_BUILTIN, HIGH);    // LED an
    last_led_event = millis();
  } else if((millis() - last_led_event) > 50) 
    digitalWrite(LED_BUILTIN, LOW);     // LED aus

  // alle fünf Sekunden einen Motortest starten
  static uint32_t last_motor_event = 0;
  if(((millis() - last_motor_event) > 5000)) {
    static bool brake = true;
    
    last_motor_event = millis();
    
    ftduino.motor_counter_set_brake(Ftduino::M1, brake);
  
    // Der TXT-Encoder-Motor liefert 63 1/3 Impulse pro Umdrehung, 190 Impulse sind demnach
    // drei volle Umdrehungen
    ftduino.motor_counter(Ftduino::M1, Ftduino::LEFT, Ftduino::MAX, 190); 
    
    Serial.println("Motor M1 gestartet");
    
    Serial.println("Warte auf Encoder-Impulse ...");
    while(ftduino.motor_counter_active(Ftduino::M1));

    if(brake)
      Serial.println("Motor wird aktiv gebremst, warte Nachlauf ab");
    else
      Serial.println("Motor aus, warte Nachlauf ab");

    delay(1000);

    Serial.print("Nachlauf: ");
    Serial.print(ftduino.counter_get(Ftduino::C1));
    Serial.println(" Impulse");

    // abwechselnd mit und ohne Bremse testen
    brake = !brake;
  }
}
