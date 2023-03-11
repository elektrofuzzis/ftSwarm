/*
 * SwOSHW.h
 *
 * internal implementation of all ftSwarm HW. Use ftSwarm-classes in ftSwarm.h to access your HW!
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include "SwOS.h"
#include <stdint.h>

#include <nvs.h>
#include <driver/ledc.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#ifdef FIRWAMRE_FTSWARM
  #include <FastLED.h>
#endif

#include <LSM6DSRSensor.h>
#include <WiFi.h>

#include "ftSwarm.h"
#include "jsonize.h"
#include "SwOSNVS.h"
#include "SwOSCom.h"

#define MAXSPEED 255
#define BRIGHTNESSDEFAULT 48

class SwOSCtrl; // forward declaration

// state
typedef enum { BOOTING, STARTWIFI, RUNNING, ERROR, WAITING, MAXSTATE } SwOSState_t;

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

  virtual void loadAliasFromNVS(  nvs_handle_t my_handle ); // write my alias to NVS
  virtual void saveAliasToNVS(  nvs_handle_t my_handle );   // load my alias from NVS
  
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
  virtual uint8_t         getPort() { return _port; };
  virtual SwOSCtrl*       getCtrl() { return _ctrl; };
	virtual FtSwarmIOType_t getIOType() { return FTSWARM_UNDEF; };
  virtual char*           getIcon() { return (char *) "UNDEFINED"; };
	virtual void            jsonize( JSONize *json, uint8_t id);

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
  int8_t            _GPIO, _PUA2, _USTX, _ADCChannel, _ADCUnit;
	FtSwarmSensor_t   _sensorType;
	uint32_t          _lastRawValue;
  FtSwarmToggle_t   _toggle;
  bool              _normallyOpen = true;
  esp_adc_cal_characteristics_t *_adc_chars = NULL;
	
	bool isDigitalSensor();
  bool isXMeter();
  virtual void _setupLocal();

public:
 
	SwOSInput(const char *name, uint8_t port, SwOSCtrl *ctrl );
  
  // administrative stuff
	virtual FtSwarmIOType_t getIOType() { return FTSWARM_INPUT; };
  virtual FtSwarmSensor_t getSensorType() { return _sensorType; };
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
	virtual      FtSwarmIOType_t  getIOType()  { return FTSWARM_ACTOR; };
  virtual      FtSwarmActor_t getActorType() { return _actorType; };
  virtual char *getIcon();
	virtual void jsonize( JSONize *json, uint8_t id); // serialize object to JSON
  virtual void onTrigger( int32_t value );

  // commands
  virtual void            setActorType( FtSwarmActor_t actorType );    // set actor type
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

class SwOSJoystick : public SwOSIO, SwOSEventInput {
protected:
	adc1_channel_t _ADCChannelLR, _ADCChannelFB;
	int16_t        _lastLR, _lastFB;
  int16_t        _zeroLR, _zeroFB;
  int16_t        _lastRawLR, _lastRawFB;

  // local HW procedures
  virtual void _setupLocal(); // initializes local HW
  
public:
  SwOSEventInput triggerLR, triggerFB;
  
  // constructors
	SwOSJoystick(const char *name, uint8_t port, SwOSCtrl *ctrl, int16_t zeroLR, int16_t zeroFB );

  // administrative stuff
	virtual FtSwarmIOType_t getIOType() { return FTSWARM_JOYSTICK; };
  virtual char *getIcon() { return (char *) "11_joystick.svg"; };
	virtual void jsonize( JSONize *json, uint8_t id);
  
  // read
	virtual void read();

  // commands
  virtual void getValue( int16_t* FB, int16_t* LR ) { *FB = _lastFB; *LR = _lastLR; };
  virtual void setValue( int16_t  FB, int16_t  lastLR );
  virtual void calibrate( int16_t *zeroLR, int16_t *zeroFB );  // uses actual readings to calibrate
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
  virtual void onTrigger( int32_t value );
    
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
	  int16_t _offset   = 128;
    
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
    virtual void onTrigger( int32_t value );
    
    // commands
	  virtual int16_t getOffset( )   { return _offset; };
	  virtual int16_t getPosition( ) { return _position; };
	  virtual void setOffset( int16_t offset );
	  virtual void setPosition( int16_t position );
};

/***************************************************
 *
 *   SwOSGyro
 *
 ***************************************************/

