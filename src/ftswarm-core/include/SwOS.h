/*
 * SwOS.h
 *
 * some common SwOS defintions
 * don't use this definitions for building ftSwarm applications
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */

#pragma once

// inter swarm communication deep debugging 
// #define DEBUG_COMMUNICATION
//#define DEBUG_READTASK
    
#define LOGFTSWARM  "FTSWARM"
#define MAXIDENTIFIER 32
#define MAXACTORS 4
#define MAXINPUTS 8
#define SWOSVERSION "0.5.2"

#include <stdint.h>
#include <cstddef>

#include <FastLED.h>
#include <LSM6DSRSensor.h>
#include <ftPwrDrive/ftPwrDrive.h>
#include <esp_camera.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "SwOSFirmware.h"

// max LEDs ftSwarm
#define MAXLEDS 18

// max Servos
#define MAXSERVOS 2

// max I2C Register
#define MAXI2CREGISTERS 8

// max # of controlers in swarm
#define MAXCTRL 32

typedef uint16_t FtSwarmSerialNumber_t;
typedef uint8_t  FtSwarmPort_t;

// **** enumerations ****

// communication
typedef enum { swarmComWifi = 1, swarmComRS485 = 2, swarmComBoth= 3 } FtSwarmCommunication_t; 

// IO types
typedef enum { FTSWARM_UNDEF = -1, FTSWARM_INPUT, FTSWARM_DIGITALINPUT, FTSWARM_ANALOGINPUT, FTSWARM_ACTOR, FTSWARM_BUTTON, FTSWARM_JOYSTICK, FTSWARM_PIXEL, FTSWARM_SERVO,  FTSWARM_OLED, FTSWARM_GYRO, FTSWARM_HC165, FTSWARM_I2C, FTSWARM_CAM, FTSWARM_MAXIOTYPE } FtSwarmIOType_t ;

// controler types
typedef enum { FTSWARM_NOCTRL = -1, FTSWARM = 0, FTSWARMCONTROL, FTSWARMCAM, FTSWARMPWRDRIVE, FTSWARMDUINO } FtSwarmControler_t;

// sensor types
typedef enum { FTSWARM_DIGITAL, FTSWARM_ANALOG, FTSWARM_SWITCH, FTSWARM_REEDSWITCH, FTSWARM_LIGHTBARRIER, FTSWARM_VOLTMETER, FTSWARM_OHMMETER, FTSWARM_THERMOMETER, FTSWARM_LDR, FTSWARM_TRAILSENSOR, FTSWARM_COLORSENSOR, FTSWARM_ULTRASONIC, FTSWARM_COUNTER, FTSWARM_MAXSENSOR } FtSwarmSensor_t;

// actor types
typedef enum { FTSWARM_XMOTOR, FTSWARM_XMMOTOR, FTSWARM_TRACTOR,  FTSWARM_ENCODER, FTSWARM_LAMP, FTSWARM_VALVE, FTSWARM_COMPRESSOR, FTSWARM_BUZZER, FTSWARM_STEPPER, FTSWARM_MAXACTOR } FtSwarmActor_t;

// HW versions
typedef enum { 
  FTSWARM_NOVERSION = -1, 
  FTSWARMJST_1V0, 
  FTSWARMCONTROL_1V3, 
  FTSWARMJST_1V15, 
  FTSWARMRS_2V0, 
  FTSWARMRS_2V1,
  FTSWARMCAM_2V11,
  FTSWARMDUINO_1V141,
  FTSWARMPWRDRIVE_1V141 } FtSwarmVersion_t;

// how to move
typedef enum { FTSWARM_COAST, FTSWARM_BRAKE, FTSWARM_ON, FTSWARM_MAXMOTION } FtSwarmMotion_t;

// toggles
typedef enum { FTSWARM_NOTOGGLE, FTSWARM_TOGGLEUP, FTSWARM_TOGGLEDOWN } FtSwarmToggle_t;

// alignment
typedef enum { FTSWARM_ALIGNLEFT, FTSWARM_ALIGNCENTER, FTSWARM_ALIGNRIGHT } FtSwarmAlign_t;

