/*
 * SwOSOLEDMenu.cpp
 *
 * OLED on Screen Menus
 * 
 * (C) 2025 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOSOLEDMenu.h"
#include "SwOSHW/SwOSHWLocal.h"
#include "SwOSSwarm.h"

OLEDMenu *oledMenu = NULL;

/***************************************************
 *
 *   OLEDMenu
 *
 ***************************************************/

OLEDMenu::OLEDMenu( SwOSCtrl *localCtrl ) {

  // cls
  oled->clearDisplay( true );
      
  // set useful default values
  oled->setTextColor(true, false);   // Draw white text
  oled->cp437(true);                 // Use full 256 char 'Code Page 437' font

  trigger( FTSWARM_NOTOGGLE, SWOSIO_BUTTON, SWOS_NOPORT, true );

}

void OLEDMenu::printButton( const char *text, int16_t x, int16_t y, FtSwarmAlign_t align,  FtSwarmToggle_t toggle ) {

  if ( ( text ) && ( text[0] != '\0' ) ) 
    oled->write( text, x, y, align, true, (toggle == FTSWARM_TOGGLEUP) );
  
}

void OLEDMenu::joystick( char *lr, char*fb, int8_t x, int8_t y, bool left ) {  

  const int8_t size = 11;
  int8_t b;

  if ( ( fb ) && ( fb[0] != '\0' ) ) {
  
    // ^
    b = y - size;
    oled->drawLine( x, b, x-3, b+3, true );
    oled->drawLine( x, b, x+3, b+3, true );

    oled->write( fb, x, b-9, FTSWARM_ALIGNCENTER, true, false );

    // v
    b = y + size;
    oled->drawLine( x, b, x-3, b-3, true );
    oled->drawLine( x, b, x+3, b-3, true );

    
  }

  if ( ( lr ) && ( lr[0] != '\0' ) ) {

    // <
    b = x - size;
    oled->drawLine( b, y, b+3, y-3, true );
    oled->drawLine( b, y, b+3, y+3, true );

    if (left) oled->write( lr, b-2, y-3, FTSWARM_ALIGNRIGHT, true, false );

      // >
    b = x + size;
    oled->drawLine( b, y, b-3, y-3, true );
    oled->drawLine( b, y, b-3, y+3, true );

    if (!left) oled->write( lr, b+2, y-3, FTSWARM_ALIGNLEFT, true, false );

  }

}

bool OLEDMenu::splashScreen( FtSwarmToggle_t toggle, SwOSIOType_t ioType, uint8_t port, bool completeRefresh ) {

  return false;

}

bool OLEDMenu::setupScreen( FtSwarmToggle_t toggle, SwOSIOType_t ioType, uint8_t port, bool completeRefresh ) {
  
  static const int16_t lowerLine = 64-16-10;
  static const int16_t width     = 127;

  bool triggered = false;

  // cls
  if ( completeRefresh ) {
    oled->clearDisplay();
    oled->write( "Configuration?", 64, 20, FTSWARM_ALIGNCENTER, true, false );
  }

  for (uint8_t i=0; i<4; i++ ) {

    triggered = triggered || ( i == port );

    if ( ( port == i ) || ( completeRefresh ) ) {

      switch ( i ) {
        case FTSWARM_S1:  printButton( "#1", 0,        lowerLine, FTSWARM_ALIGNLEFT,   toggle ); break;
        case FTSWARM_S2:  printButton( "#2", 41,       lowerLine, FTSWARM_ALIGNCENTER, toggle ); break;
        case FTSWARM_S3:  printButton( "#3", width-41, lowerLine, FTSWARM_ALIGNCENTER, toggle ); break;
        case FTSWARM_S4:  printButton( "#4", width,    lowerLine, FTSWARM_ALIGNRIGHT,  toggle ); break;
      }

      // released?
      if ( toggle == FTSWARM_TOGGLEDOWN ) {

        // set status screen
        menu = OLEDMenuStatus;

        // change config asynchronous
        SwOSCom setConfig( myOSSwarm.Ctrl[0]->macAddr, myOSSwarm.Ctrl[0]->serialNumber, CMD_SETACTIVECONFIG );
        setConfig.data.configCmd.config = i;
        xQueueSend( myOSNetwork.recvNotification, &setConfig, ESPNOW_MAXDELAY );

      }
      
    }
  }

  return triggered;

}