class SwOSGyro : public SwOSIO {
  private:
    LSM6DSRSensor *_gyro = NULL;
    int32_t       _accelerometer[3];
    int32_t       _gyroscope[3];
    
    // local HW procedures
    virtual void _setupLocal(); // initializes local HW
    
  public:
    // constructor
	  SwOSGyro(const char *name, SwOSCtrl *ctrl);

    // administrative stuff
	  virtual FtSwarmIOType_t getIOType() { return FTSWARM_GYRO; };
    virtual char *getIcon() { return (char *) "25_gyro.svg"; };
    virtual void jsonize( JSONize *json, uint8_t id);

    // read sensor
    virtual void     read();

};

/***************************************************
 *
 *   SwOSButton
 *
 ***************************************************/

class SwOSButton : public SwOSIO, public SwOSEventInput {
	bool              _lastState;
  FtSwarmToggle_t   _toggle;
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
  virtual void setState( bool state, bool clearToggle = false );
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
 *   SwOSOLED
 *
 ***************************************************/

class SwOSOLED : public SwOSIO {
  protected:

    Adafruit_SSD1306 *_display = NULL;
    
    uint8_t _textSizeX = 0;
    uint8_t _textSizeY = 0;
    uint8_t _displayType = 0;
    
    // local HW procedures
    virtual void _setupLocal(); // initializes local HW
    
  public:
    // constructor
    SwOSOLED(const char *name, SwOSCtrl *ctrl, uint8_t displayType);

    // administrative stuff
    virtual FtSwarmIOType_t getIOType() { return FTSWARM_OLED; };

      void display(void);
      void invertDisplay(bool i);
      void fillScreen( bool white);    
      void dim(bool dim);
      void setContrast(uint8_t contrast = 0x8F );
      int16_t getWidth(void);
      int16_t getHeight(void);
    
      void drawPixel(int16_t x, int16_t y, bool white);  
      void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool white);
      void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, bool fill, bool white);
      void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, bool fill, bool white);
      void drawCircle(int16_t x0, int16_t y0, int16_t r, bool fill, bool white);
      void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool fill, bool white);
      void drawChar(int16_t x, int16_t y, unsigned char c, bool color, bool bg, uint8_t size_x, uint8_t size_y);
      void write( char *str, int16_t x, int16_t y, FtSwarmAlign_t align , bool fill );
      void write( char *str );
   
      void setCursor(int16_t x, int16_t y);
      void getCursor(int16_t *x, int16_t *y);

      void setTextColor(bool c, bool bg);
      void setTextWrap(bool w);

      void setRotation(uint8_t r);
      uint8_t getRotation(void);

      void setTextSize(uint8_t sx, uint8_t sy);
      void getTextSize( uint8_t *sx, uint8_t *sy );
    
      void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);

    
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
	FtSwarmSerialNumber_t serialNumber;
  uint8_t mac[ESP_NOW_ETH_ALEN] = {0,0,0,0,0,0};
  
  // common hardware
	SwOSInput    *input[MAXINPUTS];
	SwOSActor    *actor[MAXACTORS]; 
	SwOSGyro     *gyro;
  uint8_t      inputs, actors;
	
  // constructor, destructor
  SwOSCtrl( FtSwarmSerialNumber_t SN, const uint8_t *macAddress, bool local, FtSwarmVersion_t CPU, FtSwarmVersion_t HAT, bool IAmAKelda );
  ~SwOSCtrl();

  // administrative stuff
  bool AmIAKelda() { return _IAmAKelda; };
	virtual bool cmdAlias( const char *obj, const char *alias);            // set an alias for this board or IO device
	virtual bool cmdAlias( char *device, uint8_t port, const char *alias); // set an alias for a IO device
  virtual SwOSIO *getIO( FtSwarmIOType_t ioType, FtSwarmPort_t port);    // get a pointer to an IO port via address
	virtual SwOSIO *getIO( const char *name);                              // get a pointer to an IO port via name or alias
  virtual FtSwarmControler_t getType();                                  // what I am?
	virtual char*              myType();                                   // what I am?
  virtual FtSwarmVersion_t   getCPU() { return _CPU; };                  // my CPU type
	virtual const char *       getVersionCPU();                            // my CPU type as string
  virtual FtSwarmVersion_t   getHAT() { return _HAT; };                  // my HAT type
	virtual const char *       getVersionHAT();                            // my CPU type as string
  virtual bool               isLocal() { return _local; };               // local or remote?
	virtual char *             getHostname( );                             // hostname
	virtual void               jsonize( JSONize *json, uint8_t id);        // send board & IO device information as a json string
  virtual void loadAliasFromNVS(  nvs_handle_t my_handle );              // write my alias to NVS
  virtual void saveAliasToNVS(  nvs_handle_t my_handle );                // load my alias from NVS
  virtual void setState( SwOSState_t state, uint8_t members, char *SSID ); // visualizes controler's state like booting, error,...
  virtual void factorySettings( void );                                  // reset factory settings
    
  virtual void read(); // run measurements

  // API commnds
	virtual bool apiActorCmd( char *id, int cmd );                 // send an actor's command (from api)
  virtual bool apiActorPower( char *id, int power );             // send an actors's power (from api)
	virtual bool apiLEDBrightness( char *id, int brightness );     // send a LED command (from api)
	virtual bool apiLEDColor( char *id, int color );               // send a LED command (from api)
  virtual bool apiServoOffset( char *id, int offset );           // send a Servo command (from api)
  virtual bool apiServoPosition( char *id, int position );       // send a Servo command (from api)

  virtual bool maintenanceMode();


  // Communications
  virtual bool OnDataRecv( SwOSCom *com );     // data via espnow revceived
  virtual bool recvState( SwOSCom *com );      // receive state from another ftSwarmXX
  virtual SwOSCom *state2Com( void );          // copy my state in a com struct
  virtual void registerMe( SwOSCom *com );     // fill in my own data in registerCmd datagram
  virtual void sendAlias( uint8_t *mac, FtSwarmSerialNumber_t destinationSN ) {};   // send my alias names
    
};

