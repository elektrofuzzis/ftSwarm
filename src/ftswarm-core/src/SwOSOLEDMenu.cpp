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
#include "SwOS.h"

#if FTSWARM_HAL_OLEDS > 0

FtSwarmScreenManager screenManager;

void debugEvent( const char *s1, const char *s2, FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  printf( "%s %s ", s1, s2 );

  printf( "event=%d id=%d ", event, id );

  if ( nParam == FTSWARM_NANI32 ) printf( "nParam=NAN ");
  else printf( "nParam=%d ", nParam );

  if ( sParam ) printf( "sParam=%s\n", sParam );
  else printf( "sParam=NULL\n");

}

/***************************************************
 *
 *   FtSwarmScreenObj
 *
 ***************************************************/


FtSwarmScreenObj::FtSwarmScreenObj( uint8_t id, SwOSIO *io, FtSwarmScreen *parent, const char *label, uint8_t screen, int16_t x, int16_t y, FtSwarmAlign_t align ) {
  
  this->id     = id;
  this->io     = io;
  this->parent = parent;
  this->screen = screen;
  this->x      = x;
  this->y      = y;
  this->align  = align;

  if ( screen == FTSWARM_OLED_BUTTONSCREEN ) oled.buttonScreen( true );

  setLabel( label, false );

}

FtSwarmScreenObj::~FtSwarmScreenObj() {

  if (io) io->unsubscribe( this );

  if ( label ) free( label );

  if ( next ) delete( next );

}

void FtSwarmScreenObj::add( FtSwarmScreenObj *newObject) {

  if (next) next->add( newObject );
  else      next = newObject;

}

void FtSwarmScreenObj::activate( void ) {

  if (!io) return;
  
  io->subscribe( this ); 
  value = io->getValueI32();

}

void FtSwarmScreenObj::deactivate( void ) {

  if (!io) return;
  
  io->unsubscribe( this ); 

}

void FtSwarmScreenObj::unregister( SwOSIO *io ) {

  if ( this->io == io ) this->io = NULL;

}

void FtSwarmScreenObj::setLabel( const char *label, bool autoDraw ) {

  if ( this->label) free( this->label );

  this->label = STRDUP( label );

  if ( autoDraw ) draw();

}

void FtSwarmScreenObj::setValue( int32_t newValue ) {

  value = newValue;

  draw( );

  FtSwarmScreenEvent_t event;

  if ( value > 0 ) event = FTSWARM_SCREENEVENT_UP;
  else             event = FTSWARM_SCREENEVENT_DOWN;

  if ( lastEvent != FTSWARM_SCREENEVENT_NONE ) screenManager.eventHandler( parent, event, id );

  lastEvent = event;

}

void FtSwarmScreenObj::print( void ) {

  printf("id=%d x=%d y=%d label=", id, x, y );
  if ( label ) printf( label ); else printf("NULL");
  printf("\n");
  
  if( next ) next->print();

}

/***************************************************
 *
 * FtSwarmScreenObjList
 *
 ***************************************************/

void FtSwarmScreenObjList::add( FtSwarmScreenObj *newObject ) {

  if (list) list->add( newObject );
  else      list = newObject;

}

void FtSwarmScreenObjList::draw( void ) {

  FtSwarmScreenObj *x = list;
  while ( x ) {
    x->draw();
    x = x->next;
  }

}

void FtSwarmScreenObjList::activate( void ) { 

  FtSwarmScreenObj *x = list;
  while ( x ) {
    x->activate();
    x = x->next;
  }

}

void FtSwarmScreenObjList::deactivate( void ) { 

  FtSwarmScreenObj *x = list;
  while ( x ) {
    x->deactivate();
    x = x->next;
  }

}

int16_t FtSwarmScreenObjList::getNextY( void ) {
  
  int16_t y = 0;
  int16_t textHeight = oled.getTextHeight();

  FtSwarmScreenObj *obj = list;
  while ( obj ) {

    if ( ( obj->isVisible() ) && ( obj->getY() >= y ) ) y = obj->getY() + textHeight + 2;

    obj = obj->next;

  }

  return y;

}

FtSwarmScreenObj *FtSwarmScreenObjList::prev( FtSwarmScreenObj *obj ) {

  FtSwarmScreenObj *p = list;
  while ( ( p ) && ( p->next != obj ) ) p = p->next;

  return p;

}

/***************************************************
 *
 * FtSwarmScreenSelectable - label + text
 *
 ***************************************************/

FtSwarmScreenSelectable::FtSwarmScreenSelectable( uint8_t id, FtSwarmScreen *parent, const char *label, const char *text, int16_t x, int16_t y, int16_t widthLabel, int16_t widthText ) : FtSwarmScreenObj( id, NULL, parent, label, FTSWARM_OLED_MAINSCREEN, x, y, FTSWARM_ALIGNLEFT ) {

  // if (label) printf("FtSwarmScreenSelectable %d %s %s %d %d %d %d\n", id, label, text, x, y, widthLabel, widthText );
  // else       printf("FtSwarmScreenSelectable %d NULL %s %d %d %d %d\n", id, text, x, y, widthLabel, widthText );

  // no label? Center text
  if ( widthLabel == 0) align = FTSWARM_ALIGNCENTER;

  this->widthLabel = widthLabel;
  this->widthText  = widthText;

  setText( text, false );

}

FtSwarmScreenSelectable::FtSwarmScreenSelectable( uint8_t id, FtSwarmScreen *parent, const char *text ) : FtSwarmScreenSelectable( id, parent, "", text, oled.getScreenWidth()/2, parent->getNextY() , 0, oled.getScreenWidth() ) {

}

void FtSwarmScreenSelectable::setText( const char *text, bool autoDraw ) {

  if ( this->text) {
    delete this->text;
  }

  this->text = STRDUP( text );
  
  if (autoDraw) draw();

}

void FtSwarmScreenSelectable::draw( void ) {

  if (!visible) return;

  printf("draw %s\n", text);

  oled.drawStrRect( screen, x,              y, widthLabel,                          label, align, 1, 0 );
  oled.drawStrRect( screen, x + widthLabel, y, oled.getScreenWidth() - widthLabel , text,  align, 1, 0 );
  
}

void FtSwarmScreenSelectable::select( void ) {

  if (!visible) return;

  printf("select %s\n", text);

  oled.drawStrRect( screen, x + widthLabel, y, oled.getScreenWidth() - widthLabel, text,  align, 1, 0, FTSWARM_OLED_FILLWHITE );
  
}

void FtSwarmScreenSelectable::setVisible( bool visible ) {

  this->visible = visible;

  if (visible) draw();
  else         oled.drawStrRect( screen, x + widthLabel + 1, y, widthLabel, text,  align, 1, 1 );

}

void FtSwarmScreenSelectable::print( void ) {

  printf("id=%d x=%d y=%d label=", id, x, y );
  if ( label ) printf( label ); else printf("NULL");
  printf(" text=");
  if ( text )  printf( text ); else printf("NULL");
  printf("\n");
  
  if( next ) next->print();

}

/***************************************************
 *
 * FtSwarmScreenButton - button class
 *
 ***************************************************/

void FtSwarmScreenButton::draw( void ) {

  if ( ( label ) && ( label[0] != '\0' )  ) {

    if (value>0) oled.setDrawColor(2);
    oled.drawStrRect( screen, x, y, oled.getTextWidth(label), label, align, 1, 1 );
    oled.setDrawColor(1);

  }

} 

/***************************************************
 *
 * FtSwarmScreenObjXX - local Buttons
 *
 ***************************************************/

