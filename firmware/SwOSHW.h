#pragma once

#include <stdint.h>

#include <driver/ledc.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

#include <FastLED.h>
#include <WiFi.h>

#include "ftSwarm.h"
#include "jsonize.h"
#include "SwOSNVS.h"
#include "SwOSCom.h"

#define MAXSPEED 255
#define MAXPIXEL 16
#define BRIGHTNESSDEFAULT 48

#define BOOTING   CRGB::Blue
#define STARTWIFI CRGB::Yellow
#define RUNNING   CRGB::Green
#define ERROR     CRGB::Red

class SwOSCtrl; // forward declaration

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
  
	void setName( const char *name);   // set new name
  char *getName();                   // get name
  
	void setAlias( const char *alias); // add an alias name
  char *getAlias();                  // get alias

	bool equals(const char *name);     // check if hw name or alias is equal to name

	virtual void jsonize( JSONize *, uint8_t id);
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

  // local HW 
  virtual void _setupLocal() {};

public:
  // Constructors
	SwOSIO(const char *name, SwOSCtrl *ctrl);                 // constructor name, pointer to overlying controler
	SwOSIO(const char *name, uint8_t port, SwOSCtrl *ctrl);   // constructor name, port, pointer to overlying controler

  // Administrative stuff
	virtual FtSwarmIOType_t getIOType() { return FTSWARM_UNDEF; };
  virtual char*           getIcon() { return (char *) "UNDEFINED"; };
	virtual void            jsonize( JSONize *json, uint8_t id);

};

/***************************************************
 *
 *   SwOSInput
 *
 ***************************************************/

class SwOSInput : public SwOSIO {
protected:
	gpio_num_t      _GPIO, _PUA2, _USTX;
	adc1_channel_t  _ADCChannel;
	FtSwarmSensor_t _sensorType;
	uint32_t        _lastRawValue;
  FtSwarmToggle_t _toggle;
  bool            _normallyOpen = true;
  esp_adc_cal_characteristics_t *_adc_chars = NULL;
	
	bool isDigitalSensor();
  bool isXMeter();
  virtual void _setupLocal();

public:
 
	SwOSInput(const char *name, uint8_t port, SwOSCtrl *ctrl );

  // administrative stuff
	virtual FtSwarmIOType_t getIOType() { return FTSWARM_INPUT; };
  virtual char *getIcon();
	virtual void jsonize( JSONize *json, uint8_t id);

  // read sensor
	virtual void     read();

  // external commands
	virtual void            setSensorType( FtSwarmSensor_t sensorType, bool normallyOpen = true );  // set sensor type
	virtual uint32_t        getValueUI32();                               // get raw reading
	virtual float           getValueF();                                  // get float reading
  virtual float           getVoltage();                                 // get voltage reading
  virtual float           getResistance();                              // get resistance reading
  virtual float           getKelvin();                                  // get temerature reading
  virtual float           getCelcius();                                 // get temerature reading
  virtual float           getFahrenheit();                              // get temerature reading
  virtual void            setValue( uint32_t value );                   // set value by an external call
  virtual FtSwarmToggle_t getToggle();                                  // check, on toggling signals
};

/***************************************************
 *
 *   SwOSActor
 *
 ***************************************************/

class SwOSActor : public SwOSIO {
protected:
	gpio_num_t      _IN1, _IN2;
	ledc_channel_t  _channelIN1, _channelIN2;
  FtSwarmActor_t  _actorType;
	FtSwarmMotion_t _motionType = FTSWARM_COAST;
	int16_t         _power = 0;

  // local HW procedures
  virtual void _setupLocal(); // initializes local HW
  virtual void _setLocal();   // start moving locally

  // remote HW procedures
  virtual void _setRemote();  // start moving remotely 

public:
  // Constructors
	SwOSActor(const char *name, uint8_t port, SwOSCtrl *ctrl);

  // adminstrative stuff
	virtual      FtSwarmIOType_t getIOType() { return FTSWARM_ACTOR; };
  virtual char *getIcon();
	virtual void jsonize( JSONize *json, uint8_t id); // serialize object to JSON