// Ext Port modes
typedef enum { FTSWARM_EXT_OFF, FTSWARM_EXT_I2C_MASTER, FTSWARM_EXT_I2C_SLAVE, FTSWARM_EXT_MCU6040, FTSWARM_EXT_OUTPUT  } FtSwarmExtMode_t;

// trigger events
typedef enum { FTSWARM_TRIGGERUP, FTSWARM_TRIGGERDOWN, FTSWARM_TRIGGERVALUE, FTSWARM_TRIGGERI2CREAD, FTSWARM_TRIGGERI2CWRITE, FTSWARM_MAXTRIGGER } FtSwarmTrigger_t;


typedef enum {
    Red        = 0xFF0000,
    Green      = 0x808080,
    Blue       = 0x0000FF,
    Yellow     = 0xFFFF00,
    Cyan       = 0x00FFFF,
    Aquamarine = 0x7FFFD4,
    Black      = 0x000000
} FtSwarmColor;

#define MAXSPEED 4095 

// **** port definitions ****

// switches
#define FTSWARM_S1 0
#define FTSWARM_S2 1
#define FTSWARM_S3 2
#define FTSWARM_S4 3
#define FTSWARM_F1 4
#define FTSWARM_F2 5
#define FTSWARM_J1 6
#define FTSWARM_J2 7

// inputs
#define FTSWARM_A1 0
#define FTSWARM_A2 1
#define FTSWARM_A3 2
#define FTSWARM_A4 3
#define FTSWARM_A5 4
#define FTSWARM_A6 5

// outputs
#define FTSWARM_M1 0
#define FTSWARM_M2 1 
#define FTSWARM_M3 2 
#define FTSWARM_M4 3 

// joysticks
#define FTSWARM_JOY1 0
#define FTSWARM_JOY2 1

// leds
#define FTSWARM_LED1 0
#define FTSWARM_LED2 1
#define FTSWARM_LED3 2
#define FTSWARM_LED4 3
#define FTSWARM_LED5 4
#define FTSWARM_LED6 5
#define FTSWARM_LED7 6
#define FTSWARM_LED8 7
#define FTSWARM_LED9 8
#define FTSWARM_LED10 9
#define FTSWARM_LED11 10
#define FTSWARM_LED12 11
#define FTSWARM_LED13 12
#define FTSWARM_LED14 13
#define FTSWARM_LED15 14
#define FTSWARM_LED16 15
#define FTSWARM_LED17 16
#define FTSWARM_LED18 17

// Servos
#define FTSWARM_SERVO1 0
#define FTSWARM_SERVO2 1

// **** some internal types & classes, don't use them at all ****

// handle/pointer to a swarm IO, used by FtSwarmIO base class
typedef void* SwOSIOHandle_t;      

class FtSwarmIO {
  // base class for all ftSwarm interfaces, don't use this class at all
  
  protected:
    
    // constructors
    FtSwarmIO( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmIOType_t ioType);
    FtSwarmIO( const char *name, FtSwarmIOType_t ioType );

    // destructor
    ~FtSwarmIO();

  public:
    bool isOnline();          // check, if I'm online
    SwOSIOHandle_t me = NULL; // pointer to my swarm HW, don't use!
};

class FtSwarmInput : public FtSwarmIO {
  // an input base class, don't use this class at all
  
  protected:
    FtSwarmInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmIOType_t ioType );
    FtSwarmInput( const char *name, FtSwarmIOType_t ioType );

  public:
    void onTrigger( FtSwarmTrigger_t triggerEvent, FtSwarmIO *actor, int32_t p1 ); 
    void onTrigger( FtSwarmTrigger_t triggerEvent, FtSwarmIO *actor ); 
};

// **** input / actor classes to use in your sketch ****

class FtSwarmDigitalInput : public FtSwarmInput {
  // digital inputs. ports A1..A4, all controler types

  protected:
    FtSwarmDigitalInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmSensor_t sensorType, bool normallyOpen = true);
    FtSwarmDigitalInput( const char *name, FtSwarmSensor_t sensorType, bool normallyOpen );
    void setSensorType( FtSwarmSensor_t sensorType, bool normallyOpen );
  
  public:
    FtSwarmDigitalInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen = true);
    FtSwarmDigitalInput( const char *name, bool normallyOpen );

    bool isPressed();                     // getState()
    bool isReleased();                    // !getState()
    bool hasToggledUp();                  // was the last toggle event from down to up, since a hasToggled* method called last time?
    bool hasToggledDown();                // was the last toggle event from up to down, since a hasToggled* method called last time?
    virtual bool getState();              // true if input is high, false otherwise
    virtual FtSwarmToggle_t getToggle();  // last toggle event since a hasToggled* method called last time
};

