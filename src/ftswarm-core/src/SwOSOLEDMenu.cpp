/*
 * SwOSOLEDMenu.cpp
 *
 * OLED on Screen Menus
 * 
 * (C) 2025 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOSOLEDMenu.h"
#include "SwOSLog.h"
#include "SwOSHW/SwOSHWLocal.h"
#include "SwOSSwarm.h"

SwOSScreenManager screenManager;

/***************************************************
 *
 *   SwOSScreenObj
 *
 ***************************************************/

SwOSScreenObj::SwOSScreenObj( const char *label, uint8_t x, uint8_t y, FtSwarmAlign_t align ) {
  
  this->x = x;
  this->y = y;
  this->align = align;

  setLabel( label );

}

SwOSScreenObj::~SwOSScreenObj() {

  if ( label ) free( label );

}

void SwOSScreenObj::setLabel( const char *label ) {

  if ( this->label) {
    delete this->label;
  }

  if ( label ) {

    this->label = (char *)calloc( strlen(label) + 1, sizeof( char ) );
    strcpy( this->label, label );

  } else {

    this->label = NULL;
  
  }

}

void SwOSScreenObj::draw( bool inverted ) {

  if ( ( label ) && ( label[0] != '\0' )  ) oled->write( label, x, y, align, true, inverted );

}

/***************************************************
 *
 *   SwOSScreenSlider
 *
 ***************************************************/

SwOSScreenSlider::SwOSScreenSlider( const char *label, uint8_t x, uint8_t y, uint8_t size, uint8_t direction ) : SwOSScreenObj( label, x, y, FTSWARM_ALIGNCENTER ) {

  this->size      = size;
  this->direction = direction;

}

void SwOSScreenSlider::draw( bool inverted ) {
  
  SwOSScreenObj::draw( inverted );

  if (direction==HORIZONTAL) drawHorizontal();
  else                       drawVertical();

}

void SwOSScreenSlider::drawHorizontal( void ) {

  // ^
  int8_t b = y - size;
  oled->drawLine( x, b, x-3, b+3, true );
  oled->drawLine( x, b, x+3, b+3, true );

  // oled->write( label, x, b-9, FTSWARM_ALIGNCENTER, true, false );

  // v
  b = y + size;
  oled->drawLine( x, b, x-3, b-3, true );
  oled->drawLine( x, b, x+3, b-3, true );

}

void SwOSScreenSlider::drawVertical( void ) {

  // <
  int8_t b = x - size;
  oled->drawLine( b, y, b+3, y-3, true );
  oled->drawLine( b, y, b+3, y+3, true );

  // oled->write( label, b-2, y-3, FTSWARM_ALIGNRIGHT, true, false );

  // >
  b = x + size;
  oled->drawLine( b, y, b-3, y-3, true );
  oled->drawLine( b, y, b-3, y+3, true );

}

/***************************************************
 *
 * SwOSScreenSelector - a combo box
 *
 ***************************************************/

SwOSSelectorItem::SwOSSelectorItem( int8_t id, const char *text, SwOSSelectorItem *prev ) {

  this->id = id;
  bzero( this->text, sizeof( text ) );
  strncpy( this->text, text, sizeof( text ) - 1 );
  next = NULL;
  this->prev = prev;

}

SwOSSelectorItem::~SwOSSelectorItem( ) {
  if ( next ) delete next;
}

void SwOSSelectorItem::add( int8_t id, const char *text ) {

  if (next) next->add( id, text );
  else      next = new SwOSSelectorItem( id, text, this );

}

SwOSScreenSelector::SwOSScreenSelector( uint8_t x, uint8_t y ) : SwOSScreenObj( "", x, y, FTSWARM_ALIGNLEFT ) {

}

SwOSScreenSelector::~SwOSScreenSelector( ) {

  if ( items ) delete items;

}

void SwOSScreenSelector::add( int8_t id, const char *text ) {

  if (items) items->add( id, text );
  else       items = new SwOSSelectorItem( id, text, NULL );

}