  // commands
  virtual void            setActorType( FtSwarmActor_t actorType ) { _actorType = actorType; };    // set actor type
  virtual void            setValue( FtSwarmMotion_t motionType, int16_t power ) { _motionType = motionType; _power = power; };  // set values
  virtual void            setPower( int16_t power );                   // set power
	virtual int16_t         getPower() { return _power; };               // get power
  virtual void            setMotionType( FtSwarmMotion_t motionType ); // set motion type
	virtual FtSwarmMotion_t getMotionType() { return _motionType; };     // get motion type
};

/***************************************************
 *
 *   SwOSJoystick
 *
 ***************************************************/

class SwOSJoystick : public SwOSIO {
protected:
	adc1_channel_t _ADCChannelLR, _ADCChannelFB;
	int16_t        _lastLR, _lastFB;

  // local HW procedures
  virtual void _setupLocal(); // initializes local HW
  
public:
  // constructors
	SwOSJoystick(const char *name, uint8_t port, SwOSCtrl *ctrl);

  // administrative stuff
	virtual FtSwarmIOType_t getIOType() { return FTSWARM_JOYSTICK; };
  virtual char *getIcon() { return (char *) "11_joystick.svg"; };
	virtual void jsonize( JSONize *json, uint8_t id);

  // read
	virtual void read();

  // commands
  virtual void getValue( int16_t* FB, int16_t* LR ) { *FB = _lastFB; *LR = _lastLR; };
  virtual void setValue( int16_t  FB, int16_t  lastLR );
};

/***************************************************
 *
 *   SwOSLED
 *
 ***************************************************/

class SwOSLED : public SwOSIO {
protected:
	uint32_t _color = 0;
	uint8_t  _brightness = BRIGHTNESSDEFAULT;

  // local HW procedures
  virtual void _setupLocal(); 
  virtual void _setColorLocal();
  virtual void _setBrightnessLocal();

  // remote HW procedures
  virtual void _setRemote();
  
public:
  // constructor
	SwOSLED(const char *name, uint8_t port, SwOSCtrl *ctrl);

  // administrative stuff
	virtual FtSwarmIOType_t getIOType() { return FTSWARM_LED; };
  virtual char *getIcon()   { return (char *) "15_rgbled.svg"; };
  virtual void jsonize( JSONize *json, uint8_t id);
  
  // commands
	virtual uint32_t getColor()      { return _color; };
	virtual uint8_t  getBrightness() { return _brightness; };
	virtual void     setColor(uint32_t color);
	virtual void     setBrightness(uint8_t brightness);
	virtual void     setValue( uint8_t brightness, uint32_t color ) { _brightness = brightness; _color = color; };
};

/***************************************************
 *
 *   SwOSSERVO
 *
 ***************************************************/

class SwOSServo : public SwOSIO {
  protected:
    gpio_num_t      _SERVO;
	  ledc_channel_t  _channelSERVO;
	  int16_t _position = 0;
	  int16_t _offset   = 0;
    
    // local HW procedures
    virtual void _setupLocal(); // initializes local HW
    virtual void _setLocal();   // set position locally

    // remote HW procedures
    virtual void _setRemote();  // setPosition remotely 
  
  public:
    // constructor
	  SwOSServo(const char *name, SwOSCtrl *ctrl);

    // administrative stuff
	  virtual FtSwarmIOType_t getIOType() { return FTSWARM_SERVO; };
    virtual char *    getIcon() { return (char *) "14_servo.svg"; };
    virtual void jsonize( JSONize *json, uint8_t id);

    // commands
	  virtual int16_t getOffset( )   { return _offset; };
	  virtual int16_t getPosition( ) { return _position; };
	  virtual void setOffset( int16_t offset );
	  virtual void setPosition( int16_t position );
};

/***************************************************
 *
 *   SwOSOLED
 *
 ***************************************************/

class SwOSOLED : public SwOSIO {
public:
  // constructor
	SwOSOLED(const char *name, SwOSCtrl *ctrl);

