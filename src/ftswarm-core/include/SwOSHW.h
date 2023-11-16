/*
 * SwOSHW.h
 *
 * internal implementation of all ftSwarm HW. Use ftSwarm-classes in ftSwarm.h to access your HW!
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include <stdint.h>

#include <nvs.h>
#include <driver/ledc.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

#include <WiFi.h>

#include "SwOS.h"
#include "jsonize.h"
#include "SwOSNVS.h"
#include "SwOSCom.h"

#define BRIGHTNESSDEFAULT 48

class SwOSCtrl; // forward declaration

// state
typedef enum { BOOTING, STARTWIFI, RUNNING, ERROR, WAITING, IDENTIFY, MAXSTATE } SwOSState_t;

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
	SwOSIO(const char *name, SwOSCtrl *ctrl);                 // constructor name, pointer to overlying controler
	SwOSIO(const char *name, uint8_t port, SwOSCtrl *ctrl);   // constructor name, port, pointer to overlying controler

  // Administrative stuff
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
	  uint32_t          _lastRawValue = 0;
	
    virtual void _setupLocal();
    virtual void subscription();

  public:
 
	  SwOSInput(const char *name, uint8_t port, SwOSCtrl *ctrl, FtSwarmSensor_t sensorType );
  
    // administrative stuff
	  virtual FtSwarmIOType_t getIOType() { return FTSWARM_INPUT; };
    virtual FtSwarmSensor_t getSensorType() { return _sensorType; };
    virtual char *getIcon();
	  virtual void jsonize( JSONize *json, uint8_t id) {};               // just a placeholder

    // read sensor
	  virtual void     read() {};

    // external commands
    virtual void            setSensorType( FtSwarmSensor_t sensorType );  // set sensor type
	  virtual uint32_t        getValueUI32( void );                         // get raw reading
	  virtual float           getValueF( void );                            // get float reading
    virtual void            setValue( uint32_t value ) {};                // set value by an external call
  
};

/***************************************************
 *
 *   SwOSDigitalInput
 *
 ***************************************************/

class SwOSDigitalInput : public SwOSInput {

  protected:
    gpio_num_t      _PUA2   = GPIO_NUM_NC;
    gpio_num_t      _USTX   = GPIO_NUM_NC;
    FtSwarmToggle_t _toggle = FTSWARM_NOTOGGLE;
    bool            _normallyOpen = true;

    virtual void _setupLocal();
    virtual void _setSensorTypeLHW( FtSwarmSensor_t sensorType, bool normallyOpen, bool dontSendToRemote );  // set sensor type

  public:
 
	  SwOSDigitalInput(const char *name, uint8_t port, SwOSCtrl *ctrl );
  
    // administrative stuff
	  virtual FtSwarmIOType_t getIOType() { return FTSWARM_DIGITALINPUT; };
    virtual void jsonize( JSONize *json, uint8_t id);

    // read sensor
	  virtual void     read();

    // external commands
    virtual void            setSensorType( FtSwarmSensor_t sensorType, bool normallyOpen, bool dontSendToRemote );  // set sensor type
    virtual void            setValue( uint32_t value );                   // set value by an external call
    virtual FtSwarmToggle_t getToggle( void );                                  // check, on toggling signals

};

/***************************************************
 *
 *   SwOSAnalogInput
 *
 ***************************************************/

class SwOSAnalogInput : public SwOSInput {

  protected:
    int8_t            _ADCChannel = ADC1_CHANNEL_MAX;
    int8_t            _ADCUnit    = GPIO_NUM_NC;
    esp_adc_cal_characteristics_t *_adc_chars = NULL;
	
    bool isXMeter();
    virtual void _setupLocal();
    virtual void _setSensorTypeLHW( FtSwarmSensor_t sensorType, bool dontSendToRemote );  // set sensor type

  public:
 
