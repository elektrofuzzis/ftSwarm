/*
 * SwOSOLEDMenu.cpp
 *
 * OLED on Screen Menus
 * 
 * (C) 2025 Christian Bergschneider & Stefan Fuss
 * 
 */

// #include <WiFi.h>

#include "SwOSOLEDMenu.h"
#include "SwOSLog.h"
#include "SwOSHW/SwOSHWLocal.h"
#include "SwOSSwarm.h"
#include "SwOS.h"

// #define FTSWARMSCREEN_DEBUG

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

FtSwarmScreenObj::FtSwarmScreenObj( uint8_t id, FtSwarmScreen *parent, FtSwarmOledScreen_t screen, int16_t x, int16_t y ) {
  
  this->id     = id;
  this->parent = parent;
  this->screen = screen;
  this->x      = x;
  this->y      = y;

}

void FtSwarmScreenObj::debug( void ) {

  printf("id:%d text:%s type:%d x:%d, y:%d screen:%d visible:%d\n", getID(), getText(), getType(), x, y, screen, visible );

}

/***************************************************
 *
 * FtSwarmScreenLine
 *
 ***************************************************/

FtSwarmScreenLine::FtSwarmScreenLine( uint8_t id, FtSwarmScreen *parent, FtSwarmOledScreen_t screen, int16_t x0, int16_t y0, int16_t x1, int16_t y1 ) : FtSwarmScreenObj( id, parent, screen, x0, y0 ) {

  this->x1 = x1;
  this->y1 = y1;

}

void FtSwarmScreenLine::draw( bool selected ) {

  if ( visible ) oled.drawLine( screen, x, y, x1, y1 );

}

/***************************************************
 *
 * FtSwarmScreenTriangle - draw a triangle
 *
 ***************************************************/

FtSwarmScreenTriangle::FtSwarmScreenTriangle( uint8_t id, FtSwarmScreen *parent, FtSwarmOledScreen_t screen, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool filled ) : FtSwarmScreenObj( id, parent, screen, x0, y0 ) {

  this->x1 = x1;
  this->y1 = y1;
  this->x2 = x2;
  this->y2 = y2;
  this->filled = false;

}

void FtSwarmScreenTriangle::draw( bool selected ) {

  if ( visible ) oled.drawTriangle( screen, x, y, x1, y1, x2, y2, filled ? FTSWARM_OLED_FILLWHITE : FTSWARM_OLED_NOFILL );

}

int16_t FtSwarmScreenTriangle::getHeight( void ) {

  int16_t yMin = y;
  int16_t yMax = y;

  if ( yMin < y1 ) yMin = y1;
  if ( yMin < y2 ) yMin = y2;

  if ( y1 > yMax ) yMax = y1;
  if ( y2 > yMax ) yMax = y2;

  return yMax - yMin;

}

/***************************************************
 *
 * FtSwarmScreenText
 *
 ***************************************************/

FtSwarmScreenText::FtSwarmScreenText( uint8_t id, FtSwarmScreen *parent, FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t width, FtSwarmAlign_t align, const char *text ) : FtSwarmScreenObj( id, parent, screen, x, y ) {

  this->width = width;
  this->align = align;
  this->text  = strdup( text );

}

FtSwarmScreenText::FtSwarmScreenText( uint8_t id, FtSwarmScreen *parent, FtSwarmOledScreen_t screen, int16_t x, int16_t y, FtSwarmAlign_t align, const char *text ) : FtSwarmScreenText( id, parent, screen, x, y, oled.getTextWidth( text ), align, text ) {

}

FtSwarmScreenText::~FtSwarmScreenText() {

  if ( text ) free( text );

}

void FtSwarmScreenText::draw( bool selected ) {

  #ifdef FTSWARMSCREEN_DEBUG
    printf("draw(%d) ", selected );
    debug();
  #endif

  if ( text && (visible) ) oled.drawStrRect( screen, x, y, width, text, align );

}

int16_t FtSwarmScreenText::getHeight( void ) { 
  
  return oled.getTextHeight();
  
}

void FtSwarmScreenText::setText( const char *text ) {

  #ifdef FTSWARMSCREEN_DEBUG
    printf("setText(%s) ", text );
    debug();
  #endif

  if ( this->text ) free( this->text );
  this->text  = strdup( text );

}

/***************************************************
 *
 * FtSwarmScreenIO
 *
 ***************************************************/

FtSwarmScreenIO::FtSwarmScreenIO( uint8_t id, FtSwarmScreen *parent, FtSwarmOledScreen_t screen, int16_t x, int16_t y, FtSwarmAlign_t align, const char *text, SwOSIO *io ) : FtSwarmScreenText( id, parent, screen, x, y, align, text ) {

  this->io = io;

}

FtSwarmScreenIO::~FtSwarmScreenIO() {

  if (io) io->unsubscribe( this );

}

void FtSwarmScreenIO::activate( void ) {
  
  #ifdef FTSWARMSCREEN_DEBUG
    printf("activate " );
    debug();
  #endif

  if (!io) return;
  
  io->subscribe( this ); 
  value = io->getValueI32();

}

void FtSwarmScreenIO::deactivate( void ) {

  if (!io) return;
  
  io->unsubscribe( this ); 

}

void FtSwarmScreenIO::unregister( SwOSIO *io ) {

  if ( this->io == io ) this->io = nullptr;

}

void FtSwarmScreenIO::setValue( int32_t newValue ) {

  #ifdef FTSWARMSCREEN_DEBUG
    printf("setValue(%d) ", newValue );
    debug();
  #endif

  value = newValue;

  draw( false );

  FtSwarmScreenEvent_t event;

  if ( value > 0 ) event = FTSWARM_SCREENEVENT_UP;
  else             event = FTSWARM_SCREENEVENT_DOWN;

  if ( lastEvent != FTSWARM_SCREENEVENT_NONE ) screenManager.eventHandler( parent, event, id );

  lastEvent = event;

}

/***************************************************
 *
 * FtSwarmScreenSelectable - to be selected by joy1
 *
 ***************************************************/

FtSwarmScreenSelectable::FtSwarmScreenSelectable( uint8_t id, FtSwarmScreen *parent, FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t width, FtSwarmAlign_t align, const char *text ) : FtSwarmScreenText( id, parent, screen, x, y, width, align, text ) {

}

FtSwarmScreenSelectable::FtSwarmScreenSelectable( uint8_t id, FtSwarmScreen *parent, const char *text ) : FtSwarmScreenText( id, parent, FTSWARM_OLED_MAINSCREEN, oled.getScreenWidth()/2, (parent)?parent->getNextY( FTSWARM_OLED_MAINSCREEN):0, oled.getTextWidth(text), FTSWARM_ALIGNCENTER, text ) {

}

void FtSwarmScreenSelectable::draw( bool selected ) {

  if (visible) oled.drawStrRect( screen, x, y, width, text,  align, 1, 0, selected ? FTSWARM_OLED_FILLWHITE : FTSWARM_OLED_FILLBLACK ); 
  
}

/***************************************************
 *
 * FtSwarmScreenButton - button class
 *
 ***************************************************/

void FtSwarmScreenButton::draw( bool selected ) {

  #ifdef FTSWARMSCREEN_DEBUG
    printf("Button draw(%d) ", selected );
    debug();
  #endif

  if ( ( !visible ) || ( !text ) || ( text[0] == '\0' ) ) return;

  oled.drawStrRect( screen, x, y, width, text, align, 1, 1, value > 0 ? FTSWARM_OLED_FILLWHITE : FTSWARM_OLED_FILLBLACK );

} 
  
/***************************************************
 *
 * FtSwarmScreenObjList
 *
 ***************************************************/


FtSwarmScreenObjList::~FtSwarmScreenObjList( ) {

  if (list) delete list;

}

void FtSwarmScreenObjList::add( FtSwarmScreenObj *newObject ) {

  // selection?
  if ( ( !selected ) && ( newObject->getType() == FTSWARMSCREEN_SELECTABLE ) ) selected = (FtSwarmScreenSelectable *) newObject;

  // no existing data
  if (!list) { list = newObject; return; }

  uint8_t screen = newObject->getScreen();
  int16_t y = newObject->getY();

  // compare with first in list
  if ( ( list->getScreen() == screen ) && ( list->getY() > y ) ) {
    newObject->next = list;
    list = newObject;
    return;
  }

  // insert in list
  FtSwarmScreenObj *x = list;
  while (x->next) {
    if ( ( x->next->getScreen() == screen ) && ( x->next->getY() > y ) ) {
      newObject->next = x->next;
      x->next = newObject;
      return;
    }
    x = x->next;
  }

  // append
  x->next = newObject;
  return;

}

void FtSwarmScreenObjList::del( FtSwarmScreenObj *delObject ) {

  // empty list
  if ( !list ) return;

  // first element
  if ( list == delObject ) {

    // selection?
    if ( list == selected ) nextSelectable();

    // delete
    list = list->next;
    delObject->next = nullptr;
    delete delObject;
    return;

  }

  // in list
  FtSwarmScreenObj *x = list;
  while (x->next) {

    if ( x->next == delObject ) {

      // selection?
      if ( x == selected ) nextSelectable();

      // delete
      x->next = x->next->next;
      delObject->next = nullptr;
      delete delObject;
      return;

    }

  }  

}

void FtSwarmScreenObjList::deleteAll( FtSwarmOledScreen_t screen, FtSwarmScreenObj_t type ) {

  // empty list
  if ( !list ) return;

  // selection
  if ( type == FTSWARMSCREEN_SELECTABLE ) selected = nullptr;

  // first elements
  while ( ( list ) && ( list->getScreen() == screen ) && ( list->getType() == type ) ) {
    FtSwarmScreenObj *old = list;
    list = list->next;
    old->next = nullptr;
    delete old;
  }

  FtSwarmScreenObj *x = list;
  while ( x->next ) {
    
    if ( ( x->next->getScreen() == screen ) && ( x->next->getType() == type ) ) {

      // delete next element
      FtSwarmScreenObj *old = x->next;
      x->next = x->next->next;
      old->next = nullptr;
      delete old;

    } else {

      // continue
      x = x->next;

    }

  }

}

