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

#define FTSWARM_HAL_OLEDS 1
#if FTSWARM_HAL_OLEDS > 0

typedef enum { FTSWARM_SCREENEVENT_NONE = -1, FTSWARM_SCREENEVENT_DOWN, FTSWARM_SCREENEVENT_UP, FTSWARM_SCREENEVENT_OK } FtSwarmScreenEvent_t;

#define SWOSSCREENID_BASE 30

/***************************************************
 *
 * FtSwarmScreenObj - Any element on a screen
 *
 ***************************************************/

class FtSwarmScreen;

class FtSwarmScreenObj {

  protected:
    uint8_t        id;
    SwOSIO         *io     = NULL;
    FtSwarmScreen  *parent = NULL;
    int32_t        value   = FTSWARM_NANI32;
    char           *label  = NULL;
    uint8_t        screen;
    FtSwarmAlign_t align;
    int16_t        x, y;
    FtSwarmScreenEvent_t lastEvent = FTSWARM_SCREENEVENT_NONE;
    
  public:
    FtSwarmScreenObj  *next   = NULL;

    // Constructor
    FtSwarmScreenObj( uint8_t id, SwOSIO *io, FtSwarmScreen *parent, const char *label, uint8_t screen, int16_t x, int16_t y, FtSwarmAlign_t align );

    // Destructor
    ~FtSwarmScreenObj();

    // add an object to my list
    virtual void add( FtSwarmScreenObj *newObject);

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
    virtual void setValue( int32_t value );

    // is io clickable?
    virtual bool isSelectable( void ) { return false; };

    // is io visible?
    virtual bool isVisible( void ) { return label[0] != '\0'; };

    // get my label
    virtual char *getLabel( void ) { return label; };

    // get my ID
    virtual uint8_t getID( void ) { return id; };

    // get my y-Position
    virtual int16_t getY( void ) { return y; };

    // debugging
    virtual void print( void );

};

/***************************************************
 *
 * FtSwarmScreenObjList
 *
 ***************************************************/

class FtSwarmScreenObjList {

  protected:
    FtSwarmScreenObj *list = NULL;

  public:

    // destructor
    ~FtSwarmScreenObjList() { cleanup(); };

    // delete list
    void cleanup( void ) { if (list) delete list; list = NULL; };
  
    // add an object
    void add( FtSwarmScreenObj *newObject );

    // activate all elements, my screen is getting active
    void activate( void );

    // deactivate all elements, my screen is getting offline
    void deactivate( void );

    // draw
    void draw( void );

    // get next free space on screen
    int16_t getNextY( void );

    // get prev element
    FtSwarmScreenObj *prev( FtSwarmScreenObj *obj );

    // debugging
    void print( void ) { if (list) list->print(); };

};

/***************************************************
 *
 * FtSwarmScreenSelectable - label + text
 *
 ***************************************************/

class FtSwarmScreenSelectable : public FtSwarmScreenObj {

  protected:
    int16_t widthLabel;
    int16_t widthText;
    char *text   = NULL;
    bool visible = true;

  public:

    // Constructor
    FtSwarmScreenSelectable( uint8_t id, FtSwarmScreen *parent, const char *label, const char *text, int16_t x, int16_t y, int16_t widthLabel, int16_t widthText );

    // create a text entry at  the next possible position
    FtSwarmScreenSelectable( uint8_t id, FtSwarmScreen *parent, const char *text );
    
    // change text
    virtual void setText( const char *text, bool autoDraw = true );

    // get text
    virtual const char *getText( void ) { return text; };
    
    // draw myself
    virtual void draw( void );

    // render myself as selected
    virtual void select( void );

    // is io clickable?
    virtual bool isSelectable( void ) { return true; };

    // is io visible?
    virtual bool isVisible( void ) { return visible; };

    // return my id
    virtual uint8_t getID( void ) { return id; };

    // hide/show
    virtual void setVisible( bool visible );

    // debug
    virtual void print( void );

};

/***************************************************
 *
 * FtSwarmScreenButton - button class
 *
 ***************************************************/

class FtSwarmScreenButton : public FtSwarmScreenObj {

  public:

    // Constructor
    FtSwarmScreenButton( uint8_t id, SwOSIO *io, FtSwarmScreen *parent, const char *label, uint8_t screen, int16_t x, int16_t y, FtSwarmAlign_t align ) : FtSwarmScreenObj( id, io, parent, label, screen, x, y, align ) {};

    // draw myself
    virtual void draw( void );    

};

 /***************************************************
 *
 * FtSwarmScreenXX - local Buttons
 *
 ***************************************************/

#define OLEDLOWERLINE 38

class FtSwarmScreenS1 : public FtSwarmScreenButton {
  public:
    FtSwarmScreenS1( FtSwarmScreen *parent, const char *label );
};

