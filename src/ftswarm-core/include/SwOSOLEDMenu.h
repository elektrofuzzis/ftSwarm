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
 * SwOSScreenObj - Any element on a screen
 *
 ***************************************************/

class SwOSScreenObj{

  protected:
    uint8_t x, y;
    FtSwarmAlign_t align;
    char *label = NULL;

  public:
    SwOSScreenObj( const char *label, uint8_t x, uint8_t y, FtSwarmAlign_t align );
    ~SwOSScreenObj();
    void setLabel( const char *label );
    virtual void draw( bool inverted = false );
};

/***************************************************
 *
 * SwOSScreenSlider - Any element on a screen
 *
 ***************************************************/

class SwOSScreenSlider : public SwOSScreenObj {

  protected:
    uint8_t size, direction;
    void drawHorizontal( void );
    void drawVertical( void );

  public:

    static const uint8_t HORIZONTAL = 0;
    static const uint8_t VERTICAL   = 1;
    
    SwOSScreenSlider( const char *label, uint8_t x, uint8_t y, uint8_t size, uint8_t direction );
    virtual void draw( bool inverted = false );
};

/***************************************************
 *
 * SwOSScreenSelector - a combo box
 *
 ***************************************************/

class SwOSSelectorItem {

  public:
    int8_t id;
    char text[25];
    SwOSSelectorItem *next;
    SwOSSelectorItem *prev;

    SwOSSelectorItem( int8_t id, const char *text, SwOSSelectorItem *prev );
    ~SwOSSelectorItem();
    void add( int8_t id, const char *text );

};

class SwOSScreenSelector : public SwOSScreenObj {

  protected:
    uint8_t x, y;
    SwOSSelectorItem *items = NULL;
    SwOSSelectorItem *active = NULL;

  public:
   
    SwOSScreenSelector( uint8_t x, uint8_t y );
    ~SwOSScreenSelector();
    virtual void add( int8_t id, const char *text );
    virtual void draw( bool inverted = false );
};

/***************************************************
 *
 * SwOSScreen - Base class for all screens
 *
 ***************************************************/

 class SwOSScreen {

  protected:
    SwOSScreen    *parent = NULL;
    SwOSScreenObj *obj[8];

  public:

    // constructor
    SwOSScreen( SwOSScreen *parent );

    // cls and draw all elements
    virtual void draw( void ); 

    // set a new label text, if applicable
    virtual void setLabel( SwOSIOType_t ioType, uint8_t port, const char *label ) {};

    // called all 25ms, so be minimalistic
    virtual void operate( void ) {};

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmToggle_t toggle, SwOSIOType_t ioType, uint8_t port ) { return false; };

};

/***************************************************
 *
 * SwOSChooseConfigScreenObj - asks about the config S1..S4
 *
 ***************************************************/

 class SwOSChooseConfigScreen : public SwOSScreen {

  protected:

  public:

    // constructor
    SwOSChooseConfigScreen( SwOSScreen *parent );

    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmToggle_t toggle, SwOSIOType_t ioType, uint8_t port );

};

/***************************************************
 *
 * SwOSMainScreen - Main OLED visualizer
 *
 ***************************************************/

class SwOSMainScreen : public SwOSScreen {

  protected:

    // ToDo: replace with own SwScreenObj derives class
    void joystick( char *lr, char *fb, int8_t x, int8_t y, bool left );

  public:

    // Constructor
    SwOSMainScreen( SwOSScreen *parent );

    // cls and draw all elements
    virtual void draw( void );

    // set a new label text, if applicable
    virtual void setLabel( SwOSIOType_t ioType, uint8_t port, const char *label );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmToggle_t toggle, SwOSIOType_t ioType, uint8_t port );

};

/***************************************************
 *
 * SwOSTestScreen
 *
 ***************************************************/

class SwOSTestScreen : public SwOSScreen {

  protected:

  public:

    // Constructor
    SwOSTestScreen( SwOSScreen *parent );

    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmToggle_t toggle, SwOSIOType_t ioType, uint8_t port );

};

/***************************************************
 *
 * SwOSSplashScreen 
 * startup screen, switches after 5s to MainScreen
 *
 ***************************************************/

class SwOSSplashScreen : public SwOSScreen {

  protected:
    unsigned long startTime;

  public:

    // constructor
    SwOSSplashScreen( SwOSScreen *parent ):SwOSScreen( parent ) { startTime = millis(); };
    
    // cls and draw all elements
    virtual void draw( void );
    
    // called all 25ms, so be minimalistic
    virtual void operate( void );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmToggle_t toggle, SwOSIOType_t ioType, uint8_t port );

};

/***************************************************
 *
 * SwOSFactoryResetScreen 
 * Ask user to reset controller to factory setting
 *
 ***************************************************/

class SwOSFactoryResetScreen : public SwOSScreen {

  public:

    // constructor
    SwOSFactoryResetScreen( SwOSScreen *parent );
    
    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmToggle_t toggle, SwOSIOType_t ioType, uint8_t port );

};

/***************************************************
 *
 *   SwOS404Screen - ScreenNotFound
 *
 ***************************************************/

class SwOS404Screen : public SwOSScreen {

  protected:

  public:

    // constructor
    SwOS404Screen( SwOSScreen *parent ):SwOSScreen( parent ) {  };
    
    // cls and draw all elements
    virtual void draw( void );

};

class SwOSScreenManager {

  protected:
    SwOSScreen *next       = NULL;
    bool       autoCleanUp = false;

  public:

    SwOSScreen *active     = NULL;

    // does all the stuff to replace and operate the screen
    void operate( void );

    // draw active screen
    void draw( void );

    // handle external events like pressing buttons
    bool eventHandler( FtSwarmToggle_t toggle, SwOSIOType_t ioType, uint8_t port );
    
    // Replace the active screen by a new screen and in case of autoCleanUp delete the old one.
    // Replacement is done with next operate()-call to avoid stress
    // Be aware of potential race conditions by very fast screen replacements. 
    void newScreen( SwOSScreen *newScreen, bool autoCleanUp );

    // replace a label text
    void setLabel( SwOSIOType_t ioType, uint8_t port, const char *label );

};

extern SwOSScreenManager screenManager;