/***************************************************
 *
 *   SwOSSwarmJST
 *
 ***************************************************/

class SwOSSwarmJST : public SwOSCtrl {
public:
  // specific hardware
  uint8_t   RGBLeds = 2;
	SwOSLED   *led[MAXLED];
	SwOSServo *servo;

  // constructor, destructor
	SwOSSwarmJST( FtSwarmSerialNumber_t SN, const uint8_t *macAddress, bool local, FtSwarmVersion_t CPU, FtSwarmVersion_t HAT, bool IAmAKelda, uint8_t xRGBLeds );
  SwOSSwarmJST( SwOSCom *com ); // constructor
  ~SwOSSwarmJST();
  
  // administrative stuff
	virtual bool cmdAlias( char *device, uint8_t port, const char *alias); // set an alias for a IO device
  virtual SwOSIO *getIO( FtSwarmIOType_t ioType,  FtSwarmPort_t port);   // get an pointer to the requested IO Device via type&port
	virtual SwOSIO *getIO( const char *name);                              // get an pointer to the requested IO Device via name or alias
	virtual FtSwarmControler_t getType();                                  // what I am?
	virtual char* myType();                                                // what I am?
	virtual void jsonize( JSONize *json, uint8_t id);                      // send board & IO device information as a json string
  virtual void loadAliasFromNVS(  nvs_handle_t my_handle );              // write my alias to NVS
  virtual void saveAliasToNVS(  nvs_handle_t my_handle );                // load my alias from NVS
  virtual void setState( SwOSState_t state, uint8_t members, char *SSID ); // visualizes controler's state like booting, error,...
  virtual void factorySettings( void );                                  // reset factory settings

  // API commands
  virtual bool apiLEDBrightness( char *id, int brightness );     // send a LED command (from api)
  virtual bool apiLEDColor( char *id, int color );               // send a LED command (from api)
  virtual bool apiServoOffset( char *id, int offset );           // send a Servo command (from api)
  virtual bool apiServoPosition( char *id, int position );       // send a Servo command (from api)

  // **** Communications *****
  virtual bool OnDataRecv( SwOSCom *com ); // data via espnow revceived
  virtual void registerMe( SwOSCom *com );     // fill in my own data in registerCmd datagram
  virtual void sendAlias( uint8_t *mac, FtSwarmSerialNumber_t destinationSN );

};

/***************************************************
 *
 *   SwOSSwarmControl
 *
 ***************************************************/

class SwOSSwarmControl : public SwOSCtrl {
protected:
  boolean _remoteControl = false;
  boolean _firstRead     = true;
public:
  // specific hardware
	SwOSButton   *button[8];
	SwOSHC165    *hc165;
	SwOSJoystick *joystick[2];
	SwOSOLED     *oled;