	  SwOSAnalogInput(const char *name, uint8_t port, SwOSCtrl *ctrl );
  
    // administrative stuff
	  virtual FtSwarmIOType_t getIOType() { return FTSWARM_ANALOGINPUT; };
    virtual void jsonize( JSONize *json, uint8_t id);

    // read sensor
	  virtual void     read();

    // external commands
    virtual void   setSensorType( FtSwarmSensor_t sensorType, bool dontSendToRemote );  // set sensor type
    virtual void   setValue( uint32_t value );                   // set value by an external call
    virtual float  getVoltage();
    virtual float  getResistance();
    virtual float  getKelvin();
    virtual float  getCelcius();
    virtual float  getFahrenheit();

};

/***************************************************
 *
 *   SwOSActor
 *
 ***************************************************/

class SwOSActor : public SwOSIO {
protected:

  // DC motors
	gpio_num_t      _IN1, _IN2;
	ledc_channel_t  _channelIN1, _channelIN2;
  uint32_t        _rampUpT, _rampUpY;

  // stepper motors
  long            _distance;
  long            _position;
  uint8_t         _pwrDriveMotor;
  bool            _isHoming;
  bool            _isRunning;

  // generics
  FtSwarmActor_t  _actorType;
	FtSwarmMotion_t _motionType = FTSWARM_COAST;
	int16_t         _speed = 0;

  // local HW procedures
  virtual void _setupI2C(); // initializes local HW
  virtual void _setupLocal(); // initializes local HW
  virtual void _setLocalI2C();   // start moving locally
  virtual void _setLocalLHW();   // start moving locally

  // remote HW procedures
  virtual void _setRemote();  // start moving remotely 

public:
  // Constructors
	SwOSActor(const char *name, uint8_t port, SwOSCtrl *ctrl);

  // adminstrative stuff
	virtual FtSwarmIOType_t getIOType()  { return FTSWARM_ACTOR; };
  virtual FtSwarmActor_t  getActorType() { return _actorType; };
  virtual char *          getIcon();
	virtual void            jsonize( JSONize *json, uint8_t id); // serialize object to JSON
  virtual void            onTrigger( int32_t value );
  virtual void            read( void );

  // commands
  virtual void            setActorType( FtSwarmActor_t actorType, bool dontSendToRemote );    // set actor type
  virtual void            setValue( FtSwarmMotion_t motionType, int16_t speed ) { _motionType = motionType; _speed = speed; };  // set values
	virtual void            setSpeed( int16_t speed );                                 // set speed
  virtual int16_t         getSpeed() { return _speed; };                             // get speed
  virtual void            setAcceleration( uint32_t rampUpT,  uint32_t rampUpY );    // set acceleration ramp
  virtual void            getAcceleration( uint32_t *rampUpT, uint32_t *rampUpY );   // get acceleration ramp
  virtual void            setMotionType( FtSwarmMotion_t motionType );               // set motion type
	virtual FtSwarmMotion_t getMotionType() { return _motionType; };                   // get motion type

  // steppers only
  virtual void            setValue( long distance, long position, bool isHoming, bool isRunning );
  virtual void            setDistance( long distance, bool relative, bool dontSendToRemote  );  // set a distance to go
  virtual long            getDistance( void );                          // get distance
  virtual void            startStop( bool start );                      // start/stop motor
  virtual void            setPosition( long position, bool dontSendToRemote );                 // set absolute motor position
  virtual long            getPosition( void );                          // get motor position
  virtual void            homing( long maxDistance );                   // start homing
  virtual void            setHomingOffset( long offset );               // set homing offset
  virtual bool            isHoming( void );                             // check if motor is in homing procedure
  virtual bool            isRunning( void );                            // check if motor is running
  virtual void            setIsHoming( bool isHoming );                 // used by controller during read() to set local info
  virtual void            setIsRunning( bool isRunning );               // used by controller during read() to set local info
  