  // administrative stuff
	virtual FtSwarmIOType_t getIOType() { return FTSWARM_OLED; };
};

/***************************************************
 *
 *   SwOSGyro
 *
 ***************************************************/

class SwOSGyro : public SwOSIO {
public:
  // constructor
	SwOSGyro(const char *name, SwOSCtrl *ctrl);

  // administrative stuff
	virtual FtSwarmIOType_t getIOType() { return FTSWARM_GYRO; };

};

/***************************************************
 *
 *   SwOSButton
 *
 ***************************************************/

class SwOSButton : public SwOSIO {
	bool            _lastState;
  FtSwarmToggle_t _toggle;
public:
  // constructor
	SwOSButton(const char *name, uint8_t port, SwOSCtrl *ctrl);
	
  // administrative stuff
  virtual FtSwarmIOType_t getIOType() { return FTSWARM_BUTTON; };
  virtual char *    getIcon()   { return (char *) "12_button.svg"; };
	virtual void jsonize( JSONize *json, uint8_t id);

  // commands
	virtual FtSwarmToggle_t getToggle();
	virtual bool getState();
  virtual void setState( bool state );
};

/***************************************************
 *
 *   SwOSHC165
 *
 ***************************************************/

class SwOSHC165 : public SwOSIO {
protected:
	gpio_num_t _LD, _CS, _CLK, _MISO;
	uint8_t    _lastValue;

  // local HW procedures
  virtual void _setupLocal();

public:

  // constructor
	SwOSHC165(const char *name, SwOSCtrl *ctrl);

  // administrative stuff
  virtual FtSwarmIOType_t getIOType() { return FTSWARM_HC165; };

	virtual void read();

  // commands
  virtual void    setValue( uint8_t value ) { _lastValue = value; };
  virtual uint8_t getValue( uint8_t bit )   { return _lastValue && 1<<bit; };
  virtual uint8_t getValue()                { return _lastValue; };

};

/***************************************************
 *
 *   SwOSCtrl - Base class for controllers.
 *
 ***************************************************/

class SwOSCtrl : public SwOSObj {
protected:
	FtSwarmVersion_t _CPU, _HAT;
  bool             _IAmAKelda;
  bool             _local;
	const char *     version( FtSwarmVersion_t v);
public:
  IPAddress IP;
	uint16_t  serialNumber;
  uint8_t   mac[ESP_NOW_ETH_ALEN] = {0,0,0,0,0,0};
  
  // common hardware
	SwOSInput    *input[4];
	SwOSActor    *actor[2]; 
	SwOSGyro     *gyro;
	
  // constructor, destructor
  SwOSCtrl( uint16_t SN, const uint8_t *macAddress, bool local, FtSwarmVersion_t CPU, FtSwarmVersion_t HAT, bool IAmAKelda );
  ~SwOSCtrl();

  // administrative stuff
  bool AmIAKelda() { return _IAmAKelda; };
	virtual bool cmdAlias( const char *obj, const char *alias);            // set an alias for this board or IO device
	virtual bool cmdAlias( char *device, uint8_t port, const char *alias); // set an alias for a IO device
  virtual SwOSIO *getIO( FtSwarmIOType_t ioType, FtSwarmPort_t port);   // get a pointer to an IO port via address
	virtual SwOSIO *getIO( char *name);                                    // get a pointer to an IO port via name or alias
  virtual FtSwarmControler_t getType();                                  // what I am?
	virtual char*              myType();                                   // what I am?
  virtual FtSwarmVersion_t   getCPU() { return _CPU; };                  // my CPU type
	virtual const char *       getVersionCPU();                            // my CPU type as string
  virtual FtSwarmVersion_t   getHAT() { return _HAT; };                  // my HAT type
	virtual const char *       getVersionHAT();                            // my CPU type as string
  virtual bool               isLocal() { return _local; };               // local or remote?
	virtual char *             getHostname( );                             // hostname
	virtual void               jsonize( JSONize *json, uint8_t id);        // send board & IO device information as a json string

