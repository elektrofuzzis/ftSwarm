/*
 * SwOSOLEDMenu.h
 *
 * OLED on Screen Menus
 * 
 * (C) 2025 Christian Bergschneider & Stefan Fuss
 * 
 */

#pragma once

#include "SwOS.h"
#include "SwOSHW/SwOSHWBaseCtrl.h"

/**************************************************
 *
 *   OLEDLabel
 *
 **************************************************/
  
class OLEDLabel {
  
  protected:

    char   text[4];
    int8_t x = 0;
    int8_t y = 0;
    FtSwarmAlign_t align = FTSWARM_ALIGNCENTER;  

  public:

    OLEDLabel() { text[0] = '\0'; };

    bool isActive( void ) { return text[0] != '\0'; };
    void set( const char *text, uint8_t maxChars, int8_t x, int8_t y, FtSwarmAlign_t align );
    void set( const char *text, uint8_t maxChars );
    void print( bool clicked );

  };

/***************************************************
 *
 *   OLEDMenu
 *
 ***************************************************/

class OLEDMenu {

  protected:

    uint8_t   oldHC165 = 0;
    OLEDLabel label[8];

    void joystick( int8_t x, int8_t y, bool left );

  public:

    OLEDMenu( SwOSCtrl *localCtrl );

    void setLabel( const char *text, uint8_t i, int8_t x, int8_t y, FtSwarmAlign_t align );
    void set( uint8_t i, const char *text );
    void splashScreen( void );
    void statusScreen( uint8_t hc165, bool completeRefresh );
    void operate( uint8_t newHC165 );

};