  /*virtual void setAbsDistance(long distance );            // set a absolute distance to go
  virtual long getStepsToGo( void );                        // number of needed steps to go to distance
  virtual void setMaxSpeed( long speed );                   // set a max speed
  virtual long getMaxSpeed(void );                          // get max speed
  virtual void startMoving( boolean disableOnStop = true ); // start motor moving, disableOnStop disables the motor driver at the end of the movement
  virtual void stopMoving( void );                          // stop motor moving immediately
  virtual boolean isMoving( void );                         // check, if a motor is moving
  */

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
	int16_t        _lastSubscribedLR, _lastSubscribedFB;
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
  virtual void subscription();
	virtual void read();

  // commands
  virtual void getValue( int16_t* FB, int16_t* LR ) { *FB = _lastFB; *LR = _lastLR; };
  virtual void setValue( int16_t  FB, int16_t  lastLR );
  virtual void calibrate( int16_t *zeroLR, int16_t *zeroFB );  // uses actual readings to calibrate
};

/***************************************************
 *
 *   SwOSPixel
 *
 ***************************************************/

class SwOSPixel : public SwOSIO {
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
	SwOSPixel(const char *name, uint8_t port, SwOSCtrl *ctrl);

  // administrative stuff
	virtual FtSwarmIOType_t getIOType() { return FTSWARM_PIXEL; };
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
	  SwOSServo(const char *name, uint8_t port, SwOSCtrl *ctrl);

    // administrative stuff
	  virtual FtSwarmIOType_t getIOType() { return FTSWARM_SERVO; };
    virtual char *    getIcon() { return (char *) "14_servo.svg"; };
    virtual void jsonize( JSONize *json, uint8_t id);
    virtual void onTrigger( int32_t value );
    
    // commands
	  virtual int16_t getOffset( )   { return _offset; };
	  virtual int16_t getPosition( ) { return _position; };
	  virtual void setOffset( int16_t offset, bool dontSendToRemote );
	  virtual void setPosition( int16_t position, bool dontSendToRemote );
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
 *   I2C Slave
 *
 ***************************************************/

class SwOSI2C : public SwOSIO, public SwOSEventInput {
  protected:
    uint8_t _myRegister[MAXI2CREGISTERS];

    virtual void _setupLocal(uint8_t I2CAddress); // initializes local HW
    virtual void _setLocal( uint8_t reg, uint8_t value );
    virtual void _setRemote(uint8_t reg, uint8_t value );

  public:

    SwOSI2C( const char *name, SwOSCtrl *ctrl, uint8_t I2CAddress);

    virtual void read();

    virtual void setRegister( uint8_t reg, uint8_t value );
    virtual uint8_t getRegister( uint8_t reg );

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
 *   SwOSCAM - Camera
 *
 ***************************************************/

class SwOSCAM : public SwOSIO {
  protected:

    framesize_t _framesize  = FRAMESIZE_QVGA;
    int16_t     _quality    = 0;
    int16_t     _brightness = 0;
    int16_t     _contrast   = 0;
    int16_t     _saturation = 0;
    int16_t     _specialEffect = 0;
    int16_t     _wbMode = 0; 
    bool        _vFlip = false;
    bool        _hMirror = false;
    bool        _streaming = false;
    
    // local HW procedures
    virtual void _setupLocal(); // initializes local HW
    virtual void setRemote( void );
    
  public:
    // constructor
    SwOSCAM(const char *name, SwOSCtrl *ctrl );

    // administrative stuff
    virtual FtSwarmIOType_t getIOType() { return FTSWARM_CAM; };
    virtual void jsonize( JSONize *json, uint8_t id);

