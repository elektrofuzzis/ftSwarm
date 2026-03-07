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

#define SWOSSCREENID_BASE 30

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
    int16_t        x, y;
    FtSwarmScreenEvent_t lastEvent = FTSWARM_SCREENEVENT_NONE;
    
  public:
    SwOSScreenObj  *next   = NULL;

    // Constructor
    SwOSScreenObj( uint8_t id, SwOSIO *io, SwOSScreen *parent, const char *label, int16_t x, int16_t y, FtSwarmAlign_t align );

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
    int16_t widthLabel;
    int16_t widthText;
    char *text   = NULL;
    bool visible = true;

  public:

    // Constructor
    SwOSScreenSelectable( uint8_t id, SwOSScreen *parent, const char *label, const char *text, int16_t x, int16_t y, int16_t widthLabel, int16_t widthText );
    
    // change text
    virtual void setText( const char *text, bool autoDraw = true );

    // get text
    virtual const char *getText( void ) { return text; };
    
    // draw myself
    virtual void draw( void );

    // render myself as selected
    virtual void select( void );

    // is io clickable?
    virtual bool isSelectable( void ) { return visible; };

    // return my id
    virtual uint8_t getID( void ) { return id; };

    // hide/show
    virtual void setVisible( bool visible );

};

/***************************************************
 *
 * SwOSScreenButton - button class
 *
 ***************************************************/

class SwOSScreenButton : public SwOSScreenObj {

  public:

    // Constructor
    SwOSScreenButton( uint8_t id, SwOSIO *io, SwOSScreen *parent, const char *label, int16_t x, int16_t y, FtSwarmAlign_t align ) : SwOSScreenObj( id, io, parent, label, x, y, align ) {};

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

    SwOSScreenJoystickPoti(uint8_t id, SwOSIO *io, SwOSScreen *parent, const char *label, int16_t x, int16_t y, FtSwarmAlign_t align );

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
    
    SwOSScreenCombobox( uint8_t id, SwOSIO *io, SwOSScreen *parent, const char *label, int16_t x, int16_t y, uint8_t size, uint8_t direction );
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
   
    SwOSScreenSelector( uint8_t id, SwOSIO* io, SwOSScreen *parent, int16_t x, int16_t y );
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

    bool toBeDestroyed = false;

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
    virtual bool eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, char *sParam = NULL ) { return false; };

    // get my title
    virtual const char *getTitle( void ) { return title; };

    // close myself
    virtual void close(void );

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

    void init( SwOSScreen *parent, uint8_t id, const char *title, const char *param, uint8_t maxLength );
    uint8_t keymapIndex( void ) { return cursorR[keyboard]*cols[keyboard] + cursorC[keyboard]; };
    void setKeyboard( uint8_t keyboard );
    void drawCursor( bool invert );
    void drawInput( void );
    
  public:
   
    // Constructor to enter strings
    SwOSScreenInput( SwOSScreen *parent, uint8_t id, const char *title, const char *param, uint8_t maxLength );

    // constructor to enter numbers
    SwOSScreenInput( SwOSScreen *parent, uint8_t id, const char *title, const int32_t param, uint8_t maxLength );

    // Destructor
    ~SwOSScreenInput( );

    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, char *sParam = NULL );

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

    // add an object to my list
    virtual void add( SwOSScreenObj *newObject);
    
    // cls and draw all elements
    virtual void draw( void );
    
    // eval external events like pressing buttons
    virtual bool eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, char *sParam = NULL);

    // close myself
    virtual void close(void );

};

/***************************************************
 *
 * SwOSScreenChooseOption
 *
 ***************************************************/

 class SwOSScreenChooseOption : public SwOSScreen {

  protected:
    uint8_t id;
    uint8_t value[4];
    char    *line[4];
    int8_t  maxLine = -1;

  public:

    // constructor
    SwOSScreenChooseOption( SwOSScreen *parent, uint8_t id, const char *title, const char *text, 
                            uint8_t value1,   const char *option1, 
                            uint8_t value2=0, const char *option2=NULL, 
                            uint8_t value3=0, const char *option3=NULL, 
                            uint8_t value4=0, const char *option4=NULL );

    // destructor
    ~SwOSScreenChooseOption();

    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, char *sParam = NULL );

};