FtSwarmScreenS1::FtSwarmScreenS1( FtSwarmScreen *parent, const char *label ) : FtSwarmScreenButton( FTSWARM_S1, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_S1), parent, label, FTSWARM_OLED_BUTTONSCREEN, 0,                             0, FTSWARM_ALIGNLEFT ) {};
FtSwarmScreenS2::FtSwarmScreenS2( FtSwarmScreen *parent, const char *label ) : FtSwarmScreenButton( FTSWARM_S2, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_S2), parent, label, FTSWARM_OLED_BUTTONSCREEN, 41,                            0, FTSWARM_ALIGNCENTER ) {};
FtSwarmScreenS3::FtSwarmScreenS3( FtSwarmScreen *parent, const char *label ) : FtSwarmScreenButton( FTSWARM_S3, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_S3), parent, label, FTSWARM_OLED_BUTTONSCREEN, oled.getScreenWidth() -1 -41,  0, FTSWARM_ALIGNCENTER ) {};
FtSwarmScreenS4::FtSwarmScreenS4( FtSwarmScreen *parent, const char *label ) : FtSwarmScreenButton( FTSWARM_S4, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_S4), parent, label, FTSWARM_OLED_BUTTONSCREEN, oled.getScreenWidth() -1,      0, FTSWARM_ALIGNRIGHT ) {};
FtSwarmScreenF1::FtSwarmScreenF1( FtSwarmScreen *parent, const char *label ) : FtSwarmScreenButton( FTSWARM_F1, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_F1), parent, label, FTSWARM_OLED_UPPERSCREEN,  0,                             0, FTSWARM_ALIGNLEFT ) {};
FtSwarmScreenF2::FtSwarmScreenF2( FtSwarmScreen *parent, const char *label ) : FtSwarmScreenButton( FTSWARM_F2, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_F2), parent, label, FTSWARM_OLED_UPPERSCREEN,  oled.getScreenWidth() -1,      0, FTSWARM_ALIGNRIGHT ) {};
FtSwarmScreenJ1::FtSwarmScreenJ1( FtSwarmScreen *parent, const char *label ) : FtSwarmScreenButton( FTSWARM_J1, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_J1), parent, label, FTSWARM_OLED_MAINSCREEN,   48,                           16, FTSWARM_ALIGNCENTER ) {};
FtSwarmScreenJ2::FtSwarmScreenJ2( FtSwarmScreen *parent, const char *label ) : FtSwarmScreenButton( FTSWARM_J2, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_J2), parent, label, FTSWARM_OLED_MAINSCREEN,   oled.getScreenWidth() -1 -48, 16, FTSWARM_ALIGNCENTER ) {};

/***************************************************
 *
 * FtSwarmScreenJoystickPoti - Helper class
 *
 ***************************************************/

#define JOYMINVALUE 15
#define JOYMAXVALUE 25

FtSwarmScreenJoystickPoti::FtSwarmScreenJoystickPoti(uint8_t id, SwOSIO *io, FtSwarmScreen *parent, const char *label, int16_t x, int16_t y, FtSwarmAlign_t align ):FtSwarmScreenObj( id, io, parent, label, FTSWARM_OLED_MAINSCREEN, x, y, align ) {

}

void FtSwarmScreenJoystickPoti::setValue( int32_t value ) {

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
    
    if      ( ( value > -JOYMINVALUE ) && ( maxValue < -JOYMAXVALUE ) ) { event = FTSWARM_SCREENEVENT_DOWN; maxValue = value; }
    else if ( ( value <  JOYMINVALUE ) && ( maxValue >  JOYMAXVALUE ) ) { event = FTSWARM_SCREENEVENT_UP;   maxValue = value; }
    else if ( abs(value) > abs ( maxValue ) )                           {                                   maxValue = value; }

  }

  // call eventhandler on a real event only
  if ( event != FTSWARM_SCREENEVENT_NONE ) screenManager.eventHandler( parent, event, id );

}

/***************************************************
 *
 *   FtSwarmScreen
 *
 ***************************************************/

#define SWOSJOY1LR 100
#define SWOSJOY1FB 101
#define SWOSJOY2LR 102
#define SWOSJOY2FB 103

uint32_t screenUSID = 0;

FtSwarmScreen::FtSwarmScreen( FtSwarmScreen *parent, const char *title, FtSwarmScreen *next ) {
  
  this->USID   = screenUSID++;
  this->parent = parent;

  this->title = STRDUP( title );

  oled.buttonScreen( false );
  
  // need a back button?
  if ( parent ) add( new FtSwarmScreenESC( this ) );

  // handle sliders
  this->next = next;
  if (next) {
    next->prev = this;
    addNavigation();
  }

  screenManager.registerMe( this );

}

void FtSwarmScreen::addNavigation( void ) {

  // already installed?
  if (navigation) return;

  // add left joystick elements
  add( new FtSwarmScreenJoystickPoti( SWOSJOY1LR, myOSSwarm.Ctrl[0]->getIO( SWOSIO_JOYSTICK_POTI, FTSWARM_HAL_FIRSTJPOTI   ), this, "", 0, 0, FTSWARM_ALIGNLEFT ) );
  add( new FtSwarmScreenJoystickPoti( SWOSJOY1FB, myOSSwarm.Ctrl[0]->getIO( SWOSIO_JOYSTICK_POTI, FTSWARM_HAL_FIRSTJPOTI+1 ), this, "", 0, 0, FTSWARM_ALIGNLEFT ) );
  add( new FtSwarmScreenJ1( this, "" ) );

  // add right joystick
  add( new FtSwarmScreenJoystickPoti( SWOSJOY2FB, myOSSwarm.Ctrl[0]->getIO( SWOSIO_JOYSTICK_POTI, FTSWARM_HAL_FIRSTJPOTI+3 ), this, "", 0, 0, FTSWARM_ALIGNLEFT ) );

  // all done
  navigation = true;

}

FtSwarmScreen::~FtSwarmScreen() {

  if ( title ) free( title );

}

void FtSwarmScreen::add( FtSwarmScreenObj *newObject ) {
  
  if (!newObject) return;

  if (newObject->isSelectable() ) {

    addNavigation();
    selectables.add( newObject );
    if ( (!selected) && (newObject)) selected = (FtSwarmScreenSelectable *) newObject;

  } else {
    
    objects.add( newObject );

  }

}

void FtSwarmScreen::draw( void ) {

  // cls
  oled.cls( );
      
  if ( title ) oled.drawStr( FTSWARM_OLED_UPPERSCREEN, oled.getScreenWidth() / 2, 0, title, FTSWARM_ALIGNCENTER );
  
  // cool line
  oled.drawLine( FTSWARM_OLED_UPPERSCREEN, 0, 11, oled.getScreenWidth(), 11 );

  objects.draw();
  selectables.draw();

  // selected element?
  if (selected) selected->select();

  // top slider
  uint8_t cp = countPrev();
  uint8_t cn = countNext();
  uint8_t c  = cp + cn + 1;

  if ( c > 1 ) {
    uint8_t size = oled.getScreenWidth() / c;
    oled.drawLine( FTSWARM_OLED_UPPERSCREEN, cp*size, 13, (cp+1)*size, 13 );
  }

}

void FtSwarmScreen::activate( void ) {

  screenManager.blockEvents = blockEvents;
  objects.activate();
  selectables.activate();

}

void FtSwarmScreen::deactivate( void ) {

  objects.deactivate();
  selectables.deactivate();

}

void FtSwarmScreen::close( FtSwarmScreenEvent_t event, uint8_t id, uint8_t nParam, const char *sParam ) { 
  
  FtSwarmScreen *o;
  
  // cleanup to the left
  FtSwarmScreen *p = prev;
  while (p) {
    p->toBeDestroyed = true;
    p = p->prev;
  }

  // cleanup to the right
  FtSwarmScreen *n = next;
  while (n) {
    n->toBeDestroyed = true;
    n = n->next;
  }

  toBeDestroyed = true;

  screenManager.activate( parent ); 

  if ( event != FTSWARM_SCREENEVENT_NONE ) screenManager.eventHandler( parent, event, id, nParam, sParam );

};

