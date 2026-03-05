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

typedef enum { FTSWARM_SCREENEVENT_NONE = -1, FTSWARM_SCREENEVENT_DOWN, FTSWARM_SCREENEVENT_UP, FTSWARM_SCREENEVENT_OK } FtSwarmScreenEvent_t;

/***************************************************
 *
 * SwOSScreenObj - Any element on a screen
 *
 ***************************************************/

class SwOSScreen;

class SwOSScreenObj {

  protected:
    uint8_t        id;
    SwOSIO         *io     = NULL;
    SwOSScreen     *parent = NULL;
    int32_t        value   = FTSWARM_NANI32;
    char           *label  = NULL;
    FtSwarmAlign_t align;
    int8_t         x, y;
    
  public:
    SwOSScreenObj  *next   = NULL;

    // Constructor
    SwOSScreenObj( uint8_t id, SwOSIO *io, SwOSScreen *parent, const char *label, int8_t x, int8_t y, FtSwarmAlign_t align );

    // Destructor
    ~SwOSScreenObj();

    // add an object to my list
    virtual void add( SwOSScreenObj *newObject);

    // activate myself, if my screen is getting active
    virtual void activate( void );

    // deactivate myself, if my screen is getting offline
    virtual void deactivate( void );
    
    // IO informs myself, it's going down
    virtual void unregister( SwOSIO *io );

    // change label text
    virtual void setLabel( const char *label, bool autoDraw = true );

    // draw myself
    virtual void draw( void ) {};

    // io sends new value, return true if it's processed by the screen
    virtual bool setValue( int32_t value );

    // is io clickable?
    virtual bool isSelectable( void ) { return false; };

    // get my label
    virtual char *getLabel( void ) { return label; };

};

/***************************************************
 *
 * SwOSScreenSelectable - label + text
 *
 ***************************************************/

class SwOSScreenSelectable : public SwOSScreenObj {

  protected:
    int8_t widthLabel;
    int8_t widthText;
    char *text = NULL;

  public:

    // Constructor
    SwOSScreenSelectable( uint8_t id, SwOSScreen *parent, const char *label, const char *text, int8_t x, int8_t y, int8_t widthLabel, int8_t widthText );
    
    // change text
    virtual void setText( const char *text, bool autoDraw = true );
    
    // draw myself
    virtual void draw( void );

    // render myself as selected
    virtual void select( void );

    // is io clickable?
    virtual bool isSelectable( void ) { return true; };

    // return my id
    virtual uint8_t getID( void ) { return id; };
};

/***************************************************
 *
 * SwOSScreenButton - button class
 *
 ***************************************************/

class SwOSScreenButton : public SwOSScreenObj {

  public:

    // Constructor
    SwOSScreenButton( uint8_t id, SwOSIO *io, SwOSScreen *parent, const char *label, int8_t x, int8_t y, FtSwarmAlign_t align ) : SwOSScreenObj( id, io, parent, label, x, y, align ) {};

    // draw myself
    virtual void draw( void );    

};

 /***************************************************
 *
 * SwOSScreenXX - local Buttons
 *
 ***************************************************/

#define OLEDLOWERLINE 38
#define OLEDWIDTH     128

class SwOSScreenS1 : public SwOSScreenButton {
  public:
    SwOSScreenS1( SwOSScreen *parent, const char *label );
};

class SwOSScreenS2 : public SwOSScreenButton {
  public:
    SwOSScreenS2( SwOSScreen *parent, const char *label );
};

class SwOSScreenS3 : public SwOSScreenButton {
  public:
    SwOSScreenS3( SwOSScreen *parent, const char *label );
};

class SwOSScreenS4 : public SwOSScreenButton {
  public:
    SwOSScreenS4( SwOSScreen *parent, const char *label );
};

class SwOSScreenF1 : public SwOSScreenButton {
  public:
    SwOSScreenF1( SwOSScreen *parent, const char *label );
};

class SwOSScreenF2 : public SwOSScreenButton {
  public:
    SwOSScreenF2( SwOSScreen *parent, const char *label );
};

class SwOSScreenJ1 : public SwOSScreenButton {
  public:
    SwOSScreenJ1( SwOSScreen *parent, const char *label );
};

class SwOSScreenJ2 : public SwOSScreenButton {
  public:
    SwOSScreenJ2( SwOSScreen *parent, const char *label );
};

class SwOSScreenESC : public SwOSScreenF2 {
  public:
    SwOSScreenESC( SwOSScreen *parent ) : SwOSScreenF2( parent, "^" ) {};
};