/***************************************************
 *
 * SwOSScreenYesNo
 *
 ***************************************************/

class SwOSScreenYesNo : public SwOSScreenChooseOption {

  public: 
    SwOSScreenYesNo( SwOSScreen *parent, uint8_t id, const char *title, const char *text ) : SwOSScreenChooseOption( parent, id, title, text, 0, NULL, 1, "YES", 0, "NO" ) {};

};


/***************************************************
 *
 * SwOSScreenError
 *
 ***************************************************/

class SwOSScreenError : public SwOSScreenChooseOption {

  public: 
    SwOSScreenError( SwOSScreen *parent, const char *text ) : SwOSScreenChooseOption( parent, 0, "Error", text, 0, NULL, 0, NULL, 0, NULL, 1, "OK" ) {};

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
    virtual bool eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, char *sParam = NULL );

};

/***************************************************
 *
 *   SwOSScreenWifi
 *
 ***************************************************/

class SwOSScreenWifi : public SwOSScreenSlider {

  protected:

    SwOSScreenSelectable *wifiModeSO = NULL;
    SwOSScreenSelectable *wifiSSIDSO = NULL;
    SwOSScreenSelectable *wifiPwdSO  = NULL;
    bool anythingChanged = false;
    SwOSScreenS4 *S4 = NULL;;

    char wifiPwd[64];
    char wifiSSID[64];
    FtSwarmWifi_t wifiMode;

  public:
    SwOSScreenWifi( SwOSScreen *parent, SwOSScreenSlider *next  );

  // eval external events like pressing buttons
  virtual bool eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, char *sParam = NULL );

};

/***************************************************
 *
 *   SwOSScreenWifiSSID
 *
 ***************************************************/

class SwOSScreenWifiSSID : public SwOSScreenSlider {

  protected:

    int16_t scanStatus;
    uint8_t id = 0;

  public:

    // constructor
    SwOSScreenWifiSSID( SwOSScreen *parent, uint8_t id );

    // destructor
    ~SwOSScreenWifiSSID();

    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, char *sParam = NULL );

    // called all 25ms, so be minimalistic
    virtual void operate( void );

};

/***************************************************
 *
 *   SwOSScreenSwarm
 *
 ***************************************************/

class SwOSScreenSwarm : public SwOSScreenSlider {

  protected:
    char     swarmName[MAXIDENTIFIER];
    uint16_t swarmPIN;

  public:

    // Constructor
    SwOSScreenSwarm( SwOSScreen *parent, SwOSScreenSlider *next  );

    // reload swarm member list
    virtual void reload( void );

    // eval external events like pressing buttons
    virtual bool eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, char *sParam = NULL ) override;

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
    virtual bool eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, char *sParam = NULL );

};

/***************************************************
 *
 * SwOSMainScreen - Main OLED visualizer
 *
 ***************************************************/

class SwOSMainScreen : public SwOSScreen {

  protected:

    // ToDo: replace with own SwScreenObj derives class
    void joystick( char *lr, char *fb, int16_t x, int16_t y, bool left );

  public:

    // Constructor
    SwOSMainScreen( SwOSScreen *parent, const char *title );

    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, char *sParam = NULL );
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
    virtual bool eventHandlerCallback( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, char *sParam = NULL );

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

 #define MAXSCREENS 15

class SwOSScreenManager {

  protected:
    SwOSScreen *screen[MAXSCREENS];

    // get a free index in creen[]
    uint8_t getIndex( SwOSScreen *screen );

  public:
    SwOSScreen *active = NULL;
    SwOSScreen *next   = NULL;

    SwOSScreenManager();

    // does all the stuff to replace and operate the screen
    void operate( void );

    // draw active screen
    void draw( void );
    
    // register myself in garbage collector
    void registerMe( SwOSScreen *screen );
    
    // activate
    void activate( SwOSScreen *screen );

};

extern SwOSScreenManager screenManager;
