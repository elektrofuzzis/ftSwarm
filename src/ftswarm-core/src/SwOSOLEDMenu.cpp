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

/***************************************************
 *
 *   OLEDLabel
 *
 ***************************************************/

void OLEDLabel::set( const char *text, uint8_t maxChar, int8_t x, int8_t y, FtSwarmAlign_t align ) {

  set( text, maxChar );
  this->x = x;
  this->y = y;
  this->align = align;

}

void OLEDLabel::set( const char *text, uint8_t maxChar ) {

  bzero( this->text, 4 );
  strncpy( this->text, text, (maxChar>3)?3:maxChar );

}

void OLEDLabel::print( bool clicked ) {
  
  // inactive?
  if ( text[0] == '\0' ) return;

  oled->write( text, x, y, align, true, clicked );
  
}


/***************************************************
 *
 *   OLEDMenu
 *
 ***************************************************/

OLEDMenu::OLEDMenu( SwOSCtrl *localCtrl ) {

  // Hen & Egg-Problem, can't access on oled yet
  int16_t lowerLine = 64-16-10;
  int16_t width     = 127;

  for ( uint8_t i=0; i<=7; i++ ) {
    SwOSIO *io = localCtrl->getIO( SWOSIO_BUTTON, i );
    if (io) {
      switch ( i ) {
        case FTSWARM_S1: label[i].set( io->getAlias(), 3, 0,        lowerLine, FTSWARM_ALIGNLEFT);   break;
        case FTSWARM_S2: label[i].set( io->getAlias(), 3, 41,       lowerLine, FTSWARM_ALIGNCENTER); break;
        case FTSWARM_S3: label[i].set( io->getAlias(), 3, width-41, lowerLine, FTSWARM_ALIGNCENTER); break;
        case FTSWARM_S4: label[i].set( io->getAlias(), 3, width,    lowerLine, FTSWARM_ALIGNRIGHT);  break;
        case FTSWARM_J1: label[i].set( io->getAlias(), 2, 48,       20,        FTSWARM_ALIGNCENTER); break;
        case FTSWARM_J2: label[i].set( io->getAlias(), 2, width-48, 20,        FTSWARM_ALIGNCENTER); break;
        case FTSWARM_F1: label[i].set( io->getAlias(), 3, 0,        0,         FTSWARM_ALIGNLEFT );  break;
        case FTSWARM_F2: label[i].set( io->getAlias(), 3, width,    0,         FTSWARM_ALIGNRIGHT);  break;
      }
    }
  }

  /*
  // cls
  oled->clearDisplay();
      
  // set useful default values
  oled->setTextColor(true, false);   // Draw white text
  oled->cp437(true);                 // Use full 256 char 'Code Page 437' font

  statusScreen( 0, true );
*/
}

void OLEDMenu::joystick( int8_t x, int8_t y, bool left ) {  

  const int8_t size = 11;
  int8_t b;
  
  // ^
  b = y - size;
  oled->drawLine( x, b, x-3, b+3, true );
  oled->drawLine( x, b, x+3, b+3, true );

  // v
  b = y + size;
  oled->drawLine( x, b, x-3, b-3, true );
  oled->drawLine( x, b, x+3, b-3, true );

  // <
  b = x - size;
  oled->drawLine( b, y, b+3, y-3, true );
  oled->drawLine( b, y, b+3, y+3, true );

  // >
  b = x + size;
  oled->drawLine( b, y, b-3, y-3, true );
  oled->drawLine( b, y, b-3, y+3, true );

  /*
  // J1
  oled->write( j, x+1, y-3 );

  if (left) oled->write( fb, x+1, y-size-9, FTSWARM_ALIGNCENTER );
  else      oled->write( fb, x+1, y-size-9, FTSWARM_ALIGNCENTER );

  // LR
  if ( left ) oled->write( lr, x-size-1, y-3,FTSWARM_ALIGNRIGHT );
  else        oled->write( lr, x+size+3, y-3,FTSWARM_ALIGNLEFT );

  */

}

void OLEDMenu::splashScreen( void ) {

}

void OLEDMenu::statusScreen( uint8_t hc165, bool completeRefresh ) {

  uint8_t delta = oldHC165 ^ hc165;

  for (uint8_t i=0; i<8; i++ ) {

    uint8_t bitmask = 1 << i;
    if ( ( delta & bitmask ) || ( completeRefresh ) ) label[i].print( hc165 & bitmask );
    
  }

  oldHC165 = hc165;

  if ( completeRefresh ) {

    joystick( 48,     24, true );
    joystick( 128-48, 24, false );

  }

  

}

void OLEDMenu::setLabel( const char *text, uint8_t i, int8_t x, int8_t y, FtSwarmAlign_t align ) { 
  
  uint8_t chars = 3;
  if ( ( i == FTSWARM_J1 ) || ( i == FTSWARM_J2 ) ) chars = 2;
  
  label[i].set( text, chars, x, y, align ); 

};

void OLEDMenu::set( uint8_t i, const char *text ) {

  uint8_t chars = 3;
  if ( ( i == FTSWARM_J1 ) || ( i == FTSWARM_J2 ) ) chars = 2;

  label[i].set( text, chars );

  statusScreen( 1<<i, false );

}

void OLEDMenu::operate( uint8_t newHC165 ) {

  // test on changed buttons
  uint8_t trigger = oldHC165 ^ newHC165;
  if (!trigger) return;

  oldHC165 = newHC165;
  statusScreen( newHC165, false );

}