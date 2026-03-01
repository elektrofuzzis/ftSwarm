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

SwOSScreenObj::SwOSScreenObj( uint8_t id, SwOSIO *io, SwOSScreen *parent, const char *label, int8_t x, int8_t y, FtSwarmAlign_t align ) {
  
  this->id     = id;
  this->io     = io;
  this->parent = parent;
  this->x      = x;
  this->y      = y;
  this->align  = align;

  setLabel( label );

}

SwOSScreenObj::~SwOSScreenObj() {

  if (io) io->unsubscribe( this );

  if ( label ) free( label );

  if ( next ) delete( next );

}

void SwOSScreenObj::activate( void ) {

  if (!io) return;
  
  io->subscribe( this ); 
  value = io->getValueI32();

}

void SwOSScreenObj::deactivate( void ) {

  if (!io) return;
  
  io->unsubscribe( this ); 

}
void SwOSScreenObj::unregister( SwOSIO *io ) {

  if ( this->io == io ) this->io = NULL;

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

  draw();

}

bool SwOSScreenObj::setValue( int32_t newValue ) {

  value = newValue;

  draw( );

  FtSwarmToggle_t toggle;

  if ( value > 0 ) toggle = FTSWARM_TOGGLEUP;
  else             toggle = FTSWARM_TOGGLEDOWN;

  if (parent) return parent->eventHandlerCallback( toggle, id );

  return false;

}

/***************************************************
 *
 * SwOSScreenButton - button class
 *
 ***************************************************/

void SwOSScreenButton::draw( void ) {

  if ( ( label ) && ( label[0] != '\0' )  ) oled->write( label, x, y, align, true, value > 0 );

} 

/***************************************************
 *
 * SwOSScreenObjXX - local Buttons
 *
 ***************************************************/

SwOSScreenS1::SwOSScreenS1( SwOSScreen *parent, const char *label ) : SwOSScreenButton( FTSWARM_S1, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_S1), parent, label, 0, OLEDLOWERLINE, FTSWARM_ALIGNLEFT ) {};
SwOSScreenS2::SwOSScreenS2( SwOSScreen *parent, const char *label ) : SwOSScreenButton( FTSWARM_S2, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_S2), parent, label, 41, OLEDLOWERLINE, FTSWARM_ALIGNCENTER ) {};
SwOSScreenS3::SwOSScreenS3( SwOSScreen *parent, const char *label ) : SwOSScreenButton( FTSWARM_S3, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_S3), parent, label, OLEDWIDTH -1 -41, OLEDLOWERLINE, FTSWARM_ALIGNCENTER ) {};
SwOSScreenS4::SwOSScreenS4( SwOSScreen *parent, const char *label ) : SwOSScreenButton( FTSWARM_S4, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_S4), parent, label, OLEDWIDTH -1, OLEDLOWERLINE, FTSWARM_ALIGNRIGHT ) {};
SwOSScreenF1::SwOSScreenF1( SwOSScreen *parent, const char *label ) : SwOSScreenButton( FTSWARM_F1, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_F1), parent, label, 0, -16, FTSWARM_ALIGNLEFT ) {};
SwOSScreenF2::SwOSScreenF2( SwOSScreen *parent, const char *label ) : SwOSScreenButton( FTSWARM_F2, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_F2), parent, label, OLEDWIDTH -1, -16, FTSWARM_ALIGNRIGHT ) {};
SwOSScreenJ1::SwOSScreenJ1( SwOSScreen *parent, const char *label ) : SwOSScreenButton( FTSWARM_J1, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_J1), parent, label, 48, 16, FTSWARM_ALIGNCENTER ) {};
SwOSScreenJ2::SwOSScreenJ2( SwOSScreen *parent, const char *label ) : SwOSScreenButton( FTSWARM_J2, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_J2), parent, label, OLEDWIDTH -1 -48, 16, FTSWARM_ALIGNCENTER ) {};

/***************************************************
 *
 * SwOSScreenJoystickPoti - Helper class
 *
 ***************************************************/

SwOSScreenJoystickPoti::SwOSScreenJoystickPoti(uint8_t id, SwOSIO *io, SwOSScreen *parent, const char *label, int8_t x, int8_t y, FtSwarmAlign_t align ):SwOSScreenObj( id, io, parent, label, x, y, align ) {

}

