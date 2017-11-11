// Klasse zur Rechts-/Links- und Hoch-/Runterbewegung des Turms

#include "Tower.h"

Tower::Tower() { }

void Tower::init() {
  // konfiguriere beide Endschalter-Anschlüsse für Schalter
  ftduino.input_set_mode(LOWER_ENDSTOP, Ftduino::SWITCH);
  ftduino.input_set_mode(LEFT_ENDSTOP, Ftduino::SWITCH);

  // die Zähler für die Encoder sollen auf beide Flanken reagieren
  ftduino.counter_set_mode(COUNTER_VERTICAL, Ftduino::C_EDGE_RISING);
  ftduino.counter_set_mode(COUNTER_HORIZONTAL, Ftduino::C_EDGE_RISING);
}

bool Tower::moveHome() {
  // wenn einer der Endschalter bereits betätigt ist, dann fahre kurz etwas
  // vom Endschalter weg
  if(ftduino.input_get(LOWER_ENDSTOP) || ftduino.input_get(LEFT_ENDSTOP)) {
    if(ftduino.input_get(LOWER_ENDSTOP))
      ftduino.motor_set(MOTOR_VERTICAL, MOVE_UP, Ftduino::MAX);
    if(ftduino.input_get(LEFT_ENDSTOP))
      ftduino.motor_set(MOTOR_HORIZONTAL, MOVE_RIGHT, Ftduino::MAX);
    
    delay(500);

    // stoppe beide Motoren
    ftduino.motor_set(MOTOR_VERTICAL, Ftduino::BRAKE, 0);
    ftduino.motor_set(MOTOR_HORIZONTAL, Ftduino::BRAKE, 0);

    // Die Endschalter müssen nun beide offen sein. Sind sie es
    // nicht stimmt etwas mit dem Modell nicht
    if(ftduino.input_get(LOWER_ENDSTOP) || ftduino.input_get(LEFT_ENDSTOP)) {
      Serial.println("Turm-Endschalter blockiert");
      return false;
    }
  }
  
  // Endschalter ist bisher nicht betätigt, starte Motoren
  ftduino.motor_set(MOTOR_VERTICAL, MOVE_DOWN, Ftduino::MAX);
  ftduino.motor_set(MOTOR_HORIZONTAL, MOVE_LEFT, Ftduino::MAX);

  // beide Motoren laufen
  int running_vertical = true;
  int running_horizontal = true;
  
  // warte max 2 Sekunden bis beide Endschalter geschlossen sind
  long timeout = millis() + 20000;
  do {
    // teste, ob der vertikale Motor noch läuft und seinen
    // Endschalter erreicht hat
    if(running_vertical && ftduino.input_get(LOWER_ENDSTOP)) {
      ftduino.motor_set(MOTOR_VERTICAL, Ftduino::BRAKE, 0);
      running_vertical = false;
    }
    // teste, ob der horizontale Motor noch läuft und seinen
    // Endschalter erreicht hat
    if(running_horizontal && ftduino.input_get(LEFT_ENDSTOP)) {
      ftduino.motor_set(MOTOR_HORIZONTAL, Ftduino::BRAKE, 0);
      running_horizontal = false;
    }
  } while((running_vertical || running_horizontal) && millis() < timeout);

  // Turm ist nun an der Home-Position. Alle Zähler auf Null setzen
  x_pos = y_pos = 0;

  ftduino.motor_set(MOTOR_VERTICAL, Ftduino::BRAKE, 0);
  ftduino.motor_set(MOTOR_HORIZONTAL, Ftduino::BRAKE, 0);

  if(!ftduino.input_get(LOWER_ENDSTOP))
      Serial.println("Unterer Turm-Endschalter nicht erreicht");
    
  if(!ftduino.input_get(LEFT_ENDSTOP))
      Serial.println("Linker Turm-Endschalter nicht erreicht");
      
  // melde zurück, ob beide Endschalter nun geschlossen sind
  return(ftduino.input_get(LOWER_ENDSTOP) && ftduino.input_get(LEFT_ENDSTOP));
}

int Tower::getX() {
  return x_pos;
}

int Tower::getY() {
  return y_pos;
}

void Tower::moveTo(int i, bool lift) {
  int x_move, y_move;

  if(!i) {
    // Basis soll angefahren werden
    x_move = -x_pos;
    y_move = (lift?LIFT_OFFSET:0) - y_pos;
  } else {
  
    // Fach-Nummer in Regal-Spalten und -Zeilen umrechnen
    int column= (i-1)%2;
    int row = (i-1)/2;

    // Umrechnung Spalte/Reihe in Encoder-Impulse ab Home-Position
    x_move = COLUMN_FIRST + column * COLUMN_STEP - x_pos;
    y_move = ROW_FIRST + row * ROW_STEP + (lift?LIFT_OFFSET:0) - y_pos;
  }

  bool row_reached = false;  
  bool column_reached = false;

  ftduino.counter_clear(COUNTER_VERTICAL);
  ftduino.counter_clear(COUNTER_HORIZONTAL);

  // starte beide Motoren mit der benötigten Drehrichtung
  if(x_move > 0)      ftduino.motor_set(MOTOR_HORIZONTAL, MOVE_RIGHT, Ftduino::MAX);
  else if(x_move < 0) ftduino.motor_set(MOTOR_HORIZONTAL,  MOVE_LEFT, Ftduino::MAX);
  else column_reached = true;

  if(y_move > 0)      ftduino.motor_set(MOTOR_VERTICAL,      MOVE_UP, Ftduino::MAX);
  else if(y_move < 0) ftduino.motor_set(MOTOR_VERTICAL,    MOVE_DOWN, Ftduino::MAX);
  else row_reached = true;

  // fahre Motoren, bis beide Positionen erreicht sind
  while(!row_reached || !column_reached) {
    // horizontaler Motor fährt noch
    if(!column_reached) {
      int x_moved = ftduino.counter_get(COUNTER_HORIZONTAL);
      
      // horizontaler Zählerstand erreicht?
      if(x_moved >= abs(x_move)) {
        ftduino.motor_set(MOTOR_HORIZONTAL, Ftduino::BRAKE, 0);
        column_reached = true;

        // aktualisiere aktuelle X-Position
        if(x_move > 0) x_pos += x_moved;
        if(x_move < 0) x_pos -= x_moved;
      }
    }
    
    // vertikaler Motor fährt noch
    if(!row_reached) {
      int y_moved = ftduino.counter_get(COUNTER_VERTICAL);
      
      // vretikaler Zählerstand erreicht?
      if(y_moved >= abs(y_move)) {
        ftduino.motor_set(MOTOR_VERTICAL, Ftduino::BRAKE, 0);
        row_reached = true;

        // aktualisiere aktuelle X-Position
        if(y_move > 0) y_pos += y_moved;
        if(y_move < 0) y_pos -= y_moved;
      }
    }
  }

  // Turm steht nun an neuer Position
}

