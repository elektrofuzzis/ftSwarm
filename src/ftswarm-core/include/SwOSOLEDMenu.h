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
#include "SwOSHW/SwOSHWBaseIO.h"
#include "SwOSHW/SwOSHWAnalog.h"

class SwOSJoystick;

#define FTSWARM_HAL_OLEDS 1

#if FTSWARM_HAL_OLEDS > 0

#include "SwOSHW/SwOSHWLocal.h"

typedef enum { FTSWARM_SCREENEVENT_NONE = -1, FTSWARM_SCREENEVENT_DOWN, FTSWARM_SCREENEVENT_UP, FTSWARM_SCREENEVENT_OK } FtSwarmScreenEvent_t;

#define FTSWARMSCREEN_JOY1LR 26
#define FTSWARMSCREEN_JOY1FB 27
#define FTSWARMSCREEN_JOY2LR 28
#define FTSWARMSCREEN_JOY2FB 29
#define FTSWARMSCREEN_BASEID 30
#define FTSWARMSCREEN_NOID   255

/***************************************************
 *
 * FtSwarmScreenObj - Base class for screen elements
 *
 ***************************************************/

class FtSwarmScreen;

typedef enum { FTSWARMSCREEN_OBJ, FTSWARMSCREEN_LINE, FTSWARMSCREEN_TRIANGLE, FTSWARMSCREEN_TEXT, FTSWARMSCREEN_IO, FTSWARMSCREEN_SELECTABLE, FTSWARMSCREEN_BUTTON } FtSwarmScreenObj_t;

class FtSwarmScreenObj {

  protected:
    uint8_t             id;
    FtSwarmOledScreen_t screen;
    int16_t             x, y;
    bool                visible = true;
    FtSwarmScreen      *parent = nullptr;

  public:

    FtSwarmScreenObj    *next = nullptr;

    // Constructor
    FtSwarmScreenObj( uint8_t id, FtSwarmScreen *parent, FtSwarmOledScreen_t screen, int16_t x, int16_t y );

    // destructor
    virtual ~FtSwarmScreenObj() { if (next) delete next; };

    // return my type
    virtual FtSwarmScreenObj_t getType( void ) { return FTSWARMSCREEN_OBJ; };

    // draw myself
    virtual void draw( bool selected = false ) {};

    // set my label
    virtual void setText( const char *text ) {};

    // set visibility
    virtual void setVisible( bool visible ) { this->visible = visible; };

    // get my label
    virtual const char *getText( void ) { return ""; };

    // get my ID
    virtual uint8_t getID( void ) { return id; };

    // get my y-Position
    virtual int16_t getY( void ) { return y; };

    // get my height
    virtual int16_t getHeight( void ) { return 0; };

    // get my screen
    virtual uint8_t getScreen( void ) { return screen; };

    // activate myself, if my screen is getting active
    virtual void activate( void ) {};

    // deactivate myself, if my screen is getting offline
    virtual void deactivate( void ) {};
    
    // IO informs myself, it's going down
    virtual void unregister( SwOSIO *io ) {}; 

    // debug
    virtual void debug( void );


};

/***************************************************
 *
 * FtSwarmScreenLine - draw a line
 *
 ***************************************************/

class FtSwarmScreenLine : public FtSwarmScreenObj {

  protected:
    int16_t x1, y1;

  public:

    FtSwarmScreenLine( uint8_t id, FtSwarmScreen *parent, FtSwarmOledScreen_t screen, int16_t x0, int16_t y0, int16_t x1, int16_t y1 );

    // return my type
    virtual FtSwarmScreenObj_t getType( void ) { return FTSWARMSCREEN_LINE; };

    // draw myself
    virtual void draw( bool selected = false );  

    // get my height
    virtual int16_t getHeight( void ) { return y1 - y; };

};

/***************************************************
 *
 * FtSwarmScreenTriangle - draw a triangle
 *
 ***************************************************/

class FtSwarmScreenTriangle : public FtSwarmScreenObj {

  protected:
    bool    filled;
    int16_t x1, y1, x2, y2;

