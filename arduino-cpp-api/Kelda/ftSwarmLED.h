////////////////////////////////////////////////////
//
// ftSwarmLEB.h
//
// ftSwarm LED Control
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
///////////////////////////////////////////////////

#ifndef FtSwarmLED_H
#define FtSwarmLED_H

#include "FtSwarmObj.h"
#include <arduino.h>

// ports
#define FTSWARM_LED1 0
#define FTSWARM_LED2 1

// CSS-Style color constants
#define BLACK   0x000000  
#define WHITE   0xFFFFFF
#define RED     0xFF0000
#define LIME    0x00FF00
#define BLUE    0x0000FF
#define YELLOW  0xFFFF00  
#define CYAN    0x00FFFF
#define MAGENTA 0xFF00FF
#define SILVER  0xC0C0C0
#define GRAY    0x808080
#define MAROON  0x800000
#define OLIVE   0x808000
#define GREEN   0x008000
#define PURPLE  0x800080
#define TEAL    0x008080
#define NAVY    0x000080

class FtSwarmLED : public FtSwarmObj {
  protected:
  public:
    FtSwarmLED( char* UUID, uint8_t LED );
      // constructor
      
    void setPixelColor( uint8_t r, uint8_t g, uint8_t b );
      // set color by seperated r,g,b values
      
    void setPixelColor( uint32_t c );
      // set color by color constants 
      
    void setBrightness( uint8_t brightness );
      // set brightness
      // Attention: this is a shared setting for all rgb leds
      
    void off( void );
      // switch off
    
};

#endif
