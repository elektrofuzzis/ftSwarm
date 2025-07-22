/*
 * SwOSHWBaseIO.h
 *
 * Basic classes for IO hardware impelmentation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

// #include <stdint.h>

// #include <driver/ledc.h>
// #include <driver/gpio.h>

#include <nvs.h>

#include "SwOSHW/SwOSHWDuino.h"
#include "ftPwrDrive/ftPwrDrive.h"

#include "SwOS.h"
#include "SwOSCom.h"
#include "serialize.h"

#define BRIGHTNESSDEFAULT 48

// reference to local ftPwrDrive
extern FtPwrDrive *ftPwrDrive;

// reference to local ftDuino
extern SwOSDuino *ftDuino;

// forward declaration
class SwOSCtrl; 

// comState
typedef enum { 
  COMSTATE_UNDEFINED, 
  COMSTATE_CONNECT_PHASE1, // try to connect the remote controller
  COMSTATE_CONNECT_PHASE2, // wait for IO settings & alias names
  COMSTATE_ONLINE, 
  COMSTATE_ERROR } SwOSComState_t;

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
public:
  SwOSObj() {};                     // std constructor
	SwOSObj( const char *name);		    // constructor, sets the objects HW name
  ~SwOSObj();                       // destructor

  virtual void loadFromNVS(  nvs_handle_t my_handle ); // load my port & alias settings from NVS
  virtual void saveToNVS(  nvs_handle_t my_handle );   // dave my port & alias settings from NVS
  
	void setName( const char *name);   // set new name
  char *getName();                   // get name
  
	void setAlias( const char *alias); // add an alias name
  char *getAlias();                  // get alias

	bool equals(const char *name);     // check if hw name or alias is equal to name

	virtual void serialize( Serialize *serialize, uint8_t id);
};

/***************************************************
 *
 *   SwOSIO - Base class for all sensors or actors.
 *
 ***************************************************/

class SwOSIO : public SwOSObj {
protected:
	uint8_t      port;  // local port
  SwOSCtrl     *ctrl; // pointer to my Controller
  SwOSIOType_t ioType = SWOSIO_UNDEF;
  bool         isSubscribed = false;
  int32_t      lastsubscribedValue = 0;
  int32_t      hysteresis = 0;
  char         *subscribedIOName = NULL;
  int16_t      useCounter = 0;

  // local HW 
  virtual void setupLocal() {};

public:
  // Constructors
	SwOSIO(const char *name, SwOSCtrl *ctrl, SwOSIOType_t ioType ) : SwOSIO( name, SWOS_NOPORT, ctrl,ioType ) {};
	SwOSIO(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType );   

  // Administrative stuff
  virtual void            loadFromNVS( nvs_handle_t my_handle ); // load my port & alias settings from NVS
  virtual void            saveToNVS( nvs_handle_t my_handle );   // dave my port & alias settings from NVS
  virtual void            lock(void);
  virtual void            unlock(void);
  virtual char*           subscribe( char *IOName, uint32_t hysteresis ); // subscribe sensor to display value changes as console outputs 
	virtual void            unsubscribe();                                  // clear subscription
  virtual uint8_t         getPort() { return port; };
  virtual SwOSCtrl*       getCtrl() { return ctrl; };
	virtual SwOSIOType_t    getIOType() { return ioType; };
  virtual SwOSUIClass_t   getUIClass();
  virtual const char*     getIcon();
	virtual void            serialize( Serialize *serialize, uint8_t id);
  virtual void            take( void ) { useCounter++; };                      // register an instance using this IO
  virtual void            give( void ) { if (useCounter>0) useCounter--; };   // unregister an instance using this IO
  virtual bool            isInUse( void ) { return useCounter > 0; };          // test, if an IO is used by some user elements
  virtual bool            showInApi( void ) { return SHOWIOINAPI[ ioType ]; };
  virtual void            halt( void ) {};
  virtual uint8_t         pushState( uint8_t *buffer ) { return 0; };
  virtual uint8_t         popState( uint8_t *buffer )  { return 0; };
  virtual void            setParameter( int32_t parameter ) {};
  virtual bool            isOnline( void );
  
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

  virtual void read( void ) { };
  virtual void onTrigger( int32_t value );

};

/***************************************************
 *
 *   SwOSEventHandler
 *
 ***************************************************/

class SwOSEventHandler {
  public:
    FtSwarmTrigger_t trigger;
    SwOSIO           *actor         = NULL;
    bool             useSensorValue = NULL;
    int32_t          parameter      = 0;
    SwOSEventHandler *next          = NULL;

    SwOSEventHandler( FtSwarmTrigger_t trigger, SwOSIO *actor, int32_t parameter );
    ~SwOSEventHandler( );
};

class SwOSEventInput {
  public:
    SwOSEventHandler *eventList = NULL;
  public:
   ~SwOSEventInput();
    bool deleteEvent( FtSwarmTrigger_t triggerEvent, SwOSIO *actor );
    void deleteEvents( void );
    bool addEvent( FtSwarmTrigger_t triggerEvent, SwOSIO *actor, int32_t parameter );
    void trigger( FtSwarmTrigger_t triggerEvent, int32_t value );
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
	  

  public:
 
	  SwOSInput(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType ) : SwOSIO( name, port, ctrl, ioType ), SwOSEventInput( ) { };
  
    // administrative stuff
	  virtual void serialize( Serialize *serialize, uint8_t id) {};
    virtual uint8_t pushState( uint8_t *buffer );
    virtual uint8_t popState( uint8_t *buffer );
  
    // Test, if I'm an input
    virtual bool isInput( void ) { return true; };

    // input to be used with triggers
    virtual bool isEventInput( void )   { return true; }; 

    // read sensor
	  virtual void read() {};
    virtual void setReading( int32_t newValue );

    // external commands
	  virtual int32_t getValueI32( void );                          // get raw reading
	  virtual float   getValueF( void );                            // get float reading
    virtual void    setValue( int32_t value ) {};                 // set value by an external call
  
};

