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

const char SENSORTYPE[FTSWARM_MAXSENSOR][20] = { 
  "DIGITAL", 
  "ANALOG", 
  "SWITCH", 
  "REEDSWITCH", 
  "LIGHTBARRIER", 
  "VOLTMETER", 
  "OHMMETER", 
  "THERMOMETER", 
  "LDR", 
  "TRAILSENSOR", 
  "COLORSENSOR", 
  "ULTRASONIC", 
  "CAM", 
  "COUNTER", 
  "ROTARYENCODER", 
  "FREQUENCYMETER",
  "LIDAR" };

// forward declaration
class SwOSCtrl; 

// state
typedef enum { BOOTING, STARTWIFI, RUNNING, ERROR, WAITING, IDENTIFY, MAXSTATE } SwOSState_t;

// comState
typedef enum { INITIALIZING, ASKFORDETAILS, UP } SwOSComState_t;

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
	uint8_t   _port;  // local port
  SwOSCtrl *_ctrl;  // pointer to my Controller
  bool      _isSubscribed = false;
  uint32_t  _lastsubscribedValue = 0;
  uint32_t  _hysteresis = 0;
  char     *_subscribedIOName = NULL;
  int16_t   _useCounter = 0;

  // local HW 
  virtual void _setupLocal() {};

public:
  // Constructors
	SwOSIO(const char *name, SwOSCtrl *ctrl);                 // constructor name, pointer to overlying controller
	SwOSIO(const char *name, uint8_t port, SwOSCtrl *ctrl);   // constructor name, port, pointer to overlying controller

  // Administrative stuff
  virtual void            lock(void);
  virtual void            unlock(void);
  virtual char*           subscribe( char *IOName, uint32_t hysteresis ); // subscribe sensor to display value changes as console outputs 
	virtual void            unsubscribe();                                  // clear subscription
  virtual uint8_t         getPort() { return _port; };
  virtual SwOSCtrl*       getCtrl() { return _ctrl; };
	virtual FtSwarmIOType_t getIOType() { return FTSWARM_UNDEF; };
  virtual char*           getIcon() { return (char *) "UNDEFINED"; };
	virtual void            jsonize( JSONize *json, uint8_t id);
  virtual void            take( void ) { _useCounter++; };                      // register an instance using this IO
  virtual void            give( void ) { if (_useCounter>0) _useCounter--; };   // unregister an instance using this IO
  virtual bool            isInUse( void ) { return _useCounter > 0; };          // test, if an IO is used by some user elements
  
  // Test, if I'm an Actor
  virtual bool            isActor( void ) { return false; };

  // Test, if I', an Sensor
  virtual bool            isSensor( void ) { return false; };

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
    SwOSIO           *_actor;
    boolean          _usePortValue;
    int32_t          _parameter;
  public:
    SwOSEventHandler( );
    SwOSEventHandler( SwOSIO *actor, boolean usePortValue, int32_t parameter );
    void trigger( int32_t portValue );
};

class SwOSEventHandlers {
  protected:
    SwOSEventHandler *_event[FTSWARM_MAXTRIGGER];
  public:
    SwOSEventHandlers( );
    ~SwOSEventHandlers();
    void registerEvent( FtSwarmTrigger_t triggerEvent, SwOSIO *actor, boolean usePortValue, int32_t parameter );
    void unregisterEvent( FtSwarmTrigger_t triggerEvent );
    void trigger( FtSwarmTrigger_t triggerEvent, int32_t portValue );
};

class SwOSEventInput {
  protected:
    SwOSEventHandlers *_events = NULL;
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
    gpio_num_t        _GPIO = GPIO_NUM_NC;
	  FtSwarmSensor_t   _sensorType;
	  int32_t           _lastRawValue = 0;
	
    virtual void _setupLocal();
    virtual void subscription();
    virtual void setSensorTypeLocal( FtSwarmSensor_t sensorType );
	  

  public:
 
	  SwOSInput(const char *name, uint8_t port, SwOSCtrl *ctrl, FtSwarmSensor_t sensorType );
  
    // administrative stuff
	  virtual FtSwarmIOType_t getIOType() { return FTSWARM_INPUT; };
    virtual FtSwarmSensor_t getSensorType() { return _sensorType; };
    virtual char *getIcon();
	  virtual void jsonize( JSONize *json, uint8_t id) {};               // just a placeholder

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