class FtSwarmScreenS2 : public FtSwarmScreenButton {
  public:
    FtSwarmScreenS2( FtSwarmScreen *parent, const char *label );
};

class FtSwarmScreenS3 : public FtSwarmScreenButton {
  public:
    FtSwarmScreenS3( FtSwarmScreen *parent, const char *label );
};

class FtSwarmScreenS4 : public FtSwarmScreenButton {
  public:
    FtSwarmScreenS4( FtSwarmScreen *parent, const char *label );
};

class FtSwarmScreenF1 : public FtSwarmScreenButton {
  public:
    FtSwarmScreenF1( FtSwarmScreen *parent, const char *label );
};

class FtSwarmScreenF2 : public FtSwarmScreenButton {
  public:
    FtSwarmScreenF2( FtSwarmScreen *parent, const char *label );
};

class FtSwarmScreenJ1 : public FtSwarmScreenButton {
  public:
    FtSwarmScreenJ1( FtSwarmScreen *parent, const char *label );
};

class FtSwarmScreenJ2 : public FtSwarmScreenButton {
  public:
    FtSwarmScreenJ2( FtSwarmScreen *parent, const char *label );
};

class FtSwarmScreenESC : public FtSwarmScreenF2 {
  public:
    FtSwarmScreenESC( FtSwarmScreen *parent ) : FtSwarmScreenF2( parent, "^" ) {};
};

/***************************************************
 *
 * FtSwarmScreenJoystickPoti - Helper class
 *
 ***************************************************/

class FtSwarmScreenJoystickPoti : public FtSwarmScreenObj {

  protected:
    int32_t maxValue = FTSWARM_NANI32;

  public:

    FtSwarmScreenJoystickPoti(uint8_t id, SwOSIO *io, FtSwarmScreen *parent, const char *label, int16_t x, int16_t y, FtSwarmAlign_t align );

    // io sends new value, return true if it's processed by the screen
    virtual void setValue( int32_t value );

};

/***************************************************
 *
 * FtSwarmScreen - Base class for all screens
 *
 ***************************************************/

class FtSwarmScreen {

  protected:

    FtSwarmScreen *parent     = NULL;
    char          *title      = NULL;
    bool          blockEvents = true;
    bool          navigation  = false;

    FtSwarmScreen *prev = NULL;
    FtSwarmScreen *next = NULL;

    FtSwarmScreenObjList objects;
    FtSwarmScreenObjList selectables;

    FtSwarmScreenSelectable *selected = NULL;

    void addNavigation( void );
    uint8_t countPrev( void );
    uint8_t countNext( void );
    void deleteSelectables( void );    

  public:

    uint32_t USID;
    bool     toBeDestroyed = false;

    // constructor
    FtSwarmScreen( FtSwarmScreen *parent, const char *title, FtSwarmScreen *next = NULL );

    // destructor
    ~FtSwarmScreen();

    // add a screen object
    virtual void add( FtSwarmScreenObj *newObject );

    // cls and draw all elements
    virtual void draw( void ); 

    // activate my objects, if my screen is getting active
    virtual void activate( void );

    // deactivate my objects, if my screen is getting offline
    virtual void deactivate( void );

    // called all 25ms, so be minimalistic
    virtual void operate( void ) {};

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL );

    // get my title
    virtual const char *getTitle( void ) { return title; };

    // close myself
    virtual void close( FtSwarmScreenEvent_t event = FTSWARM_SCREENEVENT_NONE, uint8_t id=0, uint8_t nParam = 0, const char *sParam = NULL );

    // calculate next free line
    virtual int16_t getNextY( void );

};

/***************************************************
 *
 * FtSwarmScreenInput
 *
 ***************************************************/

#define KEYMAPS 4

class FtSwarmScreenInput : public FtSwarmScreen {

  protected:

    uint8_t id;

    char    *input    = NULL;
    uint8_t maxLength = 0;

    FtSwarmScreenS1 *S1 = NULL;
    
