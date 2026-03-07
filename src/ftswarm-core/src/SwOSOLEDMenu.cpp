/*
 * SwOSOLEDMenu.cpp
 *
 * OLED on Screen Menus
 * 
 * (C) 2025 Christian Bergschneider & Stefan Fuss
 * 
 */

#include <WiFi.h>

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

SwOSScreenObj::SwOSScreenObj( uint8_t id, SwOSIO *io, SwOSScreen *parent, const char *label, int16_t x, int16_t y, FtSwarmAlign_t align ) {
  
  this->id     = id;
  this->io     = io;
  this->parent = parent;
  this->x      = x;
  this->y      = y;
  this->align  = align;

  setLabel( label, false );

}

SwOSScreenObj::~SwOSScreenObj() {

  if (io) io->unsubscribe( this );

  if ( label ) free( label );

  if ( next ) delete( next );

}

void SwOSScreenObj::add( SwOSScreenObj *newObject) {

  if (next) next->add( newObject );
  else      next = newObject;

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

void SwOSScreenObj::setLabel( const char *label, bool autoDraw ) {

  if ( this->label) {
    delete this->label;
  }

  if ( label ) {

    this->label = (char *)calloc( strlen(label) + 1, sizeof( char ) );
    strcpy( this->label, label );

  } else {

    this->label = NULL;
  
  }

  if ( autoDraw) draw();

}

bool SwOSScreenObj::setValue( int32_t newValue ) {

  value = newValue;

  draw( );

  FtSwarmScreenEvent_t event;

  if ( value > 0 ) event = FTSWARM_SCREENEVENT_UP;
  else             event = FTSWARM_SCREENEVENT_DOWN;

  if ( (parent) && ( lastEvent != FTSWARM_SCREENEVENT_NONE ) ) return parent->eventHandlerCallback( event, id );

  lastEvent = event;

  return false;

}

/***************************************************
 *
 * SwOSScreenSelectable - label + text
 *
 ***************************************************/

SwOSScreenSelectable::SwOSScreenSelectable( uint8_t id, SwOSScreen *parent, const char *label, const char *text, int16_t x, int16_t y, int16_t widthLabel, int16_t widthText ) : SwOSScreenObj( id, NULL, parent, label, x, y, FTSWARM_ALIGNLEFT ) {

  this->widthLabel = widthLabel;
  this->widthText  = widthText;

  setText( text, false );

}

void SwOSScreenSelectable::setText( const char *text, bool autoDraw ) {

  if ( this->text) {
    delete this->text;
  }

  if ( text ) {

    this->text = (char *)calloc( strlen(text) + 1, sizeof( char ) );
    strcpy( this->text, text );

  } else {

    this->text = NULL;
  
  }

  if (autoDraw) draw();

}

void SwOSScreenSelectable::draw( void ) {

  if (!visible) return;

  oled->writeRectangle( label, x,                  y, widthLabel, 9, align, true, false );
  oled->writeRectangle( text,  x + widthLabel + 1, y, widthText,  9, align, true, false );

}

void SwOSScreenSelectable::select( void ) {

  if (!visible) return;

  oled->writeRectangle( text,  x + widthLabel + 1, y, widthText,  9, align, true, true );

}

void SwOSScreenSelectable::setVisible( bool visible ) {

  this->visible = visible;

  if (visible) draw();
  else         oled->writeRectangle( text,  x + widthLabel + 1, y, widthText,  9, align, true, false );

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

SwOSScreenJoystickPoti::SwOSScreenJoystickPoti(uint8_t id, SwOSIO *io, SwOSScreen *parent, const char *label, int16_t x, int16_t y, FtSwarmAlign_t align ):SwOSScreenObj( id, io, parent, label, x, y, align ) {

}

bool SwOSScreenJoystickPoti::setValue( int32_t value ) {

  // new event
  FtSwarmScreenEvent_t event = FTSWARM_SCREENEVENT_NONE;

  // store value
  this->value = value;

  // draw myself
  draw();

  // check on event
  if ( value != FTSWARM_NANI32 ) {

    // to avoid races after start
    if ( maxValue == FTSWARM_NANI32 ) maxValue = value;
    
    if      ( ( value > -25 ) && ( maxValue < -50 ) ) { event = FTSWARM_SCREENEVENT_DOWN; maxValue = value; }
    else if ( ( value <  25 ) && ( maxValue >  50 ) ) { event = FTSWARM_SCREENEVENT_UP;   maxValue = value; }
    else if ( abs(value) > abs ( maxValue ) )         {                                   maxValue = value; }

  }

  // call eventhandler on a real event only
  if ( ( parent ) && ( event != FTSWARM_SCREENEVENT_NONE ) ) parent->eventHandlerCallback( event, id );

  // always true to catch all joystick events
  return true;

}
    
/***************************************************
 *
 *   SwOSScreenCombobox
 *
 ***************************************************/

SwOSScreenCombobox::SwOSScreenCombobox( uint8_t id, SwOSIO *io, SwOSScreen *parent, const char *label, int16_t x, int16_t y, uint8_t size, uint8_t direction ) : SwOSScreenObj( id, io, parent, label, x, y, FTSWARM_ALIGNCENTER ) {

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

SwOSScreenSelector::SwOSScreenSelector( uint8_t id, SwOSIO* io, SwOSScreen *parent, int16_t x, int16_t y ) : SwOSScreenObj( id, io, parent, "", x, y, FTSWARM_ALIGNLEFT ) {

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
    objects->add( newObject );

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

void SwOSScreen::close( bool autoClean ) { 
  
  screenManager.newScreen( parent, autoClean ); 

};

/***************************************************
 *
 * SwOSScreenSlider - Screen Slide show
 *
 ***************************************************/

#define SWOSJOY1LR 100
#define SWOSJOY1FB 101
 
SwOSScreenSlider::SwOSScreenSlider( SwOSScreen *parent, const char *title, SwOSScreenSlider *next ) : SwOSScreen( parent, title ) {

  this->next = next;

  // add myself as previous of next
  if (next) next->prev = this;

  add( new SwOSScreenJoystickPoti( SWOSJOY1LR, myOSSwarm.Ctrl[0]->getIO( SWOSIO_JOYSTICK_POTI, FTSWARM_HAL_FIRSTJPOTI   ), this, "", 0, 0, FTSWARM_ALIGNLEFT ) );
  add( new SwOSScreenJoystickPoti( SWOSJOY1FB, myOSSwarm.Ctrl[0]->getIO( SWOSIO_JOYSTICK_POTI, FTSWARM_HAL_FIRSTJPOTI+1 ), this, "", 0, 0, FTSWARM_ALIGNLEFT ) );
  add( new SwOSScreenESC( this ) );
  add( new SwOSScreenJ1( this, "" ) );

}

SwOSScreenSlider::~SwOSScreenSlider() {

  // cleanup chain in both directions
  if ( prev ) { prev->next = NULL; delete prev; }
  if ( next ) { next->prev = NULL; delete next; }

}

void SwOSScreenSlider::add( SwOSScreenObj *newObject) {

  SwOSScreen::add( newObject );
  if ( (!selected) && (newObject) && (newObject->isSelectable()) ) selected = (SwOSScreenSelectable*) newObject;

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

  if (selected) selected->select();

}

bool SwOSScreenSlider::eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nparam, char *sparam ) {

  // catch all joystick stuff
  if ( id == SWOSJOY1LR ) {
  
    // switch left?
    if ( ( event == FTSWARM_SCREENEVENT_DOWN ) && ( prev ) ) screenManager.newScreen( prev, false );

    // switch right?
    if ( ( event == FTSWARM_SCREENEVENT_UP ) && ( next ) )   screenManager.newScreen( next, false );

    return true;

  }

  if ( id == SWOSJOY1FB ) {

    SwOSScreenObj *nextSelection = NULL;

    // go up ?
    if ( ( event == FTSWARM_SCREENEVENT_UP ) && ( selected ) ) {

      // get next selectable object
      SwOSScreenObj *next = objects;
      while (next) {

        if ( next->isSelectable() ) {
          if ( next == selected ) break;
          nextSelection=next;
        }

        next = next->next;

      }

    }

    // go down ?
    if ( ( event == FTSWARM_SCREENEVENT_DOWN ) && ( selected ) ) {

      // get next selectable object
      nextSelection = selected->next;
      while ( (nextSelection) && ( !nextSelection->isSelectable() ) ) nextSelection = nextSelection->next;

    }

    // found?
    if (nextSelection) {

      // set old one to normal
      selected->draw();

      // select new one
      selected = (SwOSScreenSelectable *) nextSelection;
      selected->select();

    }

    return true;

  }

  // select J1
  if ( id == FTSWARM_J1 ) {
    
    if ( event == FTSWARM_SCREENEVENT_DOWN ) eventHandlerCallback( FTSWARM_SCREENEVENT_OK, selected->getID() );

    return true;

  }

  // back
  if ( id == FTSWARM_F2 )  {
    
    if ( event == FTSWARM_SCREENEVENT_DOWN ) close();

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
 *   SwOSScreenChooseOption
 *
 ***************************************************/   

SwOSScreenChooseOption::SwOSScreenChooseOption( SwOSScreen *parent, uint8_t id, const char *title, const char *text1, const char *text2, 
                                                uint8_t value1, const char *option1, 
                                                uint8_t value2, const char *option2, 
                                                uint8_t value3, const char *option3, 
                                                uint8_t value4, const char *option4 ) : SwOSScreen( parent, title) {

  this->id = id;
  this->value[0] = value1;
  this->value[1] = value2;
  this->value[2] = value3;
  this->value[3] = value4;
  

  add( new SwOSScreenESC( this ) );
  if (option1) add( new SwOSScreenS1( this, option1) );
  if (option2) add( new SwOSScreenS2( this, option2) );
  if (option3) add( new SwOSScreenS3( this, option3) );
  if (option4) add( new SwOSScreenS4( this, option4) );

  bzero( text, 2*21 );
  if (text1) strncpy( text[0], text1, 20 );
  if (text2) strncpy( text[1], text2, 20 );

}

void SwOSScreenChooseOption::draw( void ) {

  SwOSScreen::draw();
  oled->write( text[0], oled->getWidth()/2, 7, FTSWARM_ALIGNCENTER, true, false );  
  oled->write( text[1], oled->getWidth()/2, 16, FTSWARM_ALIGNCENTER, true, false );  

}

bool SwOSScreenChooseOption::eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nparam, char *sparam ) {

  // if ( SwOSScreen::eventHandlerCallback( event, id,nparam, sparam ) ) return true;

  if ( event == FTSWARM_SCREENEVENT_DOWN ) {
  
    // S1..S4
    if ( id != FTSWARM_F2 ) parent->eventHandlerCallback( FTSWARM_SCREENEVENT_OK, this->id, value[id - FTSWARM_S1], NULL );

    // close
    close();

    return true;
  }

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

bool SwOSScreenChooseConfig::eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nparam, char *sparam ) {

  if ( SwOSScreenSlider::eventHandlerCallback( event, id, nparam, sparam ) ) return true;

  if ( event == FTSWARM_SCREENEVENT_DOWN ) {

    // change config asynchronous
    SwOSCom setConfig( myOSSwarm.Ctrl[0]->macAddr, myOSSwarm.Ctrl[0]->serialNumber, CMD_SETACTIVECONFIG );
    setConfig.data.configCmd.config = id - FTSWARM_S1;
    xQueueSend( myOSNetwork.recvNotification, &setConfig, ESPNOW_MAXDELAY );

    // replace myself with the previous screen
    close();

    return true;

  }

  return false;
  
}

/***************************************************
 *
 *   SwOSScreenWifi
 *
 ***************************************************/

#define SWOSSCREENWIFI_MODE      30
#define SWOSSCREENWIFI_SSID      31
#define SWOSSCREENWIFI_PASSWD    32
#define SWOSSCREENWIFI_CB_MODE   33
#define SWOSSCREENWIFI_CB_SSID   34
#define SWOSSCREENWIFI_CB_PASSWD 35
#define SWOSSCREENWIFI_CB_SAVE   36

SwOSScreenWifi::SwOSScreenWifi( SwOSScreen *parent, SwOSScreenSlider *next  ) : SwOSScreenSlider( parent, "Wifi", next ) {

  add( wifiMode = new SwOSScreenSelectable( SWOSSCREENWIFI_MODE,   this, "Mode", WIFI[nvs.wifiMode], 0,  1, 40, OLEDWIDTH-40 ) );
  add( wifiSSID = new SwOSScreenSelectable( SWOSSCREENWIFI_SSID,   this, "SSID", nvs.wifiSSID,       0, 10, 40, OLEDWIDTH-40 ) );
  add( wifiPwd  = new SwOSScreenSelectable( SWOSSCREENWIFI_PASSWD, this, "Passwd", "*****",          0, 19, 40, OLEDWIDTH-40 ) );
  add( S4 = new SwOSScreenS4( this, "" ) );

  wifiSSID->setVisible( ( nvs.wifiMode != wifiOFF ) );
  wifiPwd->setVisible ( ( nvs.wifiMode != wifiOFF ) );

}

bool SwOSScreenWifi::eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nparam , char *sparam ) {

  if ( SwOSScreenSlider::eventHandlerCallback( event, id, nparam, sparam ) ) return true;

  if ( event == FTSWARM_SCREENEVENT_OK ) {

    switch (id) {

      case SWOSSCREENWIFI_MODE:       screenManager.newScreen( new SwOSScreenChooseOption( this, SWOSSCREENWIFI_CB_MODE, "wifi mode", "Choose wifi mode.", "", wifiOFF, "off", wifiAP, "AP", wifiClient, "Client" ), false );
                                      break;

      case SWOSSCREENWIFI_CB_MODE:    nvs.wifiMode = (FtSwarmWifi_t) nparam;
                                      wifiMode->setText( WIFI[nvs.wifiMode] );
                                      wifiSSID->setVisible( ( nvs.wifiMode != wifiOFF ) );
                                      wifiPwd->setVisible ( ( nvs.wifiMode != wifiOFF ) );
                                      anythingChanged = true;
                                      break;

      case SWOSSCREENWIFI_SSID:       if ( nvs.wifiMode == wifiAP ) screenManager.newScreen( new SwOSScreenInput( this, SWOSSCREENWIFI_CB_SSID, "SSID", nvs.wifiSSID, 63 ), false );
                                      else                          screenManager.newScreen( new SwOSScreenWifiSSID( this, SWOSSCREENWIFI_CB_SSID ), false );
                                      break;

      case SWOSSCREENWIFI_CB_SSID:    if (sparam) { 
                                        strcpy( nvs.wifiSSID, sparam); 
                                        wifiSSID->setText( nvs.wifiSSID );
                                        anythingChanged = true;
                                      }
                                      break;

      case SWOSSCREENWIFI_PASSWD:     screenManager.newScreen( new SwOSScreenInput( this, SWOSSCREENWIFI_CB_PASSWD, "Password", "", 63 ), false );
                                      break;

      case SWOSSCREENWIFI_CB_PASSWD:  if ( sparam) {
                                        if ( ( strlen(sparam) > 0 ) && ( strlen(sparam) < 8 ) ) screenManager.newScreen( new SwOSScreenError( this, "Password needs at", "minimum 8 chars." ), false );
                                        else {
                                          strcpy( nvs.wifiPwd, sparam);
                                          anythingChanged = true;
                                        }
                                      }
                                      break;

      case SWOSSCREENWIFI_CB_SAVE:    if (nparam) nvs.saveAndRestart();
                                      break;

    }

    if ( anythingChanged ) S4->setLabel("Save");
    
    return true;
    
  }
  
  if ( event == FTSWARM_SCREENEVENT_DOWN ) {

    if ( ( id == FTSWARM_S4 ) && ( anythingChanged ) )  screenManager.newScreen( new SwOSScreenYesNo( this, SWOSSCREENWIFI_CB_SAVE, "wifi", "Save new settings", "and reboot?" ), false );
    return true;

  }

  return false;

}

/***************************************************
 *
 *   SwOSScreenWifiSSID
 *
 ***************************************************/

SwOSScreenWifiSSID::SwOSScreenWifiSSID( SwOSScreen *parent, uint8_t id  ): SwOSScreenSlider( parent, "SSID", NULL ) {

  this->id = id;
  scanStatus = WIFI_SCAN_RUNNING;
  WiFi.scanNetworks(true);

}

SwOSScreenWifiSSID::~SwOSScreenWifiSSID() {
  WiFi.scanDelete();
}

void SwOSScreenWifiSSID::draw( void ) {
  
  SwOSScreenSlider::draw();

  if ( scanStatus == WIFI_SCAN_RUNNING ) oled->write( "scanning...", OLEDWIDTH/2, 32, FTSWARM_ALIGNCENTER, false, false );

}

bool SwOSScreenWifiSSID::eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nparam, char *sparam ) {

  if ( SwOSScreenSlider::eventHandlerCallback( event, id, nparam, sparam ) ) return true;

  // must be a J1 click to select and close
  parent->eventHandlerCallback( FTSWARM_SCREENEVENT_OK, this->id, selected->getID(), (char *) selected->getText() );
  close( this->id );

  return true;

}

void SwOSScreenWifiSSID::operate( void ) {

  // done?
  if ( scanStatus > 0 ) return;

  // continue?
  scanStatus = WiFi.scanComplete();

  if ( scanStatus > 0 ) {

    for ( uint8_t i=0; i<scanStatus; i++ ) {

      bool unique = true;

      for ( uint8_t j=0; j<i; j++ ) {
        if ( WiFi.SSID(i) == WiFi.SSID(j) ) {
          unique = false;
          break;
        }
      }

      if ( unique ) add( new SwOSScreenSelectable( i, this, "", WiFi.SSID(i).c_str(), 0, i*9+1, 0, OLEDWIDTH ) );

    }

    draw();

  } else if ( scanStatus != WIFI_SCAN_RUNNING ) {

    close();

  }

};

/***************************************************
 *
 * SwOSScreenInput
 *
 ***************************************************/

void SwOSScreenInput::init(SwOSScreen *parent,  uint8_t id, const char *title, const char *param, uint8_t maxLength ) {

  this->id = id;

  this->maxLength = maxLength;
  input = (char *) calloc( this->maxLength+1, sizeof( char ) );

  uint8_t l = strlen( param );
  if ( l > this->maxLength ) l=this->maxLength;
  strncpy( input, param, l );

  setKeyboard( keyboard );

  add( new SwOSScreenJoystickPoti( SWOSJOY1LR, myOSSwarm.Ctrl[0]->getIO( SWOSIO_JOYSTICK_POTI, FTSWARM_HAL_FIRSTJPOTI   ), this, "", 0, 0, FTSWARM_ALIGNLEFT ) );
  add( new SwOSScreenJoystickPoti( SWOSJOY1FB, myOSSwarm.Ctrl[0]->getIO( SWOSIO_JOYSTICK_POTI, FTSWARM_HAL_FIRSTJPOTI+1 ), this, "", 0, 0, FTSWARM_ALIGNLEFT ) );
  add( new SwOSScreenJ1( this, "" ) );

  add( new SwOSScreenESC( this ) );

  if (!numKeyboard) add( S1 = new SwOSScreenS1( this, S1Label[keyboard] ) );

  add( new SwOSScreenS3( this, "<]") );
  add( new SwOSScreenS4( this, "OK") );

}

SwOSScreenInput::SwOSScreenInput( SwOSScreen *parent, uint8_t id, const char *title, const char *param, uint8_t maxLength ) : SwOSScreen( parent, title ) {

  init( parent, id, title, param, maxLength );

}

SwOSScreenInput::SwOSScreenInput( SwOSScreen *parent, uint8_t id, const char *title, int32_t param, uint8_t maxLength ) : SwOSScreen( parent, title ) {

  char str[32];
  itoa( param, str, 10 );

  keyboard    = 2;
  numKeyboard = true;
  init( parent, id, title, str, maxLength );


}

SwOSScreenInput::~SwOSScreenInput() {
  free ( input );
}

void SwOSScreenInput::drawCursor( bool invert ) {
  
  uint8_t cx = keyboardX + cursorC[keyboard] *  9 +1;
  uint8_t cy = keyboardY + cursorR[keyboard] * 11 +1;

  oled->drawRect( cx,   cy, 8, 10, true, invert );
  oled->drawChar( cx+2, cy+1, keyboardMap[keyboard][keymapIndex()], !invert, invert, 1, 1 );

}

void SwOSScreenInput::drawInput( void ) {

  char str[25];

  // copy max 20 chars into str
  uint8_t len = strlen( input );
  uint8_t start = 0;
  if (len>20) { start = len-20; len = 20; }
  strncpy( str, &input[start], len );
  str[len]='\0';
  
  // clear area
  oled->drawRect( 0, 0, OLEDWIDTH, 10, true, false );

  // write string
  oled->write( str, 0, 0, FTSWARM_ALIGNLEFT, false, false );

  // write cursor
  uint16_t x = len*6 + 2;
  oled->drawLine( x, 0, x, 8, true );

}

void SwOSScreenInput::draw( void ) {

  SwOSScreen::draw();
  drawInput();

  uint16_t x1 = keyboardX;
  uint16_t y1 = keyboardY;

  for (uint8_t r=0; r<rows[keyboard]; r++) {

    x1 = keyboardX;
    for (uint8_t c=0; c<cols[keyboard]; c++ ) {
      char ch = keyboardMap[keyboard][ r*cols[keyboard] + c ];
      if (ch) {
        oled->drawChar( x1+2, y1+2, ch, true, false, 1, 1 );
      }
      x1 += 9;
    }

    y1 += 11;

  }

  // keys area
  oled->drawRect( keyboardX, keyboardY, keyboardWidth, keyboardHeight, false, true );

  drawCursor( true );

}

void SwOSScreenInput::setKeyboard( uint8_t keyboard ) {

  if ( keyboard > KEYMAPS-1 ) this->keyboard = 0;
  else                        this->keyboard = keyboard;

  keyboardWidth  = cols[this->keyboard] *  9 + 1;
  keyboardHeight = rows[this->keyboard] * 11 + 1;
  keyboardX      = ( OLEDWIDTH - this->keyboardWidth ) / 2;
  keyboardY      = 11;

}

bool SwOSScreenInput::eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nparam, char *sparam ) {

  uint8_t len;

  // joystick
  switch ( id ) {

    case SWOSJOY1LR:  drawCursor( false );
                      if ( ( event == FTSWARM_SCREENEVENT_DOWN ) && ( cursorC[keyboard] ) )                   cursorC[keyboard]--;
                      if ( ( event == FTSWARM_SCREENEVENT_UP )   && ( cursorC[keyboard] < cols[keyboard]-1) ) cursorC[keyboard]++;
                      drawCursor( true );
                      return true;

    case SWOSJOY1FB:  drawCursor( false );
                      if ( ( event == FTSWARM_SCREENEVENT_UP )   && ( cursorR[keyboard] ) )                   cursorR[keyboard]--;
                      if ( ( event == FTSWARM_SCREENEVENT_DOWN ) && ( cursorR[keyboard] < rows[keyboard]-1) ) cursorR[keyboard]++;
                      drawCursor( true );
                      return true;

  }

  // buttons
  if ( event == FTSWARM_SCREENEVENT_DOWN ) {

    switch (id) {

      case FTSWARM_S1:  setKeyboard( keyboard + 1 );
                        S1->setLabel( S1Label[keyboard] );
                        draw();
                        return true;

      case FTSWARM_S3:  len = strlen( input );
                        if ( len > 0 ) {
                          input[ len-1 ] = '\0';
                          drawInput();
                        }
                        return true;

      case FTSWARM_J1:  len = strlen( input );
                        if ( len < maxLength ) {
                          input[len]   = keyboardMap[keyboard][keymapIndex()];
                          input[len+1] = '\0';
                          drawInput();
                        }
                        return true;

      case FTSWARM_S4:  close();
                        if (parent) parent->eventHandlerCallback( FTSWARM_SCREENEVENT_OK, this->id, atoi(input), input );
                        return true;

      case FTSWARM_F2:  close();
                        return true;

    }

  }

  return false;

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

bool SwOSScreenFactoryReset::eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nparam, char *sparam ) {

  if ( SwOSScreenSlider::eventHandlerCallback( event, id ) ) return true;

  // YES
  if ( id == FTSWARM_S2 ) {
    if ( event == FTSWARM_SCREENEVENT_DOWN )  myOSSwarm.factoryReset();
    return true;
  }

  // NO
  if ( id == FTSWARM_S3 ) {
    if ( event == FTSWARM_SCREENEVENT_DOWN ) screenManager.newScreen( new SwOSMainScreen( NULL, "Main" ), true );
    return true;
  }

  return false;

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

void SwOSMainScreen::joystick( char *lr, char*fb, int16_t x, int16_t y, bool left ) {  

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

bool SwOSMainScreen::eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nparam, char *sparam ) {

  if ( id == FTSWARM_S4 ) {

      // set new screen and all done
      if ( event == FTSWARM_SCREENEVENT_DOWN ) {

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

}

bool SwOSSplashScreen::eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nparam, char *sparam ) {
  
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