void FtSwarmScreen::deleteSelectables( void ) {

  selectables.cleanup();
  selected = NULL;

}

bool FtSwarmScreen::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  if ( !navigation ) return false;

  // catch all joystick stuff
  if ( id == SWOSJOY1LR ) {
  
    // switch left?
    if ( ( event == FTSWARM_SCREENEVENT_DOWN ) && ( prev ) ) screenManager.activate( prev ); 

    // switch right?
    if ( ( event == FTSWARM_SCREENEVENT_UP ) && ( next ) )   screenManager.activate( next );

    return true;

  }

  if ( id == SWOSJOY1FB ) {

    FtSwarmScreenObj *nextSelection = NULL;

    // go up ?
    if ( ( event == FTSWARM_SCREENEVENT_UP ) && ( selected ) ) nextSelection = selectables.prev( (FtSwarmScreenObj *) selected );

    // go down ?
    if ( ( event == FTSWARM_SCREENEVENT_DOWN ) && ( selected ) ) nextSelection = selected->next;

    // found?
    if (nextSelection) {

      // set old one to normal
      selected->draw();

      // select new one
      selected = (FtSwarmScreenSelectable *) nextSelection;
      selected->select();

    }

    return true;

  }

  // select J1
  if ( id == FTSWARM_J1 ) {
    
    if ( event == FTSWARM_SCREENEVENT_DOWN ) screenManager.eventHandler( this, FTSWARM_SCREENEVENT_OK, selected->getID() );

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

uint8_t FtSwarmScreen::countPrev( void ) {

  uint8_t count = 0;
  FtSwarmScreen *screen = prev;

  while( screen ) { count++; screen = screen->prev; }

  return count;

}

uint8_t FtSwarmScreen::countNext( void ) {

  uint8_t count = 0;
  FtSwarmScreen *screen = next;

  while( screen ) { count++; screen = screen->next; }

  return count;

}

int16_t FtSwarmScreen::getNextY( void ) {

  return selectables.getNextY();

}

/***************************************************
 *
 *   FtSwarmScreenChooseOption
 *
 ***************************************************/   

FtSwarmScreenChooseOption::FtSwarmScreenChooseOption( FtSwarmScreen *parent, uint8_t id, const char *title, const char *text,
                                                int32_t value1, const char *option1, 
                                                int32_t value2, const char *option2, 
                                                int32_t value3, const char *option3, 
                                                int32_t value4, const char *option4 ) : FtSwarmScreen( parent, title) {

  this->id = id;
  this->value[0] = value1;
  this->value[1] = value2;
  this->value[2] = value3;
  this->value[3] = value4;

  add( new FtSwarmScreenESC( this ) );
  if (option1) add( new FtSwarmScreenS1( this, option1) );
  if (option2) add( new FtSwarmScreenS2( this, option2) );
  if (option3) add( new FtSwarmScreenS3( this, option3) );
  if (option4) add( new FtSwarmScreenS4( this, option4) );

  // ** split text **
  uint8_t len = strlen( text );
  uint8_t maxCharsPerRow = oled.getScreenWidth() / oled.getTextWidth( text );
  uint8_t neededRows = len / maxCharsPerRow + 1;
  uint8_t optLength  = len / neededRows;

  // add some chars to be more flexible
  uint8_t diff = maxCharsPerRow - optLength;
  if ( diff > 4 ) optLength += 4;
  else            optLength += diff;
  
  // start splitting
  uint16_t cut;
  char *todo = (char *)text;
  maxLine = -1;

  while ( ( todo[0] != '\0' ) && ( maxLine < 4) ) {

    if ( strlen( todo ) <= optLength )
      // last line
      cut = strlen( todo );

    else {
    
      // search back from optLength to find a space
      cut = optLength;
      while ( ( todo[cut] != ' ' ) && ( cut > 0 ) ) cut--;

      // no space found, need a hard cut
      if (cut == 0) cut = optLength;

    }

    // copy
    line[++maxLine] = (char *) calloc( cut+1, sizeof( char ) );
    strncpy( line[maxLine], todo, cut );

    // skip space?
    if ( todo[cut] == ' ' ) cut++;

    // move pointer
    todo += cut;

  }

}

FtSwarmScreenChooseOption::~FtSwarmScreenChooseOption() {

  for ( uint8_t i=0; i<=maxLine; i++ ) free( line[i] );
}

void FtSwarmScreenChooseOption::draw( void ) {

  FtSwarmScreen::draw();

  int8_t rowHeight = oled.getTextHeight()+1;
  int8_t space     = 48 - rowHeight; 
  int8_t y         = ( space - ( (maxLine+2) * oled.getTextHeight() ) ) / 2;

  // just in case
  if ( y<0 ) y = 0;

  for ( uint8_t i=0; i<=maxLine; i++ ) {
    oled.drawStr( FTSWARM_OLED_MAINSCREEN, oled.getScreenWidth()/2, y, line[i], FTSWARM_ALIGNCENTER );
    y += rowHeight;
  }

}

bool FtSwarmScreenChooseOption::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  if ( FtSwarmScreen::eventHandler( event, id,nParam, sParam ) ) return true;

  if ( event == FTSWARM_SCREENEVENT_DOWN ) {
  
    // S1..S4
    if ( id != FTSWARM_F2 ) screenManager.eventHandler( parent, FTSWARM_SCREENEVENT_OK, this->id, value[id - FTSWARM_S1], NULL );

    // close
    close();

    return true;
  }

}

/***************************************************
 *
 *   FtSwarmScreenSelectList
 *
 ***************************************************/

template<typename... Args>
FtSwarmScreenSelectList::FtSwarmScreenSelectList( FtSwarmScreen *parent, const char *title, uint8_t callbackID, Args... args ) : FtSwarmScreen( parent, title, NULL ) {

  this->callbackID = callbackID;

  static_assert(sizeof...(args) % 2 == 0, "FtSwarmScreenSelectList::FtSwarmScreenSelectList - please call in tuples <uint8_t>, <char*>");

  process( args... );

};

template<typename... Tail>
void FtSwarmScreenSelectList::process(uint8_t id, const char* str, Tail... tail) {

  // printf("FtSwarmScreenSelectList::process %d %s\n", id, str );

  add( new FtSwarmScreenSelectable( id, this, str ) );
  process(tail...);

}

bool FtSwarmScreenSelectList::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam , const char *sParam ) {

  if ( FtSwarmScreen::eventHandler( event, id, nParam, sParam ) ) return true;

  if ( event == FTSWARM_SCREENEVENT_OK ) { close( event, callbackID, selected->getID() ); return true; }

  return false;

}

/***************************************************
 *
 *   FtSwarmScreenChooseConfig
 *
 ***************************************************/

FtSwarmScreenChooseConfig::FtSwarmScreenChooseConfig( FtSwarmScreen *parent, FtSwarmScreen *next  ) : FtSwarmScreen( parent, "Configuration", next ) {

  add( new FtSwarmScreenS1( this, "#1" ) );
  add( new FtSwarmScreenS2( this, "#2" ) );
  add( new FtSwarmScreenS3( this, "#3" ) );
  add( new FtSwarmScreenS4( this, "#4" ) );

}

 void FtSwarmScreenChooseConfig::draw( void ) {

  FtSwarmScreen::draw();
  oled.drawStr( FTSWARM_OLED_MAINSCREEN, oled.getScreenWidth()/2, 7, "Choose new", FTSWARM_ALIGNCENTER );
  oled.drawStr( FTSWARM_OLED_MAINSCREEN, oled.getScreenWidth()/2, 16, "configuration", FTSWARM_ALIGNCENTER );
  
}

bool FtSwarmScreenChooseConfig::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  if ( FtSwarmScreen::eventHandler( event, id, nParam, sParam ) ) return true;

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
 *   FtSwarmScreenWifi
 *
 ***************************************************/