void FtSwarmScreenObjList::activate( void ) {

  FtSwarmScreenObj *x = list;

  while (x) {
    x->activate();
    x=x->next;
  }

}

void FtSwarmScreenObjList::deactivate( void ) {

  FtSwarmScreenObj *x = list;

  while (x) {
    x->deactivate();
    x=x->next;
  }

}

void FtSwarmScreenObjList::draw( void ) {

  FtSwarmScreenObj *x = list;

  while (x) {
    x->draw( x == selected );
    x=x->next;
  }

}

int16_t FtSwarmScreenObjList::getNextY( FtSwarmOledScreen_t screen ) {

  int16_t y = INT16_MIN;
  FtSwarmScreenObj *x = list;

  while (x) {
    if ( ( screen == x->getScreen() ) && ( y < x->getY() + x->getHeight() ) ) y = x->getY() + x->getHeight();
    x=x->next;
  }

  return (y>=0)? y : 0;


}

FtSwarmScreenSelectable *FtSwarmScreenObjList::nextSelectable( void ) {

  // no data
  if ( !list ) return nullptr;

  FtSwarmScreenObj *x;

  // start with selected or list
  if ( selected ) x = selected->next;
  else            x = list;

  while ( x ) {

    // next selectable found?
    if ( ( x->getScreen() == FTSWARM_OLED_MAINSCREEN ) && ( x->getType() == FTSWARMSCREEN_SELECTABLE ) ) {
      selected = (FtSwarmScreenSelectable *) x;
      return selected;
    }

    x = x->next;

  }

  return selected;

}

FtSwarmScreenSelectable *FtSwarmScreenObjList::prevSelectable( void ) {

  // no data
  if ( !list ) return nullptr;

  FtSwarmScreenObj *x    = list;
  FtSwarmScreenObj *prev = nullptr;
  
  while (x) {

    // found a selectable
    if ( ( x->getScreen() == FTSWARM_OLED_MAINSCREEN ) && ( x->getType() == FTSWARMSCREEN_SELECTABLE ) ) {

      // if selected is the actual element, so return the previous one
      if ( x == selected ) {
        if (prev) selected = (FtSwarmScreenSelectable *) prev;
        return selected;

      // store actual element and continue
      } else { prev = x; }

    }

    x = x->next;

  }

  return selected;

}

void FtSwarmScreenObjList::debug( void ) {

  printf("FtSwarmScreenObjList::debug\n");
  if (!list) { printf("null\n"); return; }

  printf("selected: ");
  if (selected) selected->debug();
  else printf("null\n");

  FtSwarmScreenObj *x = list;
  while (x) {
    x->debug();
    x = x->next;
  }

}

/***************************************************
 *
 * FtSwarmScreenJoystickPoti - Helper class
 *
 ***************************************************/

#define JOYMINVALUE 15
#define JOYMAXVALUE 25

FtSwarmScreenJoystickPoti::FtSwarmScreenJoystickPoti( uint8_t id, FtSwarmScreen *parent, FtSwarmOledScreen_t screen, int16_t x, int16_t y, FtSwarmAlign_t align, const char *text, SwOSIO *IO ):FtSwarmScreenIO( id, parent, screen, x, y, align, text, IO ) {

}

void FtSwarmScreenJoystickPoti::setValue( int32_t value ) {

    #ifdef FTSWARMSCREEN_DEBUG
    printf("FtSwarmScreenJoystickPoti %s.setValue(%d) ", getText(), value );
    debug();
  #endif

  // new event
  FtSwarmScreenEvent_t event = FTSWARM_SCREENEVENT_NONE;

  // store value
  this->value = value;

  // draw myself
  draw( false );

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

uint32_t screenUSID = 0;

FtSwarmScreen::FtSwarmScreen( FtSwarmScreen *parent, const char *title, const char *text ) {
  
  this->USID   = screenUSID++;
  this->parent = parent;

  oled.scroll( FTSWARM_OLED_MAINSCREEN, 0, 0 );
  oled.buttonScreen( false );

  // title, line
  add( new FtSwarmScreenText( FTSWARMSCREEN_NOID, this, FTSWARM_OLED_UPPERSCREEN, oled.getScreenWidth() / 2, 0, FTSWARM_ALIGNCENTER, title ) );
  add( new FtSwarmScreenLine( FTSWARMSCREEN_NOID, this, FTSWARM_OLED_UPPERSCREEN, 0, 12, oled.getScreenWidth()-1, 12 ) );

  // text in main screen
  splitText( text );

  // need a back button?
  if ( parent ) {
    addESC();
    ESC=true;
  }

  screenManager.registerMe( this );

}

void FtSwarmScreen::splitText( const char *text ) {

  if ( ( !text)  || ( text[0]=='\0' ) ) return;

  // ** split text **
  int16_t len = strlen( text );
  int16_t screenWidth = oled.getScreenWidth();
  int16_t textWidth =  oled.getTextWidth( text );

  // 1. Calculate average pixels per character
  // We use float or multiply by 10 to avoid integer truncation issues
  float avgCharWidth = (float)textWidth / len;

  // 2. Calculate how many characters actually fit in the screen width
  int16_t maxCharsPerRow = screenWidth / avgCharWidth;

  // Safety check: if a single char is wider than the screen
  if (maxCharsPerRow <= 0) maxCharsPerRow = 1;

  // Use maxCharsPerRow as your baseline for optLength
  int16_t optLength = maxCharsPerRow;
  
  // add some chars to be more flexible
  uint8_t diff = maxCharsPerRow - optLength;
  if ( diff > 4 ) optLength += 4;
  else            optLength += diff;
  
  // start splitting
  uint16_t cut;
  char *todo = (char *)text;
  char line[30];

  while ( todo[0] != '\0' ) {

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

    strncpy( line, todo, cut );
    line[cut] = '\0';
    addText( FTSWARM_OLED_MAINSCREEN, oled.getScreenWidth() / 2, getNextY( FTSWARM_OLED_MAINSCREEN ), FTSWARM_ALIGNCENTER, line );

    // skip space?
    if ( todo[cut] == ' ' ) cut++;

    // move pointer
    todo += cut;

  }

}

void FtSwarmScreen::addNavigation( void ) {

  // already installed?
  if (navigation) return;

  // add joysticks
  addJoystick( FTSWARM_OLED_NOSCREEN, "JOY1", 0 );
  // addJoystick( FTSWARM_OLED_MAINSCREEN, 1 );

  // all done
  navigation = true;

}

FtSwarmScreenObj *FtSwarmScreen::add( FtSwarmScreenObj *newObject ) {
  
  if (!newObject) return newObject;

  objects.add( newObject );

  if ( newObject->getType() == FTSWARMSCREEN_SELECTABLE ) addNavigation();

  if ( newObject->getScreen() == FTSWARM_OLED_BUTTONSCREEN ) {
    buttonScreen = true;
    oled.buttonScreen( true );
  }

  return newObject;

}

void FtSwarmScreen::draw( void ) {

  // cls
  oled.cls( );

  // draw objects in all screens
  objects.draw( );
  
  // vertical slider?
  oled.drawVSlider( FTSWARM_OLED_MAINSCREEN, objects.getNextY( FTSWARM_OLED_MAINSCREEN ) );

}

void FtSwarmScreen::activate( void ) {

  screenManager.blockEvents = blockEvents;
  objects.activate();
  oled.buttonScreen( buttonScreen );

}

void FtSwarmScreen::deactivate( void ) {

  objects.deactivate();

}

void FtSwarmScreen::close( FtSwarmScreenEvent_t event, uint8_t id, uint8_t nParam, const char *sParam ) { 

  screenManager.activate( parent ); 

  if ( event != FTSWARM_SCREENEVENT_NONE ) screenManager.eventHandler( parent, event, id, nParam, sParam );

  toBeDestroyed = true;

};

bool FtSwarmScreen::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  // explizit no handling?
  if ( id == FTSWARMSCREEN_NOID ) return true;

  // back
  if ( ( id == FTSWARM_F2 ) && ESC ) {
    
    if ( event == FTSWARM_SCREENEVENT_DOWN ) close();

    return true;

  }

  // I don't use navigation? done!
  if ( !navigation ) return false;

  if ( id == FTSWARMSCREEN_JOY1FB ) {

    FtSwarmScreenSelectable *lastSelection = objects.getSelected();
    FtSwarmScreenSelectable *nextSelection = nullptr;

    // go up ?
    if ( event == FTSWARM_SCREENEVENT_UP ) {
      if ( lastSelection ) nextSelection = objects.prevSelectable();
    }

    // go down ?
    if ( event == FTSWARM_SCREENEVENT_DOWN ) {
      if ( lastSelection ) nextSelection = objects.nextSelectable();
    }

    // found?
    if ( nextSelection ) {

      // need to scoll?
      if ( oled.scroll( FTSWARM_OLED_MAINSCREEN, nextSelection->getY(), nextSelection->getY() + oled.getTextHeight() ) ) draw( );

      // redraw old one unselected
      if ( lastSelection ) lastSelection->draw( false );

      // redraw new one selected
      if ( nextSelection ) nextSelection->draw( true );

    } else {

    }

    return true;

  }

  // select J1
  if ( id == FTSWARM_J1 ) {
    
    if ( ( event == FTSWARM_SCREENEVENT_DOWN ) && ( objects.getSelected() ) ) screenManager.eventHandler( this, FTSWARM_SCREENEVENT_OK, objects.getSelected()->getID() );

    return true;

  }

  // other events
  return false;

}

int16_t FtSwarmScreen::getNextY( FtSwarmOledScreen_t screen ) {

  return objects.getNextY( screen );

}


FtSwarmScreenButton *FtSwarmScreen::addS1( const char *text ) {

  return (FtSwarmScreenButton*) add( new FtSwarmScreenButton( FTSWARM_S1, this, FTSWARM_OLED_BUTTONSCREEN, 0, 0, FTSWARM_ALIGNLEFT, text, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_S1 ) ) );

}

FtSwarmScreenButton *FtSwarmScreen::addS2( const char *text ) {

  return (FtSwarmScreenButton*) add( new FtSwarmScreenButton( FTSWARM_S2, this, FTSWARM_OLED_BUTTONSCREEN, 41, 0, FTSWARM_ALIGNCENTER, text, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_S2 ) ) );

}