  public:

    FtSwarmScreenTriangle( uint8_t id, FtSwarmScreen *parent, FtSwarmOledScreen_t screen, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool filled );

    // return my type
    virtual FtSwarmScreenObj_t getType( void ) { return FTSWARMSCREEN_TRIANGLE; };

    // draw myself
    virtual void draw( bool selected = false );  

    // get my height
    virtual int16_t getHeight( void );

    // change filled
    virtual void setFilled( bool filled ) { this->filled = filled; draw(); };

};

/***************************************************
 *
 * FtSwarmScreenText
 *
 ***************************************************/

class FtSwarmScreenText : public FtSwarmScreenObj {

  protected:
    int16_t        width;
    char           *text  = nullptr;
    FtSwarmAlign_t align = FTSWARM_ALIGNLEFT;

  public:

    // constructor
    FtSwarmScreenText( uint8_t id, FtSwarmScreen *parent, FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t width, FtSwarmAlign_t align, const char *text );

    // return my type
    virtual FtSwarmScreenObj_t getType( void ) { return FTSWARMSCREEN_TEXT; };

    // constructor
    FtSwarmScreenText( uint8_t id, FtSwarmScreen *parent, FtSwarmOledScreen_t screen, int16_t x, int16_t y, FtSwarmAlign_t align, const char *text );

    // destructor
    ~FtSwarmScreenText();

    // draw myself
    virtual void draw( bool selected = false );      

    // get my height
    virtual int16_t getHeight( void );

    // set my label
    virtual void setText( const char *text );

    // get text
    virtual const char *getText( void ) { return text; };

};

/***************************************************
 *
 * FtSwarmScreenIO
 *
 ***************************************************/

class FtSwarmScreenIO : public FtSwarmScreenText {

  protected:
    SwOSIO               *io       = nullptr;
    int32_t              value     = FTSWARM_NANI32;
    FtSwarmScreenEvent_t lastEvent = FTSWARM_SCREENEVENT_NONE;

  public:

    // constructor
    FtSwarmScreenIO( uint8_t id, FtSwarmScreen *parent, FtSwarmOledScreen_t screen, int16_t x, int16_t y, FtSwarmAlign_t align, const char *text, SwOSIO *io );

    // return my type
    virtual FtSwarmScreenObj_t getType( void ) { return FTSWARMSCREEN_IO; };

    // destructor
    ~FtSwarmScreenIO();

    // activate myself, if my screen is getting active
    virtual void activate( void );

    // deactivate myself, if my screen is getting offline
    virtual void deactivate( void );
    
    // IO informs myself, it's going down
    virtual void unregister( SwOSIO *io );    

    // io sends new value, return true if it's processed by the screen
    virtual void setValue( int32_t value );

};

/***************************************************
 *
 * FtSwarmScreenSelectable - to be selected by joy1
 *
 ***************************************************/

class FtSwarmScreenSelectable : public FtSwarmScreenText {

  protected:
    int16_t widthLabel;
    int16_t widthText;

  public:

    // Constructor
    FtSwarmScreenSelectable( uint8_t id, FtSwarmScreen *parent, FtSwarmOledScreen_t screen, int16_t x, int16_t y, int16_t width, FtSwarmAlign_t align, const char *text );

    // Constructor
    FtSwarmScreenSelectable( uint8_t id, FtSwarmScreen *parent, const char *text );

    // return my type
    virtual FtSwarmScreenObj_t getType( void ) { return FTSWARMSCREEN_SELECTABLE; };
    
    // draw myself
    virtual void draw( bool selected = false );

};

/***************************************************
 *
 * FtSwarmScreenButton - button class
 *
 ***************************************************/

class FtSwarmScreenButton : public FtSwarmScreenIO {

  public:

    // Constructor
    FtSwarmScreenButton( uint8_t id, FtSwarmScreen *parent, FtSwarmOledScreen_t screen, int16_t x, int16_t y, FtSwarmAlign_t align, const char *text, SwOSIO *IO ) : FtSwarmScreenIO( id, parent, screen, x, y, align, text, IO ) { };

