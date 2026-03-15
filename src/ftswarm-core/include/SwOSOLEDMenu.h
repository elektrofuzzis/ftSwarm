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
 * SwOSScreenObjList
 *
 ***************************************************/

class SwOSScreenObjList {

  protected:
    SwOSScreenObj *list = NULL;

  public:

    // destructor
    ~SwOSScreenObjList() { cleanup(); };

    // delete list
    void cleanup( void ) { if (list) delete list; list = NULL; };
  
    // add an object
    void add( SwOSScreenObj *newObject );

    // activate all elements, my screen is getting active
    void activate( void );

    // deactivate all elements, my screen is getting offline
    void deactivate( void );

    // draw
    void draw( void );

    // get next free space on screen
    int16_t getNextY( void );

    // get prev element
    SwOSScreenObj *prev( SwOSScreenObj *obj );

    // debugging
    void print( void ) { if (list) list->print(); };

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

    // create a text entry at  the next possible position
    SwOSScreenSelectable( uint8_t id, SwOSScreen *parent, const char *text );
    
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
    virtual void setValue( int32_t value );

};

/***************************************************
 *
 * SwOSScreen - Base class for all screens
 *
 ***************************************************/

class SwOSScreen {

  protected:

    SwOSScreen *parent  = NULL;
    char       *title   = NULL;
    bool       blockEvents = true;
    bool       navigation  = false;

    SwOSScreen *prev = NULL;
    SwOSScreen *next = NULL;

    SwOSScreenObjList objects;
    SwOSScreenObjList selectables;

    SwOSScreenSelectable *selected = NULL;

    void addNavigation( void );
    uint8_t countPrev( void );
    uint8_t countNext( void );
    void deleteSelectables( void );    

  public:

    uint32_t USID;
    bool     toBeDestroyed = false;

    // constructor
    SwOSScreen( SwOSScreen *parent, const char *title, SwOSScreen *next = NULL );

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

    const uint8_t cols[KEYMAPS] = { 14, 14,  7, 14 };
    const uint8_t rows[KEYMAPS] = {  2,  2,  2,  2 };

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
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL );

}; 

/***************************************************
 *
 * SwOSScreenChooseOption
 *
 ***************************************************/

 class SwOSScreenChooseOption : public SwOSScreen {

  protected:
    uint8_t id;
    int32_t value[4];
    char    *line[4];
    int8_t  maxLine = -1;

  public:

    // constructor
    SwOSScreenChooseOption( SwOSScreen *parent, uint8_t id, const char *title, const char *text, 
                            int32_t value1,   const char *option1, 
                            int32_t value2=0, const char *option2=NULL, 
                            int32_t value3=0, const char *option3=NULL, 
                            int32_t value4=0, const char *option4=NULL );

    // destructor
    ~SwOSScreenChooseOption();

    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL );

};

/***************************************************
 *
 * SwOSScreenYesNo
 *
 ***************************************************/

class SwOSScreenYesNo : public SwOSScreenChooseOption {

  public: 
    SwOSScreenYesNo( SwOSScreen *parent, uint8_t id, const char *title, const char *text, int32_t yes = 1 ) : SwOSScreenChooseOption( parent, id, title, text, 0, NULL, yes, "YES", 0, "NO" ) {};

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

 class SwOSScreenChooseConfig : public SwOSScreen {

  protected:
    
  public:

    // constructor
    SwOSScreenChooseConfig( SwOSScreen *parent, SwOSScreen *next  );

    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL );

};

/***************************************************
 *
 *   SwOSScreenWifi
 *
 ***************************************************/

class SwOSScreenWifi : public SwOSScreen {

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
    SwOSScreenWifi( SwOSScreen *parent, SwOSScreen *next  );

  // eval external events like pressing buttons
  virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL );

};

/***************************************************
 *
 *   SwOSScreenWifiSSID
 *
 ***************************************************/

class SwOSScreenWifiSSID : public SwOSScreen {

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
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL );

    // called all 25ms, so be minimalistic
    virtual void operate( void );

};

/***************************************************
 *
 *   SwOSScreenSwarm
 *
 ***************************************************/

class SwOSScreenSwarm : public SwOSScreen {

  protected:

    void addMembers( void );

  public:

    // Constructor
    SwOSScreenSwarm( SwOSScreen *parent, SwOSScreen *next  );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL ) override;

};

/***************************************************
 *
 *   SwOSScreenSwarmDetail
 *
 ***************************************************/

class SwOSScreenSwarmDetail : public SwOSScreen {

  protected:
    FtSwarmSerialNumber_t device;

  public:

    // Constructor
    SwOSScreenSwarmDetail( SwOSScreen *parent, const char *title, FtSwarmSerialNumber_t device  );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL ) override;

};

/***************************************************
 *
 *   SwOSScreenQuickCfg
 *
 ***************************************************/

typedef struct {
  SwOSIOType_t      sensorIoType;
  uint8_t           sensorPort;
  SwOSIOType_t      actorIoType;
  uint8_t           actorPort;
  FtSwarmTrigger_t  event;
  FtSwarmOperator_t op;
  FtSwarmOperand_t  v1;
  FtSwarmOperand_t  v2;
  int32_t           parameter;
} EventCfg_t;

class SwOSScreenQuickCfg : public SwOSScreen {

  protected:

    uint8_t function = 0;
    FtSwarmSerialNumber_t device;

    void loadCfg( FtSwarmSerialNumber_t ctrl, FtSwarmSerialNumber_t device, uint8_t configuration, EventCfg_t *cfg, uint8_t items );

  public:

    // Constructor
    SwOSScreenQuickCfg( SwOSScreen *parent, FtSwarmSerialNumber_t device);

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL ) override;

};

/***************************************************
 *
 * SwOSScreenFactoryReset 
 * Ask user to reset controller to factory setting
 *
 ***************************************************/

class SwOSScreenFactoryReset : public SwOSScreen {

  public:

    // constructor
    SwOSScreenFactoryReset( SwOSScreen *parent, SwOSScreen *next );
    
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
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL );
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
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL );

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
 * SwOSScreenEvent
 *
 ***************************************************/

struct SwOSScreenEventQueueElement_t { 
  uint32_t             USID; 
  FtSwarmScreenEvent_t event;
  uint8_t              id;
  int32_t              nParam;
  char                 *sParam;
} __attribute__((packed));

/***************************************************
 *
 * SwOSScreenManager - class to handle the active screen
 *
 ***************************************************/

 #define MAXSCREENS 15

class SwOSScreenManager {

  protected:
    SwOSScreen *screen[MAXSCREENS];

    // get a free index in screen[]
    uint8_t getIndex( SwOSScreen *screen );

  public:
    SwOSScreen *active = NULL;
    bool blockEvents   = false;

    SwOSScreenManager();

    // send an event to active screen
    void eventHandler( SwOSScreen *screen, FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = NULL );

    // receive an event and forward it to the active screen
    void eventHandler( SwOSScreenEventQueueElement_t *event );

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

#endif