#define SWOSSCREENWIFI_MODE      ( SWOSSCREENID_BASE + 0 )
#define SWOSSCREENWIFI_SSID      ( SWOSSCREENID_BASE + 1 )
#define SWOSSCREENWIFI_PASSWD    ( SWOSSCREENID_BASE + 2 )
#define SWOSSCREENWIFI_CB_MODE   ( SWOSSCREENID_BASE + 3 )
#define SWOSSCREENWIFI_CB_SSID   ( SWOSSCREENID_BASE + 4 )
#define SWOSSCREENWIFI_CB_PASSWD ( SWOSSCREENID_BASE + 5 )
#define SWOSSCREENWIFI_CB_SAVE   ( SWOSSCREENID_BASE + 6 )

FtSwarmScreenWifi::FtSwarmScreenWifi( FtSwarmScreen *parent, FtSwarmScreen *next  ) : FtSwarmScreen( parent, "Wifi", next ) {

  strcpy( wifiSSID, nvs.wifiSSID );
  strcpy( wifiPwd,  nvs.wifiPwd );
  wifiMode = nvs.wifiMode;

  add( wifiModeSO = new FtSwarmScreenSelectable( SWOSSCREENWIFI_MODE,   this, "Mode",   WIFI[wifiMode], 0,  1, 40, oled.getScreenWidth()-40 ) );
  add( wifiSSIDSO = new FtSwarmScreenSelectable( SWOSSCREENWIFI_SSID,   this, "SSID",   wifiSSID, 0, 10, 40, oled.getScreenWidth()-40 ) );
  add( wifiPwdSO  = new FtSwarmScreenSelectable( SWOSSCREENWIFI_PASSWD, this, "Passwd", "*****",  0, 19, 40, oled.getScreenWidth()-40 ) );
  add( S4 = new FtSwarmScreenS4( this, "" ) );

  wifiSSIDSO->setVisible( ( wifiMode != wifiOFF ) );
  wifiPwdSO->setVisible ( ( wifiMode != wifiOFF ) );

}

bool FtSwarmScreenWifi::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam , const char *sParam ) {

  if ( FtSwarmScreen::eventHandler( event, id, nParam, sParam ) ) return true;

  if ( event == FTSWARM_SCREENEVENT_OK ) {

    switch (id) {

      case SWOSSCREENWIFI_MODE:       screenManager.activate( new FtSwarmScreenChooseOption( this, SWOSSCREENWIFI_CB_MODE, "wifi mode", "choose wifi mode", wifiOFF, "off", wifiAP, "AP", wifiClient, "client" ) );
                                      break;

      case SWOSSCREENWIFI_CB_MODE:    wifiMode = (FtSwarmWifi_t) nParam;
                                      wifiModeSO->setText( WIFI[wifiMode] );
                                      wifiSSIDSO->setVisible( ( wifiMode != wifiOFF ) );
                                      wifiPwdSO->setVisible ( ( wifiMode != wifiOFF ) );
                                      anythingChanged = true;
                                      break;

      case SWOSSCREENWIFI_SSID:       if ( wifiMode == wifiAP ) screenManager.activate( new FtSwarmScreenInput( this, SWOSSCREENWIFI_CB_SSID, "SSID", wifiSSID, 63 ) );
                                      else                      screenManager.activate( new FtSwarmScreenWifiSSID( this, SWOSSCREENWIFI_CB_SSID ) );
                                      break;

      case SWOSSCREENWIFI_CB_SSID:    if (sParam) { 
                                        strcpy( wifiSSID, sParam); 
                                        wifiSSIDSO->setText( wifiSSID );
                                        anythingChanged = true;
                                      }
                                      break;

      case SWOSSCREENWIFI_PASSWD:     screenManager.activate( new FtSwarmScreenInput( this, SWOSSCREENWIFI_CB_PASSWD, "Password", "", 63 ) );
                                      break;

      case SWOSSCREENWIFI_CB_PASSWD:  if ( sParam) {
                                        if ( ( strlen(sParam) > 0 ) && ( strlen(sParam) < 8 ) ) screenManager.activate( new FtSwarmScreenError( this, "wifi passwords needs at minimum 8 chars" ) );
                                        else {
                                          strcpy( wifiPwd, sParam);
                                          anythingChanged = true;
                                        }
                                      }
                                      break;

      case SWOSSCREENWIFI_CB_SAVE:    if (nParam) {
                                        nvs.wifiMode = wifiMode;
                                        strcpy( nvs.wifiSSID, wifiSSID );
                                        strcpy( nvs.wifiPwd,  wifiPwd );
                                        nvs.saveAndRestart();
                                      }
                                      break;

    }

    if ( anythingChanged ) S4->setLabel("Save");
    
    return true;
    
  }
  
  if ( event == FTSWARM_SCREENEVENT_DOWN ) {

    if ( ( id == FTSWARM_S4 ) && ( anythingChanged ) )  screenManager.activate( new FtSwarmScreenYesNo( this, SWOSSCREENWIFI_CB_SAVE, "wifi", "Save new settings and reboot?" ) );
    return true;

  }

  return false;

}

/***************************************************
 *
 *   FtSwarmScreenWifiSSID
 *
 ***************************************************/

FtSwarmScreenWifiSSID::FtSwarmScreenWifiSSID( FtSwarmScreen *parent, uint8_t id  ): FtSwarmScreen( parent, "SSID", NULL ) {

  this->id = id;
  scanStatus = WIFI_SCAN_RUNNING;
  WiFi.scanNetworks(true);

}

FtSwarmScreenWifiSSID::~FtSwarmScreenWifiSSID() {
  WiFi.scanDelete();
}

void FtSwarmScreenWifiSSID::draw( void ) {
  
  FtSwarmScreen::draw();

  if ( scanStatus == WIFI_SCAN_RUNNING ) 
  oled.drawStr( FTSWARM_OLED_MAINSCREEN, oled.getScreenWidth()/2, 32,  "scanning...", FTSWARM_ALIGNCENTER );

}

bool FtSwarmScreenWifiSSID::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  if ( FtSwarmScreen::eventHandler( event, id, nParam, sParam ) ) return true;

  // must be a J1 click to select and close
  close( FTSWARM_SCREENEVENT_OK, this->id, selected->getID(), (char *) selected->getText() );

  return true;

}

void FtSwarmScreenWifiSSID::operate( void ) {

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

      if ( unique ) add( new FtSwarmScreenSelectable( i, this, "", WiFi.SSID(i).c_str(), 0, i*9+1, 0, oled.getScreenWidth() ) );

    }

    draw();

  } else if ( scanStatus != WIFI_SCAN_RUNNING ) {

    close();

  }

};

/***************************************************
 *
 * FtSwarmScreenInput
 *
 ***************************************************/

void FtSwarmScreenInput::init(FtSwarmScreen *parent,  uint8_t id, const char *title, const char *param, uint8_t maxLength ) {

  this->id = id;

  this->maxLength = maxLength;
  input = (char *) calloc( this->maxLength+1, sizeof( char ) );

  uint8_t l = strlen( param );
  if ( l > this->maxLength ) l=this->maxLength;
  strncpy( input, param, l );

  setKeyboard( keyboard );

  add( new FtSwarmScreenJoystickPoti( SWOSJOY1LR, myOSSwarm.Ctrl[0]->getIO( SWOSIO_JOYSTICK_POTI, FTSWARM_HAL_FIRSTJPOTI   ), this, "", 0, 0, FTSWARM_ALIGNLEFT ) );
  add( new FtSwarmScreenJoystickPoti( SWOSJOY1FB, myOSSwarm.Ctrl[0]->getIO( SWOSIO_JOYSTICK_POTI, FTSWARM_HAL_FIRSTJPOTI+1 ), this, "", 0, 0, FTSWARM_ALIGNLEFT ) );
  add( new FtSwarmScreenJ1( this, "" ) );

  add( new FtSwarmScreenESC( this ) );

  if (!numKeyboard) add( S1 = new FtSwarmScreenS1( this, S1Label[keyboard] ) );

  add( new FtSwarmScreenS3( this, "<]") );
  add( new FtSwarmScreenS4( this, "OK") );

}

