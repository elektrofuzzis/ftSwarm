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
#include <ftDuino.h>

#include "ftPwrDrive/ftPwrDrive.h"

#include "SwOS.h"
#include "SwOSCom.h"
#include "jsonize.h"

#define BRIGHTNESSDEFAULT 48

// reference to local ftPwrDrive
extern ftPwrDrive *pwrDrive;

// reference to local ftDuino
extern FtDuino *ftDuino;

// forward declaration
class SwOSCtrl; 

// state
typedef enum { BOOTING, STARTWIFI, RUNNING, ERROR, WAITING, IDENTIFY, MAXSTATE } SwOSState_t;

// comState
typedef enum { COMSTATE_UNDEFINED, COMSTATE_CONNECT_PHASE1, COMSTATE_CONNECT_PHASE2, COMSTATE_ONLINE, COMSTATE_ERROR } SwOSComState_t;

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

  virtual void loadAliasFromNVS(  nvs_handle_t my_handle ); // load my alias from NVS
  virtual void saveAliasToNVS(  nvs_handle_t my_handle );   // dave my alias from NVS
  
	void setName( const char *name);   // set new name
  char *getName();                   // get name
  
	void setAlias( const char *alias); // add an alias name
  char *getAlias();                  // get alias

	bool equals(const char *name);     // check if hw name or alias is equal to name

	virtual void jsonize( JSONize *json, uint8_t id);
};

/***************************************************
 *
 *   SwOSIO - Base class for all sensors or actors.
 *
 ***************************************************/

class SwOSIO : public SwOSObj {
protected:
	uint8_t    port;  // local port
  SwOSCtrl  *ctrl;  // pointer to my Controller
  bool       isSubscribed = false;
  uint32_t   lastsubscribedValue = 0;
  uint32_t   hysteresis = 0;
  char      *subscribedIOName = NULL;
  int16_t    useCounter = 0;

  // local HW 
  virtual void setupLocal() {};

public:
  // Constructors
	SwOSIO(const char *name, SwOSCtrl *ctrl);                 // constructor name, pointer to overlying controller
	SwOSIO(const char *name, uint8_t port, SwOSCtrl *ctrl);   // constructor name, port, pointer to overlying controller

  // Administrative stuff
  virtual void            lock(void);
  virtual void            unlock(void);
  virtual char*           subscribe( char *IOName, uint32_t hysteresis ); // subscribe sensor to display value changes as console outputs 
	virtual void            unsubscribe();                                  // clear subscription
  virtual uint8_t         getPort() { return port; };
  virtual SwOSCtrl*       getCtrl() { return ctrl; };
	virtual FtSwarmIOType_t getIOType() { return FTSWARM_UNDEF; };
  virtual FtSwarmIcon_t   getIcon() { return FTSWARM_XX_UNDEF; };
	virtual void            jsonize( JSONize *json, uint8_t id);
  virtual void            take( void ) { useCounter++; };                      // register an instance using this IO
  virtual void            give( void ) { if (useCounter>0) useCounter--; };   // unregister an instance using this IO
  virtual bool            isInUse( void ) { return useCounter > 0; };          // test, if an IO is used by some user elements
  
  // Test, if I'm an ...
  virtual bool isActor( void ) { return false; };
  virtual bool isInput( void ) { return false; };
  virtual bool isServo( void ) { return false; };
  virtual bool isGyro( void )  { return false; };
  virtual bool isI2C( void )   { return false; };
  virtual bool isOLED( void )  { return false; };
  virtual bool isPixel( void ) { return false; };

  virtual void read( void ) { };
  virtual void onTrigger( int32_t value );

};

/***************************************************
 *
 *   SwOSEventHandler
 *
 ***************************************************/

class SwOSEventHandler {
  protected:
    SwOSIO           *actor;
    boolean          usePortValue;
    int32_t          parameter;
  public:
    SwOSEventHandler( );
    SwOSEventHandler( SwOSIO *actor, boolean usePortValue, int32_t parameter );
    void trigger( int32_t portValue );
};

class SwOSEventHandlers {
  protected:
    SwOSEventHandler *event[FTSWARM_MAXTRIGGER];
  public:
    SwOSEventHandlers( );
    ~SwOSEventHandlers();
    void registerEvent( FtSwarmTrigger_t triggerEvent, SwOSIO *actor, boolean usePortValue, int32_t parameter );
    void unregisterEvent( FtSwarmTrigger_t triggerEvent );
    void trigger( FtSwarmTrigger_t triggerEvent, int32_t portValue );
};

class SwOSEventInput {
  protected:
    SwOSEventHandlers *events = NULL;
  public:
   ~SwOSEventInput();
    void registerEvent( FtSwarmTrigger_t triggerEvent, SwOSIO *actor, boolean usePortValue, int32_t p1 );
    void unregisterEvent( FtSwarmTrigger_t triggerEvent );
    void trigger( FtSwarmTrigger_t triggerEvent, int32_t portValue );
};

/***************************************************
 *
 *   SwOSInput
 *
 ***************************************************/

class SwOSInput : public SwOSIO, public SwOSEventInput {
  
  protected:
    gpio_num_t        GPIO = GPIO_NUM_NC;
	  FtSwarmSensor_t   sensorType;
	  int32_t           lastRawValue = 0;
	
    virtual void setupLocal();
    virtual void subscription();
    virtual void setSensorTypeLocal( FtSwarmSensor_t sensorType );
	  

  public:
 
	  SwOSInput(const char *name, uint8_t port, SwOSCtrl *ctrl, FtSwarmSensor_t sensorType );
  
    // administrative stuff
	  virtual FtSwarmIOType_t getIOType() { return FTSWARM_INPUT; };
    virtual FtSwarmSensor_t getSensorType() { return sensorType; };
    virtual FtSwarmIcon_t   getIcon();
	  virtual void jsonize( JSONize *json, uint8_t id) {};

    // Test, if I', an Sensor
    virtual bool            isSensor( void ) { return true; };

    // read sensor
	  virtual void read() {};
    virtual void setReading( int32_t newValue ) {};

    // external commands
    virtual void            setSensorType( FtSwarmSensor_t sensorType );  // set sensor type
	  virtual int32_t         getValueI32( void );                          // get raw reading
	  virtual float           getValueF( void );                            // get float reading
    virtual void            setValue( int32_t value ) {};                 // set value by an external call
  
};