    // return my type
    virtual FtSwarmScreenObj_t getType( void ) { return FTSWARMSCREEN_BUTTON; };

    // draw myself
    virtual void draw( bool selected = false );    

};

/***************************************************
 *
 * FtSwarmScreenJoystickPoti - Helper class
 *
 ***************************************************/

class FtSwarmScreenJoystickPoti : public FtSwarmScreenIO {

  protected:
    int32_t maxValue = FTSWARM_NANI32;

  public:

    FtSwarmScreenJoystickPoti(uint8_t id, FtSwarmScreen *parent, FtSwarmOledScreen_t screen, int16_t x, int16_t y, FtSwarmAlign_t align, const char *label, SwOSIO *IO );

    // io sends new value, return true if it's processed by the screen
    virtual void setValue( int32_t value );

};

/***************************************************
 *
 * FtSwarmScreenObjList
 *
 ***************************************************/

class FtSwarmScreenObjList {

  protected:
    FtSwarmScreenObj        *list     = nullptr;
    FtSwarmScreenSelectable *selected = nullptr;    
    int16_t drawCursor = -1;


  public:

    // destructor
    ~FtSwarmScreenObjList();
  
    // add an object, returns false if out ouf space
    void add( FtSwarmScreenObj *newObject );

    // delete an object and release it
    void del( FtSwarmScreenObj *delObject );

    // delete all of one type
    void deleteAll( FtSwarmOledScreen_t screen, FtSwarmScreenObj_t type );

    // activate all elements, my screen is getting active
    void activate( void );

    // deactivate all elements, my screen is getting offline
    void deactivate( void );

    // draw
    void draw( void );

    // get next free line within the referenced screen
    int16_t getNextY( FtSwarmOledScreen_t screen );

    // moves selected element and returns it
    FtSwarmScreenSelectable *nextSelectable( void );

    // moves selected element and returns it
    FtSwarmScreenSelectable *prevSelectable( void );

    // get actual selected element
    FtSwarmScreenSelectable *getSelected( void ) { return selected; };

    // debug
    void debug( void );

};

/***************************************************
 *
 * FtSwarmScreen - Base class for all screens
 *
 ***************************************************/

class FtSwarmScreen {

  protected:

    FtSwarmScreen *parent      = nullptr;
    bool          blockEvents  = true;
    bool          navigation   = false;
    bool          ESC          = false;
    bool          buttonScreen = false;

    FtSwarmScreenObjList objects;

    void splitText( const char *text );
    void addNavigation( void );

  public:

    uint32_t USID;
    bool     toBeDestroyed = false;

    // constructor
    FtSwarmScreen( FtSwarmScreen *parent, const char *title, const char *text = nullptr );

    // add a screen object
    virtual FtSwarmScreenObj *add( FtSwarmScreenObj *newObject );

    // cls and draw all elements
    virtual void draw( void ); 

    // activate my objects, if my screen is getting active
    virtual void activate( void );

    // deactivate my objects, if my screen is getting offline
    virtual void deactivate( void );

    // called all 25ms, so be minimalistic
    virtual void operate( void ) {};

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = nullptr );

    // close myself
    virtual void close( FtSwarmScreenEvent_t event = FTSWARM_SCREENEVENT_NONE, uint8_t id=0, uint8_t nParam = 0, const char *sParam = nullptr );

    // calculate next free line
    virtual int16_t getNextY( FtSwarmOledScreen_t screen );

    // Button S1
    FtSwarmScreenButton *addS1( const char *text );

    // Button S2
    FtSwarmScreenButton *addS2( const char *text );

    // Button S3
    FtSwarmScreenButton *addS3( const char *text );

    // Button S4
    FtSwarmScreenButton *addS4( const char *text );

    // Button F1
    FtSwarmScreenButton *addF1( const char *text );

    // Button F2
    FtSwarmScreenButton *addF2( const char *text );

    // Button J1
    FtSwarmScreenButton *addJ1( const char *text, FtSwarmOledScreen_t screen = FTSWARM_OLED_MAINSCREEN );

    // Button J2
    FtSwarmScreenButton *addJ2( const char *text, FtSwarmOledScreen_t screen = FTSWARM_OLED_MAINSCREEN );

    // Button ESC
    FtSwarmScreenButton *addESC( void );

    // Simple Text
    FtSwarmScreenText *addText( FtSwarmOledScreen_t screen, int16_t x, int16_t y, FtSwarmAlign_t align, const char *text );
    
    // joysticks
    void addJoystick( FtSwarmOledScreen_t screen, const char *text, uint8_t joystick );

    // Line
    FtSwarmScreenLine *addLine( FtSwarmOledScreen_t screen, int16_t x1, int16_t y1, int16_t x2, int16_t y2 );

};