bool SwOSScreenJoystickPoti::setValue( int32_t value ) {

  // new toggle
  FtSwarmToggle_t toggle = FTSWARM_NOTOGGLE;

  // store value
  this->value = value;

  // draw myself
  draw();

  // check on toggle
  if ( value != FTSWARM_NANI32 ) {

    // to avoid races after start
    if ( maxValue == FTSWARM_NANI32 ) maxValue = value;
    
    if      ( ( value > -25 ) && ( maxValue < -50 ) ) { toggle = FTSWARM_TOGGLEDOWN; maxValue = value; }
    else if ( ( value <  25 ) && ( maxValue >  50 ) ) { toggle = FTSWARM_TOGGLEUP;   maxValue = value; }
    else if ( abs(value) > abs ( maxValue ) )         {                              maxValue = value; }

  }

  // call eventhandler on a real toggle only
  if ( ( parent ) && ( toggle != FTSWARM_NOTOGGLE ) ) parent->eventHandlerCallback( toggle, id );

  // always true to catch all joystick events
  return true;

}
    
/***************************************************
 *
 *   SwOSScreenCombobox
 *
 ***************************************************/

SwOSScreenCombobox::SwOSScreenCombobox( uint8_t id, SwOSIO *io, SwOSScreen *parent, const char *label, int8_t x, int8_t y, uint8_t size, uint8_t direction ) : SwOSScreenObj( id, io, parent, label, x, y, FTSWARM_ALIGNCENTER ) {

  this->size      = size;
  this->direction = direction;

}

void SwOSScreenCombobox::draw( void ) {
  
  SwOSScreenObj::draw( );

  if (direction==HORIZONTAL) drawHorizontal();
  else                       drawVertical();

}