    const char keyboardMap[KEYMAPS][30] = { R"(abcdefghijklmnopqrstuvwxyz@ )",
                                            R"(ABCDEFGHIJKLMNOPQRSTUVWXYZ_ )",
                                            R"(0123456789+-.)",
                                            R"(!"#$%&':;<=>?\^`|~[](){})*/,)" };

    const char S1Label[KEYMAPS][4] = { "A-Z", "NUM", "#$@", "a-z" };

    const uint8_t cols[KEYMAPS] = { 14, 14,  7, 14 };
    const uint8_t rows[KEYMAPS] = {  2,  2,  2,  2 };

    uint keyboard = 0;
    bool numKeyboard = false;

    uint8_t keyboardX, keyboardY, keyboardWidth, keyboardHeight;

    uint8_t cursorR[5] = { 0, 0, 0, 0, 0 };
    uint8_t cursorC[5] = { 0, 0, 0, 0, 0 };

    void init( FtSwarmScreen *parent, uint8_t id, const char *title, const char *param, uint8_t maxLength );
    uint8_t keymapIndex( void ) { return cursorR[keyboard]*cols[keyboard] + cursorC[keyboard]; };
    void setKeyboard( uint8_t keyboard );
    void drawCursor( bool invert );
    void drawInput( void );
    
  public:
   
    // Constructor to enter strings
    FtSwarmScreenInput( FtSwarmScreen *parent, uint8_t id, const char *title, const char *param, uint8_t maxLength );

    // constructor to enter numbers
    FtSwarmScreenInput( FtSwarmScreen *parent, uint8_t id, const char *title, const int32_t param, uint8_t maxLength );

    // Destructor
    ~FtSwarmScreenInput( );

    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL );

}; 

/***************************************************
 *
 * FtSwarmScreenChooseOption
 *
 ***************************************************/

 class FtSwarmScreenChooseOption : public FtSwarmScreen {

  protected:
    uint8_t id;
    int32_t value[4];
    char    *line[4];
    int8_t  maxLine = -1;

  public:

    // constructor
    FtSwarmScreenChooseOption( FtSwarmScreen *parent, uint8_t id, const char *title, const char *text, 
                               int32_t value1,   const char *option1, 
                               int32_t value2=0, const char *option2=NULL, 
                               int32_t value3=0, const char *option3=NULL, 
                               int32_t value4=0, const char *option4=NULL );

    // destructor
    ~FtSwarmScreenChooseOption();

    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL );

};

/***************************************************
 *
 * FtSwarmScreenYesNo
 *
 ***************************************************/

class FtSwarmScreenYesNo : public FtSwarmScreenChooseOption {

  public: 
    FtSwarmScreenYesNo( FtSwarmScreen *parent, uint8_t id, const char *title, const char *text, int32_t yes = 1 ) : FtSwarmScreenChooseOption( parent, id, title, text, 0, NULL, yes, "YES", 0, "NO" ) {};

};


/***************************************************
 *
 * FtSwarmScreenError
 *
 ***************************************************/

class FtSwarmScreenError : public FtSwarmScreenChooseOption {

  public: 
    FtSwarmScreenError( FtSwarmScreen *parent, const char *text ) : FtSwarmScreenChooseOption( parent, 0, "Error", text, 0, NULL, 0, NULL, 0, NULL, 1, "OK" ) {};

};


/***************************************************
 *
 * FtSwarmScreenChooseConfig - asks about the config S1..S4
 *
 ***************************************************/

 class FtSwarmScreenChooseConfig : public FtSwarmScreen {

  protected:
    
  public:

    // constructor
    FtSwarmScreenChooseConfig( FtSwarmScreen *parent, FtSwarmScreen *next  );

    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL );

};

/***************************************************
 *
 *   FtSwarmScreenSelectList
 *
 ***************************************************/

 class FtSwarmScreenSelectList : public FtSwarmScreen {

  protected:

    uint8_t callbackID;

    void process() {};

    template<typename... Tail>
    void process(uint8_t id, const char* str, Tail... tail);

  public:

    // Constructor - parent gets a callback: eventHandler( FTSWARM_SCREENEVENT_OK, callbackID, selected object's ID )
    // example: new FtSwarmScreenSelectList( this, "Number", NULL, SELECTMYNUMBER_CB, 42, "fourty-two", 17, "seventeen" )
    // template<typename... Args>
    // FtSwarmScreenSelectList( FtSwarmScreen *parent, char *title, FtSwarmScreen *next, uint8_t callbackID, Args... args);

    // Constructor - parent gets a callback: eventHandler( FTSWARM_SCREENEVENT_OK, callbackID, selected object's ID )
    // example: new FtSwarmScreenSelectList( this, "Number", NULL, SELECTMYNUMBER_CB, 42, "fourty-two", 17, "seventeen" )
    template<typename... Args>
    FtSwarmScreenSelectList( FtSwarmScreen *parent, const char *title, uint8_t callbackID, Args... args);

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL );
 };


/*--------------------------------------------------------------------------------------------------------------------------------------------------*/

/***************************************************
 *
 *   FtSwarmScreenWifi
 *
 ***************************************************/

class FtSwarmScreenWifi : public FtSwarmScreen {

  protected:

    FtSwarmScreenSelectable *wifiModeSO = NULL;
    FtSwarmScreenSelectable *wifiSSIDSO = NULL;
    FtSwarmScreenSelectable *wifiPwdSO  = NULL;
    bool anythingChanged = false;
    FtSwarmScreenS4 *S4 = NULL;;