/***************************************************
 *
 * FtSwarmScreenChooseOption
 *
 ***************************************************/

 class FtSwarmScreenChooseOption : public FtSwarmScreen {

  protected:
    uint8_t callbackID;
    int32_t value[4];

  public:

    // constructor
    FtSwarmScreenChooseOption( FtSwarmScreen *parent, const char *title, const char *text, uint8_t callbackID, 
                               int32_t value1,   const char *option1, 
                               int32_t value2=0, const char *option2=nullptr, 
                               int32_t value3=0, const char *option3=nullptr, 
                               int32_t value4=0, const char *option4=nullptr );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = nullptr );

};

/***************************************************
 *
 * FtSwarmScreenSelectList & FtSwarmScreenSelectList2
 *
 ***************************************************/

 class FtSwarmScreenSelectList : public FtSwarmScreen {

  protected:

    uint8_t callbackID;

  public:

    // same with a two arrays callbackID and str
    FtSwarmScreenSelectList( FtSwarmScreen *parent, const char *title, const char *text, uint8_t items, uint8_t callbackID[], char *str[] );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = nullptr );

 };

 class FtSwarmScreenSelectList2 : public FtSwarmScreen {

  protected:

    uint8_t callbackID;
    void process() {};

    template<typename... Tail>
    void process(uint8_t id, const char* str, Tail... tail);

  public:

    // Constructor - parent gets a callback: eventHandler( FTSWARM_SCREENEVENT_OK, callbackID, selected object's ID )
    // example: new FtSwarmScreenSelectList( this, "Number", nullptr, SELECTMYNUMBER_CB, 42, "fourty-two", 17, "seventeen" )
    // template<typename... Args>
    // example: new FtSwarmScreenSelectList( this, "Number", nullptr, SELECTMYNUMBER_CB, 42, "fourty-two", 17, "seventeen" )
    template<typename... Args>
    FtSwarmScreenSelectList2( FtSwarmScreen *parent, const char *title, const char *text, uint8_t callbackID, Args... args);

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = nullptr );

};

 /***************************************************
 *
 * FtSwarmScreenInput
 *
 ***************************************************/

#define KEYMAPS 4

class FtSwarmScreenInput : public FtSwarmScreen {

  protected:

    uint8_t callbackID;

    char    *input    = nullptr;
    uint8_t maxLength = 0;

    int16_t lineHeight;
    int16_t charWidth;

    FtSwarmScreenButton *S1 = nullptr;
    