class FtSwarmSwitch : public FtSwarmDigitalInput {
  // all kind of mechanical switches, A1..A4 all controlers
  // fischertechnik switches: 1-3 is normally open, 1-2 is normally closed
  public:
    FtSwarmSwitch( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen = true);
    FtSwarmSwitch( const char *name, bool normallyOpen = true );
};

class FtSwarmReedSwitch : public FtSwarmDigitalInput {
  // reed switches, A1..A4 all controlers
  public:
    FtSwarmReedSwitch( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen = true);
    FtSwarmReedSwitch( const char *name, bool normallyOpen = true);
};

class FtSwarmLightBarrier: public FtSwarmDigitalInput {
  // photo transistor as light barrier, A1..A4 all controlers
  public:
    FtSwarmLightBarrier( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen = true);
    FtSwarmLightBarrier( const char *name, bool normallyOpen = true);
};

class FtSwarmButton : public FtSwarmIO {
  // onboard buttons, FtSwarmControl only
  public:
    FtSwarmButton( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port );
    FtSwarmButton( const char *name );
    
    bool isPressed();                     // button is pressed
    bool isReleased();                    // button is released
    bool hasToggledUp();                  // was the last toggle event from down to up, since a hasToggled* method called last time?
    bool hasToggledDown();                // was the last toggle event from up to down, since a hasToggled* method called last time?
    virtual bool getState();              // true, if the button is pressed
    virtual FtSwarmToggle_t getToggle();  // last toggle event since a hasToggled* method called last time
    void onTrigger( FtSwarmTrigger_t triggerEvent, FtSwarmIO *actor, int32_t p1 ); 
    void onTrigger( FtSwarmTrigger_t triggerEvent, FtSwarmIO *actor ); 
};

class FtSwarmAnalogInput : public FtSwarmInput { 
  // general analog input, A1..A4 ftSwarm only
  protected:
    FtSwarmAnalogInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmSensor_t sensorType);
    FtSwarmAnalogInput( const char *name, FtSwarmSensor_t sensorType );
    void setSensorType( FtSwarmSensor_t sensorType );

  public:
    FtSwarmAnalogInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port );
    FtSwarmAnalogInput( const char *name );
    int32_t getValue();  // get 12 bit raw reading
};

class FtSwarmVoltmeter : public FtSwarmAnalogInput {
  // Voltmeter, ftSwarm.A2 only
  public:
    FtSwarmVoltmeter( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmVoltmeter( const char *name );
    
    float getVoltage();  // get calibrated value 150 mV ~ 2450 mV
};

class FtSwarmOhmmeter : public FtSwarmAnalogInput {
  // Ohmmeter, A11.A4 ftSwarm only
  
  public:
    FtSwarmOhmmeter( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmOhmmeter( const char * name );
    
    float getResistance(); 
};

class FtSwarmThermometer : public FtSwarmAnalogInput {
  // Thermometer based on 1.5kOhm NTC, A1..A4 ftSwarm only
  
  public:
    FtSwarmThermometer( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmThermometer( const char *name );
    
    float getCelcius();
    float getKelvin();
    float getFahrenheit();
};

class FtSwarmLDR : public FtSwarmAnalogInput {
  // LDR, A1..A4 ftSwarm only
  // At FtSwarmControl you could use a LDR with a FtSwarmDigitalInput as well
  public:
    FtSwarmLDR( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmLDR( const char * name );
};

class FtSwarmActor : public FtSwarmIO {
  // an actor base class, don't use this class at all
  protected:
    FtSwarmActor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmActor_t actorType );
    FtSwarmActor( const char *name, FtSwarmActor_t actorType );
};


class FtSwarmMotor : public FtSwarmActor {
  // general motor class, use this class for (old) gray motors, mini motors, XS motors
  // M1..M2 all contollers - keep power budget in mind!
  public:
    FtSwarmMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmActor_t actorType = FTSWARM_XMOTOR );
    FtSwarmMotor( const char *name, FtSwarmActor_t actorType = FTSWARM_XMOTOR );
    
