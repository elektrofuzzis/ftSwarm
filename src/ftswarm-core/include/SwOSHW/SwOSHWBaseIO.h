/*
 * SwOSHWBaseIO.h
 *
 * Basic classes for IO hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */

#pragma once

#include <nvs.h>

#include "SwOSHW/SwOSHWDuino.h"
#include "ftPwrDrive/ftPwrDrive.h"

#include "SwOS.h"
#include "SwOSCom.h"
#include "serialize.h"
#include "SwOSFilter.h"


#define BRIGHTNESSDEFAULT 48

// reference to local ftPwrDrive
extern FtPwrDrive *ftPwrDrive;

// reference to local ftDuino
extern SwOSDuino *ftDuino;

// forward declaration
class SwOSCtrl; 
class FtSwarmScreenIO;

// comState
typedef enum { 
  COMSTATE_UNDEFINED, 
  COMSTATE_CONNECT_PHASE1, // try to connect the remote controller
  COMSTATE_CONNECT_PHASE2, // wait for IO settings & alias names
  COMSTATE_ONLINE, 
  COMSTATE_ERROR,
  COMSTATE_MAX } SwOSComState_t;

  
const char SWOSCOMSTATE[COMSTATE_MAX][11] = { "OFFLINE", "OFFLINE", "CONNECTING", "ONLINE", "ERROR" };

typedef enum {
    UICLASS_NONE,
    UICLASS_SENSOR,
    UICLASS_PIXEL,
    UICLASS_ONOFF,
    UICLASS_MOTOR,
    UICLASS_SERVO,
    UICLASS_JOYSTICK,
    UICLASS_CAM
} SwOSUIClass_t;


/***************************************************
 *
 *   SwOSObj - Base class for all SwOS objects.
 *
 ***************************************************/

class SwOSObj {

  protected:
  	char *_name  = NULL;
	  char *_alias = NULL;
    uint8_t flags  = 0;

  public:

    // Constructor
    SwOSObj( uint8_t flags ) { this->flags = flags; };                     

    // constructor, sets the objects HW name
	  SwOSObj( const char *name, uint8_t flags);		    

	  virtual ~SwOSObj();                       // destructor

    // load my port & alias settings from NVS
    virtual void loadFromNVS( nvs_handle_t myHandle );

    // save my port & alias settings from NVS
    virtual void saveToNVS( nvs_handle_t myHandle );

    // print my nvs settings
    virtual void printNVS( nvs_handle_t myHandle );

    // set flags
    void setFlags( uint8_t flags ) { this->flags = flags;};

    // set flag
    void setFlag( uint8_t flag ) { this->flags |= flag; };

    // reset flag
    void resetFlag( uint8_t flag ) { this->flags &= ~( flag ); };
    
    // get flags
    uint8_t getFlags( void ) { return flags; };

    // test flags
    bool testFlag( uint8_t flag ) { return ( ( flags & flag) > 0 ); };

    // getNVSParameter to send nvs parameters like servo offset via createIO
    virtual uint8_t *getNVSParameter( uint8_t *size ) { *size = 0; return nullptr; };
    
    // setNVSParameter to receive nvs parameters like servo offset via createIO
    virtual void setNVSParameter( uint8_t parameter[], uint8_t *size ) { };
  
    // set new name
	  void setName( const char *name);

    // get name
    const char *getName();             
  
    // add an alias name
	  virtual void setAlias( const char *alias); 

    // get alias
    const char *getAlias();            

    // get alias or name (if an alis isn't set)
    const char *getAliasOrName();      

    // check if hw name or alias is equal to name
	  bool equals(const char *name);     

    // show my settings
	  virtual void serialize( Serialize *serialize );

};

/***************************************************
 *
 *   SwOSIO - Base class for all sensors or actors.
 *
 ***************************************************/

class SwOSIO : public SwOSObj {
protected:
	uint8_t       port;  // local port
  SwOSCtrl      *ctrl; // pointer to my Controller
  FtSwarmScreenIO *subscribedScreenIO = NULL;
  SwOSIOType_t  ioType               = SWOSIO_UNDEF;
  bool          isSubscribed         = false;
  int32_t       lastsubscribedValue  = 0;
  int32_t       hysteresis           = 0;
  char          *subscribedIOName    = NULL;
  int16_t       useCounter           = 0;

  // local HW 
  virtual void setupLocal() {};
  static int32_t evalOperand( FtSwarmOperand_t v, int32_t sensor, int32_t actor, int32_t parameter );
  static int32_t evalTriggerMath( SwOSTriggerMath triggerMath, int32_t sensor, int32_t actor, int32_t parameter, int32_t minValue, int32_t maxValue );

public:
  // Constructor
	SwOSIO(const char *name, SwOSCtrl *ctrl, SwOSIOType_t ioType, uint8_t flags ) : SwOSIO( name, SWOS_NOPORT, ctrl, ioType, flags ) {};
	
  // Constructor
	SwOSIO(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType, uint8_t flags );   

  // Destructor
  ~SwOSIO();

  // load my port & alias settings from NVS
  virtual void loadFromNVS( nvs_handle_t my_handle );

  // save my port & alias settings from NVS
  virtual void saveToNVS( nvs_handle_t my_handle );
  
  // print my nvs settings
  virtual void printNVS( nvs_handle_t my_handle );

  // lock io
  virtual void lock(void);

  // unlock io
  virtual void unlock(void);

  // subscribe io to display value changes as console outputs 
  virtual char* subscribe( const char *IOName, uint32_t hysteresis ); 

  // subscribe io to send status information to a Screen
  virtual void subscribe( FtSwarmScreenIO *screenIO );

  // clear subscription
	virtual void unsubscribe();

