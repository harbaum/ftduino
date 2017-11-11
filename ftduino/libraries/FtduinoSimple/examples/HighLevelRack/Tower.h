// Klasse zur Rechts-/Links- und Hoch-/Runterbewegung des Turms

#include <Ftduino.h>

// Positionen werden der Einfachheit halber in cm angeben, aber
// die Positionssignale stammen aus Encodern, die 75 Impulse pro Motorumdrehung
// liefern und vertikal 1:1 auf eine Antriebsschnecke gehen sowie 
// horizontal via 2:1 auf Schnecke (4,734mm/Umdrehung)
#define SPINDLE    0.4734
#define CM2X(a)    (int)(((a)/SPINDLE)*75*2)
#define CM2Y(a)    (int)(((a)/SPINDLE)*75)

class Tower {
  public:                              // öffentlich
    Tower();                           // der Default-Konstruktor
    void init();                       // Initialisierung
    bool moveHome();                   // Turm zur Ablageposition fahren
    void moveTo(int i, bool l);        // Turm zu Fach i fahren

    int getX();
    int getY();
    
  private:                             // privat
    
    const int MOTOR_VERTICAL = Ftduino::M3;
    const int COUNTER_VERTICAL = Ftduino::C3;
    const int LOWER_ENDSTOP = Ftduino::I3;

    const int MOTOR_HORIZONTAL = Ftduino::M1;
    const int COUNTER_HORIZONTAL = Ftduino::C1;
    const int LEFT_ENDSTOP = Ftduino::I1;

    // Motordrehrichtungen auf Turm-Fahrtrichtung abbilden
    const int MOVE_RIGHT = Ftduino::LEFT;
    const int MOVE_LEFT = Ftduino::RIGHT;
    const int MOVE_UP = Ftduino::LEFT;
    const int MOVE_DOWN = Ftduino::RIGHT;

    // Positionen der Regal-Spalten und -Reihen
    const int COLUMN_FIRST = CM2X(9.5);
    const int COLUMN_STEP = CM2X(4*1.5);
    const int ROW_FIRST = CM2Y(8*1.5+0.3);
    const int ROW_STEP = CM2Y(-4*1.5);;
    const int LIFT_OFFSET = CM2Y(1.5);
    
    int x_pos;
    int y_pos;
};
