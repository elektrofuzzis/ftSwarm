#pragma once

#include <stdint.h>
#include <cstddef>

#define LOGFTSWARM  "FTSWARM"
#define SWOSVERSION "0.10"

#define MAXIDENTIFIER 32

typedef void*    FtSwarmIOHandle_t;
typedef uint16_t FtSwarmSerialNumber_t;
typedef uint8_t  FtSwarmPort_t;

// some internal stuff, don't use.
typedef enum { FTSWARM_UNDEF = -1, FTSWARM_INPUT, FTSWARM_ACTOR, FTSWARM_BUTTON, FTSWARM_JOYSTICK, FTSWARM_LED, FTSWARM_SERVO,	FTSWARM_OLED, FTSWARM_GYRO, FTSWARM_HC165, FTSWARM_MAXIOTYPE } FtSwarmIOType_t;
typedef enum { FTSWARM_NOCTRL = -1, FTSWARM = 0, FTSWARMCONTROL = 1 } FtSwarmControler_t;
typedef enum { FTSWARM_DIGITAL, FTSWARM_ANALOG, FTSWARM_SWITCH, FTSWARM_REEDSWITCH, FTSWARM_VOLTMETER, FTSWARM_OHMMETER, FTSWARM_THERMOMETER, FTSWARM_LDR, FTSWARM_TRAILSENSOR, FTSWARM_COLORSENSOR, FTSWARM_ULTRASONIC, FTSWARM_MAXSENSOR } FtSwarmSensor_t;
typedef enum { FTSWARM_XMOTOR, FTSWARM_TRACTOR,  FTSWARM_ENCODER, FTSWARM_LAMP, FTSWARM_MAXACTOR } FtSwarmActor_t;


// more interesting stuff
typedef enum { FTSWARM_NOVERSION = -1, FTSWARM_1V0, FTSWARM_1V3, FTSWARM_1V15 } FtSwarmVersion_t;
typedef enum { FTSWARM_COAST, FTSWARM_BRAKE, FTSWARM_ON } FtSwarmMotion_t;
typedef enum { FTSWARM_NOTOGGLE, FTSWARM_TOGGLEUP, FTSWARM_TOGGLEDOWN } FtSwarmToggle_t;

#define FTSWARM_S1 0
#define FTSWARM_S2 1
#define FTSWARM_S3 2
#define FTSWARM_S4 3
#define FTSWARM_F1 4
#define FTSWARM_F2 5
#define FTSWARM_J1 6
#define FTSWARM_J2 7

#define FTSWARM_A1 0
#define FTSWARM_A2 1
#define FTSWARM_A3 2
#define FTSWARM_A4 3

#define FTSWARM_M1 0
#define FTSWARM_M2 1 

#define FTSWARM_JOY1 0
#define FTSWARM_JOY2 1

#define FTSWARM_LED1 0
#define FTSWARM_LED2 1

class FtSwarmIO {
  // base class, don't use this class at all
  protected:
    FtSwarmIOHandle_t me = NULL;
  public:
    FtSwarmIO() {};
    FtSwarmIO( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmIOType_t ioType);
    bool isOnline();
};

class FtSwarmInput : public FtSwarmIO {
  // an input base class, don't use this class at all
  public:
    FtSwarmInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmSensor_t sensorType, bool normallyOpen = true);
};

class FtSwarmDigitalInput : public FtSwarmInput {
  protected:
    FtSwarmDigitalInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmSensor_t sensorType, bool normallyOpen = true);
  public:
    FtSwarmDigitalInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    bool isPressed();
    bool isReleased();
    bool hasToggledUp();
    bool hasToggledDown();
    virtual bool getState();
    virtual FtSwarmToggle_t getToggle();
};

class FtSwarmSwitch : public FtSwarmDigitalInput {
  public:
    FtSwarmSwitch( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen = true);
};

class FtSwarmReedSwitch : public FtSwarmDigitalInput {
  public:
    FtSwarmReedSwitch( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen = true);
    // fischertechnik switches: 1-3 is normally open, 1-2 is normally closed
};

class FtSwarmButton : public FtSwarmIO {
  public:
    FtSwarmButton( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    bool isPressed();
    bool isReleased();
    bool hasToggledUp();
    bool hasToggledDown();
    virtual bool getState();
    virtual FtSwarmToggle_t getToggle();
};

class FtSwarmAnalogInput : public FtSwarmInput { 
  protected:
    FtSwarmAnalogInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmSensor_t sensorType);
  public:
    FtSwarmAnalogInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    int32_t getValue();
};

class FtSwarmVoltmeter : public FtSwarmAnalogInput {
  public:
    FtSwarmVoltmeter( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    float getVoltage();
};

class FtSwarmOhmmeter : public FtSwarmAnalogInput {
  public:
    FtSwarmOhmmeter( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    float getResistance();
};

class FtSwarmThermometer : public FtSwarmAnalogInput {
  public:
    FtSwarmThermometer( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    float getCelcius();
    float getKelvin();
    float getFahrenheit();
};

class FtSwarmLDR : public FtSwarmAnalogInput {
  public:
    FtSwarmLDR( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
};

class FtSwarmActor : public FtSwarmIO {
  // an input base class, don't use this class at all
  public:
    FtSwarmActor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmActor_t actorType );
};


class FtSwarmMotor : public FtSwarmActor {
  public:
    FtSwarmMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmActor_t actorType = FTSWARM_XMOTOR );
    void run( int16_t speed );
    uint16_t getSpeed();
};

class FtSwarmTractorMotor : public FtSwarmMotor {
  public:
    // FtSwarmTractorMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    FtSwarmTractorMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmActor_t actorType = FTSWARM_TRACTOR);
    virtual void setMotionType( FtSwarmMotion_t motionType );
    virtual void coast( void ) { setMotionType( FTSWARM_COAST ); };
    virtual void brake( void ) { setMotionType( FTSWARM_BRAKE ); };
    FtSwarmMotion_t getMotionType();
};

class FtSwarmEncoderMotor : public FtSwarmTractorMotor {
  public:
    FtSwarmEncoderMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
};

class FtSwarmLamp : public FtSwarmActor {
  public:
    FtSwarmLamp( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    void on( uint8_t power = 255 );
    void off( void );
};

class FtSwarmJoystick : public FtSwarmIO {
  public:
    FtSwarmButton *button = NULL;
    FtSwarmJoystick( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    int16_t getFB();
    int16_t getLR();
    void getValue( int16_t *FB, int16_t *LR );
};

class FtSwarmLED : public FtSwarmIO {
  public:
    FtSwarmLED( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    uint8_t getBrightness();
    void setBrightness(uint8_t brightness);
    uint32_t getColor();
    void setColor(uint32_t color);
};

class FtSwarmServo : public FtSwarmIO {
  public:
    FtSwarmServo( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port);
    int16_t getPosition();
    void setPosition( int16_t position );
    int16_t getOffset();
    void setOffset( int16_t position );
};

class FtSwarm {
  protected:
    bool _IAmKelda;
  public:
    void begin( bool IAmAKelda = true, bool verbose = false ) ;
    void setReadDelay( uint16_t readDelay );
    void setWifi( const char *ssid, const char *pwd, bool APMode = false, bool writeNVS = true );
    void setup(void);
};

extern FtSwarm ftSwarm;