    void streaming( bool onOff, bool dontSendToRemote );
    void setFramesize( framesize_t framesize, bool dontSendToRemote );
    void setQuality( int quality, bool dontSendToRemote );
    void setBrightness( int brightness, bool dontSendToRemote );
    void setContrast( int contrast, bool dontSendToRemote );
    void setSaturation( int saturation, bool dontSendToRemote );
    void setSpecialEffect( int specialEffect, bool dontSendToRemote );
    void setWbMode( int wbMode, bool dontSendToRemote );
    void setHMirror( bool hMirror, bool dontSendToRemote );
    void setVFlip( bool vFlip, bool dontSendToRemote );

};

/***************************************************
 *
 *   SwOSCtrl - Base class for controllers.
 *
 ***************************************************/

class SwOSCtrl : public SwOSObj {
protected:
	FtSwarmVersion_t _CPU;
  bool             _local;
  unsigned long    _lastContact = 0;
  bool             _isSubscribed = false;
  char             *_subscribedCtrlName = NULL;
	const char *     version( FtSwarmVersion_t v);
  virtual void     _sendAlias( SwOSCom *alias );
public:
	FtSwarmSerialNumber_t serialNumber;
  MacAddr               macAddr;
  bool                  IAmAKelda;
  
  // common hardware
	SwOSInput    *input[MAXINPUTS];
	SwOSActor    *actor[MAXACTORS]; 
	SwOSPixel    *led[MAXLEDS];
  uint8_t      inputs, actors, leds;
	
  // constructor, destructor
  SwOSCtrl( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmAKelda, FtSwarmExtMode_t extentionPort );
  ~SwOSCtrl();

  // administrative stuff
  virtual bool cmdAlias( const char *obj, const char *alias);            // set an alias for this board or IO device
	virtual bool cmdAlias( char *device, uint8_t port, const char *alias); // set an alias for a IO device
  virtual SwOSIO *getIO( FtSwarmIOType_t ioType, FtSwarmPort_t port);    // get a pointer to an IO port via address
	virtual SwOSIO *getIO( const char *name);                              // get a pointer to an IO port via name or alias
  virtual FtSwarmControler_t getType();                                  // what I am?
	virtual char*              myType();                                   // what I am?
  virtual FtSwarmVersion_t   getCPU() { return _CPU; };                  // my CPU type
	virtual const char *       getVersionCPU();                            // my CPU type as string
  virtual bool               isLocal() { return _local; };               // local or remote?
	virtual char *             getHostname( );                             // hostname
	        void               jsonize( JSONize *json, uint8_t id);        // send board & IO device information as a json string
  virtual void               jsonizeIO( JSONize *json, uint8_t id);      // send IO device information as a json string
  virtual void loadAliasFromNVS(  nvs_handle_t my_handle );              // write my alias to NVS
  virtual void saveAliasToNVS(  nvs_handle_t my_handle );                // load my alias from NVS
  virtual void setState( SwOSState_t state, uint8_t members = 0, char *SSID = NULL ); // visualizes controler's state like booting, error,...
  virtual void factorySettings( void );                                  // reset factory settings
  virtual void halt( void );                                             // stop all actors
  virtual void unsubscribe( bool cascade );                              // unsubscribe userevents and if cascade = true all IOs
  virtual bool isI2CSwarmCtrl( void );                                   // is a ftSwarmI2C-Board 
  virtual unsigned long networkAge( void );                              // ms since last received package
  virtual void identify( void );                                         // set LEDs to aquamarine / OLED to "it's me" to identify HW 
  virtual char *subscribe( char *ctrlName );                             // listen on user event data
  virtual bool changeIOType( uint8_t port, FtSwarmIOType_t oldIOType, FtSwarmIOType_t newIOType ); // change port's IO Type if possible

  virtual void read(); // run measurements