/***************************************************
 *
 * SwOSScreenJoystickPoti - Helper class
 *
 ***************************************************/

class SwOSScreenJoystickPoti : public SwOSScreenObj {

  protected:
    int32_t maxValue = FTSWARM_NANI32;

  public:

    SwOSScreenJoystickPoti(uint8_t id, SwOSIO *io, SwOSScreen *parent, const char *label, int8_t x, int8_t y, FtSwarmAlign_t align );

    // io sends new value, return true if it's processed by the screen
    virtual bool setValue( int32_t value );

};

/***************************************************
 *
 * SwOSScreenCombobox - ComboBox
 *
 ***************************************************/

class SwOSScreenCombobox : public SwOSScreenObj {

  protected:
    uint8_t size, direction;
    void drawHorizontal( void );
    void drawVertical( void );

  public:

    static const uint8_t HORIZONTAL = 0;
    static const uint8_t VERTICAL   = 1;
    
    SwOSScreenCombobox( uint8_t id, SwOSIO *io, SwOSScreen *parent, const char *label, int8_t x, int8_t y, uint8_t size, uint8_t direction );
    virtual void draw( void );
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
    SwOSSelectorItem *items = NULL;
    SwOSSelectorItem *active = NULL;

  public:
   
    SwOSScreenSelector( uint8_t id, SwOSIO* io, SwOSScreen *parent, int8_t x, int8_t y );
    ~SwOSScreenSelector();
    virtual void add( int8_t id, const char *text );
    virtual void draw( void );
};

/***************************************************
 *
 * SwOSScreen - Base class for all screens
 *
 ***************************************************/

class SwOSScreen {

  protected:
    SwOSScreen    *parent  = NULL;
    SwOSScreenObj *objects = NULL;
    char          *title   = NULL;

  public:

    // constructor
    SwOSScreen( SwOSScreen *parent, const char *title );

    // destructor
    ~SwOSScreen();

    // add a screen object
    virtual void add( SwOSScreenObj *newObject );

    // cls and draw all elements
    virtual void draw( void ); 

    // activate my objects, if my screen is getting active
    virtual void activate( void );

    // deactivate my objects, if my screen is getting offline
    virtual void deactivate( void );

    // called all 25ms, so be minimalistic
    virtual void operate( void ) {};

    // eval external events like pressing buttons
    virtual bool eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nparam = FTSWARM_NANI32, char *sparam = NULL ) { return false; };

    // get my title
    virtual const char *getTitle( void ) { return title; };

  };

/***************************************************
 *
 * SwOSScreenInput
 *
 ***************************************************/

 #define KEYMAPS 4

class SwOSScreenInput : public SwOSScreen {

  protected:

    uint8_t id;

    char    *input    = NULL;
    uint8_t maxLength = 0;

    SwOSScreenS1 *S1 = NULL;
    
    const char keyboardMap[KEYMAPS][30] = { R"(abcdefghijklmnopqrstuvwxyz@ )",
                                            R"(ABCDEFGHIJKLMNOPQRSTUVWXYZ_ )",
                                            R"(0123456789+-.)",
                                            R"(!"#$%&':;<=>?\^`|~[](){})*/,)" };

    const char S1Label[KEYMAPS][4] = { "A-Z", "NUM", "#$@", "a-z" };

    const uint8_t cols[KEYMAPS] = { 14, 14, 13, 14 };
    const uint8_t rows[KEYMAPS] = {  2,  2,  1,  2 };

    uint keyboard = 0;
    bool numKeyboard = false;

    uint8_t keyboardX, keyboardY, keyboardWidth, keyboardHeight;

    uint8_t cursorR[5] = { 0, 0, 0, 0, 0 };
    uint8_t cursorC[5] = { 0, 0, 0, 0, 0 };

    void init( uint8_t id, SwOSScreen *parent, const char *title, const char *param, uint8_t maxLength );
    uint8_t keymapIndex( void ) { return cursorR[keyboard]*cols[keyboard] + cursorC[keyboard]; };
    void setKeyboard( uint8_t keyboard );
    void drawCursor( bool invert );
    void drawInput( void );
    
  public:
   
    // Constructor to enter strings
    SwOSScreenInput( uint8_t id, SwOSScreen *parent, const char *title, const char *param, uint8_t maxLength );

    // constructor to enter numbers
    SwOSScreenInput( uint8_t id, SwOSScreen *parent, const char *title, const int32_t param, uint8_t maxLength );

    // Destructor
    ~SwOSScreenInput( );

    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nparam = FTSWARM_NANI32, char *sparam = NULL );

}; 