FtSwarmScreenButton *FtSwarmScreen::addS3( const char *text ) {

  return (FtSwarmScreenButton*) add( new FtSwarmScreenButton( FTSWARM_S3, this, FTSWARM_OLED_BUTTONSCREEN, oled.getScreenWidth() -1 -41, 0, FTSWARM_ALIGNCENTER, text, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_S3 ) ) );

}

FtSwarmScreenButton *FtSwarmScreen::addS4( const char *text ) {

  return (FtSwarmScreenButton*) add( new FtSwarmScreenButton( FTSWARM_S4, this, FTSWARM_OLED_BUTTONSCREEN, oled.getScreenWidth() -1, 0, FTSWARM_ALIGNRIGHT, text, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_S4 ) ) );

}

FtSwarmScreenButton *FtSwarmScreen::addF1( const char *text ) {

  return (FtSwarmScreenButton*) add( new FtSwarmScreenButton( FTSWARM_F1, this, FTSWARM_OLED_UPPERSCREEN, 0, 0, FTSWARM_ALIGNLEFT, text, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_F1  ) ) );

}

FtSwarmScreenButton *FtSwarmScreen::addF2( const char *text ) {

  return (FtSwarmScreenButton*) add( new FtSwarmScreenButton( FTSWARM_F2, this, FTSWARM_OLED_UPPERSCREEN, oled.getScreenWidth() -1, 0, FTSWARM_ALIGNRIGHT, text, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_F2 ) ) );

}

FtSwarmScreenButton *FtSwarmScreen::addESC( void ) { 
  
  return addF2( "^" ) ; 

};

FtSwarmScreenButton *FtSwarmScreen::addJ1( const char *text, FtSwarmOledScreen_t screen ) {

  return (FtSwarmScreenButton*) add( new FtSwarmScreenButton( FTSWARM_J1, this, screen, 48, 16, FTSWARM_ALIGNLEFT, text, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_J1 ) ) );

}

FtSwarmScreenButton *FtSwarmScreen::addJ2( const char *text, FtSwarmOledScreen_t screen ) {

  return (FtSwarmScreenButton*) add( new FtSwarmScreenButton( FTSWARM_J2, this, screen, oled.getScreenWidth() -1 -48, 16, FTSWARM_ALIGNRIGHT, text, myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_J2 ) ) );

}

FtSwarmScreenText *FtSwarmScreen::addText( FtSwarmOledScreen_t screen, int16_t x, int16_t y, FtSwarmAlign_t align, const char *text ) {

  if ( y + oled.getTextHeight() > oled.getScreenHeight( screen ) ) addNavigation();

  return (FtSwarmScreenText*) add( new FtSwarmScreenText( FTSWARMSCREEN_NOID, this, screen, x, y, align, text ) );

}

FtSwarmScreenLine *FtSwarmScreen::addLine( FtSwarmOledScreen_t screen, int16_t x1, int16_t y1, int16_t x2, int16_t y2 ) {

  if ( ( y1 > oled.getScreenHeight( screen ) ) || ( y2 > oled.getScreenHeight( screen ) ) ) addNavigation();

  return (FtSwarmScreenLine*) add( new FtSwarmScreenLine( FTSWARMSCREEN_NOID, this, screen, x1, y1, x2, y2 ) );

}

void FtSwarmScreen::addJoystick( FtSwarmOledScreen_t screen, const char *text, uint8_t joystick ) {

  switch ( joystick ) {
    case 0: add( new FtSwarmScreenJoystickPoti( FTSWARMSCREEN_JOY1LR, this, screen, 0, 0, FTSWARM_ALIGNLEFT, "", myOSSwarm.Ctrl[0]->getIO( SWOSIO_JOYSTICK_POTI, FTSWARM_HAL_FIRSTJPOTI   ) ) );
            add( new FtSwarmScreenJoystickPoti( FTSWARMSCREEN_JOY1FB, this, screen, 0, 0, FTSWARM_ALIGNLEFT, "", myOSSwarm.Ctrl[0]->getIO( SWOSIO_JOYSTICK_POTI, FTSWARM_HAL_FIRSTJPOTI+1 ) ) );
            addJ1( text, screen );
            break;

    case 1: add( new FtSwarmScreenJoystickPoti( FTSWARMSCREEN_JOY2LR, this, screen, 0, 0, FTSWARM_ALIGNLEFT, "", myOSSwarm.Ctrl[0]->getIO( SWOSIO_JOYSTICK_POTI, FTSWARM_HAL_FIRSTJPOTI+2 ) ) );
            add( new FtSwarmScreenJoystickPoti( FTSWARMSCREEN_JOY2FB, this, screen, 0, 0, FTSWARM_ALIGNLEFT, "", myOSSwarm.Ctrl[0]->getIO( SWOSIO_JOYSTICK_POTI, FTSWARM_HAL_FIRSTJPOTI+3 ) ) );
            addJ2( text, screen );
            break;
  }
  
}

/***************************************************
 *
 *   FtSwarmScreenChooseOption
 *
 ***************************************************/   

FtSwarmScreenChooseOption::FtSwarmScreenChooseOption( FtSwarmScreen *parent, const char *title, const char *text, uint8_t callbackID,
                                                      int32_t value1, const char *option1, 
                                                      int32_t value2, const char *option2, 
                                                      int32_t value3, const char *option3, 
                                                      int32_t value4, const char *option4 ) : FtSwarmScreen( parent, title, text ) {

  this->callbackID = callbackID;

  addESC();
  if (option1) addS1( option1 );
  if (option2) addS2( option2 );
  if (option3) addS3( option3 );
  if (option4) addS4( option4 );

  value[0] = value1;
  value[1] = value2;
  value[2] = value3;
  value[3] = value4;

}

bool FtSwarmScreenChooseOption::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  if ( FtSwarmScreen::eventHandler( event, id, nParam, sParam ) ) return true;

  if ( event == FTSWARM_SCREENEVENT_DOWN ) {
  
    // S1..S4
    if ( ( id >= FTSWARM_S1 ) && ( id <= FTSWARM_S4 ) ) screenManager.eventHandler( parent, FTSWARM_SCREENEVENT_OK, this->callbackID, value[id - FTSWARM_S1], nullptr );

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

FtSwarmScreenSelectList::FtSwarmScreenSelectList( FtSwarmScreen *parent, const char *title, const char *text, uint8_t items, uint8_t callbackID[], char *str[] ) : FtSwarmScreen( parent, title, text ) {

  for ( uint8_t i=0; i<items; i++ ) {
    add( new FtSwarmScreenSelectable( callbackID[i], this, (const char*) str[i] ) );
  }

}

bool FtSwarmScreenSelectList::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam , const char *sParam ) {

  if ( FtSwarmScreen::eventHandler( event, id, nParam, sParam ) ) return true;

  if ( event == FTSWARM_SCREENEVENT_OK ) { close( event, callbackID, objects.getSelected()->getID(), objects.getSelected()->getText() ); return true; }

  return false;

}

template<typename... Args>
FtSwarmScreenSelectList2::FtSwarmScreenSelectList2( FtSwarmScreen *parent, const char *title, const char *text, uint8_t callbackID, Args... args ) : FtSwarmScreen( parent, title, text ) {

  this->callbackID = callbackID;

  static_assert(sizeof...(args) % 2 == 0, "FtSwarmScreenSelectList::FtSwarmScreenSelectList - please call in tuples <uint8_t>, <char*>");

  process( args... );

};

template<typename... Tail>
void FtSwarmScreenSelectList2::process(uint8_t id, const char* str, Tail... tail) {

  add( new FtSwarmScreenSelectable( id, this, str ) );
  process(tail...);

}

bool FtSwarmScreenSelectList2::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam , const char *sParam ) {

  if ( FtSwarmScreen::eventHandler( event, id, nParam, sParam ) ) return true;

  if ( event == FTSWARM_SCREENEVENT_OK ) { close( event, callbackID, objects.getSelected()->getID(), objects.getSelected()->getText() ); return true; }

  return false;

}

/***************************************************
 *
 * FtSwarmScreenInput
 *
 ***************************************************/

void FtSwarmScreenInput::init(FtSwarmScreen *parent, const char *title, uint8_t callbackID, const char *param, uint8_t maxLength ) {

  lineHeight = oled.getTextHeight() + 3;
  charWidth  = oled.getTextWidth( " " ) + 3;

  this->callbackID = callbackID;

  this->maxLength = maxLength;
  input = (char *) calloc( this->maxLength+1, sizeof( char ) );

  uint8_t l = strlen( param );
  if ( l > this->maxLength ) l=this->maxLength;
  strncpy( input, param, l );

  setKeyboard( keyboard );

  addJoystick( FTSWARM_OLED_NOSCREEN, "", 0 );

  if (!numKeyboard) S1 = addS1( S1Label[keyboard] );
  
  addS3( "<]" );
  addS4( "OK" );

}

FtSwarmScreenInput::FtSwarmScreenInput( FtSwarmScreen *parent, const char *title, uint8_t callbackID, const char *param, uint8_t maxLength ) : FtSwarmScreen( parent, title, "" ) {

  init( parent, title, callbackID, param, maxLength );

}

FtSwarmScreenInput::FtSwarmScreenInput( FtSwarmScreen *parent, const char *title, uint8_t callbackID, int32_t param, uint8_t maxLength ) : FtSwarmScreen( parent, title, "" ) {

  char str[32];
  if ( param == FTSWARM_NANI32 ) str[0] = '\0';
  else                           itoa( param, str, 10 );

  keyboard    = 2;
  numKeyboard = true;
  init( parent, title, callbackID, str, maxLength );


}

FtSwarmScreenInput::~FtSwarmScreenInput() {
  free ( input );
}