  // API commnds
	virtual bool apiActorCmd( char *id, int cmd );                 // send an actor's command (from api)
  virtual bool apiActorSpeed( char *id, int speed );             // send an actors's speed (from api)
	virtual bool apiLEDBrightness( char *id, int brightness );     // send a LED command (from api)
	virtual bool apiLEDColor( char *id, int color );               // send a LED command (from api)
  virtual bool apiServoOffset( char *id, int offset );           // send a Servo command (from api)
  virtual bool apiServoPosition( char *id, int position );       // send a Servo command (from api)
  virtual bool apiCAMStreaming( char *id, bool onOff );            // start/stops CAM streaming
  virtual bool apiCAMFramesize( char *id, int framesize );         // set CAM framzesize / resolution
  virtual bool apiCAMQuality( char *id, int quality );             // set CAM quality
  virtual bool apiCAMBrightness( char *id, int brightness );       // set CAM brightness
  virtual bool apiCAMContrast( char *id, int contrast );           // set CAM contrast
  virtual bool apiCAMSaturation( char *id, int saturation );       // set CAM saturation
  virtual bool apiCAMSpecialEffect( char *id, int specialEffect ); // set CAM special effect
  virtual bool apiCAMWbMode( char *id, int wbMode );               // set CAM wbMode    
  virtual bool apiCAMHMirror( char *id, bool hMirror );            // set CAM H-Mirror
  virtual bool apiCAMVFlip( char *id, bool vFlip );                // set CAM V-Flip 
  virtual bool maintenanceMode();

  // Communications
  virtual bool OnDataRecv( SwOSCom *com );                         // data via espnow revceived
  virtual bool recvState( SwOSCom *com );                          // receive state from another ftSwarmXX
  virtual SwOSCom *state2Com( MacAddr destination );               // copy my state in a com struct
  virtual void registerMe( SwOSCom *com );                         // fill in my own data in registerCmd datagram
  virtual void sendAlias( MacAddr destination );                   // send my alias names
    
};


/***************************************************
 *
 *   SwOSSwarmI2CCtrl
 *
 ***************************************************/

class SwOSSwarmI2CCtrl : public SwOSCtrl {
  public:

    // constructor, destructor
    SwOSSwarmI2CCtrl( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmAKelda );
    ~SwOSSwarmI2CCtrl();

};

/***************************************************
 *
 *   SwOSSwarmPwrDrive - ftPwrDrive using FtSwarmI2C-Board
 *
 ***************************************************/

class SwOSSwarmPwrDrive : public SwOSSwarmI2CCtrl {
  private:
    uint8_t _microstepMode = 0;
  public:
    // constructor, destructor
    SwOSSwarmPwrDrive( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmAKelda );
    SwOSSwarmPwrDrive( SwOSCom *com ); 
    ~SwOSSwarmPwrDrive();

    virtual char*              myType();                                   // what I am?
	  virtual FtSwarmControler_t getType();                                  // what I am?
    virtual SwOSCom *          state2Com( MacAddr destination );
    virtual bool               recvState( SwOSCom *com );                  // receive state from another ftSwarmXX
    virtual void               read( void );
  
    // Communications
    virtual bool OnDataRecv( SwOSCom *com );     // data via espnow revceived
  
    virtual void setMicrostepMode( uint8_t mode, bool dontSendToRemote );
      // set microstep mode
      // FILLSTEP, HALFSTEP, QUARTERSTEP, EIGTHSTEP, SIXTEENTHSTEP
      
    virtual uint8_t getMicrostepMode( void );
      // get microstep mode
      // FILLSTEP, HALFSTEP, QUARTERSTEP, EIGTHSTEP, SIXTEENTHSTEP

};

/***************************************************
 *
 *   SwOSSwarmDuino - ftDuino using FtSwarmI2C-Board
 *
 ***************************************************/

class SwOSSwarmDuino : public SwOSSwarmI2CCtrl {
  public:
    // constructor, destructor
    SwOSSwarmDuino( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmAKelda );
    ~SwOSSwarmDuino();