	SwOSSwarmControl(FtSwarmSerialNumber_t SN, const uint8_t *macAddress, bool local, FtSwarmVersion_t CPU, FtSwarmVersion_t HAT, bool IAmAKelda, int16_t zero[2][2], uint8_t displayType ); // constructor
  SwOSSwarmControl( SwOSCom *com ); // constructor
  ~SwOSSwarmControl(); // destructor
  
  // administrative stuff
	virtual bool cmdAlias( char *device, uint8_t port, const char *alias); // set an alias for a IO device
  virtual SwOSIO *getIO( FtSwarmIOType_t ioType,  FtSwarmPort_t port);   // get a pointer to the requested IO Device via type & port
	virtual SwOSIO *getIO( const char *name);                              // get a pointer to the requested IO Device via name or alias
	virtual char* myType();                                                // what I am?
	virtual FtSwarmControler_t getType();                                  // what I am?
	virtual void jsonize( JSONize *json, uint8_t id);                      // send board & IO device information as a json string
  virtual void loadAliasFromNVS(  nvs_handle_t my_handle );              // write my alias to NVS
  virtual void saveAliasToNVS(  nvs_handle_t my_handle );                // load my alias from NVS
  virtual void setState( SwOSState_t state, uint8_t members, char *SSID ); // visualizes controler's state like booting, error,...
  virtual void factorySettings( void );                                  // reset factory settings
  virtual boolean getRemoteControl( void );                              // get remote control setting
  virtual void setRemoteControl( boolean remoteControl );                // set remote control setting

	virtual void read();                       // run measurements

  // **** Communications *****
  virtual bool recvState( SwOSCom *com );    // receive state from another FtSwarmControl
  virtual SwOSCom *state2Com( void );        // copy my state in a com struct
  virtual bool OnDataRecv( SwOSCom *com );   // data via espnow revceived
  virtual void registerMe( SwOSCom *com );   // fill in my own data in registerCmd datagram
  virtual void sendAlias( uint8_t *mac, FtSwarmSerialNumber_t destinationSN );    // send my alias names

};

/***************************************************
 *
 *   SwOSSwarmCAM
 *
 ***************************************************/

class SwOSSwarmCAM : public SwOSCtrl {
protected:
  boolean _remoteControl = false;
public:
  // specific hardware

  SwOSSwarmCAM(FtSwarmSerialNumber_t SN, const uint8_t *macAddress, bool local, FtSwarmVersion_t CPU, FtSwarmVersion_t HAT, bool IAmAKelda ); // constructor
  SwOSSwarmCAM( SwOSCom *com ); // constructor
  ~SwOSSwarmCAM(); // destructor
  
  // administrative stuff
  virtual bool cmdAlias( char *device, uint8_t port, const char *alias); // set an alias for a IO device
  virtual SwOSIO *getIO( FtSwarmIOType_t ioType,  FtSwarmPort_t port);   // get a pointer to the requested IO Device via type & port
  virtual SwOSIO *getIO( const char *name);                              // get a pointer to the requested IO Device via name or alias
  virtual char* myType();                                                // what I am?
  virtual FtSwarmControler_t getType();                                  // what I am?
  virtual void jsonize( JSONize *json, uint8_t id);                      // send board & IO device information as a json string
  virtual void loadAliasFromNVS(  nvs_handle_t my_handle );              // write my alias to NVS
  virtual void saveAliasToNVS(  nvs_handle_t my_handle );                // load my alias from NVS
  virtual void setState( SwOSState_t state, uint8_t members, char *SSID ); // visualizes controler's state like booting, error,...
  virtual void factorySettings( void );                                  // reset factory settings
  virtual boolean getRemoteControl( void );                              // get remote control setting
  virtual void setRemoteControl( boolean remoteControl );                // set remote control setting

  virtual void read();                       // run measurements

  // **** Communications *****
  virtual bool recvState( SwOSCom *com );    // receive state from another FtSwarmControl
  virtual SwOSCom *state2Com( void );        // copy my state in a com struct
  virtual bool OnDataRecv( SwOSCom *com );   // data via espnow revceived
  virtual void registerMe( SwOSCom *com );   // fill in my own data in registerCmd datagram
  virtual void sendAlias( uint8_t *mac, FtSwarmSerialNumber_t destinationSN );    // send my alias names

};