void SwOSScreenSelector::draw( bool inverted ) {

  if (!active) return;

  SwOSScreenObj::draw( inverted );
  if (active->prev) oled->write( active->prev->text, x, y-8, align, true, false );
                    oled->write( active->text,       x, y,   align, true, true );
  if (active->next) oled->write( active->next->text, x, y+8, align, true, false );

}

/***************************************************
 *
 *   SwOSScreen
 *
 ***************************************************/

#define OLEDLOWERLINE 38
#define OLEDWIDTH     128

SwOSScreen::SwOSScreen( SwOSScreen *parent ) {
  
  this->parent = parent;
  
  obj[0] = new SwOSScreenObj( NULL, 0,                OLEDLOWERLINE, FTSWARM_ALIGNLEFT   ); /* FTSWARM_S1 */ 
  obj[1] = new SwOSScreenObj( NULL, 41,               OLEDLOWERLINE, FTSWARM_ALIGNCENTER ); /* FTSWARM_S2 */ 
  obj[2] = new SwOSScreenObj( NULL, OLEDWIDTH -1 -41, OLEDLOWERLINE, FTSWARM_ALIGNCENTER ); /* FTSWARM_S3 */ 
  obj[3] = new SwOSScreenObj( NULL, OLEDWIDTH -1,     OLEDLOWERLINE, FTSWARM_ALIGNRIGHT  ); /* FTSWARM_S4 */ 
  obj[4] = new SwOSScreenObj( NULL, 0,                1,             FTSWARM_ALIGNLEFT   ); /* FTSWARM_F1 */ 
  obj[5] = new SwOSScreenObj( NULL, OLEDWIDTH -1,     1,             FTSWARM_ALIGNRIGHT  ); /* FTSWARM_F2 */ 
  obj[6] = new SwOSScreenObj( NULL, 48,               16,            FTSWARM_ALIGNCENTER ); /* FTSWARM_J1 */ 
  obj[7] = new SwOSScreenObj( NULL, OLEDWIDTH -1 -48, 16,            FTSWARM_ALIGNCENTER ); /* FTSWARM_J2 */ 
  
}

void SwOSScreen::draw( void ) {

  // cls
  oled->clearDisplay( );
      
  // set useful default values
  oled->setTextColor(true, false);   // Draw white text
  oled->cp437(true);                 // Use full 256 char 'Code Page 437' font

  for (uint8_t i=0; i<8; i++ ) if (obj[i]) obj[i]->draw();

}

/***************************************************
 *
 *   SwOSSplashScreen
 *
 ***************************************************/

 void SwOSSplashScreen::draw( void ) {

  SwOSScreen::draw();
  
  // Logo
  oled->setTextSize( 3, 3 );
  oled->write( "ftSwarm", oled->getWidth()/2, 0, FTSWARM_ALIGNCENTER, true, false );
  
  // hostname & version
  oled->setTextSize(1,1);            
  char line[100];
  sprintf( line, "%s %s", nvs.swarmName, SWOSVERSION );
  oled->write( line, oled->getWidth()/2, 32, FTSWARM_ALIGNCENTER, true, false );
 
  // additional default values
  oled->setTextSize(1, 1);

  for ( uint8_t i=0; i<8; i++ ) if (obj[i]) obj[i]->draw( );

}

void SwOSSplashScreen::operate( void ) {

  // change to Main Screen after 5 seconds
  if ( millis()-startTime > 5000L ) screenManager.newScreen( new SwOSMainScreen( NULL ), true );

}

bool SwOSSplashScreen::eventHandler( FtSwarmToggle_t toggle, SwOSIOType_t ioType, uint8_t port ) {
  
  // some pressed a key, so I change to the main screen
  screenManager.newScreen( new SwOSMainScreen( NULL ), true );
  return true;

}

/***************************************************
 *
 * SwOSFactoryResetScreen 
 * Ask user to reset controller to factory setting
 *
 ***************************************************/