void FtSwarmScreenInput::drawCursor( bool invert ) {
  
  uint8_t cx = keyboardX + cursorC[keyboard] * charWidth;
  uint8_t cy = keyboardY + cursorR[keyboard] * lineHeight;

  char key[2];
  key[0] = keyboardMap[keyboard][keymapIndex()];
  key[1] = '\0';

  // uint8_t flags = U8G2_BTN_BW1;
  // if ( invert) flags |= U8G2_BTN_INV;

  // oled.drawButton( FTSWARM_OLED_MAINSCREEN, cx, cy, 9, key, flags, 1, 1 );

  oled.drawStrRect( FTSWARM_OLED_MAINSCREEN, cx, cy, lineHeight, key, FTSWARM_ALIGNLEFT, 2, 2, invert? FTSWARM_OLED_FILLWHITE:FTSWARM_OLED_FILLBLACK );
  oled.drawRect( FTSWARM_OLED_MAINSCREEN, cx, cy, charWidth+1, lineHeight+1 );

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

  for (uint8_t r=0; r<rows[keyboard]; r++) {

    x1 = keyboardX;
    for (uint8_t c=0; c<cols[keyboard]; c++ ) {
      key[0] = keyboardMap[keyboard][ r*cols[keyboard] + c ];
      oled.drawStrRect( FTSWARM_OLED_MAINSCREEN, x1, y1, lineHeight, key, FTSWARM_ALIGNLEFT, 2, 2 );
      oled.drawRect( FTSWARM_OLED_MAINSCREEN, x1, y1, charWidth+1, lineHeight+1 );
      // oled.drawButton( FTSWARM_OLED_MAINSCREEN, x1+2, y1+2, 9, key, U8G2_BTN_BW1, 1, 1 );
      x1 += charWidth;
    }

    y1 += lineHeight;

  }

  // keys area
  // oled.drawRect( FTSWARM_OLED_MAINSCREEN, keyboardX, keyboardY, keyboardWidth, keyboardHeight );

  drawCursor( true );

}

void FtSwarmScreenInput::setKeyboard( uint8_t keyboard ) {

  if ( keyboard > KEYMAPS-1 ) this->keyboard = 0;
  else                        this->keyboard = keyboard;

  keyboardWidth  = cols[this->keyboard] * charWidth + 1;
  keyboardHeight = rows[this->keyboard] * lineHeight + 1;
  keyboardX      = ( oled.getScreenWidth() - this->keyboardWidth ) / 2;
  keyboardY      = 10;

}

bool FtSwarmScreenInput::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  uint8_t len;

  // joystick
  switch ( id ) {

    case FTSWARMSCREEN_JOY1LR:  drawCursor( false );
                      
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

    case FTSWARMSCREEN_JOY1FB:  drawCursor( false );

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
                        S1->setText( S1Label[keyboard] );
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
                        screenManager.eventHandler( parent, FTSWARM_SCREENEVENT_OK, callbackID, atoi(input), input );
                        return true;

      case FTSWARM_F2:  close();
                        return true;

    }

  }

  return false;

}

/******************************* FIRMWARE SCREENS ***********************************/

/***************************************************
 *
 *   FtSwarmScreenWifi
 *
 ***************************************************/

#define FTSWARMSCREENWIFI_MODE      ( FTSWARMSCREEN_BASEID + 0 )
#define FTSWARMSCREENWIFI_SSID      ( FTSWARMSCREEN_BASEID + 1 )
#define FTSWARMSCREENWIFI_PASSWD    ( FTSWARMSCREEN_BASEID + 2 )
#define FTSWARMSCREENWIFI_CB_MODE   ( FTSWARMSCREEN_BASEID + 3 )
#define FTSWARMSCREENWIFI_CB_SSID   ( FTSWARMSCREEN_BASEID + 4 )
#define FTSWARMSCREENWIFI_CB_PASSWD ( FTSWARMSCREEN_BASEID + 5 )
#define FTSWARMSCREENWIFI_CB_SAVE   ( FTSWARMSCREEN_BASEID + 6 )

FtSwarmScreenWifi::FtSwarmScreenWifi( FtSwarmScreen *parent  ) : FtSwarmScreen( parent, "Wifi Settings", "" ) {

  strcpy( wifiSSID, nvs.wifi.SSID );
  strcpy( wifiPwd,  nvs.wifi.Password );
  wifiMode = nvs.wifi.mode;

  int16_t y = getNextY( FTSWARM_OLED_MAINSCREEN );
  addText( FTSWARM_OLED_MAINSCREEN, 0, y, FTSWARM_ALIGNLEFT, "Mode" );
  wifiModeSelect = ( FtSwarmScreenSelectable* ) add( new FtSwarmScreenSelectable( FTSWARMSCREENWIFI_MODE,    this, FTSWARM_OLED_MAINSCREEN, 40,  1, oled.getScreenWidth()-40, FTSWARM_ALIGNLEFT, WIFI[wifiMode] ) );

  y = getNextY( FTSWARM_OLED_MAINSCREEN );
  wifiSSIDText   = addText( FTSWARM_OLED_MAINSCREEN, 0, y, FTSWARM_ALIGNLEFT, "SSID" );
  wifiSSIDSelect = ( FtSwarmScreenSelectable* ) add( new FtSwarmScreenSelectable( FTSWARMSCREENWIFI_SSID,    this, FTSWARM_OLED_MAINSCREEN, 40, 10, oled.getScreenWidth()-40, FTSWARM_ALIGNLEFT, wifiSSID ) );

  y = getNextY( FTSWARM_OLED_MAINSCREEN );
  wifiPwdText    = addText( FTSWARM_OLED_MAINSCREEN, 0, y, FTSWARM_ALIGNLEFT, "Passwd" );
  wifiPwdSelect  = ( FtSwarmScreenSelectable* ) add( new FtSwarmScreenSelectable( FTSWARMSCREENWIFI_PASSWD, this, FTSWARM_OLED_MAINSCREEN, 40, 19, oled.getScreenWidth()-40, FTSWARM_ALIGNLEFT, "*****" ) );

  if (wifiMode == wifiOFF ) {
    wifiSSIDText->setVisible( false );
    wifiSSIDSelect->setVisible( false );
    wifiPwdText->setVisible( false );
    wifiPwdSelect->setVisible( false );
  }

  S4 = addS4( "Save" );
  S4->setVisible(false);

}

bool FtSwarmScreenWifi::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam , const char *sParam ) {

  if ( FtSwarmScreen::eventHandler( event, id, nParam, sParam ) ) return true;

  if ( event == FTSWARM_SCREENEVENT_OK ) {

    bool changes = false;
    FtSwarmWifi_t newWifiMode;

    switch (id) {

      case FTSWARMSCREENWIFI_MODE:      screenManager.activate( new FtSwarmScreenChooseOption( this, "wifi mode", "Choose wifi mode.", FTSWARMSCREENWIFI_CB_MODE, wifiOFF, "off", wifiAP, "AP", wifiClient, "client" ) );
                                        break;

      case FTSWARMSCREENWIFI_CB_MODE:   newWifiMode = (FtSwarmWifi_t) nParam;
                                        if ( newWifiMode != wifiMode ) {
                                          wifiMode = newWifiMode;
                                          wifiModeSelect->setText( WIFI[wifiMode] );
                                          wifiSSIDText->setVisible( ( wifiMode != wifiOFF ) );
                                          wifiSSIDSelect->setVisible( ( wifiMode != wifiOFF ) );
                                          wifiPwdText->setVisible( ( wifiMode != wifiOFF ) );
                                          wifiPwdSelect->setVisible ( ( wifiMode != wifiOFF ) );
                                          changes = true;

                                        }
                                        break;

      case FTSWARMSCREENWIFI_SSID:      if ( ( wifiHandler ) && ( wifiMode == wifiClient ) ) screenManager.activate( new FtSwarmScreenWifiSSID( this, FTSWARMSCREENWIFI_CB_SSID ) ); 
                                        else                                                 screenManager.activate( new FtSwarmScreenInput( this, "SSID", FTSWARMSCREENWIFI_CB_SSID, wifiSSID, 63 ) );
                                        break;

      case FTSWARMSCREENWIFI_CB_SSID:   if (sParam) { 
                                          strcpy( wifiSSID, sParam); 
                                          wifiSSIDSelect->setText( wifiSSID );
                                          changes = true;
                                        }
                                        break;

      case FTSWARMSCREENWIFI_PASSWD:    screenManager.activate( new FtSwarmScreenInput( this, "Password", FTSWARMSCREENWIFI_CB_PASSWD, "", 63 ) );
                                        break;

      case FTSWARMSCREENWIFI_CB_PASSWD: if ( sParam) {
                                          if ( ( strlen(sParam) > 0 ) && ( strlen(sParam) < 8 ) ) screenManager.activate( new FtSwarmScreenError( this, "wifi passwords needs at minimum 8 chars" ) );
                                          else {
                                            strcpy( wifiPwd, sParam);
                                            changes = true;
                                          }
                                        }
                                        break;

      case FTSWARMSCREENWIFI_CB_SAVE:   if (nParam) {
                                          nvs.wifi.mode = wifiMode;
                                          strcpy( nvs.wifi.SSID, wifiSSID );
                                          strcpy( nvs.wifi.Password,  wifiPwd );
                                          nvs.saveAndRestart( FTSWARM_NVSSCOPE_WIFI );
                                        }
                                        break;

    }

    if ( changes ) {
      anythingChanged = true;
      S4->setVisible( true );
      draw();
    }

    return true;
    
  }
  
  if ( event == FTSWARM_SCREENEVENT_DOWN ) {

    if ( ( id == FTSWARM_S4 ) && ( anythingChanged ) )  screenManager.activate( new FtSwarmScreenYesNo( this, "wifi", "Save new settings and reboot?", FTSWARMSCREENWIFI_CB_SAVE ) );
    return true;

  }

  return false;

}

/***************************************************
 *
 *   FtSwarmScreenWifiSSID
 *
 ***************************************************/

FtSwarmScreenWifiSSID::FtSwarmScreenWifiSSID( FtSwarmScreen *parent, uint8_t id  ): FtSwarmScreen( parent, "SSID", "" ) {

  this->id = id;
  
  waitForScan = true;
  addNavigation();

  wifiHandler->startScan();  

}

FtSwarmScreenWifiSSID::~FtSwarmScreenWifiSSID() {
  
  wifiHandler->stopScan();
  
}

