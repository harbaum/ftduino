/*
  FtduinoSimple.h - Library for ftduino

  (c) 2017 by Till Harbaum <till@harbaum.org>
*/

#ifndef ftduino_h
#define ftduino_h

#include "Arduino.h"

#ifndef IN_FTDUINO_SIMPLE_LIB
  #if defined(OUTPUT_DRIVER_MC33879A)
    #warning "Building for MC33879A boards only"
    #ifndef MC33879A_ONLY  // user has to define this
      #error "Please select 'Automatic' output driver or define MC33879A_ONLY when using MC33879A output driver."
    #endif
  #elif defined(OUTPUT_DRIVER_TLE94108EL)
    #warning "Building for TLE94108EL boards only"
    #ifndef TLE94108EL_ONLY  // user has to define this
      #error "Please select 'Automatic' output driver or define TLE94108EL_ONLY when using TLE94108EL output driver."
    #endif
  #elif defined(OUTPUT_DRIVER_AUTO)
    // #warning "Building for output auto detection"
  #else
    #warning "Error, no output driver configured"
  #endif

  // make sure WebUSB is being used with correct settings
  #if USB_VERSION == 0x210 && !defined(WebUSB_h)
    #error "Please include WebUSB.h if WebUSB is being used!"
  #endif
#endif

class Ftduino {
  public:
    Ftduino();

    // constant names for the counter inputs
    static const uint8_t C1 = 0, C2 = 1, C3 = 2, C4 = 3;

    // constant names for the inputs
    static const uint8_t I1 = 0, I2 = 1, I3 = 2, I4 = 3;
    static const uint8_t I5 = 4, I6 = 5, I7 = 6, I8 = 7;
    
    // constant names for the outputs
    static const uint8_t O1 = 0, O2 = 1, O3 = 2, O4 = 3;
    static const uint8_t O5 = 4, O6 = 5, O7 = 6, O8 = 7;
    static const uint8_t M1 = 0, M2 = 1, M3 = 2, M4 = 3;

    // 
    static const uint8_t OFF = 0, HI = 1, LO = 2;               // OFF = 0 also applies

    // motor modes
    static const uint8_t LEFT = 1, RIGHT = 2, BRAKE = 3;

    bool input_get(uint8_t ch);

    void output_set(uint8_t port, uint8_t mode);
    void motor_set(uint8_t port, uint8_t mode);

    bool counter_get_state(uint8_t ch);

  private:
    void cd4051_init();
    void input_init();
    void output_init();
    void pulldown_c1_init();
    void counter_init(void);

    void cd4051_set(char mode);
    void pulldown_c1_enable(char on);
#if defined(OUTPUT_DRIVER_MC33879A) || defined(OUTPUT_DRIVER_AUTO)
    void output_spi_tx(void);
    uint32_t spi_tx;
#endif
#if defined(OUTPUT_DRIVER_TLE94108EL) || defined(OUTPUT_DRIVER_AUTO)
    void write_spi_reg(uint8_t reg, uint8_t data);
    uint16_t state;
#endif
#if defined(OUTPUT_DRIVER_AUTO)
    void output_set_mc33879a(uint8_t port, uint8_t mode);
    void output_set_tle94108el(uint8_t port, uint8_t mode);
    bool odrv_mc33879a;
#endif
};

extern Ftduino ftduino;
 
#endif