SwOSFactoryResetScreen::SwOSFactoryResetScreen( SwOSScreen *parent ):SwOSScreen( parent ) {

  // set buttons
  obj[FTSWARM_S2]->setLabel( "YES" );
  obj[FTSWARM_S3]->setLabel( "NO" );

}

void SwOSFactoryResetScreen::draw( void ) {

  SwOSScreen::draw();
  oled->write( "Reset controller to", oled->getWidth()/2, 7, FTSWARM_ALIGNCENTER, true, false );  
  oled->write( "factory settings?", oled->getWidth()/2, 16, FTSWARM_ALIGNCENTER, true, false );  

}

bool SwOSFactoryResetScreen::eventHandler( FtSwarmToggle_t toggle, SwOSIOType_t ioType, uint8_t port ) {

  if ( ioType == SWOSIO_BUTTON ) {

    obj[port]->draw( toggle == FTSWARM_TOGGLEUP );

    // YES
    if ( ( port == FTSWARM_S2 ) && ( toggle == FTSWARM_TOGGLEDOWN ) ) myOSSwarm.factoryReset();

    // NO
    if ( ( port == FTSWARM_S3 ) && ( toggle == FTSWARM_TOGGLEDOWN ) ) screenManager.newScreen( new SwOSMainScreen( NULL ), true );

  }

  return false;

}

/***************************************************
 *
 * SwOSTestScreen
 *
 ***************************************************/

SwOSTestScreen::SwOSTestScreen( SwOSScreen *parent ) : SwOSScreen( parent) {

}

void SwOSTestScreen::draw( void ) {

}

bool SwOSTestScreen::eventHandler( FtSwarmToggle_t toggle, SwOSIOType_t ioType, uint8_t port ) {
  
}


/***************************************************
 *
 *   SwOSMainScreen
 *
 ***************************************************/

SwOSMainScreen::SwOSMainScreen( SwOSScreen *parent ) : SwOSScreen( parent ) {

  for ( uint8_t i=0; i<8; i++ ) if ( obj[i] ) obj[i]->setLabel( nvs.oledLabel[nvs.activeEventConfig][i] );

  obj[FTSWARM_S4]->setLabel( "SET" );

}