void FtSwarmScreenWifiSSID::draw( void ) {
  
  FtSwarmScreen::draw();

  if ( wifiHandler->scanActive ) oled.drawStr( FTSWARM_OLED_MAINSCREEN, oled.getScreenWidth()/2, 32,  "scanning...", FTSWARM_ALIGNCENTER );

}

bool FtSwarmScreenWifiSSID::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  if ( FtSwarmScreen::eventHandler( event, id, nParam, sParam ) ) return true;

  // must be a J1 click to select and close
  close( FTSWARM_SCREENEVENT_OK, this->id, objects.getSelected()->getID(), (char *) objects.getSelected()->getText() );

  return true;

}

void FtSwarmScreenWifiSSID::operate( void ) {

  if ( !wifiHandler )             return;
  if ( wifiHandler->scanActive )  return;
  if ( !waitForScan )             return;

  // scan done
  waitForScan = false;

  // cleanup
  wifiHandler->uniqueScanResult();

  // list 
  for ( uint16_t i=0; i<wifiHandler->aps; i++ ) add( new FtSwarmScreenSelectable( i + FTSWARMSCREEN_BASEID, this, FTSWARM_OLED_MAINSCREEN, oled.getScreenWidth()/2, getNextY( FTSWARM_OLED_MAINSCREEN ), oled.getScreenWidth(), FTSWARM_ALIGNCENTER, (char *) wifiHandler->ap[i].ssid ) );

  draw();

};

/***************************************************
 *
 *   FtSwarmScreenSwarm
 *
 ***************************************************/

 #define FTSWARMSCREENSWARM_CB_PIN  ( FTSWARMSCREEN_BASEID + 0 )
 #define FTSWARMSCREENSWARM_CB_ADD  ( FTSWARMSCREEN_BASEID + 1 )
 #define FTSWARMSCREENSWARM_CB_DEL  ( FTSWARMSCREEN_BASEID + 2 )
 #define FTSWARMSCREENSWARM_CB_SEL  ( FTSWARMSCREEN_BASEID + 3 )

 FtSwarmScreenSwarm::FtSwarmScreenSwarm( FtSwarmScreen *parent  ) : FtSwarmScreen( parent, "Swarm Config", "" ) {

  addS1( "add" );
  S2 = addS2( "del" );
  addS4( "pin" );

  addMembers();

}

void FtSwarmScreenSwarm::addMembers( void ) {

  uint8_t members = myOSSwarm.members();
  uint8_t item    = 0;

  for (uint8_t i=1; i<members ; i++ ) {
    
    if ( myOSSwarm.Ctrl[i] )  {

      int16_t y = getNextY( FTSWARM_OLED_MAINSCREEN );
      addText( FTSWARM_OLED_MAINSCREEN, 0, y, FTSWARM_ALIGNLEFT, myOSSwarm.Ctrl[i]->isOnline()? " " : "X" );
      add( new FtSwarmScreenSelectable( FTSWARMSCREENSWARM_CB_SEL + i, this, FTSWARM_OLED_MAINSCREEN, 10, y, oled.getScreenWidth()-10, FTSWARM_ALIGNLEFT, myOSSwarm.Ctrl[i]->getAliasOrName() ) );

    }

  }

  S2->setVisible( members>1 );
  
}

bool FtSwarmScreenSwarm::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  if ( FtSwarmScreen::eventHandler( event, id, nParam, sParam ) ) return true;

  if ( event == FTSWARM_SCREENEVENT_DOWN ) {

    uint8_t i;

    switch ( id ) {

      case FTSWARM_S1:  // ask for new device, call FTSWARMSCREENSWARM_CB_ADD afterwards
                        screenManager.activate( new FtSwarmScreenInput( this, "SN to add", FTSWARMSCREENSWARM_CB_ADD, FTSWARM_NANI32, 4 ) );
                        return true;

      case FTSWARM_S2:  // ask to delete the selected device, call FTSWARMSCREENSWARM_CB_DEL afterwards
                        if ( objects.getSelected() ) {
                          i = objects.getSelected()->getID() - FTSWARMSCREENSWARM_CB_SEL;
                          if ( ( i < MAXCTRL ) && ( myOSSwarm.Ctrl[i] ) ) screenManager.activate( new FtSwarmScreenYesNo( this, objects.getSelected()->getText(), "Revoke controller?", FTSWARMSCREENSWARM_CB_DEL, myOSSwarm.Ctrl[i]->serialNumber ) );
                        }
                        return true;

      case FTSWARM_S4:  // ask for a new swarm pin, call FTSWARMSCREENSWARM_CB_PIN afterwards
                        screenManager.activate( new FtSwarmScreenInput( this, "Swarm Pin", FTSWARMSCREENSWARM_CB_PIN, nvs.swarm.pin, 4 ) );
                        return true;

    }

  }

  if ( event == FTSWARM_SCREENEVENT_OK ) {

    char    error[100];
    uint8_t i;

    switch ( id )  {

      case FTSWARMSCREENSWARM_CB_PIN: // user entered new pin
                                      nvs.swarm.pin = nParam;
                                      nvs.save( FTSWARM_NVSSCOPE_SWARM );
                                      return true;

      case FTSWARMSCREENSWARM_CB_ADD: // nParam is the sn to be added to the swarm
                                      if (nParam) {

                                        if ( myOSSwarm.addController( nParam ) ) {

                                          nvs.save( FTSWARM_NVSSCOPE_SWARM );
                                          objects.deleteAll( FTSWARM_OLED_MAINSCREEN, FTSWARMSCREEN_SELECTABLE );
                                          objects.deleteAll( FTSWARM_OLED_MAINSCREEN, FTSWARMSCREEN_TEXT );
                                          addMembers();
                                          draw();
                                        
                                        } else {

                                          sprintf( error, "Could not add ftSwarm%d to swarm.", nParam);
                                          screenManager.activate( new FtSwarmScreenError( this, error) );

                                        }

                                      }

                                      return true;

      case FTSWARMSCREENSWARM_CB_DEL: // delete selected controller, nParam is sn
                                      if (nParam) { 

                                        if ( myOSSwarm.deleteController( nParam ) ) {

                                          nvs.save( FTSWARM_NVSSCOPE_SWARM );
                                          objects.deleteAll( FTSWARM_OLED_MAINSCREEN, FTSWARMSCREEN_SELECTABLE ); 
                                          objects.deleteAll( FTSWARM_OLED_MAINSCREEN, FTSWARMSCREEN_TEXT );
                                          addMembers();
                                          draw(); 

                                        } else {

                                          sprintf( error, "Couldn't revoke % from swarm.", objects.getSelected()->getText() );
                                          screenManager.activate( new FtSwarmScreenError( this, error) );
                                        }

                                      }
                                      return true;

    }

  }

  return false;

}

/***************************************************
 *
 * FtSwarmScreenRemote
 *
 ***************************************************/

#define FTSWARMSCREENREMOTE_CB_CHOOSECFG  ( FTSWARMSCREEN_BASEID + 0 )
#define FTSWARMSCREENREMOTE_CB_CHOOSECTRL ( FTSWARMSCREEN_BASEID + 30 )