    const char keyboardMap[KEYMAPS][30] = { R"(abcdefghijklmnopqrstuvwxyz@ )",
                                            R"(ABCDEFGHIJKLMNOPQRSTUVWXYZ_ )",
                                            R"(0123456789+-.)",
                                            R"(!"#$%&':;<=>?\^`|~[](){})*/,)" };

    const char S1Label[KEYMAPS][4] = { "A-Z", "NUM", "#$@", "a-z" };

    const uint8_t cols[KEYMAPS] = { 14, 14,  7, 14 };
    const uint8_t rows[KEYMAPS] = {  2,  2,  2,  2 };

    uint8_t keyboard = 0;
    bool numKeyboard = false;

    uint8_t keyboardX, keyboardY, keyboardWidth, keyboardHeight;

    uint8_t cursorR[5] = { 0, 0, 0, 0, 0 };
    uint8_t cursorC[5] = { 0, 0, 0, 0, 0 };

    void init( FtSwarmScreen *parent, const char *title, uint8_t callbackID, const char *param, uint8_t maxLength );
    uint8_t keymapIndex( void ) { return cursorR[keyboard]*cols[keyboard] + cursorC[keyboard]; };
    void setKeyboard( uint8_t keyboard );
    void drawCursor( bool invert );
    void drawInput( void );
    
  public:
   
    // Constructor to enter strings
    FtSwarmScreenInput( FtSwarmScreen *parent, const char *title, uint8_t callbackID, const char *param, uint8_t maxLength );

    // constructor to enter numbers
    FtSwarmScreenInput( FtSwarmScreen *parent, const char *title, uint8_t callbackID, const int32_t param, uint8_t maxLength );

    // Destructor
    ~FtSwarmScreenInput( );

    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = nullptr );

}; 

/***************************************************
 *
 * FtSwarmScreenConfirm
 *
 ***************************************************/

class FtSwarmScreenConfirm : public FtSwarmScreenChooseOption {

  public: 
    FtSwarmScreenConfirm( FtSwarmScreen *parent, const char *title, const char *text, uint8_t callbackID, int32_t ok = 1 ) : FtSwarmScreenChooseOption( parent, title, text, callbackID, ok, "OK" ) {};

};

/***************************************************
 *
 * FtSwarmScreenYesNo
 *
 ***************************************************/

class FtSwarmScreenYesNo : public FtSwarmScreenChooseOption {

  public: 
    FtSwarmScreenYesNo( FtSwarmScreen *parent, const char *title, const char *text, uint8_t callbackID, int32_t yes = 1 ) : FtSwarmScreenChooseOption( parent, title, text, callbackID, 0, nullptr, yes, "YES", 0, "NO" ) {};

};

/***************************************************
 *
 * FtSwarmScreenError
 *
 ***************************************************/

class FtSwarmScreenError : public FtSwarmScreenChooseOption {

  public: 
    FtSwarmScreenError( FtSwarmScreen *parent, const char *text ) : FtSwarmScreenChooseOption( parent, "Error", text, FTSWARMSCREEN_NOID,  0, nullptr, 0, nullptr, 0, nullptr, 1, "OK" ) {};

};

class FtSwarmScreenInfo : public FtSwarmScreenChooseOption {

  public: 
    FtSwarmScreenInfo( FtSwarmScreen *parent, const char *text ) : FtSwarmScreenChooseOption( parent, "Info", text, FTSWARMSCREEN_NOID,  0, nullptr, 0, nullptr, 0, nullptr, 1, "OK" ) {};

};

/***************************************************
 *
 *   FtSwarmScreenWifi
 *
 ***************************************************/

class FtSwarmScreenWifi : public FtSwarmScreen {

  protected:

    FtSwarmScreenSelectable *wifiModeSelect = nullptr;
    FtSwarmScreenSelectable *wifiSSIDSelect = nullptr;
    FtSwarmScreenText       *wifiSSIDText   = nullptr;
    FtSwarmScreenSelectable *wifiPwdSelect  = nullptr;
    FtSwarmScreenText       *wifiPwdText    = nullptr;

    bool anythingChanged = false;
    FtSwarmScreenButton *S4 = nullptr;

    char wifiPwd[64];
    char wifiSSID[64];
    FtSwarmWifi_t wifiMode;

  public:
    FtSwarmScreenWifi( FtSwarmScreen *parent  );

  // eval external events like pressing buttons
  virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = nullptr );

};

/***************************************************
 *
 *   FtSwarmScreenWifiSSID
 *
 ***************************************************/

class FtSwarmScreenWifiSSID : public FtSwarmScreen {

  protected:

    bool waitForScan = false;
    uint8_t id = 0;

  public:

    // constructor
    FtSwarmScreenWifiSSID( FtSwarmScreen *parent, uint8_t id );