void SwOSMainScreen::joystick( char *lr, char*fb, int8_t x, int8_t y, bool left ) {  

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

void SwOSMainScreen::draw( void ) {

  SwOSScreen::draw();

  joystick( nvs.oledLabel[nvs.activeEventConfig][SWOSLABEL_JOY1LR], nvs.oledLabel[nvs.activeEventConfig][SWOSLABEL_JOY1FB], 48,     20, true );
  joystick( nvs.oledLabel[nvs.activeEventConfig][SWOSLABEL_JOY2LR], nvs.oledLabel[nvs.activeEventConfig][SWOSLABEL_JOY2FB], 128-48, 20, false );

  char cfg[5];
  sprintf( cfg, "#%d", nvs.activeEventConfig+1 );
  oled->write( cfg, 64, 38, FTSWARM_ALIGNCENTER, true, false );

}

bool SwOSMainScreen::eventHandler( FtSwarmToggle_t toggle, SwOSIOType_t ioType, uint8_t port ) {

  if ( ioType == SWOSIO_BUTTON ) {

    obj[port]->draw( toggle == FTSWARM_TOGGLEUP );

    if ( port == FTSWARM_S4 ) {

      // set new screen and all done
      if ( toggle == FTSWARM_TOGGLEDOWN ) screenManager.newScreen( new SwOSChooseConfigScreen( this ), false );
      return true;

    }

  }

  return false;

}

void SwOSMainScreen::setLabel( SwOSIOType_t ioType, uint8_t port, const char *label ) {

  if ( ( ioType == SWOSIO_BUTTON ) && ( port < 8 ) && ( obj[port] ) ) {
    
    obj[port]->setLabel( label );
    obj[port]->draw();

  }

}

/***************************************************
 *
 *   SwOSChooseConfigScreen
 *
 ***************************************************/

SwOSChooseConfigScreen::SwOSChooseConfigScreen( SwOSScreen *parent ) : SwOSScreen( parent ) {

  char label[4];

  // label S1..S4 with #1..#4
  for ( uint8_t i=FTSWARM_S1; i<=FTSWARM_S4; i++) {
    sprintf( label, "#%d", i-FTSWARM_S1+1 );
    obj[i]->setLabel( label );
  }

}

 void SwOSChooseConfigScreen::draw( void ) {

  SwOSScreen::draw();
  oled->write( "Configuration?", 64, 20, FTSWARM_ALIGNCENTER, true, false );
  
}

bool SwOSChooseConfigScreen::eventHandler( FtSwarmToggle_t toggle, SwOSIOType_t ioType, uint8_t port ) {

  if ( ioType == SWOSIO_BUTTON ) {

    obj[ port ]->draw( toggle == FTSWARM_TOGGLEUP );

    if ( toggle == FTSWARM_TOGGLEDOWN ) {

      // change config asynchronous
      SwOSCom setConfig( myOSSwarm.Ctrl[0]->macAddr, myOSSwarm.Ctrl[0]->serialNumber, CMD_SETACTIVECONFIG );
      setConfig.data.configCmd.config = port;
      xQueueSend( myOSNetwork.recvNotification, &setConfig, ESPNOW_MAXDELAY );

      // replace myself with the previous screen
      screenManager.newScreen( parent, true );

      return true;

    }

  }

  return false;
  
}

/***************************************************
 *
 *   SwOS404Screen
 *
 ***************************************************/

void SwOS404Screen::draw( void ) {

  SwOSScreen::draw();
  oled->setTextSize( 3, 3 );
  oled->write( "404",             64, 0,  FTSWARM_ALIGNCENTER, true, false );
  oled->setTextSize( 1, 1 );
  oled->write( "Page not found.", 64, 30, FTSWARM_ALIGNCENTER, true, false );
  
}

/***************************************************
 *
 *   SwOSScreenManager
 *
 ***************************************************/

void SwOSScreenManager::operate( void ) {

  if (next) {
    // a new screen shall to be used
    // based on the today's sequence on working on events, it's not needed to lock the device. Maybe in the future. 

    // save old one
    SwOSScreen *outdated = active;

    // replace by new one
    active = next;
    next   = NULL;

    // draw new one
    active->draw();

    // delete outdated
    if ( ( outdated ) && ( autoCleanUp ) ) {
      delete( outdated );
      autoCleanUp = false;
    }

  }

  // operate avtive screen
  if (active) active->operate();

}

void SwOSScreenManager::draw( void ) {

  if ( active ) active->draw();

}

bool SwOSScreenManager::eventHandler( FtSwarmToggle_t toggle, SwOSIOType_t ioType, uint8_t port ) { 
  
  if (active) return active->eventHandler( toggle, ioType, port ); 

  return false;

}

void SwOSScreenManager::newScreen( SwOSScreen *newScreen, bool autoCleanUp ) { 

  SwOSScreen *outdated = NULL;

  if ( next ) {

    // Ran in a race condition, last change did not take place yet.
    SWARM_LOG_INFO( "SwOSScreenManager: New screen appears before displaying the last one");

    // mark to be deleted
    SwOSScreen *outdated = next;

  }

  if ( newScreen ) next = newScreen;
  else next = new SwOS404Screen( NULL ); 

  // release old one
  if ( ( outdated ) && ( autoCleanUp ) ) { 
    delete( outdated ); 
    this->autoCleanUp = false; 
  } else {
    this->autoCleanUp = autoCleanUp;
  }

}

void SwOSScreenManager::setLabel( SwOSIOType_t ioType, uint8_t port, const char *label ) {

  if (active) active->setLabel( ioType, port, label );

}