bool OLEDMenu::statusScreen( FtSwarmToggle_t toggle, SwOSIOType_t ioType, uint8_t port, bool completeRefresh ) {

  static const int16_t lowerLine = 64-16-10;
  static const int16_t width     = 127;

  bool triggered = false;

  if ( completeRefresh ) {
    oled->clearDisplay();
    joystick( nvs.oledLabel[nvs.activeEventConfig][SWOSLABEL_JOY1LR], nvs.oledLabel[nvs.activeEventConfig][SWOSLABEL_JOY1FB], 48,     20, true );
    joystick( nvs.oledLabel[nvs.activeEventConfig][SWOSLABEL_JOY2LR], nvs.oledLabel[nvs.activeEventConfig][SWOSLABEL_JOY2FB], 128-48, 20, false );

    char cfg[5];
    sprintf( cfg, "#%d", nvs.activeEventConfig+1 );
    oled->write( cfg, width/2, lowerLine, FTSWARM_ALIGNCENTER, true, false );

  }

  for (uint8_t i=0; i<8; i++ ) {

    if ( ( i == port ) || ( completeRefresh ) ) {

      switch ( i ) {
        case FTSWARM_S1:  printButton( nvs.oledLabel[nvs.activeEventConfig][i], 0,        lowerLine, FTSWARM_ALIGNLEFT,   toggle ); break;
        case FTSWARM_S2:  printButton( nvs.oledLabel[nvs.activeEventConfig][i], 41,       lowerLine, FTSWARM_ALIGNCENTER, toggle ); break;
        case FTSWARM_S3:  printButton( nvs.oledLabel[nvs.activeEventConfig][i], width-41, lowerLine, FTSWARM_ALIGNCENTER, toggle ); break;
        case FTSWARM_J1:  printButton( nvs.oledLabel[nvs.activeEventConfig][i], 48,       16,        FTSWARM_ALIGNCENTER, toggle ); break;
        case FTSWARM_J2:  printButton( nvs.oledLabel[nvs.activeEventConfig][i], width-48, 16,        FTSWARM_ALIGNCENTER, toggle ); break;
        case FTSWARM_F1:  printButton( nvs.oledLabel[nvs.activeEventConfig][i], 0,        1,         FTSWARM_ALIGNLEFT,   toggle ); break;
        case FTSWARM_F2:  printButton( nvs.oledLabel[nvs.activeEventConfig][i], width,    1,         FTSWARM_ALIGNRIGHT,  toggle ); break;

        case FTSWARM_S4:  printButton( "SET",                                   width,    lowerLine, FTSWARM_ALIGNRIGHT,  toggle ); 
                          
                          triggered = true;

                          // released?
                          if ( toggle == FTSWARM_TOGGLEDOWN ) {
                            menu = OLEDMenuSetup;
                            setupScreen( FTSWARM_NOTOGGLE, ioType, SWOS_NOPORT, true );
                          }
                          break;

      }
      
    }
    
  }

  return triggered;

}

bool OLEDMenu::trigger( FtSwarmToggle_t toggle, SwOSIOType_t ioType, uint8_t port, bool completeRefresh ) {

  switch (menu) {
    case OLEDMenuSplash: return splashScreen( toggle, ioType, port, completeRefresh ); break;
    case OLEDMenuStatus: return statusScreen( toggle, ioType, port, completeRefresh ); break;
    case OLEDMenuSetup:  return setupScreen ( toggle, ioType, port, completeRefresh ); break;
  }

  return false;

}