  virtual void read(); // run measurements

  // API commnds
	virtual bool apiActorCmd( char *id, int cmd );                 // send an actor's command (from api)
  virtual bool apiActorPower( char *id, int power );             // send an actors's power (from api)
	virtual bool apiLED( char *id, int brightness, int color );    // send a LED command (from api)
	virtual bool apiServo( char *id, int offset, int position );   // send a Servo command (from api)

  virtual bool maintenanceMode();

  virtual void setState( int state );
    // visualizes controler's state

  // Communications
  virtual bool OnDataRecv( SwOSCom *com );  // data via espnow revceived
  virtual SwOSCom *state2Com( void );    // copy my state in a com struct

  // setup
  virtual void setup( void );
    
};

/***************************************************
 *
 *   SwOSSwarmJST
 *
 ***************************************************/

#define MAXLED 16

class SwOSSwarmJST : public SwOSCtrl {
public:
  // specific hardware
	SwOSLED   *led[MAXLED];
	SwOSServo *servo;

  // constructor, destructor
	SwOSSwarmJST( uint16_t SN, const uint8_t *macAddress, bool local, FtSwarmVersion_t CPU, FtSwarmVersion_t HAT, bool IAmAKelda );
  ~SwOSSwarmJST();
  
  // administrative stuff
	virtual bool cmdAlias( char *device, uint8_t port, const char *alias); // set an alias for a IO device
  virtual SwOSIO *getIO( FtSwarmIOType_t ioType,  FtSwarmPort_t port);   // get an pointer to the requested IO Device via type&port
	virtual SwOSIO *getIO( char *name);                                    // get an pointer to the requested IO Device via name or alias
	virtual FtSwarmControler_t getType();                                  // what I am?
	virtual char* myType();                                                // what I am?
	virtual void jsonize( JSONize *json, uint8_t id);                      // send board & IO device information as a json string

  // API commands
	virtual bool apiLED( char *id, int brightness, int color );    // send a LED command (from api)
	virtual bool apiServo( char *id, int offset, int position );   // send a Servo command (from api)

  virtual void setState( int state );      // visualizes controler's state

  // **** Communications *****
  virtual bool recvState( SwOSCom *com );  // receive state from another ftSwarmControl
  virtual SwOSCom *state2Com( void  );     // copy my state in a com struct
  virtual bool OnDataRecv( SwOSCom *com ); // data via espnow revceived

};

/***************************************************
 *
 *   SwOSSwarmControl
 *
 ***************************************************/

class SwOSSwarmControl : public SwOSCtrl {
public:
  // specific hardware
	SwOSButton   *button[8];
	SwOSHC165    *hc165;
	SwOSJoystick *joystick[2];
	SwOSOLED     *oled;

	SwOSSwarmControl(uint16_t SN, const uint8_t *macAddress, bool local, FtSwarmVersion_t CPU, FtSwarmVersion_t HAT, bool IAmAKelda ); // constructor
  ~SwOSSwarmControl(); // destructor
  
  // administrative stuff
	virtual bool cmdAlias( char *device, uint8_t port, const char *alias); // set an alias for a IO device
  virtual SwOSIO *getIO( FtSwarmIOType_t ioType,  FtSwarmPort_t port);   // get a pointer to the requested IO Device via type & port
	virtual SwOSIO *getIO( char *name);                                    // get a pointer to the requested IO Device via name or alias
	virtual char* myType();                                                // what I am?
	virtual FtSwarmControler_t getType();                                  // what I am?
	virtual void jsonize( JSONize *json, uint8_t id);                      // send board & IO device information as a json string

	virtual void read();                       // run measurements

  virtual void setState( int state );        // visualizes controler's state

  // **** Communications *****
  virtual bool recvState( SwOSCom *com );    // receive state from another FtSwarmControl
  virtual SwOSCom *state2Com( void );        // copy my state in a com struct
  virtual bool OnDataRecv( SwOSCom *com );   // data via espnow revceived

};