    void     setSpeed( int16_t speed );                                // speed +/-4095, speed 0 motor stopss
    uint16_t getSpeed();                                               // actual speed
    void     setAcceleration( uint32_t rampUpT,  uint32_t rampUpY );  
    void     getAcceleration( uint32_t *rampUpT, uint32_t *rampUpY );
    
};

class FtSwarmTractorMotor : public FtSwarmMotor {
  // tractor & XM motor
  // M1..M2 all contollers - keep power budget in mind!
  protected:
    FtSwarmTractorMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmActor_t actorType );
    FtSwarmTractorMotor( const char * name, FtSwarmActor_t actorType );
      
  public:
    FtSwarmTractorMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port );
    FtSwarmTractorMotor( const char * name );

    // FtSwarmTractor has different options for stopping motor:
    virtual void setMotionType( FtSwarmMotion_t motionType );
    FtSwarmMotion_t getMotionType();
    virtual void coast( void ) { setMotionType( FTSWARM_COAST ); };
    virtual void brake( void ) { setMotionType( FTSWARM_BRAKE ); };
};

class FtSwarmXMMotor : public FtSwarmTractorMotor {
  // xm motor
  // M1..M2 all contollers - keep power budget in mind!
  public:
    FtSwarmXMMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmXMMotor( const char * name );
};

class FtSwarmEncoderMotor : public FtSwarmTractorMotor {
  // encoder motor
  // M1..M2 all contollers - keep power budget in mind!
  // TODO: implement encoder input
  public:
    FtSwarmEncoderMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmEncoderMotor( const char * name );
};

class FtSwarmStepperMotor : public FtSwarmTractorMotor {
  // stepper motor
  // M1..M4 ftSwarmPwrDrive
  public:
    FtSwarmStepperMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmStepperMotor( const char * name );
    virtual void setDistance( long distance, bool relative = false );  // set steps to go
    virtual long getDistance( void );                                  // get steps to go
    virtual void run( void );                                          // start motor
    virtual bool isRunning( void );                                    // true, if motor is running, false if not
    virtual void stop( void );                                         // stop immediately
    virtual void setPosition( long position );                         // set an absolute position
    virtual long getPosition( void );                                  // get absolute position
    virtual void homing( long maxDistance );                           // run a homing cycle
    virtual bool isHoming( void );                                     // true, if homing cycle is active
    virtual void setHomingOffset( long offset );                       // set Offset to run in homing, after endstop is free again
};


class FtSwarmOnOffActor : public FtSwarmActor {
  // classic on/off devices
  // M1..M2 all contollers - keep power budget in mind!
  public:
    FtSwarmOnOffActor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmActor_t actorType );
    FtSwarmOnOffActor( const char *name, FtSwarmActor_t actorType );
    
    void on( int16_t power = MAXSPEED );
    void off( void );
};


class FtSwarmLamp : public FtSwarmOnOffActor {
  // classic lamps and LEDs
  // M1..M2 all contollers - keep power budget in mind!
  public:
    FtSwarmLamp( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmLamp( const char *name );

};

class FtSwarmValve : public FtSwarmOnOffActor {
  // Valve
  // M1..M2 all contollers - keep power budget in mind!
  public:
    FtSwarmValve( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmValve( const char *name );

};

class FtSwarmCompressor : public FtSwarmOnOffActor {
  // Compressor
  // M1..M2 all contollers - keep power budget in mind!
  public:
    FtSwarmCompressor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmCompressor( const char *name );

};

class FtSwarmBuzzer : public FtSwarmOnOffActor {
  // Buzzer
  // M1..M2 all contollers - keep power budget in mind!
  public:
    FtSwarmBuzzer( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmBuzzer( const char *name );

};


class FtSwarmJoystick : public FtSwarmIO {
  protected:
    SwOSIOHandle_t button = NULL;
  public:
    FtSwarmJoystick( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmJoystick( const char *name );
    
    int16_t getFB();         // front/back position
    int16_t getLR();         // left/right position
    bool getButtonState();   // button pressed/released
    void getValue( int16_t *FB, int16_t *LR, bool *buttonState );
    void onTriggerLR( FtSwarmTrigger_t triggerEvent, FtSwarmIO *actor, int32_t p1 ); 
    void onTriggerLR( FtSwarmTrigger_t triggerEvent, FtSwarmIO *actor ); 
    void onTriggerFB( FtSwarmTrigger_t triggerEvent, FtSwarmIO *actor, int32_t p1 ); 
    void onTriggerFB( FtSwarmTrigger_t triggerEvent, FtSwarmIO *actor ); 

};

class FtSwarmPixel : public FtSwarmIO {
  // RGB LEDs, ftSwarm only
  // one LED takes up to 60mA, keep power budget in mind!
  public:
    FtSwarmPixel( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmPixel( const char *name );

    // brightness 0..255
    uint8_t getBrightness();
    void setBrightness(uint8_t brightness);

    // color
    uint32_t getColor();
    void setColor(uint32_t color);
};

class FtSwarmI2C : public FtSwarmIO {
  // I2C slave with 4 registers
  public:
    FtSwarmI2C( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port );
    FtSwarmI2C( const char *name );