/***************************************************
 *
 * SwOSScreenSlider - Screen Slide show
 *
 ***************************************************/

 class SwOSScreenSlider : public SwOSScreen {

  protected:

    SwOSScreenSlider     *prev     = NULL;
    SwOSScreenSlider     *next     = NULL;
    SwOSScreenSelectable *selected = NULL;

    virtual uint8_t countPrev( void );
    virtual uint8_t countNext( void );

  public:

    // constructor
    SwOSScreenSlider( SwOSScreen *parent, const char *title, SwOSScreenSlider *next );

    // destructor
    ~SwOSScreenSlider();

    // add an object to my list
    virtual void add( SwOSScreenObj *newObject);
    
    // cls and draw all elements
    virtual void draw( void );
    
    // eval external events like pressing buttons
    virtual bool eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nparam = FTSWARM_NANI32, char *sparam = NULL);

};

/***************************************************
 *
 * SwOSScreenChooseOption
 *
 ***************************************************/

 class SwOSScreenChooseOption : public SwOSScreen {

  protected:
    uint8_t id;
    char text[2][21];

  public:

    // constructor
    SwOSScreenChooseOption( SwOSScreen *parent, uint8_t id, const char *title, const char *text1, const char *text2, const char *option1, const char *option2=NULL, const char *option3=NULL, const char *option4=NULL );

    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nparam = FTSWARM_NANI32, char *sparam = NULL );

};

/***************************************************
 *
 * SwOSScreenChooseConfig - asks about the config S1..S4
 *
 ***************************************************/

 class SwOSScreenChooseConfig : public SwOSScreenSlider {

  protected:
    
  public:

    // constructor
    SwOSScreenChooseConfig( SwOSScreen *parent, SwOSScreenSlider *next  );

    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nparam = FTSWARM_NANI32, char *sparam = NULL );

};

/***************************************************
 *
 *   SwOSScreenWifi
 *
 ***************************************************/

class SwOSScreenWifi : public SwOSScreenSlider {

  protected:
    SwOSScreenSelectable *wifiMode = NULL;
    SwOSScreenSelectable *wifiSSID = NULL;
    SwOSScreenSelectable *wifiPwd  = NULL;

  public:
    SwOSScreenWifi( SwOSScreen *parent, SwOSScreenSlider *next  );

  // eval external events like pressing buttons
  virtual bool eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nparam = FTSWARM_NANI32, char *sparam = NULL );

};

/***************************************************
 *
 *   SwOSScreenSwarm
 *
 ***************************************************/

class SwOSScreenSwarm : public SwOSScreenSlider {

  public:
    SwOSScreenSwarm( SwOSScreen *parent, SwOSScreenSlider *next  );

};

/***************************************************
 *
 * SwOSScreenFactoryReset 
 * Ask user to reset controller to factory setting
 *
 ***************************************************/

class SwOSScreenFactoryReset : public SwOSScreenSlider {

  public:

    // constructor
    SwOSScreenFactoryReset( SwOSScreen *parent, SwOSScreenSlider *next );
    
    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nparam = FTSWARM_NANI32, char *sparam = NULL );

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
    SwOSMainScreen( SwOSScreen *parent, const char *title );

    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nparam = FTSWARM_NANI32, char *sparam = NULL );

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
    SwOSSplashScreen( SwOSScreen *parent, const char *title );
    
    // cls and draw all elements
    virtual void draw( void );
    
    // called all 25ms, so be minimalistic
    virtual void operate( void );

    // eval external events like pressing buttons
    virtual bool eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nparam = FTSWARM_NANI32, char *sparam = NULL );

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
    SwOS404Screen( SwOSScreen *parent ):SwOSScreen( parent, "Page not found" ) {  };
    
    // cls and draw all elements
    virtual void draw( void );

};

/***************************************************
 *
 * SwOSScreenManager - class to handle the active screen
 *
 ***************************************************/

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
    
    // Replace the active screen by a new screen and in case of autoCleanUp delete the old one.
    // Replacement is done with next operate()-call to avoid stress
    // Be aware of potential race conditions by very fast screen replacements. 
    void newScreen( SwOSScreen *newScreen, bool autoCleanUp );

};

extern SwOSScreenManager screenManager;