FtSwarmScreenRemote::FtSwarmScreenRemote( FtSwarmScreen *parent  ) : FtSwarmScreen( parent, "Remote Control" ) {

  char text[50];

  sprintf( text, "config type:  %s", FTSWARMQUICKCONFIG[ nvs.events.quickConfig[nvs.events.activeConfig] ] );
  addText( FTSWARM_OLED_MAINSCREEN, 0, getNextY( FTSWARM_OLED_MAINSCREEN ), FTSWARM_ALIGNLEFT, text );
  
  sprintf( text, "active config: %d", nvs.events.activeConfig );
  addText( FTSWARM_OLED_MAINSCREEN, 0, getNextY( FTSWARM_OLED_MAINSCREEN ), FTSWARM_ALIGNLEFT, text );

  for ( uint8_t i=0; i<MAXNVSEVENTS; i++ ) {

    if ( nvs.events.events[ nvs.events.activeConfig ][i].sensor.serialNumber == 0 ) break;

    SwOSIO *sensor = myOSSwarm.getIO( nvs.events.events[ nvs.events.activeConfig ][i].sensor );
    SwOSIO *actor  = myOSSwarm.getIO( nvs.events.events[ nvs.events.activeConfig ][i].actor );
      
    if ( ( sensor ) && ( actor ) ) {

      sprintf( text, "%s -> %s", sensor->getAliasOrName(), actor->getAliasOrName() );
      addText( FTSWARM_OLED_MAINSCREEN, 0, getNextY( FTSWARM_OLED_MAINSCREEN ), FTSWARM_ALIGNLEFT, text );

    }

  }

  addS1( "Quick" );

}

 
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
void FtSwarmScreenRemote::configureSelected( uint8_t config ) {

  // is a controller selected?
  if (!objects.getSelected()) return;
  
  // get the controller
  uint i = objects.getSelected()->getID() - FTSWARMSCREENSWARM_CB_SEL;
  if ( ( i < 1 ) || ( i >= myOSSwarm.members() ) ) return;
  if ( !myOSSwarm.Ctrl[i] ) return;
  SwOSCtrl *ctrl = myOSSwarm.Ctrl[i];

  // test, if remote controller is online
  if ( !ctrl->isOnline() ) {
    screenManager.activate( new FtSwarmScreenInfo( this, "Remote controller is offline." ) );
    return;
  }

  // get serial numbers
  FtSwarmSerialNumber_t localSN  = myOSSwarm.Ctrl[0]->serialNumber;
  FtSwarmSerialNumber_t remoteSN = ctrl->serialNumber;

  // clean my config
  nvs.deleteAllEvents( nvs.events.activeConfig );

  if ( config  == FTSWARM_CFG_CAR ) {

    // Drive: JOY1.FB RC     M4 (WHEELDRIVE)
    //                others M1 (XS)
    nvs.addEvent( new SwOSNVSEvent( SwOSIOUID( localSN, SWOSIO_JOYSTICK_POTI, FTSWARM_JOY1FB ), 
                                    ( ctrl->getCPU() == FTSWARMRC_1V141 ) ? SwOSIOUID( remoteSN, SWOSIO_WHEELDRIVE, FTSWARM_M4 ) : 
                                                                            SwOSIOUID( remoteSN, SWOSIO_SMOTOR,     FTSWARM_M1 ), 
                                    SwOSTriggerMath( FTSWARM_TRIGGERVALUE, FTSWARM_ASSIGN, FTSWARM_SENSORVALUE, FTSWARM_MAXOPERAND ),
                                    0 
                                  )
                );
    strcpy( nvs.events.oledLabel[ nvs.events.activeConfig ][ SWOSLABEL_JOY1FB ], "FB" );

    // Steer: JOY2.LR RC     M1 (RCSERVO)
    //                others SERVO1
    nvs.addEvent( new SwOSNVSEvent( SwOSIOUID( localSN, SWOSIO_JOYSTICK_POTI, FTSWARM_JOY2LR ), 
                                    ( ctrl->getCPU() == FTSWARMRC_1V141 ) ? SwOSIOUID( remoteSN, SWOSIO_RCSERVO, FTSWARM_M1 ) : 
                                                                            SwOSIOUID( remoteSN, SWOSIO_SERVO,   FTSWARM_SERVO1 ), 
                                    SwOSTriggerMath( FTSWARM_TRIGGERVALUE, FTSWARM_ASSIGN, FTSWARM_SENSORVALUE, FTSWARM_MAXOPERAND ),
                                    0 
                                  )
                );
    strcpy( nvs.events.oledLabel[ nvs.events.activeConfig ][ SWOSLABEL_JOY2LR ], "LR" );

    // Gear: S1/S2 RS M2 (RCServo)
    if ( ctrl->getCPU() == FTSWARMRC_1V141 ) {
      nvs.addEvent( new SwOSNVSEvent( SwOSIOUID( localSN, SWOSIO_BUTTON, FTSWARM_S1 ), 
                                      SwOSIOUID( remoteSN, SWOSIO_RCSERVO, FTSWARM_M2 ), 
                                      SwOSTriggerMath( FTSWARM_TRIGGERUP, FTSWARM_ADD, FTSWARM_CONSTANT, FTSWARM_MAXOPERAND ),
                                      45
                                    )
                  );
      strcpy( nvs.events.oledLabel[ nvs.events.activeConfig ][ SWOSLABEL_S1 ], "G+" );
      nvs.addEvent( new SwOSNVSEvent( SwOSIOUID( localSN, SWOSIO_BUTTON, FTSWARM_S2 ), 
                                      SwOSIOUID( remoteSN, SWOSIO_RCSERVO, FTSWARM_M2 ), 
                                      SwOSTriggerMath( FTSWARM_TRIGGERUP, FTSWARM_ADD, FTSWARM_CONSTANT, FTSWARM_MAXOPERAND ),
                                      -45
                                    )
                  );
      strcpy( nvs.events.oledLabel[ nvs.events.activeConfig ][ SWOSLABEL_S2 ], "G-" );
    }

    // Function: F1/F2 RC     M3 XSMOTOR
    //                 others M2 XSMOTOR
    nvs.addEvent( new SwOSNVSEvent( SwOSIOUID( localSN, SWOSIO_BUTTON, FTSWARM_F1 ), 
                                    ( ctrl->getCPU() == FTSWARMRC_1V141 ) ? SwOSIOUID( remoteSN, SWOSIO_XSMOTOR, FTSWARM_M3 ) : 
                                                                            SwOSIOUID( remoteSN, SWOSIO_XSMOTOR, FTSWARM_M2 ), 
                                    SwOSTriggerMath( FTSWARM_TRIGGERUP, FTSWARM_ASSIGN, FTSWARM_CONSTANT, FTSWARM_MAXOPERAND ),
                                    50 
                                  )
                );
    nvs.addEvent( new SwOSNVSEvent( SwOSIOUID( localSN, SWOSIO_BUTTON, FTSWARM_F1 ), 
                                    ( ctrl->getCPU() == FTSWARMRC_1V141 ) ? SwOSIOUID( remoteSN, SWOSIO_XSMOTOR, FTSWARM_M3 ) : 
                                                                            SwOSIOUID( remoteSN, SWOSIO_XSMOTOR, FTSWARM_M2 ), 
                                    SwOSTriggerMath( FTSWARM_TRIGGERDOWN, FTSWARM_ASSIGN, FTSWARM_CONSTANT, FTSWARM_MAXOPERAND ),
                                    0 
                                  )
                );
    strcpy( nvs.events.oledLabel[ nvs.events.activeConfig ][ SWOSLABEL_F1 ], "F+" );
    nvs.addEvent( new SwOSNVSEvent( SwOSIOUID( localSN, SWOSIO_BUTTON, FTSWARM_F2 ), 
                                    ( ctrl->getCPU() == FTSWARMRC_1V141 ) ? SwOSIOUID( remoteSN, SWOSIO_XSMOTOR, FTSWARM_M3 ) : 
                                                                            SwOSIOUID( remoteSN, SWOSIO_XSMOTOR, FTSWARM_M2 ), 
                                    SwOSTriggerMath( FTSWARM_TRIGGERUP, FTSWARM_ASSIGN, FTSWARM_CONSTANT, FTSWARM_MAXOPERAND ),
                                    -50 
                                  )
                );
    nvs.addEvent( new SwOSNVSEvent( SwOSIOUID( localSN, SWOSIO_BUTTON, FTSWARM_F2 ), 
                                    ( ctrl->getCPU() == FTSWARMRC_1V141 ) ? SwOSIOUID( remoteSN, SWOSIO_XSMOTOR, FTSWARM_M3 ) : 
                                                                            SwOSIOUID( remoteSN, SWOSIO_XSMOTOR, FTSWARM_M2 ), 
                                    SwOSTriggerMath( FTSWARM_TRIGGERDOWN, FTSWARM_ASSIGN, FTSWARM_CONSTANT, FTSWARM_MAXOPERAND ),
                                    0 
                                  )
                );
    strcpy( nvs.events.oledLabel[ nvs.events.activeConfig ][ SWOSLABEL_F2 ], "F-" );

  } else if ( config == FTSWARM_CFG_CATAPILLAR ) {

    // Drive: JOY1.FB RC     M4 (WHEELDRIVE)
    //                others M1 (XS)
    nvs.addEvent( new SwOSNVSEvent( SwOSIOUID( localSN, SWOSIO_JOYSTICK_POTI, FTSWARM_JOY1FB ), 
                                    ( ctrl->getCPU() == FTSWARMRC_1V141 ) ? SwOSIOUID( remoteSN, SWOSIO_WHEELDRIVE, FTSWARM_M4 ) : 
                                                                            SwOSIOUID( remoteSN, SWOSIO_SMOTOR,     FTSWARM_M1 ), 
                                    SwOSTriggerMath( FTSWARM_TRIGGERVALUE, FTSWARM_ASSIGN, FTSWARM_SENSORVALUE, FTSWARM_MAXOPERAND ),
                                    0 
                                  )
                );
    strcpy( nvs.events.oledLabel[ nvs.events.activeConfig ][ SWOSLABEL_JOY1FB ], "FB" );

    // Steer: JOY2.LR RC     M1 (RCSERVO)
    //                others SERVO1
    nvs.addEvent( new SwOSNVSEvent( SwOSIOUID( localSN, SWOSIO_JOYSTICK_POTI, FTSWARM_JOY2LR ), 
                                    ( ctrl->getCPU() == FTSWARMRC_1V141 ) ? SwOSIOUID( remoteSN, SWOSIO_RCSERVO, FTSWARM_M1 ) : 
                                                                            SwOSIOUID( remoteSN, SWOSIO_SERVO,   FTSWARM_SERVO1 ), 
                                    SwOSTriggerMath( FTSWARM_TRIGGERVALUE, FTSWARM_ASSIGN, FTSWARM_SENSORVALUE, FTSWARM_MAXOPERAND ),
                                    0 
                                  )
                );
    strcpy( nvs.events.oledLabel[ nvs.events.activeConfig ][ SWOSLABEL_JOY2LR ], "LR" );

  }

  nvs.saveEvents();
  myOSSwarm.Ctrl[0]->loadFromNVS( );

}
*/

bool FtSwarmScreenRemote::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  if ( FtSwarmScreen::eventHandler( event, id, nParam, sParam ) ) return true;

  if ( event == FTSWARM_SCREENEVENT_DOWN ) {

    uint8_t i;
    uint8_t callbackID[MAXCTRL];
    char    *str[MAXCTRL];
    uint8_t items = 0;

    switch ( id ) {

      case FTSWARM_S1:  // Generate a list of controllers
                        for ( uint8_t i=0; i<MAXCTRL; i++ ) {
                          if ( ( myOSSwarm.Ctrl[i] ) && ( myOSSwarm.Ctrl[i]->isOnline() ) ) {
                            callbackID[items] = FTSWARMSCREENREMOTE_CB_CHOOSECTRL + i;
                            str[items]        = (char *) myOSSwarm.Ctrl[i]->getAliasOrName();
                            items++;
                          }
                        }

                        screenManager.activate( new FtSwarmScreenSelectList( this, "Select Controller", nullptr, items, callbackID, str ) );
      
                        return true;

    }

  }

  if ( event == FTSWARM_SCREENEVENT_OK ) {

    char    error[100];
    uint8_t i;

    switch ( id )  {

      case FTSWARMSCREENREMOTE_CB_CHOOSECFG:  // nParam = type
                                              return true;

      default:                                selectedCtrl = nParam - FTSWARMSCREENREMOTE_CB_CHOOSECTRL;
                                              screenManager.activate( new FtSwarmScreenSelectList2( this, 
                                                                                                    sParam, 
                                                                                                    nullptr, 
                                                                                                    FTSWARMSCREENREMOTE_CB_CHOOSECFG, 
                                                                                                    FTSWARM_CFG_CAR,     "Car", 
                                                                                                    FTSWARM_CFG_CAR,     "Catapillar", 
                                                                                                    FTSWARM_CFG_CRANE1,  "Crane Type 1", 
                                                                                                    FTSWARM_CFG_CRANE2,  "Crane Type 2", 
                                                                                                    FTSWARM_CFG_TRAILER, "Trailer" ) );
      
    }

  }

  return false;

}

/***************************************************
 *
 *   FtSwarmScreenSetup
 *
 ***************************************************/

