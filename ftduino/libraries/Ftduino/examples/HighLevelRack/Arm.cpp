// Klasse zur Vor- und Rückbewegung des Ausleger-Arms

#include "Arm.h"

Arm::Arm() { }

void Arm::init() {
  // konfiguriere beide Endschalter-Anschlüsse für Schalter
  ftduino.input_set_mode(OUTER_ENDSTOP, Ftduino::SWITCH);
  ftduino.input_set_mode(INNER_ENDSTOP, Ftduino::SWITCH);
}

bool Arm::moveHome() {
  // wenn der Endschalter betätigt ist, dann fahre kurz etwas
  // vom Endschalter weg
  if(ftduino.input_get(OUTER_ENDSTOP)) {
    ftduino.motor_set(MOTOR, Ftduino::LEFT, Ftduino::MAX);
    delay(250);
    ftduino.motor_set(MOTOR, Ftduino::BRAKE, 0);
  }

  // Endschalter darf nun nicht mehr betätigt sein
  if(ftduino.input_get(OUTER_ENDSTOP)) {
    Serial.println("Äußerer Endschalter immer noch betätigt");    
    return false;
  }
  
  // Endschalter ist bisher nicht betätigt, starte Motor
  ftduino.motor_set(MOTOR, Ftduino::RIGHT, Ftduino::MAX);

  // warte auf Endschalter
  long timeout = millis() + 5000;
  while(!ftduino.input_get(OUTER_ENDSTOP) && millis() < timeout);

  // stoppe Motor
  ftduino.motor_set(MOTOR, Ftduino::BRAKE, 0);

  if(!ftduino.input_get(OUTER_ENDSTOP))
    Serial.println("Äußerer Endschalter in 5 Sekunden nicht erreicht");    

  // liefere "wahr" zurück, wenn Endschalter erreicht wurde
  return ftduino.input_get(OUTER_ENDSTOP);
}

void Arm::moveIn() {
  if(ftduino.input_get(OUTER_ENDSTOP))
    return;
  
  // Endschalter ist bisher nicht betätigt, starte Motor
  ftduino.motor_set(MOTOR, Ftduino::RIGHT, Ftduino::MAX);

  // warte auf Endschalter
  long timeout = millis() + 5000;
  while(!ftduino.input_get(OUTER_ENDSTOP) && millis() < timeout);

  // stoppe Motor
  ftduino.motor_set(MOTOR, Ftduino::BRAKE, 0);

  if(!ftduino.input_get(OUTER_ENDSTOP))
    Serial.println("Äußerer Endschalter in 5 Sekunden nicht erreicht");    

  // liefere "wahr" zurück, wenn Endschalter erreicht wurde
  return ftduino.input_get(OUTER_ENDSTOP);
}

void Arm::moveOut() {
  if(ftduino.input_get(INNER_ENDSTOP))
    return;
  
  // Endschalter ist bisher nicht betätigt, starte Motor
  ftduino.motor_set(MOTOR, Ftduino::LEFT, Ftduino::MAX);

  // warte auf Endschalter
  long timeout = millis() + 5000;
  while(!ftduino.input_get(INNER_ENDSTOP) && millis() < timeout);

  // stoppe Motor
  ftduino.motor_set(MOTOR, Ftduino::BRAKE, 0);

  if(!ftduino.input_get(INNER_ENDSTOP))
    Serial.println("Innerer Endschalter in 5 Sekunden nicht erreicht");    

  // liefere "wahr" zurück, wenn Endschalter erreicht wurde
  return ftduino.input_get(INNER_ENDSTOP);
}
    