    virtual char* myType();                                                // what I am?
	  virtual FtSwarmControler_t getType();                                  // what I am?
	  virtual void read();                                                   // run measurements

};

/***************************************************
 *
 *   SwOSSwarmXX - Base class for ftSwarmXX
 *
 ***************************************************/

class SwOSSwarmXX : public SwOSCtrl {
  protected:
    virtual void _sendAlias( SwOSCom *alias );
  public:
	  SwOSGyro     *gyro;
    SwOSI2C      *I2C;
    
    // constructor, destructor
    SwOSSwarmXX( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmAKelda, FtSwarmExtMode_t extentionPort );
    ~SwOSSwarmXX();
    
    virtual SwOSIO *getIO( FtSwarmIOType_t ioType, FtSwarmPort_t port);    // get a pointer to an IO port via address
	  virtual SwOSIO *getIO( const char *name);                              // get a pointer to an IO port via name or alias
    virtual void factorySettings( void );                                  // reset factory settings  
    virtual void unsubscribe(void );                                       // unsubscribe all IOs

    virtual bool OnDataRecv( SwOSCom *com );     // data via espnow revceived

};

/***************************************************
 *
 *   SwOSSwarmJST
 *
 ***************************************************/

class SwOSSwarmJST : public SwOSSwarmXX {

  protected:
    virtual void _sendAlias( SwOSCom *alias );
  
  public:
    // specific hardware
    uint8_t   servos  = 1;
	  SwOSServo *servo[MAXSERVOS];

    // constructor, destructor
	  SwOSSwarmJST( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmAKelda, uint8_t xLed, FtSwarmExtMode_t extentionPort );
    SwOSSwarmJST( SwOSCom *com ); // constructor
    ~SwOSSwarmJST();
  
    // administrative stuff
	  virtual bool cmdAlias( char *device, uint8_t port, const char *alias); // set an alias for a IO device
    virtual SwOSIO *getIO( FtSwarmIOType_t ioType,  FtSwarmPort_t port);   // get an pointer to the requested IO Device via type&port
	  virtual SwOSIO *getIO( const char *name);                              // get an pointer to the requested IO Device via name or alias
	  virtual FtSwarmControler_t getType();                                  // what I am?
	  virtual char* myType();                                                // what I am?
	  virtual void jsonizeIO( JSONize *json, uint8_t id);                    // send IO device information as a json string
    virtual void loadAliasFromNVS(  nvs_handle_t my_handle );              // write my alias to NVS
    virtual void saveAliasToNVS(  nvs_handle_t my_handle );                // load my alias from NVS
    virtual void factorySettings( void );                                  // reset factory settings
    virtual void unsubscribe(void );                                       // unsubscribe all IOs
    virtual bool changeIOType( uint8_t port, FtSwarmIOType_t oldIOType, FtSwarmIOType_t newIOType ); // change port's IO Type if possible

    // API commands
    virtual bool apiServoOffset( char *id, int offset );           // send a Servo command (from api)
    virtual bool apiServoPosition( char *id, int position );       // send a Servo command (from api)

    // **** Communications *****
    virtual bool OnDataRecv( SwOSCom *com ); // data via espnow revceived

};

/***************************************************
 *
 *   SwOSSwarmControl
 *
 ***************************************************/

class SwOSSwarmControl : public SwOSSwarmXX {

  protected:
    boolean _remoteControl = false;
    boolean _firstRead     = true;
    virtual void _sendAlias( SwOSCom *alias );

  public:
    // specific hardware
	  SwOSButton   *button[8];
	  SwOSHC165    *hc165;
	  SwOSJoystick *joystick[2];

    SwOSOLED     *oled;
 
	  SwOSSwarmControl(FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmAKelda, int16_t zero[2][2], uint8_t displayType ); // constructor
    SwOSSwarmControl( SwOSCom *com ); // constructor
    ~SwOSSwarmControl(); // destructor
  