#define FTSWARMSCREENSETUP_CONFIG      ( FTSWARMSCREEN_BASEID + 0 )
#define FTSWARMSCREENSETUP_CONFIG_CB   ( FTSWARMSCREEN_BASEID + 1 )
#define FTSWARMSCREENSETUP_WIFI        ( FTSWARMSCREEN_BASEID + 2 )
#define FTSWARMSCREENSETUP_SWARM       ( FTSWARMSCREEN_BASEID + 4 )
#define FTSWARMSCREENSETUP_REMOTE      ( FTSWARMSCREEN_BASEID + 5 )
#define FTSWARMSCREENSETUP_RESET       ( FTSWARMSCREEN_BASEID + 6 )
#define FTSWARMSCREENSETUP_RESET_CB    ( FTSWARMSCREEN_BASEID + 7 )
#define FTSWARMSCREENSETUP_CALIBRATION ( FTSWARMSCREEN_BASEID + 8 )
#define FTSWARMSCREENSETUP_SERVOOFFSET ( FTSWARMSCREEN_BASEID + 9 )

class FtSwarmScreenSetup:public FtSwarmScreen {

  public:

    // constructor
    FtSwarmScreenSetup( FtSwarmScreen *parent );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam );

};

 FtSwarmScreenSetup::FtSwarmScreenSetup( FtSwarmScreen *parent ):FtSwarmScreen( parent, "Setup", "" ) {
  
  blockEvents = true;

  add( new FtSwarmScreenSelectable( FTSWARMSCREENSETUP_CONFIG,      this, "Select Config") );
  add( new FtSwarmScreenSelectable( FTSWARMSCREENSETUP_CALIBRATION, this, "Calibration") );
  add( new FtSwarmScreenSelectable( FTSWARMSCREENSETUP_SERVOOFFSET, this, "Servo Offset") );
  add( new FtSwarmScreenSelectable( FTSWARMSCREENSETUP_REMOTE,      this, "Remote Control") );
  add( new FtSwarmScreenSelectable( FTSWARMSCREENSETUP_WIFI,        this, "Wifi Settings") );
  add( new FtSwarmScreenSelectable( FTSWARMSCREENSETUP_SWARM,       this, "Swarm Config") );
  add( new FtSwarmScreenSelectable( FTSWARMSCREENSETUP_RESET,       this, "Factory Reset") );
  
}

bool FtSwarmScreenSetup::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  if ( FtSwarmScreen::eventHandler( event, id ) ) return true;

  if ( event == FTSWARM_SCREENEVENT_OK ) {

    switch (id) {

      case FTSWARMSCREENSETUP_CONFIG:       screenManager.activate( new FtSwarmScreenChooseOption( this, "Select Config", "Choose acive configuration", FTSWARMSCREENSETUP_CONFIG_CB, 0, "#1", 1, "#2", 2, "#3", 3, "#4" ) );
                                            break;

      case FTSWARMSCREENSETUP_CONFIG_CB:    nvs.events.activeConfig = nParam;
                                            nvs.save( FTSWARM_NVSSCOPE_EVENTS );
                                            myOSSwarm.deleteEvents();
                                            myOSSwarm.addEvents( nParam );
                                            break;

      case FTSWARMSCREENSETUP_CALIBRATION:  screenManager.activate( new FtSwarmScreenCalibrateList( this ) );
                                            break;

      case FTSWARMSCREENSETUP_SERVOOFFSET:  screenManager.activate( new FtSwarmScreenServoOffsetList( this ) );
                                            break;

      case FTSWARMSCREENSETUP_WIFI:         screenManager.activate( new FtSwarmScreenWifi( this ) );
                                            break;

      case FTSWARMSCREENSETUP_SWARM:        screenManager.activate( new FtSwarmScreenSwarm( this ) );
                                            break;

      case FTSWARMSCREENSETUP_REMOTE:       screenManager.activate( new FtSwarmScreenRemote( this ) );
                                            break;

      case FTSWARMSCREENSETUP_RESET:        screenManager.activate( new FtSwarmScreenYesNo( this, "Factory Reset", "Reset to factory settings and reboot?", FTSWARMSCREENSETUP_RESET_CB ) );
                                            break;

      case FTSWARMSCREENSETUP_RESET_CB:     if (nParam) myOSSwarm.factoryReset();
                                            break;

    }

    return true;

  }

  return false;

}

/***************************************************
 *
 *   FtSwarmScreenSelectIO
 *
 ***************************************************/

#define FTSWARMSCREENSELECTIO_CB ( FTSWARMSCREEN_BASEID + 0 )

FtSwarmScreenSelectIO::FtSwarmScreenSelectIO( FtSwarmScreen *parent, const char *title, const char *text ) : FtSwarmScreen( parent, title, text ) {

}

void FtSwarmScreenSelectIO::addIO( SwOSIOType_t ioType, bool localOnly ) {

  // check all Controllers for ios with ioType
  for ( uint8_t ctrl=0; ctrl < (localOnly?1:MAXCTRL); ctrl++ ) {

    if (myOSSwarm.Ctrl[ctrl]) {

      // check all ports
      for ( int8_t i=0; i<myOSSwarm.Ctrl[ctrl]->IOs; i++ ) {

        if ( ( myOSSwarm.Ctrl[ctrl]->io[i] ) && ( myOSSwarm.Ctrl[ctrl]->io[i]->getIOType() == ioType ) ) {
          io[++maxIO] = myOSSwarm.Ctrl[ctrl]->io[i];
          char text[MAXIDENTIFIER*2+2];
          myOSSwarm.Ctrl[ctrl]->io[i]->getUniqueName( text );
          add( new FtSwarmScreenSelectable( FTSWARMSCREENSELECTIO_CB + maxIO, this, FTSWARM_OLED_MAINSCREEN, oled.getScreenWidth()/2,  getNextY( FTSWARM_OLED_MAINSCREEN ), oled.getScreenWidth(), FTSWARM_ALIGNCENTER, text ) );
        }

        // end of space in array?
        if ( maxIO>=19 ) return;

      }

    }

  }

}

/***************************************************
 *
 *   FtSwarmScreenCalibrateList
 *
 ***************************************************/

FtSwarmScreenCalibrateList::FtSwarmScreenCalibrateList( FtSwarmScreen *parent ) : FtSwarmScreenSelectIO( parent, "Calibrate" ) {

  addIO( SWOSIO_JOYSTICK, true );
  addIO( SWOSIO_RCSERVO, false );

}

bool FtSwarmScreenCalibrateList::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  if ( FtSwarmScreenSelectIO::eventHandler( event, id, nParam, sParam ) ) return true;

  if ( ( event == FTSWARM_SCREENEVENT_OK ) && ( id >= FTSWARMSCREENSELECTIO_CB ) ) {

    switch ( io[ id - FTSWARMSCREENSELECTIO_CB ]->getIOType() ) {

      case SWOSIO_JOYSTICK: screenManager.activate( new FtSwarmScreenCalibrateJoystick( parent, (SwOSJoystick*) io[ id - FTSWARMSCREENSELECTIO_CB ] ) );
                            return true;

    }

  }

  return false;

}

/***************************************************
 *
 *   FtSwarmScreenCalibrateJoystick
 *
 ***************************************************/

FtSwarmScreenCalibrateJoystick::FtSwarmScreenCalibrateJoystick( FtSwarmScreen *parent, SwOSJoystick *joystick ) : FtSwarmScreen( parent, joystick->getAliasOrName() ) {

  if (!joystick) SWARM_LOG_FATAL( "Parameter joystick is NULL.");
  this->joystick = joystick;

  // revoke joystick filters
  joystick->deleteFilters();

  // start with a new set of calibration values
  calibration[0] = { 1000, 2000, 3000 };
  calibration[1] = { 1000, 2000, 3000 };

  // calculate oled joystick visualize position
  int16_t size   = 8;
  int16_t midX   = ( joystick->getPort() ? oled.getScreenWidth() - 3 * size : 2 * size + 2 );
  int16_t midY   = 2 * size + 2;
  int16_t space  = 2;

  // print joystick on oled
  uint8_t port = joystick->getPort();
  triangle[2+port] = (FtSwarmScreenTriangle *) add( new FtSwarmScreenTriangle( 0, this, FTSWARM_OLED_MAINSCREEN, midX - 2*size-space, midY,                midX-size-space, midY-size,       midX-size-space, midY+size,       false ) );
  triangle[3-port] = (FtSwarmScreenTriangle *) add( new FtSwarmScreenTriangle( 1, this, FTSWARM_OLED_MAINSCREEN, midX + 2*size+space, midY,                midX+size+space, midY-size,       midX+size+space, midY+size,       false ) );
  triangle[0+port] = (FtSwarmScreenTriangle *) add( new FtSwarmScreenTriangle( 2, this, FTSWARM_OLED_MAINSCREEN, midX,                midY - 2*size-space, midX-size,       midY-size-space, midX+size,       midY-size-space, false ) );
  triangle[1-port] = (FtSwarmScreenTriangle *) add( new FtSwarmScreenTriangle( 3, this, FTSWARM_OLED_MAINSCREEN, midX,                midY + 2*size+space, midX-size,       midY+size+space, midX+size,       midY+size+space, false ) );
  
  // add text on oled
  int16_t textOffset = ( joystick->getPort() ? 0 : 4*size + 10 );
  text[0] = addText( FTSWARM_OLED_MAINSCREEN, textOffset, 10, FTSWARM_ALIGNLEFT, "Rotate to fill" );
  text[1] = addText( FTSWARM_OLED_MAINSCREEN, textOffset, 20, FTSWARM_ALIGNLEFT, "all triangles" );

}


bool FtSwarmScreenCalibrateJoystick::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  // don't call the parent class' event handler since we need to work on ESC

  if ( event == FTSWARM_SCREENEVENT_DOWN ) {

    uint8_t i = 2*joystick->getPort();

    if ( id == FTSWARM_S4 ) {
      // save new settings
      memcpy( &nvs.joystick[i], joystick, 2 * sizeof( SwOSJoyCalibration_t ) );
      nvs.save( FTSWARM_NVSSCOPE_JOYSTICK );
    }

    // restore filters
    joystick->addFilters();

    close();
    
  }

  return true;

}