  // clear subscription of a screen object
  virtual void unsubscribe(  FtSwarmScreenIO *screenIO );

  // get my port
  virtual uint8_t getPort() { return port; };

  // get my controller
  virtual SwOSCtrl* getCtrl() { return ctrl; };

  // get my ioType
	virtual SwOSIOType_t getIOType() { return ioType; };

  // get my UIClass
  virtual SwOSUIClass_t getUIClass();

  // get my unique ID UID
  virtual void getUID( SwOSIOUID *uid );

  // return controller.ioname or alias in name. Name needs to be 2*MAXIDENTIFIER+2
  virtual void getUniqueName( char *name );

  // show my settings
	virtual void serialize( Serialize *serialize ) override;

  // register an instance using this IO
  virtual void take( void ) { useCounter++; };

  // unregister an instance using this IO
  virtual void give( void ) { if (useCounter>0) useCounter--; };   

  // test, if an IO is used by some user elements
  virtual bool isInUse( void ) { return useCounter > 0; };

  // test, if IO shall be shown in the API
  virtual bool showInApi( void ) { return ( (!testFlag( FTSWARM_HAL_FLAG_HIDDEN ) ) && ( SHOWIOINAPI[ ioType ]) );  };

  // halt all motors
  virtual void halt( void ) {};

  // push my state to a buffer
  virtual uint8_t pushState( uint8_t *buffer ) { return 0; };

  // pop my state from a buffer
  virtual uint8_t popState( uint8_t *buffer )  { return 0; };

  // set parameter from remote 
  virtual void setParameter( int32_t parameter ) {};

  // is the io online?
  virtual bool isOnline( void );

  // set my OLED label text
  virtual void setLabelText( char *text );

  // get my OLED label type
  virtual SwOSLabel_t  getLabel( void );
  
  // Test, if I'm an ...
  virtual bool isMotor( void )        { return false; };  
  virtual bool isActor( void )        { return false; }; // actor to be used with triggers
  virtual bool isInput( void )        { return false; }; 
  virtual bool isEventInput( void )   { return false; }; // input to be used with triggers
  virtual bool isGPIOInput( void )    { return false; };  
  virtual bool isDigitalInput( void ) { return false; };  
  virtual bool isAnalogInput( void )  { return false; };  
  virtual bool isServo( void )        { return false; };  
  virtual bool isGyro( void )         { return false; };
  virtual bool isI2C( void )          { return false; };
  virtual bool isOLED( void )         { return false; };
  virtual bool isPixel( void )        { return false; };
  virtual bool isCAM( void )          { return false; };
  virtual bool isCounter( void )      { return false; };
  virtual bool isStepper( void )      { return false; };

  virtual void operate( void ) { };
  virtual void onTrigger( SwOSTriggerMath triggerMath, int32_t sensor, int32_t parameter );

  // get my raw value
  virtual int32_t getValueI32( void ) { return FTSWARM_NANI32; };

};

/***************************************************
 *
 *   SwOSEventHandler
 *
 ***************************************************/

class SwOSEventHandler {
  public:
    SwOSTriggerMath   triggerMath;
    SwOSIO            *actor         = NULL;
    int32_t           parameter      = 0;
    SwOSEventHandler  *next          = NULL;

    SwOSEventHandler( SwOSTriggerMath triggerMath, SwOSIO *actor, int32_t parameter );
    ~SwOSEventHandler( );
};

class SwOSEventInput {
  public:
    SwOSEventHandler *eventList = NULL;
  public:
   ~SwOSEventInput();
    bool deleteEvent( FtSwarmTrigger_t triggerEvent, FtSwarmOperator_t op, SwOSIO *actor );
    void deleteEvents( void );
    bool addEvent( FtSwarmTrigger_t triggerEvent, FtSwarmOperator_t op, FtSwarmOperand_t v1, FtSwarmOperand_t v2, SwOSIO *actor, int32_t parameter );
    void trigger( FtSwarmTrigger_t triggerEvent, int32_t sensor );
};

/***************************************************
 *
 *   SwOSInput
 *
 ***************************************************/

class SwOSInput : public SwOSIO, public SwOSEventInput {
  
  protected:
    gpio_num_t GPIO = GPIO_NUM_NC;
	  int32_t    lastRawValue = 0;
	
    virtual void setupLocal();
    virtual void subscription();

    virtual uint8_t pushState8( uint8_t *buffer );
    virtual uint8_t pushState16( uint8_t *buffer );
    virtual uint8_t pushState32( uint8_t *buffer );

    virtual uint8_t popState8( uint8_t *buffer );
    virtual uint8_t popState16( uint8_t *buffer );
    virtual uint8_t popState32( uint8_t *buffer );
	  

  public:
 
	  SwOSInput(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType, uint8_t flags ) : SwOSIO( name, port, ctrl, ioType, flags ), SwOSEventInput( ) { };
  
    // administrative stuff
	  virtual void serialize( Serialize *serialize ) {};
	  void serializeEvents( Serialize *serialize );

    // push my state to a buffer
    virtual uint8_t pushState( uint8_t *buffer ) { return pushState32( buffer ); };

    // pop my state from a buffer
    virtual uint8_t popState( uint8_t *buffer )  { return popState32( buffer ); };
  
    // Test, if I'm an input
    virtual bool isInput( void ) { return true; };

    // input to be used with triggers
    virtual bool isEventInput( void ) { return true; }; 

    // read sensor
	  virtual void operate() {};
    virtual void setReading( int32_t newValue, FtSwarmTrigger_t secondTriggerEvent );

    // external commands
	  virtual int32_t getValueI32( void );                          // get raw reading
	  virtual float   getValueF( void );                            // get float reading
  
};