FtSwarmScreenInput::FtSwarmScreenInput( FtSwarmScreen *parent, uint8_t id, const char *title, const char *param, uint8_t maxLength ) : FtSwarmScreen( parent, title ) {

  init( parent, id, title, param, maxLength );

}

FtSwarmScreenInput::FtSwarmScreenInput( FtSwarmScreen *parent, uint8_t id, const char *title, int32_t param, uint8_t maxLength ) : FtSwarmScreen( parent, title ) {

  char str[32];
  if ( param == FTSWARM_NANI32 ) str[0] = '\0';
  else                           itoa( param, str, 10 );

  keyboard    = 2;
  numKeyboard = true;
  init( parent, id, title, str, maxLength );


}

FtSwarmScreenInput::~FtSwarmScreenInput() {
  free ( input );
}

void FtSwarmScreenInput::drawCursor( bool invert ) {
  
  uint8_t cx = keyboardX + cursorC[keyboard] *  9 +1;
  uint8_t cy = keyboardY + cursorR[keyboard] * 11 +1;

  char key[2];
  key[0] = keyboardMap[keyboard][keymapIndex()];
  key[1] = '\0';

  uint8_t flags = U8G2_BTN_BW1;
  if ( invert) flags |= U8G2_BTN_INV;

  oled.drawButton( FTSWARM_OLED_MAINSCREEN, cx, cy, 9, key, flags, 1, 1 );

}

void FtSwarmScreenInput::drawInput( void ) {

  char str[25];

  // copy max 20 chars into str
  uint8_t len = strlen( input );
  uint8_t start = 0;
  if (len>20) { start = len-20; len = 20; }
  strncpy( str, &input[start], len );
  str[len]='\0';
  
  // clear area
  oled.drawRect( FTSWARM_OLED_MAINSCREEN, 0, 0, oled.getScreenWidth(), oled.getTextHeight(), FTSWARM_OLED_FILLBLACK );

  // write string
  oled.drawStr( FTSWARM_OLED_MAINSCREEN, 0, 0, str, FTSWARM_ALIGNLEFT );

  // write cursor
  uint16_t x = len*6 + 2;
  oled.drawLine( FTSWARM_OLED_MAINSCREEN, x, 0, x, 8 );

}

void FtSwarmScreenInput::draw( void ) {

  FtSwarmScreen::draw();
  drawInput();

  uint16_t x1 = keyboardX;
  uint16_t y1 = keyboardY;

  char key[2];
  key[1] = '\0';

  int16_t lineHeight = oled.getTextHeight() + 3;
  int16_t charWidth  = oled.getTextWidth( " " ) + 3;

  for (uint8_t r=0; r<rows[keyboard]; r++) {

    x1 = keyboardX;
    for (uint8_t c=0; c<cols[keyboard]; c++ ) {
      key[0] = keyboardMap[keyboard][ r*cols[keyboard] + c ];
      oled.drawButton( FTSWARM_OLED_MAINSCREEN, x1+2, y1+2, 9, key, U8G2_BTN_BW1, 1, 1 );
      x1 += charWidth;
    }

    y1 += lineHeight;

  }

  // keys area
  oled.drawRect( FTSWARM_OLED_MAINSCREEN, keyboardX, keyboardY, keyboardWidth, keyboardHeight );

  drawCursor( true );

}

void FtSwarmScreenInput::setKeyboard( uint8_t keyboard ) {

  if ( keyboard > KEYMAPS-1 ) this->keyboard = 0;
  else                        this->keyboard = keyboard;

  keyboardWidth  = cols[this->keyboard] *  9 + 1;
  keyboardHeight = rows[this->keyboard] * 11 + 1;
  keyboardX      = ( oled.getScreenWidth() - this->keyboardWidth ) / 2;
  keyboardY      = 11;

}

bool FtSwarmScreenInput::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  uint8_t len;

  // joystick
  switch ( id ) {

    case SWOSJOY1LR:  drawCursor( false );
                      
                      if ( event == FTSWARM_SCREENEVENT_DOWN ) {
                        if ( cursorC[keyboard] ) cursorC[keyboard]--;
                        else                     cursorC[keyboard] = cols[keyboard]-1;
                      }

                      if ( event == FTSWARM_SCREENEVENT_UP ) {
                        if ( cursorC[keyboard] < cols[keyboard]-1) cursorC[keyboard]++;
                        else                                       cursorC[keyboard] = 0;
                      }

                      drawCursor( true );
                      return true;

    case SWOSJOY1FB:  drawCursor( false );

                      if ( event == FTSWARM_SCREENEVENT_UP ) {
                        if ( cursorR[keyboard] ) cursorR[keyboard]--;
                        else                     cursorR[keyboard] = rows[keyboard]-1;
                      }

                      if ( event == FTSWARM_SCREENEVENT_DOWN ) {
                        if ( cursorR[keyboard] < rows[keyboard]-1) cursorR[keyboard]++;
                        else                                       cursorR[keyboard] = 0;
                      }

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
                        screenManager.eventHandler( parent, FTSWARM_SCREENEVENT_OK, this->id, atoi(input), input );
                        return true;

      case FTSWARM_F2:  close();
                        return true;

    }

  }

  return false;

}


/***************************************************
 *
 *   FtSwarmScreenSwarm
 *
 ***************************************************/

 #define SWOSSCREENSWARM_CB_CFG  ( SWOSSCREENID_BASE + 0 )
 #define SWOSSCREENSWARM_CB_PIN  ( SWOSSCREENID_BASE + 1 )
 #define SWOSSCREENSWARM_CB_ADD  ( SWOSSCREENID_BASE + 2 )
 #define SWOSSCREENSWARM_CB_DEL  ( SWOSSCREENID_BASE + 3 )

 #define SWOSSCREENSWARM_CB_SEL  ( SWOSSCREENID_BASE + 10 )
 
FtSwarmScreenSwarm::FtSwarmScreenSwarm( FtSwarmScreen *parent, FtSwarmScreen *next  ) : FtSwarmScreen( parent, "Remote", next ) {

  addMembers();

  add( new FtSwarmScreenS1( this, "add" ) );
  add( new FtSwarmScreenS2( this, "del" ) );
  add( new FtSwarmScreenS3( this, "config" ) );
  add( new FtSwarmScreenS4( this, "pin" ) );

}

void FtSwarmScreenSwarm::addMembers( void ) {

  uint8_t members = myOSSwarm.members();
  uint8_t item    = 0;

  for (uint8_t i=1; i<members ; i++ ) {
    
    if ( myOSSwarm.Ctrl[i] ) 
      add( new FtSwarmScreenSelectable( SWOSSCREENSWARM_CB_SEL + i, this, myOSSwarm.Ctrl[i]->isOnline()? " " : "X", myOSSwarm.Ctrl[i]->getAliasOrName(), 0, getNextY(), 10, oled.getScreenWidth()-10 ) );

  }

}