void FtSwarmScreenCalibrateJoystick::operate( void ) {

  int32_t value[2];
  value[0] = joystick->fb->getValueI32();
  value[1] = joystick->lr->getValueI32();

  // status stores the calibration progress
  // 0x00..0x0F - step 1 test all directions
  // 0x0F..0x3F - step 2 wait for released joystick
  // 0x4F                wait for user

  if ( status < 0x0F ) {

    // first step, get min/max poti values

    for (uint8_t i=0; i<=1; i++) {

      if ( value[i] < calibration[i].minValue ) {
        calibration[i].minValue = value[i];
        triangle[2*i]->setFilled( true );
        status = status | ( 1 << i );
      }

      if ( value[i] > calibration[i].maxValue ) {
        calibration[i].maxValue = value[i];
        triangle[2*i+1]->setFilled( true );
        status = status | ( 4 << i );
      }
    
    }

    // switch to 2nd step?
    if ( status >= 0x0F ) {
      text[0]->setText("Release to");
      text[1]->setText("get mid pos");
      this->draw();
    }

  } else if ( status < 0x40 ) {
    // 2nd step: stable value for 3 times?

    value[0] = value[0] >> 1;
    value[1] = value[1] >> 1;
    
    if ( ( lastValue[0] == value[0] ) && ( lastValue[1] == value[1] ) ) {
      status += 0x10;
    
    } else { 

      lastValue[0] = value[0]; 
      lastValue[1] = value[1];
      status = 0x0F;

    }

    if ( status > 0x3F ) {

      calibration[0].midValue = value[0]<<1;
      calibration[1].midValue = value[1]<<1;

      text[0]->setText( "Save new" );
      text[1]->setText( "calibration?" );

      addS4( "save" )->activate();

      this->draw();

    }

  }

}

/***************************************************
 *
 *   FtSwarmScreenServoOffsetList
 *
 ***************************************************/

FtSwarmScreenServoOffsetList::FtSwarmScreenServoOffsetList( FtSwarmScreen *parent ) : FtSwarmScreenSelectIO( parent, "Servo Offset" ) {

  addIO( SWOSIO_SERVO, false );
  addIO( SWOSIO_RCSERVO, false );

}

bool FtSwarmScreenServoOffsetList::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  if ( FtSwarmScreenSelectIO::eventHandler( event, id, nParam, sParam ) ) return true;

  if ( ( event == FTSWARM_SCREENEVENT_OK ) && ( id >= FTSWARMSCREENSELECTIO_CB ) ) {

    screenManager.activate( new FtSwarmScreenServoOffset( parent, (SwOSServo*) io[ id - FTSWARMSCREENSELECTIO_CB ] ) );
    return true;

  }

  return false;

}

/***************************************************
 *
 *   FtSwarmScreenServoOffset
 *
 ***************************************************/

FtSwarmScreenServoOffset::FtSwarmScreenServoOffset( FtSwarmScreen *parent, SwOSServo *servo ) : FtSwarmScreen( parent, servo->getAliasOrName() ) {

  this->servo = servo;

  char x[20];
  sprintf( x, "Offset: %2d", servo->getOffset() );
  text = addText( FTSWARM_OLED_MAINSCREEN, oled.getScreenWidth()/2, 15, FTSWARM_ALIGNCENTER, x );

  addS1( "-" );
  addS2( "+" );
  addS4( "Save" );

}

bool FtSwarmScreenServoOffset::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  if ( FtSwarmScreen::eventHandler( event, id, nParam, sParam ) ) return true;

  if ( event == FTSWARM_SCREENEVENT_DOWN ) {

    int16_t offset = servo->getOffset();

    switch ( id ) {

      case FTSWARM_S1:  if ( offset > 5 ) offset = offset - 5;
                        break;

      case FTSWARM_S2:  if ( offset < 85 ) offset = offset + 5;
                        break;
                        
      case FTSWARM_S4:  servo->getCtrl()->save( FTSWARM_NVSSCOPE_SERVO, servo->getPort() );
                        close();
                        return true;

    }

    servo->setOffset( offset );
    char x[20];
    sprintf( x, "Offset: %2d", servo->getOffset() );
    text->setText( x );
    text->draw();

    return true;

  }

  return false;

}

/***************************************************
 *
 *   SwOSMainScreen
 *
 ***************************************************/

SwOSMainScreen::SwOSMainScreen( void ) : FtSwarmScreen( nullptr, myOSSwarm.Ctrl[0]->getAliasOrName() ) {

  blockEvents = false;

  addS1( nvs.events.oledLabel[nvs.events.activeConfig][ SWOSLABEL_S1 ] );
  addS2( nvs.events.oledLabel[nvs.events.activeConfig][ SWOSLABEL_S2 ] );
  addS3( nvs.events.oledLabel[nvs.events.activeConfig][ SWOSLABEL_S3 ] );
  addS4( "SET" );
  addJ1( nvs.events.oledLabel[nvs.events.activeConfig][ SWOSLABEL_J1 ] );
  addJ2( nvs.events.oledLabel[nvs.events.activeConfig][ SWOSLABEL_J2 ] );
  addF1( nvs.events.oledLabel[nvs.events.activeConfig][ SWOSLABEL_F1 ] );
  addF2( nvs.events.oledLabel[nvs.events.activeConfig][ SWOSLABEL_F2 ] );

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

  /*
  // Members
  uint8_t members = myOSSwarm.members();
  if ( members > 0) {
    char m[3];
    sprintf( m, "%d", members );
    oled.drawStr( FTSWARM_OLED_UPPERSCREEN, oled.getScreenWidth()-1, 0, m, FTSWARM_ALIGNRIGHT );
  }

  // Kelda
  if (myOSSwarm.Ctrl[0]->IAmKelda) oled.drawStr( FTSWARM_OLED_UPPERSCREEN, 0, 0, "K", FTSWARM_ALIGNLEFT );
  */

  joystick( nvs.events.oledLabel[nvs.events.activeConfig][SWOSLABEL_JOY1LR], nvs.events.oledLabel[nvs.events.activeConfig][SWOSLABEL_JOY1FB], 48,     20, true );
  joystick( nvs.events.oledLabel[nvs.events.activeConfig][SWOSLABEL_JOY2LR], nvs.events.oledLabel[nvs.events.activeConfig][SWOSLABEL_JOY2FB], 128-48, 20, false );

  char cfg[5];
  sprintf( cfg, "#%d", nvs.events.activeConfig+1 );
  oled.drawStr( FTSWARM_OLED_BUTTONSCREEN, oled.getScreenWidth()/2, 0, cfg, FTSWARM_ALIGNCENTER );

}

bool SwOSMainScreen::eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam, const char *sParam ) {

  if ( id == FTSWARM_S4 ) {

      // set new screen and all done
      if ( event == FTSWARM_SCREENEVENT_DOWN ) {

        FtSwarmScreen *config = ( FtSwarmScreen * ) new FtSwarmScreenSetup( this );

        // FtSwarmScreen *config = ( FtSwarmScreen * ) new FtSwarmScreenChooseConfig( this, nullptr );

        /*
        FtSwarmScreen *config = ( FtSwarmScreen * ) new FtSwarmScreenChooseConfig( this,          
                                                      new FtSwarmScreenSwarm( this, 
                                                        new FtSwarmScreenWifi( this, 
                                                          // new FtSwarmScreenFactoryReset( this, 
                                                            nullptr ) ) );
        */
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

 SwOSSplashScreen::SwOSSplashScreen( void ):FtSwarmScreen( nullptr, "Booting..." ) {
  
  blockEvents = false;
  addText( FTSWARM_OLED_MAINSCREEN, oled.getScreenWidth()/2, 16, FTSWARM_ALIGNCENTER, "ftSwarm" );

  // hostname & version 
  char line[100];
  sprintf( line, "%s %s", nvs.swarm.name, SWOSVERSION );
  info = addText( FTSWARM_OLED_MAINSCREEN, oled.getScreenWidth()/2, 32, FTSWARM_ALIGNCENTER, line );

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
  for ( uint8_t i=0; i<MAXSCREENS; i++ ) screen[i] = nullptr;

  // create queue & receiver task
  FtSwarmScreenEventQueue = xQueueCreate( 10, sizeof( FtSwarmScreenEventQueueElement_t * ) );
  xTaskCreatePinnedToCore( screenEventTask, "EventTask", 10000, nullptr, 1, nullptr, ARDUINO_EVENT_RUNNING_CORE );

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
  FtSwarmScreen *eventScreen = nullptr;

  for (uint8_t i=0; i<MAXSCREENS; i++) {

    if ( ( screen[i] ) && ( event->USID == screen[i]->USID ) ) {
      eventScreen = screen[i];
      break;
    }

  }

  // screen already deallocated
  if (!eventScreen) return;

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
      screen[i] = nullptr;
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

    FtSwarmScreen *vier0vier = new SwOS404Screen( );
    registerMe( vier0vier );
    activate( vier0vier );

  }

}

void FtSwarmScreenManager::setState( SwOSState_t state, const char *text, uint8_t members, const char *SSID ) {

  switch (state) {

    case BOOTING:   if (!splashScreen) splashScreen = new SwOSSplashScreen( );
                    activate( splashScreen );
                    break;

    case STARTWIFI: if ( (splashScreen) && ( splashScreen->info )  ) splashScreen->info->setText( text );
                    break;

    case RUNNING:   activate( new SwOSMainScreen( ) );
                    // if (splashScreen) splashScreen->close();
                    break;

    case ERROR:     activate( new FtSwarmScreenError( active, text ) );
                    break;

    case WAITING:   activate( new FtSwarmScreenInfo( active, text ) );
                    break;

    case IDENTIFY:  activate( new FtSwarmScreenInfo( active, text ) );
                    break;

    case FATAL:     activate( new FtSwarmScreen( active, "Fatal Error", text ) );
                    break;

    case FACTORY1:  if ( (splashScreen) && ( splashScreen->info )  ) splashScreen->info->setText( "Factory Reset?" );
                    break;

    case FACTORY2:  if ( (splashScreen) && ( splashScreen->info )  ) splashScreen->info->setText( "Factory Reset!" );
                    break;
  }

}

#endif