    // administrative stuff
	  virtual bool cmdAlias( char *device, uint8_t port, const char *alias); // set an alias for a IO device
    virtual SwOSIO *getIO( FtSwarmIOType_t ioType,  FtSwarmPort_t port);   // get a pointer to the requested IO Device via type & port
	  virtual SwOSIO *getIO( const char *name);                              // get a pointer to the requested IO Device via name or alias
	  virtual char* myType();                                                // what I am?
	  virtual FtSwarmControler_t getType();                                  // what I am?
	  virtual void jsonizeIO( JSONize *json, uint8_t id);                    // send IO device information as a json string
    virtual void loadAliasFromNVS(  nvs_handle_t my_handle );              // write my alias to NVS
    virtual void saveAliasToNVS(  nvs_handle_t my_handle );                // load my alias from NVS
    virtual void setState( SwOSState_t state, uint8_t members = 0, char *SSID = NULL ); // visualizes controler's state like booting, error,...
    virtual void factorySettings( void );                                  // reset factory settings
    virtual boolean getRemoteControl( void );                              // get remote control setting
    virtual void setRemoteControl( boolean remoteControl );                // set remote control setting
    virtual void unsubscribe(void );                                       // unsubscribe all IOs

	  virtual void read();                       // run measurements

    // **** Communications *****
    virtual bool recvState( SwOSCom *com );    // receive state from another FtSwarmControl
    virtual SwOSCom *state2Com( MacAddr destination );        // copy my state in a com struct
    virtual bool OnDataRecv( SwOSCom *com );   // data via espnow revceived
  
};

/***************************************************
 *
 *   SwOSSwarmCAM
 *
 ***************************************************/

class SwOSSwarmCAM : public SwOSCtrl {

  protected:

  public:
    SwOSCAM *cam = NULL;

    SwOSSwarmCAM(FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmAKelda ); // constructor
    SwOSSwarmCAM( SwOSCom *com ); // constructor
    ~SwOSSwarmCAM(); // destructor
  
    // administrative stuff
    virtual bool cmdAlias( char *device, uint8_t port, const char *alias); // set an alias for a IO device
    virtual SwOSIO *getIO( FtSwarmIOType_t ioType,  FtSwarmPort_t port);   // get a pointer to the requested IO Device via type & port
    virtual SwOSIO *getIO( const char *name);                              // get a pointer to the requested IO Device via name or alias
    virtual char* myType();                                                // what I am?
    virtual FtSwarmControler_t getType();                                  // what I am?
    virtual void jsonizeIO( JSONize *json, uint8_t id);                    // send IO device information as a json string
    virtual void loadAliasFromNVS(  nvs_handle_t my_handle );              // write my alias to NVS
    virtual void saveAliasToNVS(  nvs_handle_t my_handle );                // load my alias from NVS
    virtual void setState( SwOSState_t state, uint8_t members = 0, char *SSID = NULL ); // visualizes controler's state like booting, error,...

    // api commands
    virtual bool apiCAMStreaming(  char *id, bool onOff );           // start/stops CAM streaming
    virtual bool apiCAMFramesize( char *id, int framesize );         // set CAM framzesize / resolution
    virtual bool apiCAMQuality( char *id, int quality );             // set CAM quality
    virtual bool apiCAMBrightness( char *id, int brightness );       // set CAM brightness
    virtual bool apiCAMContrast( char *id, int contrast );           // set CAM contrast
    virtual bool apiCAMSaturation( char *id, int saturation );       // set CAM saturation
    virtual bool apiCAMSpecialEffect( char *id, int specialEffect ); // set CAM special effect
    virtual bool apiCAMWbMode( char *id, int wbMode );               // set CAM wbMode
    virtual bool apiCAMHMirror( char *id, bool hMirror );            // set CAM H-Mirror
    virtual bool apiCAMVFlip( char *id, bool vFlip );                // set CAM V-Flip 

    // **** Communications *****
    virtual bool OnDataRecv( SwOSCom *com );   // data via espnow revceived


};
