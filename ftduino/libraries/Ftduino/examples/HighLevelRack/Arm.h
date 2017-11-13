// Klasse zur Vor- und Rückbewegung des Ausleger-Arms

#include <Ftduino.h>

class Arm {
  public:                              // öffentlich
    Arm();                             // der Default-Konstruktor
    void init();
    bool moveHome();                   // Arm einziehen
    void moveOut();                    // Arm ausstrecken
    void moveIn();                     // Arm einziehen
    
  private:                             // privat
    const int OUTER_ENDSTOP = Ftduino::I2;
    const int INNER_ENDSTOP = Ftduino::I4;
    const int MOTOR = Ftduino::M2;
};