    uint8_t getRegister(uint8_t reg);
    void    setRegister(uint8_t reg, uint8_t value);

    void onTrigger( FtSwarmTrigger_t triggerEvent, FtSwarmIO *actor, int32_t p1 ); 

};

class FtSwarmServo : public FtSwarmIO {
  // Servo, ftSwarm only
  // A servo has 150mA typically, higher values with load. Keep power budget in mind!
  public:
    FtSwarmServo( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmServo( const char *name );

    // position
    int16_t getPosition();
    void setPosition( int16_t position );

    // offset
    int16_t getOffset();
    void setOffset( int16_t position );
};

class FtSwarmOLED : public FtSwarmIO {
  // OLED display, ftSwarmControl only
  // TODO: add remote calls. Actually the display is limited to local displays.
  public:
    void display(void);
    void invertDisplay(bool i);
    void fillScreen( bool white=true);    
    void dim(bool dim);
    int16_t getWidth(void);
    int16_t getHeight(void);
    
    void drawPixel(int16_t x, int16_t y, bool white=true);  
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool white=true);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, bool fill=false, bool white=true);
    void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, bool fill=false, bool white=true);
    void drawCircle(int16_t x0, int16_t y0, int16_t r, bool fill=false, bool white=true);
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool fill, bool white=true);
    void drawChar(int16_t x, int16_t y, unsigned char c, bool color=true, bool bg=false, uint8_t size_x=1, uint8_t size_y=1);
    void write( char *str, int16_t x, int16_t y, FtSwarmAlign_t align = FTSWARM_ALIGNCENTER, bool fill = true );
    void write( char *str );
   
    void setCursor(int16_t x, int16_t y);
    void getCursor(int16_t *x, int16_t *y);

    void setTextColor(bool c, bool bg=false);
    void setTextWrap(bool w);

    void setRotation(uint8_t r);
    uint8_t getRotation(void);

    void setTextSize(uint8_t sx, uint8_t sy=1);
    void getTextSize( uint8_t *sx, uint8_t *sy );
    
    void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
};

class FtSwarmCAM : public FtSwarmIO {
  public:
    FtSwarmCAM( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmCAM( const char *name );

    void streaming( bool onOff );
    void setFramesize( framesize_t framesize );
    void setQuality( int quality );
    void setBrightness( int brightness );
    void setContrast( int contrast );
    void setSaturation( int saturation );
    void setSpecialEffect( int specialEffect );
    void setWbMode( int wbMode );
    void setHMirror( bool hMirror );
    void setVFlip( bool vFlip );
};

class FtSwarm {
  // my swarm...
    
  public:
    FtSwarmSerialNumber_t begin( bool verbose = false );   // start my swarm
    void setReadDelay( uint16_t readDelay );               // set delay between two measures
    void halt( void );                                     // stop all actors
    bool waitOnUserEvent( int parameter[10], TickType_t xTicksToWait = 512 );
    bool sendEventData( uint8_t *buffer, size_t size );
};

// There is one only
extern FtSwarm ftSwarm;

extern void forever( char *prompt);

extern FtSwarmIOType_t sensorType2IOType( FtSwarmSensor_t sensor2IOType );