bool FtSwarmScreenSwarm::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  if ( FtSwarmScreen::eventHandler( event, id, nParam, sParam ) ) return true;

  if ( event == FTSWARM_SCREENEVENT_DOWN ) {

    uint8_t i;

    switch ( id ) {

      case FTSWARM_S1:  // ask for new device, call SWOSSCREENSWARM_CB_ADD afterwards
                        screenManager.activate( new FtSwarmScreenInput( this, SWOSSCREENSWARM_CB_ADD, "SN to add", FTSWARM_NANI32, 4 ) );
                        return true;

      case FTSWARM_S2:  // ask to delete the selected device, call SWOSSCREENSWARM_CB_DEL afterwards
                        if (selected ) {
                          i = selected->getID() - SWOSSCREENSWARM_CB_SEL;
                          if ( ( i < MAXCTRL ) && ( myOSSwarm.Ctrl[i] ) ) screenManager.activate( new FtSwarmScreenYesNo( this, SWOSSCREENSWARM_CB_DEL, selected->getText(), "Revoke controller?", myOSSwarm.Ctrl[i]->serialNumber ) );
                        }
                        return true;

      case FTSWARM_S3:  screenManager.activate( new FtSwarmScreenSelectList( this, "Quick Config", SWOSSCREENSWARM_CB_CFG, 0, "Car", 1, "Car + signal", 2, "Catapillar", 3, "Catapillar + signal", 4, "Crane", 5, "Trailer") );
                        return true;

      case FTSWARM_S4:  // ask for a new swarm pin, call SWOSSCREENSWARM_CB_PIN afterwards
                        screenManager.activate( new FtSwarmScreenInput( this, SWOSSCREENSWARM_CB_PIN, "Swarm Pin", nvs.swarmPIN, 4 ) );
                        return true;

    }

  }

  if ( event == FTSWARM_SCREENEVENT_OK ) {

    char    error[100];
    uint8_t i;

    switch ( id )  {

      case SWOSSCREENSWARM_CB_PIN:  // user entered new pin
                                    nvs.swarmPIN = nParam;
                                    nvs.save();
                                    return true;

      case SWOSSCREENSWARM_CB_ADD:  // nParam is the sn to be added to the swarm
                                    if (nParam) {

                                      if ( myOSSwarm.addController( nParam ) ) {

                                        nvs.save();
                                        deleteSelectables();
                                        addMembers();
                                        draw();
                                        
                                      } else {

                                        sprintf( error, "Could not add ftSwarm%d to swarm.", nParam);
                                        screenManager.activate( new FtSwarmScreenError( this, error) );

                                      }

                                    }

                                    return true;

      case SWOSSCREENSWARM_CB_DEL:  // delete selected controller, nParam is sn
                                    if (nParam) { 

                                      if ( myOSSwarm.deleteController( nParam ) ) {

                                        nvs.save( );
                                        deleteSelectables( ); 
                                        addMembers();
                                        draw(); 

                                      } else {

                                        sprintf( error, "Couldn't revoke % from swarm.", selected->getText() );
                                        screenManager.activate( new FtSwarmScreenError( this, error) );
                                      }

                                    }
                                    return true;

      case SWOSSCREENSWARM_CB_CFG:  printf("SWOSSCREENSWARM_CB_CFG %d\n", nParam);
                                    return true;

      default:                      if (id >= SWOSSCREENSWARM_CB_SEL) {
                                      // SN is endorsed in id
                                      i = id - SWOSSCREENSWARM_CB_SEL;
                                      if ( ( i < MAXCTRL ) && ( myOSSwarm.Ctrl[i] ) ) screenManager.activate( new FtSwarmScreenSwarmDetail( this, selected->getText(), myOSSwarm.Ctrl[i]->serialNumber ) );
                                    }
                                    return true;


    }

  }

  return false;

}

/***************************************************
 *
 *   FtSwarmScreenSwarmDetail
 *
 ***************************************************/

#define SWOSSCREENSWARMDETAIL_CB_DEL ( SWOSSCREENID_BASE + 0 )

FtSwarmScreenSwarmDetail::FtSwarmScreenSwarmDetail( FtSwarmScreen *parent, const char *title, FtSwarmSerialNumber_t device  ) : FtSwarmScreen( parent, title, NULL ) {

  this->device = device;

  add( new FtSwarmScreenS1( this, "Configure" ) );
  add( new FtSwarmScreenS4( this, "Save" ) );

  // need to activate navigation until more details are shown in the screen
  addNavigation();

}

bool FtSwarmScreenSwarmDetail::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  if ( FtSwarmScreen::eventHandler( event, id, nParam, sParam ) ) return true;

  if ( event == FTSWARM_SCREENEVENT_DOWN ) {

    switch ( id ) {

      case FTSWARM_S1:  // Quick Config
                        // screenManager.activate( new FtSwarmScreenQuickCfg( this, device ) );
                        return true;

      case FTSWARM_S4:  // save
                        close( );
                        return true;

    }
  
  }

  return false;
  
}

/***************************************************
 *
 *   FtSwarmScreenQuickCfg
 *
 ***************************************************/

#define SWOSSCREENQUICKCFG_CAR     ( SWOSSCREENID_BASE + 0 )
#define SWOSSCREENQUICKCFG_CARTS   ( SWOSSCREENID_BASE + 1 )
#define SWOSSCREENQUICKCFG_CAT     ( SWOSSCREENID_BASE + 2 )
#define SWOSSCREENQUICKCFG_CATTS   ( SWOSSCREENID_BASE + 3 )
#define SWOSSCREENQUICKCFG_CRANE   ( SWOSSCREENID_BASE + 4 )
#define SWOSSCREENQUICKCFG_TRAILER ( SWOSSCREENID_BASE + 5 )

/* Standard configuration types

  Type        Controller    Function    Key      Settings

  Car         ftSwarmRC     drive       JOY1FB - Wheeldrive M4
                            steer       JOY2LR - RCServo M1
                            gear        S1/S2  - RCServo M2
                            light       S3     - LED4/4 white, LED7/8 forward: red backward: white

                            - option "Car + turn signal" -
                            turn signal F1     - LED2/6 blink orange
                                        F2     - LED5/9 blink orange

                            - option "Car" -
                            addon       F1/F2  - XSMotor M3

              ftSwarmJST    drive       JOY1FB - XSMotor M1
              ftSwarmRS     steer       JOY2LR - Servo Servo1 
              ftSwarmXL     gear        S1/S2  - maxspeed M1
                            light       S3     - LED4/5 white, LED8/9 forward: red backward: white

                            -- option "Car + turn signal" -
                            turn signal F1     - LED3/7 blink orange
                                        F2     - LED6/10 blink orange

                            -- option "Car" -
                            addon       F1/F2  - XSMotor M2

  Catapillar  ftSwarmJST    drive       JOY1FB - XSMotor M1+M2
              ftSwarmRS     steer       JOY2LR - XSMotor M1+M2
                            gear        S1/S2  - maxspeed M1+M2
                            light       S3     - LED4/5 white, LED8/9 forward: red backward: white
                            turn signal F1     - LED3/7 blink orange
                                        F2     - LED6/10 blink orange

              ftSwarmRC     drive       JOY1FB - XSMotor M1+M2
              ftSwarmXL     steer       JOY2LR - XSMotor M1+M2
                            gear        S1/S2  - maxspeed M1+M2
                            light       S3     - LED4/5 white, LED8/9 forward: red backward: white

                            -- option "Catapillar + turn signal" -
                            turn signal F1     - LED3/7 blink orange
                                        F2     - LED6/10 blink orange

                            -- option "Catapillar" -
                            addon       F1/F2  - XSMotor M3

  Crane       ftSwarmJST    turn        JOY1LR - XSMotor M1
              fTSwarmRS     up and down JOY2FB - XSMotor M2
                            light       S3     - LED3 white

              ftSwarmRC     turn        JOY1LR - XSMotor M1
              ftSwarmXL     up and down JOY2FB - XSMotor M2
                            boom        F1/F2  - XSMotor M3
                            angle       S1/S2  - XSMotor M4
                            light       S3     - LED3 white

  Trailer                   addon       JOY1FB - XSMotor M1
                            light       S3     - LED4/5 white, LED8/9 forward: red backward: white
                            turn signal F1     - LED3/7 blink orange
                                        F2     - LED6/10 blink orange

*/