    // destructor
    ~FtSwarmScreenWifiSSID();

    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = nullptr );

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

    FtSwarmScreenButton *S2 = nullptr;

    void addMembers( void );

  public:

    // Constructor
    FtSwarmScreenSwarm( FtSwarmScreen *parent );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = nullptr );

};

/***************************************************
 *
 *   FtSwarmScreenRemote
 *
 ***************************************************/

class FtSwarmScreenRemote : public FtSwarmScreen {

  protected:
    // void configureSelected( uint8_t config );
    int8_t selectedCtrl = -1; // index of selected controller

  public:

    // Constructor
    FtSwarmScreenRemote( FtSwarmScreen *parent  );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = nullptr );

};

/***************************************************
 *
 *   FtSwarmScreenSelectIO
 *
 ***************************************************/

class FtSwarmScreenSelectIO : public FtSwarmScreen {

  protected:

    SwOSIO *io[20];
    int8_t maxIO = -1;

    void addIO( SwOSIOType_t ioType, bool localOnly );

  public:

    // Constructor
    FtSwarmScreenSelectIO( FtSwarmScreen *parent, const char *title, const char *text = nullptr );

};

/***************************************************
 *
 *   FtSwarmScreenCalibrateJoystick
 *
 ***************************************************/

class FtSwarmScreenCalibrateJoystick : public FtSwarmScreen {

  protected:

    uint8_t status = 0;
    int32_t lastValue[2] = {FILTER_INVALID, FILTER_INVALID};
    SwOSJoystick *joystick;
    SwOSJoyCalibration_t calibration[2];
    FtSwarmScreenTriangle *triangle[4];
    FtSwarmScreenText *text[2];

  public:

    // Constructor
    FtSwarmScreenCalibrateJoystick( FtSwarmScreen *parent, SwOSJoystick *joystick );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = nullptr );

    // called all 25ms, so be minimalistic
    virtual void operate( void );
    
};

/***************************************************
 *
 *   FtSwarmScreenCalibrateList
 *
 ***************************************************/

class FtSwarmScreenCalibrateList : public FtSwarmScreenSelectIO {

  public:

    // Constructor
    FtSwarmScreenCalibrateList( FtSwarmScreen *parent );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = nullptr );

};

/***************************************************
 *
 *   FtSwarmScreenServoOffsetList
 *
 ***************************************************/

class FtSwarmScreenServoOffsetList : public FtSwarmScreenSelectIO {

  public:

    // Constructor
    FtSwarmScreenServoOffsetList( FtSwarmScreen *parent );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = nullptr );

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
    SwOSMainScreen( void );

    // cls and draw all elements
    virtual void draw( void );

    // eval external events like pressing buttons
    virtual bool eventHandler( FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = nullptr );


};

/***************************************************
 *
 * SwOSSplashScreen 
 * startup screen, switches after 5s to MainScreen
 *
 ***************************************************/

class SwOSSplashScreen : public FtSwarmScreen {

  public:

    FtSwarmScreenText *info = nullptr;

    // constructor
    SwOSSplashScreen( void );

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
    SwOS404Screen( void ):FtSwarmScreen( nullptr, "Page not found", "Internal error - the requested page is not available." ) {};

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

class FtSwarmScreenManager {

  protected:

    static const uint8_t MAXSCREENS = 10;

    SwOSSplashScreen *splashScreen = nullptr;

    FtSwarmScreen *screen[ MAXSCREENS];

    // get a free index in screen[]
    uint8_t getIndex( FtSwarmScreen *screen );

  public:
    FtSwarmScreen *active = nullptr;
    bool blockEvents = false;

    FtSwarmScreenManager();

    // send an event to active screen
    void eventHandler( FtSwarmScreen *screen, FtSwarmScreenEvent_t event, uint8_t id, int32_t nParam = FTSWARM_NANI32, const char *sParam = nullptr );

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

    // display swarm Status
    void setState( SwOSState_t state, const char *text, uint8_t members, const char *SSID );

    void wifiMenu( void ) { activate( new FtSwarmScreenWifi( active ) ); };

};

extern FtSwarmScreenManager screenManager;

#endif