    char wifiPwd[64];
    char wifiSSID[64];
    FtSwarmWifi_t wifiMode;

  public:
    FtSwarmScreenWifi( FtSwarmScreen *parent, FtSwarmScreen *next  );

  // eval external events like pressing buttons
  virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL );

};

/***************************************************
 *
 *   FtSwarmScreenWifiSSID
 *
 ***************************************************/

class FtSwarmScreenWifiSSID : public FtSwarmScreen {

  protected:

    int16_t scanStatus;
    uint8_t id = 0;

  public:

    // constructor
    FtSwarmScreenWifiSSID( FtSwarmScreen *parent, uint8_t id );

    // destructor
    ~FtSwarmScreenWifiSSID();

    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL );

    // called all 25ms, so be minimalistic
    virtual void operate( void );

};

/***************************************************
 *
 *   FtSwarmScreenSwarm
 *
 ***************************************************/

class FtSwarmScreenSwarm : public FtSwarmScreen {

  protected:

    void addMembers( void );

  public:

    // Constructor
    FtSwarmScreenSwarm( FtSwarmScreen *parent, FtSwarmScreen *next  );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL ) override;

};

/***************************************************
 *
 *   FtSwarmScreenSwarmDetail
 *
 ***************************************************/

class FtSwarmScreenSwarmDetail : public FtSwarmScreen {

  protected:
    FtSwarmSerialNumber_t device;

  public:

    // Constructor
    FtSwarmScreenSwarmDetail( FtSwarmScreen *parent, const char *title, FtSwarmSerialNumber_t device  );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL ) override;

};

/***************************************************
 *
 * FtSwarmScreenFactoryReset 
 * Ask user to reset controller to factory setting
 *
 ***************************************************/

class FtSwarmScreenFactoryReset : public FtSwarmScreen {

  public:

    // constructor
    FtSwarmScreenFactoryReset( FtSwarmScreen *parent, FtSwarmScreen *next );
    
    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL );

};

/***************************************************
 *
 * SwOSMainScreen - Main OLED visualizer
 *
 ***************************************************/

class SwOSMainScreen : public FtSwarmScreen {

  protected:

    // ToDo: replace with own SwScreenObj derives class
    void joystick( char *lr, char *fb, int16_t x, int16_t y, bool left );

  public:

    // Constructor
    SwOSMainScreen( FtSwarmScreen *parent, const char *title );

    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL );
};

/***************************************************
 *
 * SwOSSplashScreen 
 * startup screen, switches after 5s to MainScreen
 *
 ***************************************************/

class SwOSSplashScreen : public FtSwarmScreen {

  protected:
    unsigned long startTime;

  public:

    // constructor
    SwOSSplashScreen( FtSwarmScreen *parent, const char *title );
    
    // cls and draw all elements
    virtual void draw( void );
    
    // called all 25ms, so be minimalistic
    virtual void operate( void );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL );

};

/***************************************************
 *
 *   SwOS404Screen - ScreenNotFound
 *
 ***************************************************/

class SwOS404Screen : public FtSwarmScreen {

  protected:

  public:

    // constructor
    SwOS404Screen( FtSwarmScreen *parent ):FtSwarmScreen( parent, "Page not found" ) {  };
    
    // cls and draw all elements
    virtual void draw( void );

};

/***************************************************
 *
 * FtSwarmScreenEvent
 *
 ***************************************************/

struct FtSwarmScreenEventQueueElement_t { 
  uint32_t             USID; 
  FtSwarmScreenEvent_t event;
  uint8_t              id;
  int32_t              nParam;
  char                 *sParam;
} __attribute__((packed));

/***************************************************
 *
 * FtSwarmScreenManager - class to handle the active screen
 *
 ***************************************************/

 #define MAXSCREENS 15

class FtSwarmScreenManager {

  protected:
    FtSwarmScreen *screen[MAXSCREENS];

    // get a free index in screen[]
    uint8_t getIndex( FtSwarmScreen *screen );

  public:
    FtSwarmScreen *active = NULL;
    bool blockEvents = false;

    FtSwarmScreenManager();

    // send an event to active screen
    void eventHandler( FtSwarmScreen *screen, FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL );

    // receive an event and forward it to the active screen
    void eventHandler( FtSwarmScreenEventQueueElement_t *event );

    // does all the stuff to replace and operate the screen
    void operate( void );

    // draw active screen
    void draw( void );
    
    // register myself in garbage collector
    void registerMe( FtSwarmScreen *screen );
    
    // activate
    void activate( FtSwarmScreen *screen );

};

extern FtSwarmScreenManager screenManager;

#endif