// PwmSpeed.ino - PWM-abhängige Drehzahlmessung
//
// (c) 2017 by Till Harbaum <till@harbaum.org>
//

#include <Ftduino.h>

void setup() {
  // LED initialisieren
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.begin(115200);
  while(!Serial);

  ftduino.init();

  // Zähler an Eingang C1 auf steigende Flanke reagieren lassen
  ftduino.counter_set_mode(C1, Ftduino::C_EDGE_RISING);
}

void loop() {
  static int pwm=0;
  static int x=0;

  // messen, solange maximale PWM noch nicht erreicht
  if(pwm <= Ftduino::MAX) {

    // Motor auf PWM einstellen
    ftduino.motor_set(Ftduino::M1, Ftduino::LEFT, pwm);
    
    // etwas warten, damit der Motor seine Solldrehzahl erreichen kann
    delay(250);

    // LED einschalten
    digitalWrite(LED_BUILTIN, HIGH);
    // genau eine Sekunde lang Impulse zählen
    ftduino.counter_clear(C1);
    _delay_ms(1000);
    // Impulse aus Zähler C1 lesen
    int impulses = ftduino.counter_get(C1);
    // LED ausschalten
    digitalWrite(LED_BUILTIN, LOW);

    // Gemessenen Wert so oft ausgeben, dass am Ende die ganze
    // Breite der 500 Messwerte des seriellen Plotters der Arduino-IDE
    // genutzt wird
    while(x < 500*pwm/Ftduino::MAX) {
      // der TXT-Motor erzeugt 63 Impulse pro Umdrehung, also dreht er
      // Impulse/63 mal je Sekunde bzw. 60*Impulse/63 mal pro Minute (U/min)    
      Serial.println(60*impulses/63, DEC);
      x++;
    }

    // PWM um eine Stufe erhöhen
    pwm++;
  }
}