/*

FtSwarmScreenQuickCfg::FtSwarmScreenQuickCfg( FtSwarmScreen *parent, FtSwarmSerialNumber_t device ) : FtSwarmScreen( parent, "Quick Config", NULL ) {

  this->device = device;

  add( new FtSwarmScreenSelectable( SWOSSCREENQUICKCFG_CAR,     this, "Car") );
  add( new FtSwarmScreenSelectable( SWOSSCREENQUICKCFG_CARTS,   this, "Car + signal") );
  add( new FtSwarmScreenSelectable( SWOSSCREENQUICKCFG_CAT,     this, "Catapillar") );
  add( new FtSwarmScreenSelectable( SWOSSCREENQUICKCFG_CATTS,   this, "Catapillar + signal") );
  add( new FtSwarmScreenSelectable( SWOSSCREENQUICKCFG_CRANE,   this, "Crane") );
  add( new FtSwarmScreenSelectable( SWOSSCREENQUICKCFG_TRAILER, this, "Trailer") );

}


void FtSwarmScreenQuickCfg::loadCfg( FtSwarmSerialNumber_t ctrl, FtSwarmSerialNumber_t device, uint8_t configuration, EventCfg_t *cfg, uint8_t items ) {

  for (uint8_t i=0; i<items; i++ ) {

    SwOSNVSEvent event( SwOSIOUID( ctrl,   cfg[i].sensorIoType, cfg[i].sensorPort), 
                        SwOSIOUID( device, cfg[i].actorIoType,  cfg[i].actorPort), 
                        SwOSTriggerMath( cfg[i].event, cfg[i].op, cfg[i].v1, cfg[i].v2),
                        cfg[i].parameter 
                      );
    nvs.addEvent( configuration, &event );

  }

}

bool FtSwarmScreenQuickCfg::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam  ) {

  if ( FtSwarmScreen::eventHandler( event, id, nParam, sParam ) ) return true;

  // function selected?
  if ( event == FTSWARM_SCREENEVENT_OK ) {

    if (id == 1234 ) {

      switch ( function ) {
        case SWOSSCREENQUICKCFG_CAR:    // loadCfg( myOSSwarm.Ctrl[0]->serialNumber, myOSSwarm.getController())
                                        return true;
        case SWOSSCREENQUICKCFG_CAT:    return true;
        case SWOSSCREENQUICKCFG_CRANE:  return true;
      }

      printf("hier gehts los %d %d.\n", id, function );
      return true;

    } else {
      function = id;
      screenManager.activate( new FtSwarmScreenChooseOption( this, 1234, "Configuration", "In which configuration should this function be applied?", 0, "#0", 1, "#1", 2, "#3", 3, "#4" ) );
      return true;

    }

  }

  return false;

}

*/

/***************************************************
 *
 * SwOSFactoryResetScreen 
 * Ask user to reset controller to factory setting
 *
 ***************************************************/

FtSwarmScreenFactoryReset::FtSwarmScreenFactoryReset( FtSwarmScreen *parent, FtSwarmScreen *next ) : FtSwarmScreen( parent, "Factory Reset", next ) {

  // set buttons
  add( new FtSwarmScreenS2( this, "YES" ) );
  add( new FtSwarmScreenS3( this, "NO" ) );
  
}

void FtSwarmScreenFactoryReset::draw( void ) {

  FtSwarmScreen::draw();
  oled.drawStr( FTSWARM_OLED_MAINSCREEN, oled.getScreenWidth()/2,  7, "Reset controller to", FTSWARM_ALIGNCENTER );
  oled.drawStr( FTSWARM_OLED_MAINSCREEN, oled.getScreenWidth()/2, 16, "factory settings?",   FTSWARM_ALIGNCENTER );
  
}

bool FtSwarmScreenFactoryReset::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  if ( FtSwarmScreen::eventHandler( event, id ) ) return true;

  // YES
  if ( id == FTSWARM_S2 ) {
    if ( event == FTSWARM_SCREENEVENT_DOWN )  myOSSwarm.factoryReset();
    return true;
  }

  // NO
  if ( id == FTSWARM_S3 ) {
    if ( event == FTSWARM_SCREENEVENT_DOWN ) screenManager.activate( new SwOSMainScreen( NULL, "Main" ) );
    return true;
  }

  return false;

}

/***************************************************
 *
 *   SwOSMainScreen
 *
 ***************************************************/

SwOSMainScreen::SwOSMainScreen( FtSwarmScreen *parent, const char *title ) : FtSwarmScreen( parent, title ) {

  blockEvents = false;

  add( new FtSwarmScreenS1( this, nvs.oledLabel[nvs.activeEventConfig][ SWOSLABEL_S1 ] ) );
  add( new FtSwarmScreenS2( this, nvs.oledLabel[nvs.activeEventConfig][ SWOSLABEL_S2 ] ) );
  add( new FtSwarmScreenS3( this, nvs.oledLabel[nvs.activeEventConfig][ SWOSLABEL_S3 ] ) );
  add( new FtSwarmScreenS4( this, "SET" ) );
  add( new FtSwarmScreenJ1( this, nvs.oledLabel[nvs.activeEventConfig][ SWOSLABEL_J1 ] ) );
  add( new FtSwarmScreenJ2( this, nvs.oledLabel[nvs.activeEventConfig][ SWOSLABEL_J2 ] ) );
  add( new FtSwarmScreenF1( this, nvs.oledLabel[nvs.activeEventConfig][ SWOSLABEL_F1 ] ) );
  add( new FtSwarmScreenF2( this, nvs.oledLabel[nvs.activeEventConfig][ SWOSLABEL_F2 ] ) );

}

void SwOSMainScreen::joystick( char *lr, char*fb, int16_t x, int16_t y, bool left ) {  

  const int8_t size = 11;
  int8_t b;

  if ( ( fb ) && ( fb[0] != '\0' ) ) {
  
    // ^
    b = y - size;
    oled.drawLine( FTSWARM_OLED_MAINSCREEN, x, b, x-3, b+3 );
    oled.drawLine( FTSWARM_OLED_MAINSCREEN, x, b, x+3, b+3 );

    oled.drawStr( FTSWARM_OLED_MAINSCREEN, x, b-9, fb, FTSWARM_ALIGNCENTER );

    // v
    b = y + size;
    oled.drawLine( FTSWARM_OLED_MAINSCREEN, x, b, x-3, b-3 );
    oled.drawLine( FTSWARM_OLED_MAINSCREEN, x, b, x+3, b-3 );
    
  }

  if ( ( lr ) && ( lr[0] != '\0' ) ) {

    // <
    b = x - size;
    oled.drawLine( FTSWARM_OLED_MAINSCREEN, b, y, b+3, y-3 );
    oled.drawLine( FTSWARM_OLED_MAINSCREEN, b, y, b+3, y+3 );

    if (left) oled.drawStr( FTSWARM_OLED_MAINSCREEN, b-2, y-3, lr, FTSWARM_ALIGNRIGHT );

      // >
    b = x + size;
    oled.drawLine( FTSWARM_OLED_MAINSCREEN, b, y, b-3, y-3 );
    oled.drawLine( FTSWARM_OLED_MAINSCREEN, b, y, b-3, y+3 );

    if (!left) oled.drawStr( FTSWARM_OLED_MAINSCREEN, b+2, y-3, lr, FTSWARM_ALIGNLEFT );

  }

}

