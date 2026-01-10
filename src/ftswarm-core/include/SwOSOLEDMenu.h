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

/***************************************************
 *
 *   OLEDMenu
 *
 ***************************************************/

typedef enum { OLEDMenuSplash, OLEDMenuStatus, OLEDMenuSetup } OLEDMenu_t;

class OLEDMenu {

  protected:

    OLEDMenu_t menu = OLEDMenuStatus;

    void joystick( char *lr, char *fb, int8_t x, int8_t y, bool left );
    void printButton( const char *text, int16_t x, int16_t y, FtSwarmAlign_t align, FtSwarmToggle_t trigger );

    bool splashScreen( FtSwarmToggle_t trigger, SwOSIOType_t ioType, uint8_t port, bool completeRefresh );
    bool statusScreen( FtSwarmToggle_t trigger, SwOSIOType_t ioType, uint8_t port, bool completeRefresh );
    bool setupScreen(  FtSwarmToggle_t trigger, SwOSIOType_t ioType, uint8_t port, bool completeRefresh );

  public:

    OLEDMenu( SwOSCtrl *localCtrl );

    bool trigger( FtSwarmToggle_t trigger, SwOSIOType_t ioType, uint8_t port, bool completeRefresh );

};

extern OLEDMenu *oledMenu;