void SwOSScreenCombobox::drawHorizontal( void ) {

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

void SwOSScreenCombobox::drawVertical( void ) {

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
  bzero( this->text, sizeof( this->text ) );
  strncpy( this->text, text, sizeof( this->text ) - 1 );
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

SwOSScreenSelector::SwOSScreenSelector( uint8_t id, SwOSIO* io, SwOSScreen *parent, int8_t x, int8_t y ) : SwOSScreenObj( id, io, parent, "", x, y, FTSWARM_ALIGNLEFT ) {

}

SwOSScreenSelector::~SwOSScreenSelector( ) {

  if ( items ) delete items;

}

void SwOSScreenSelector::add( int8_t id, const char *text ) {

  if (items) items->add( id, text );
  else       items = new SwOSSelectorItem( id, text, NULL );

  if (!active) active = items;

}

void SwOSScreenSelector::draw( void ) {
  
  if (!active) return;

  SwOSScreenObj::draw( );
  if (active->prev) oled->write( active->prev->text, x, y-8, align, true, false );
                    oled->write( active->text,       x, y,   align, true, true );
  if (active->next) oled->write( active->next->text, x, y+8, align, true, false );

}

/***************************************************
 *
 *   SwOSScreen
 *
 ***************************************************/

SwOSScreen::SwOSScreen( SwOSScreen *parent, const char *title ) {
  
  this->parent = parent;
  
  if ( title ) {

    this->title = (char *) calloc( strlen( title)+1, sizeof( char ) );
    strncpy( this->title, title, strlen( title ) );

  }
  
}

SwOSScreen::~SwOSScreen() {

  if ( objects ) delete objects;

  if ( title ) free( title );

}

void SwOSScreen::add( SwOSScreenObj *newObject ) {

  if (!newObject) return;

  if (objects) {
    newObject->next = objects;
    objects = newObject;

  } else {
    objects = newObject;
  }

}

void SwOSScreen::draw( void ) {

  // cls
  oled->clearDisplay( true );
      
  // set useful default values
  oled->setTextColor(true, false);   // Draw white text
  oled->cp437(true);                 // Use full 256 char 'Code Page 437' font

  if ( title ) {
    oled->drawRect( 16, -16, oled->getWidth() - 32, 8, true, false );
    oled->write( title, OLEDWIDTH / 2, -16, FTSWARM_ALIGNCENTER, true, false );
  }

  // cool line
  oled->drawLine( 0, -5, OLEDWIDTH, -5, true );

  // register my objects and draw them
  SwOSScreenObj *obj = objects;
  while (obj) {
    obj->draw();
    obj = obj->next;
  }

}

void SwOSScreen::activate( void ) {
  
  SwOSScreenObj *obj = objects;
  while (obj) {
    obj->activate();
    obj = obj->next;
  }

}

void SwOSScreen::deactivate( void ) {
  
  SwOSScreenObj *obj = objects;
  while (obj) {
    obj->deactivate();
    obj = obj->next;
  }

}

/***************************************************
 *
 * SwOSScreenSlider - Screen Slide show
 *
 ***************************************************/

#define SWOSJOY1LR 99
 
SwOSScreenSlider::SwOSScreenSlider( SwOSScreen *parent, const char *title, SwOSScreenSlider *next ) : SwOSScreen( parent, title ) {

  this->next = next;

  // add myself as previous of next
  if (next) next->prev = this;

  add( new SwOSScreenJoystickPoti( SWOSJOY1LR, myOSSwarm.Ctrl[0]->getIO( SWOSIO_JOYSTICK_POTI, FTSWARM_HAL_FIRSTJPOTI ), this, "", 0, 0, FTSWARM_ALIGNLEFT ) );
  add( new SwOSScreenF2( this, "^" ) );

}

SwOSScreenSlider::~SwOSScreenSlider() {

  // cleanup chain in both directions
  if ( prev ) { prev->next = NULL; delete prev; }
  if ( next ) { next->prev = NULL; delete next; }

}

void SwOSScreenSlider::draw( void ) {

  SwOSScreen::draw();

  // top slider
  uint8_t cp = countPrev();
  uint8_t cn = countNext();
  uint8_t c  = cp + cn + 1;

  if ( c > 1 ) {
    uint8_t size = OLEDWIDTH / c;
    oled->drawLine( 0, -3, OLEDWIDTH, -3, false );
    oled->drawLine( cp*size, -3, (cp+1)*size, -3, true );
  }

}

bool SwOSScreenSlider::eventHandlerCallback( FtSwarmToggle_t toggle, uint8_t id ) {

  // catch all joystick stuff
  if ( id == SWOSJOY1LR ) {
  
    // switch left?
    if ( ( toggle == FTSWARM_TOGGLEDOWN ) && ( prev ) ) screenManager.newScreen( prev, false );

    // switch right?
    if ( ( toggle == FTSWARM_TOGGLEUP ) && ( next ) )   screenManager.newScreen( next, false );

    return true;

  }

  // back
  if ( id == FTSWARM_F2 )  {
    
    if ( ( toggle == FTSWARM_TOGGLEDOWN ) && ( parent ) ) screenManager.newScreen( parent, true );

    return true;

  }

  // other events
  return false;

}

uint8_t SwOSScreenSlider::countPrev( void ) {

  uint8_t count = 0;
  SwOSScreenSlider *screen = prev;

  while( screen ) { count++; screen = screen->prev; }

  return count;

}

uint8_t SwOSScreenSlider::countNext( void ) {

  uint8_t count = 0;
  SwOSScreenSlider *screen = next;

  while( screen ) { count++; screen = screen->next; }

  return count;

}

/***************************************************
 *
 *   SwOSScreenChooseConfig
 *
 ***************************************************/

SwOSScreenChooseConfig::SwOSScreenChooseConfig( SwOSScreen *parent, SwOSScreenSlider *next  ) : SwOSScreenSlider( parent, "Configuration", next ) {

  add( new SwOSScreenS1( this, "#1" ) );
  add( new SwOSScreenS2( this, "#2" ) );
  add( new SwOSScreenS3( this, "#3" ) );
  add( new SwOSScreenS4( this, "#4" ) );

}

 void SwOSScreenChooseConfig::draw( void ) {

  SwOSScreenSlider::draw();
  oled->write( "Choose new", oled->getWidth()/2, 7, FTSWARM_ALIGNCENTER, true, false );  
  oled->write( "configuration", oled->getWidth()/2, 16, FTSWARM_ALIGNCENTER, true, false );  
  
}

bool SwOSScreenChooseConfig::eventHandlerCallback( FtSwarmToggle_t toggle, uint8_t id ) {

  if ( SwOSScreenSlider::eventHandlerCallback( toggle, id ) ) return true;

  if ( toggle == FTSWARM_TOGGLEDOWN ) {

    // change config asynchronous
    SwOSCom setConfig( myOSSwarm.Ctrl[0]->macAddr, myOSSwarm.Ctrl[0]->serialNumber, CMD_SETACTIVECONFIG );
    setConfig.data.configCmd.config = id - FTSWARM_S1;
    xQueueSend( myOSNetwork.recvNotification, &setConfig, ESPNOW_MAXDELAY );

    // replace myself with the previous screen
    screenManager.newScreen( parent, true );

    return true;

  }

  return false;
  
}

/***************************************************
 *
 *   SwOSScreenWifi
 *
 ***************************************************/

SwOSScreenWifi::SwOSScreenWifi( SwOSScreen *parent, SwOSScreenSlider *next  ) : SwOSScreenSlider( parent, "Wifi", next ) {
}

/***************************************************
 *
 *   SwOSScreenSwarm
 *
 ***************************************************/

SwOSScreenSwarm::SwOSScreenSwarm( SwOSScreen *parent, SwOSScreenSlider *next  ) : SwOSScreenSlider( parent, "Swarm", next ) {
}

/***************************************************
 *
 * SwOSFactoryResetScreen 
 * Ask user to reset controller to factory setting
 *
 ***************************************************/

SwOSScreenFactoryReset::SwOSScreenFactoryReset( SwOSScreen *parent, SwOSScreenSlider *next ) : SwOSScreenSlider( parent, "Factory Reset", next ) {

  // set buttons
  add( new SwOSScreenS2( this, "YES" ) );
  add( new SwOSScreenS3( this, "NO" ) );
  
}

void SwOSScreenFactoryReset::draw( void ) {

  SwOSScreenSlider::draw();
  oled->write( "Reset controller to", oled->getWidth()/2, 7, FTSWARM_ALIGNCENTER, true, false );  
  oled->write( "factory settings?", oled->getWidth()/2, 16, FTSWARM_ALIGNCENTER, true, false );  

}

bool SwOSScreenFactoryReset::eventHandlerCallback( FtSwarmToggle_t toggle, uint8_t id ) {

  if ( SwOSScreenSlider::eventHandlerCallback( toggle, id ) ) return true;

  // YES
  if ( id == FTSWARM_S2 ) {
    if ( toggle == FTSWARM_TOGGLEDOWN )  myOSSwarm.factoryReset();
    return true;
  }

  // NO
  if ( id == FTSWARM_S3 ) {
    if ( toggle == FTSWARM_TOGGLEDOWN ) screenManager.newScreen( new SwOSMainScreen( NULL, "Main" ), true );
    return true;
  }

  return false;

}

/***************************************************
 *
 * SwOSTestScreen
 *
 ***************************************************/

SwOSTestScreen::SwOSTestScreen( SwOSScreen *parent ) : SwOSScreen( parent, "Test" ) {

  /*
  SwOSScreenSelector *selector = new SwOSScreenSelector( 25, 24 );
  selector->add( 1, "Config" );
  selector->add( 2, "Factory Reset" );
  selector->add( 3, "Nase" );

  add( selector );

  */

}

bool SwOSTestScreen::eventHandlerCallback( FtSwarmToggle_t toggle, uint8_t id ) {

  printf( "eventHandle %d %d\n", toggle, id );
  return true;
  
}


/***************************************************
 *
 *   SwOSMainScreen
 *
 ***************************************************/

SwOSMainScreen::SwOSMainScreen( SwOSScreen *parent, const char *title ) : SwOSScreen( parent, title ) {

  add( new SwOSScreenS1( this, nvs.oledLabel[nvs.activeEventConfig][ SWOSLABEL_S1 ] ) );
  add( new SwOSScreenS2( this, nvs.oledLabel[nvs.activeEventConfig][ SWOSLABEL_S2 ] ) );
  add( new SwOSScreenS3( this, nvs.oledLabel[nvs.activeEventConfig][ SWOSLABEL_S3 ] ) );
  add( new SwOSScreenS4( this, "SET" ) );
  add( new SwOSScreenJ1( this, nvs.oledLabel[nvs.activeEventConfig][ SWOSLABEL_J1 ] ) );
  add( new SwOSScreenJ2( this, nvs.oledLabel[nvs.activeEventConfig][ SWOSLABEL_J2 ] ) );
  add( new SwOSScreenF1( this, nvs.oledLabel[nvs.activeEventConfig][ SWOSLABEL_F1 ] ) );
  add( new SwOSScreenF2( this, nvs.oledLabel[nvs.activeEventConfig][ SWOSLABEL_F2 ] ) );

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

  // Members
  uint8_t members = myOSSwarm.members();
  if ( members > 0) {
    char m[3];
    sprintf( m, "%d", members );
    oled->write( m, OLEDWIDTH-1, -YELLOWPIXELS, FTSWARM_ALIGNRIGHT, false, false );
  }

  // Kelda
  if (myOSSwarm.Ctrl[0]->IAmKelda) oled->write( "K", 0, -YELLOWPIXELS, FTSWARM_ALIGNLEFT, false, false );

  joystick( nvs.oledLabel[nvs.activeEventConfig][SWOSLABEL_JOY1LR], nvs.oledLabel[nvs.activeEventConfig][SWOSLABEL_JOY1FB], 48,     20, true );
  joystick( nvs.oledLabel[nvs.activeEventConfig][SWOSLABEL_JOY2LR], nvs.oledLabel[nvs.activeEventConfig][SWOSLABEL_JOY2FB], 128-48, 20, false );

  char cfg[5];
  sprintf( cfg, "#%d", nvs.activeEventConfig+1 );
  oled->write( cfg, 64, 38, FTSWARM_ALIGNCENTER, true, false );

}

bool SwOSMainScreen::eventHandlerCallback( FtSwarmToggle_t toggle, uint8_t id ) {

  if ( id == FTSWARM_S4 ) {

      // set new screen and all done
      if ( toggle == FTSWARM_TOGGLEDOWN ) {

        SwOSScreenSlider *config = ( SwOSScreenSlider * ) new SwOSScreenChooseConfig( this, 
                                                          new SwOSScreenWifi( this, 
                                                          new SwOSScreenSwarm( this, 
                                                          new SwOSScreenFactoryReset( this, NULL ) ) ) );

        screenManager.newScreen( (SwOSScreen *) config, false );

        return true;
      }

  }

  return false;

}

/***************************************************
 *
 *   SwOSSplashScreen
 *
 ***************************************************/

 SwOSSplashScreen::SwOSSplashScreen( SwOSScreen *parent, const char *title ):SwOSScreen( parent, title ) { 
  
  startTime = millis();

  add( new SwOSScreenS1( this, "" ) );
  add( new SwOSScreenS2( this, "" ) );
  add( new SwOSScreenS3( this, "" ) );
  add( new SwOSScreenS4( this, "" ) );
  add( new SwOSScreenJ1( this, "" ) );
  add( new SwOSScreenJ2( this, "" ) );
  add( new SwOSScreenF1( this, "" ) );
  add( new SwOSScreenF2( this, "" ) );

}

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

}

void SwOSSplashScreen::operate( void ) {

  // change to Main Screen after 5 seconds
  if ( millis()-startTime > 5000L ) screenManager.newScreen( new SwOSMainScreen( NULL, title ), true );
  // if ( millis()-startTime > 1000L ) screenManager.newScreen( new SwOSTestScreen( NULL ), true );

}

bool SwOSSplashScreen::eventHandlerCallback( FtSwarmToggle_t toggle, uint8_t id ) {
  
  // some pressed a key, so I change to the main screen
  screenManager.newScreen( new SwOSMainScreen( NULL, title ), true );
  return true;

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

    // deactivate old screen objects
    if (active) active->deactivate();

    // replace by new one
    active = next;
    next   = NULL;

    // draw and activate new one
    active->draw();
    active->activate();

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