void SwOSMainScreen::draw( void ) {

  FtSwarmScreen::draw();

  // Members
  uint8_t members = myOSSwarm.members();
  if ( members > 0) {
    char m[3];
    sprintf( m, "%d", members );
    oled.drawStr( FTSWARM_OLED_UPPERSCREEN, oled.getScreenWidth()-1, 0, m, FTSWARM_ALIGNRIGHT );
  }

  // Kelda
  if (myOSSwarm.Ctrl[0]->IAmKelda) oled.drawStr( FTSWARM_OLED_UPPERSCREEN, 0, 0, "K", FTSWARM_ALIGNLEFT );

  joystick( nvs.oledLabel[nvs.activeEventConfig][SWOSLABEL_JOY1LR], nvs.oledLabel[nvs.activeEventConfig][SWOSLABEL_JOY1FB], 48,     20, true );
  joystick( nvs.oledLabel[nvs.activeEventConfig][SWOSLABEL_JOY2LR], nvs.oledLabel[nvs.activeEventConfig][SWOSLABEL_JOY2FB], 128-48, 20, false );

  char cfg[5];
  sprintf( cfg, "#%d", nvs.activeEventConfig+1 );
  oled.drawStr( FTSWARM_OLED_MAINSCREEN, 64, 38, cfg, FTSWARM_ALIGNCENTER );

}

bool SwOSMainScreen::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  if ( id == FTSWARM_S4 ) {

      // set new screen and all done
      if ( event == FTSWARM_SCREENEVENT_DOWN ) {

        FtSwarmScreen *config = ( FtSwarmScreen * ) new FtSwarmScreenChooseConfig( this, 
                                                      new FtSwarmScreenSwarm( this, 
                                                        new FtSwarmScreenWifi( this, 
                                                          // new FtSwarmScreenFactoryReset( this, 
                                                            NULL ) ) );

        screenManager.activate( (FtSwarmScreen *) config );

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

 SwOSSplashScreen::SwOSSplashScreen( FtSwarmScreen *parent, const char *title ):FtSwarmScreen( parent, title ) {
  
  startTime = millis();
  blockEvents = false;

  add( new FtSwarmScreenS1( this, "" ) );
  add( new FtSwarmScreenS2( this, "" ) );
  add( new FtSwarmScreenS3( this, "" ) );
  add( new FtSwarmScreenS4( this, "" ) );
  add( new FtSwarmScreenJ1( this, "" ) );
  add( new FtSwarmScreenJ2( this, "" ) );
  add( new FtSwarmScreenF1( this, "" ) );
  add( new FtSwarmScreenF2( this, "" ) );

}

 void SwOSSplashScreen::draw( void ) {

  FtSwarmScreen::draw();
  
  // Logo
  oled.drawStr( FTSWARM_OLED_UPPERSCREEN, oled.getScreenWidth()/2, 0, "ftSwarm", FTSWARM_ALIGNCENTER );
  
  // hostname & version          
  char line[100];
  sprintf( line, "%s %s", nvs.swarmName, SWOSVERSION );
  oled.drawStr( FTSWARM_OLED_MAINSCREEN, oled.getScreenWidth()/2, 32, line, FTSWARM_ALIGNCENTER );
 
}

void SwOSSplashScreen::operate( void ) {

  // change to Main Screen after 5 seconds
  if ( millis()-startTime > 5000L ) {
    screenManager.activate( new SwOSMainScreen( NULL, title ) );
    toBeDestroyed = true;
  }

}

bool SwOSSplashScreen::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {
  
  // some pressed a key, so I change to the main screen
  screenManager.activate( new SwOSMainScreen( NULL, title ) );
  toBeDestroyed = true;
  return true;

}

/***************************************************
 *
 *   SwOS404Screen
 *
 ***************************************************/

void SwOS404Screen::draw( void ) {

  FtSwarmScreen::draw();
  oled.drawStr( FTSWARM_OLED_MAINSCREEN, 64, 0, "404", FTSWARM_ALIGNCENTER );
  oled.drawStr( FTSWARM_OLED_MAINSCREEN, 30, 0, "Page not found.", FTSWARM_ALIGNCENTER );  
}

/***************************************************
 *
 * EventQueue
 *
 ***************************************************/

QueueHandle_t FtSwarmScreenEventQueue;

static void screenEventTask( void *parameter ) {

  while (1) {

    FtSwarmScreenEventQueueElement_t *event;
  
    if ( xQueueReceive(FtSwarmScreenEventQueue, &event, portMAX_DELAY) ) {
      
      // process
      screenManager.eventHandler( event );

      // cleanup
      if ( event->sParam ) free( event->sParam );
      delete event;

    }

    // give other processes a change to run
    vTaskDelay(pdMS_TO_TICKS(1));

  }

}

/***************************************************
 *
 * FtSwarmScreenManager
 *
 ***************************************************/

FtSwarmScreenManager::FtSwarmScreenManager() {

  // initialize screen array
  for (uint8_t i=0; i<MAXSCREENS; i++ ) screen[i] = NULL;

  // create queue & receiver task
  FtSwarmScreenEventQueue = xQueueCreate( 10, sizeof( FtSwarmScreenEventQueueElement_t * ) );
  xTaskCreatePinnedToCore( screenEventTask, "EventTask", 10000, NULL, 1, NULL, ARDUINO_EVENT_RUNNING_CORE );

}

void FtSwarmScreenManager::eventHandler( FtSwarmScreen *screen, FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  FtSwarmScreenEventQueueElement_t *e = new FtSwarmScreenEventQueueElement_t();

  // fill event
  e->USID   = screen->USID;
  e->event  = event;
  e->id     = id;
  e->nParam = nParam;
  e->sParam = STRDUP(sParam);
  
  // send
  if ( xQueueSend( FtSwarmScreenEventQueue, &e, portMAX_DELAY ) != pdPASS ) SWARM_LOG_ERROR( "Enqueue screen event failed.");

}

void FtSwarmScreenManager::eventHandler( FtSwarmScreenEventQueueElement_t *event ) {

  if (!event) return;

  // test on valid pointers
  FtSwarmScreen *eventScreen = NULL;

  for (uint8_t i=0; i<MAXSCREENS; i++) {

    if ( ( screen[i] ) && ( event->USID == screen[i]->USID ) ) {
      eventScreen = screen[i];
      break;
    }

  }

  // screen already deallocated
  if (!eventScreen) return;

  // debugEvent( "eventHandler dequeue", eventScreen->getTitle(), event->event, event->id, event->nParam, event->sParam );

  // send event
  eventScreen->eventHandler( event->event, event->id, event->nParam, event->sParam );

}

void FtSwarmScreenManager::operate( void ) {

  // operate active screen
  if (active) active->operate();

  // garbage collector
  for ( uint8_t i=0; i<MAXSCREENS; i++ ) {
    if ( ( screen[i] ) && ( screen[i]->toBeDestroyed )  && ( screen[i] != active ) ) {
      FtSwarmScreen *obsolete = screen[i];
      screen[i] = NULL;
      delete obsolete;
    }

  }
  
}

void FtSwarmScreenManager::draw( void ) {

  if ( active ) active->draw();

}

uint8_t FtSwarmScreenManager::getIndex( FtSwarmScreen *screen ) {

  int8_t free = -1;

  for (uint8_t i=0; i<MAXSCREENS; i++ ) {

    // free space?
    if ( !this->screen[i] ) free = i;

    // myself?
    if ( this->screen[i] == screen ) return i;

  }

  if ( free >= 0 ) return free;

  SWARM_LOG_ERROR( "Screenmanager out of space. Garbage collection will not cleanup all screens any more." );
  return 0;

}

void FtSwarmScreenManager::registerMe( FtSwarmScreen *screen ) { 

  if ( screen ) {
    this->screen[ getIndex( screen ) ] = screen;
  }

}

void FtSwarmScreenManager::activate( FtSwarmScreen *screen ) { 

  if ( screen ) {
    
    if (active) active->deactivate( );
    active = screen;
    if ( active ) active->activate( );

    draw();

  } else {

    FtSwarmScreen *vier0vier = new SwOS404Screen( NULL );
    registerMe( vier0vier );
    activate( vier0vier );

  }

}

#endif