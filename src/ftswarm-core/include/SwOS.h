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
// #define DEBUG_COMMUNICATION_SWARM
// #define DEBUG_READTASK

#define MAXIDENTIFIER 32
#define MAXACTORS 8
#define MAXINPUTS 11
#define SWOSVERSION "0.7.0"

#define SWOS_NOPORT 255
#define SWOS_PWRCTL SWOS_NOPORT

#include <stdint.h>
#include <cstddef>

#include <FastLED.h>
#include "ftPwrDrive/ftPwrDrive.h"
#include "esp_camera.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "SwOSFirmware.h"

// max LEDs ftSwarm
#define MAXLEDS 18

// max Servos
#define MAXSERVOS 4

// max I2C Register
#define MAXI2CREGISTERS 8

// max # of controllers in swarm
#define MAXCTRL 32

typedef uint16_t FtSwarmSerialNumber_t;
typedef uint8_t  FtSwarmPort_t;

// some delays
#define shortDelay()  vTaskDelay( 25 / portTICK_PERIOD_MS )
#define longDelay()   vTaskDelay( 1000 / portTICK_PERIOD_MS )

// **** enumerations ****

// error types
typedef enum { SWOS_OK, SWOS_TIMEOUT, SWOS_DENY } SwOSError_t;

// communication
typedef enum { swarmComWifi = 1, swarmComRS485 = 2, swarmComBoth= 3 } FtSwarmCommunication_t; 

// state
typedef enum { OFFLINE, BOOTING, STARTWIFI, RUNNING, ERROR, WAITING, IDENTIFY, FATAL, MAXSTATE } SwOSState_t;

// controller types
typedef enum { FTSWARM_NOCTRL = -1, FTSWARM = 0, FTSWARMCONTROL, FTSWARMCAM, FTSWARMPWRDRIVE, FTSWARMDUINO, FTSWARM_MAXCONTROLLERTYPE } FtSwarmController_t;

// technologies
typedef enum { SWOSIOCLASS_INPUT,
               SWOSIOCLASS_BUTTON,
               SWOSIOCLASS_JOYSTICK,
               SWOSIOCLASS_MOTOR,
               SWOSIOCLASS_SINGULAR 
} SwOSIOClass_t;

// io types
typedef enum { SWOSIO_UNDEF = -1,
               SWOSIO_DIGITAL, 
               SWOSIO_SWITCH, 
               SWOSIO_REEDSWITCH, 
               SWOSIO_LIGHTBARRIER, 
               SWOSIO_BUTTON,              
               SWOSIO_ANALOG, 
               SWOSIO_VOLTMETER, 
               SWOSIO_OHMMETER,
               SWOSIO_THERMOMETER, 
               SWOSIO_LDR, 
               SWOSIO_JOYSTICK,
               SWOSIO_MOTOR, 
               SWOSIO_XMOTOR, 
               SWOSIO_XMMOTOR, 
               SWOSIO_TRACTOR,  
               SWOSIO_ENCODER, 
               SWOSIO_LAMP, 
               SWOSIO_VALVE, 
               SWOSIO_COMPRESSOR, 
               SWOSIO_BUZZER, 
               SWOSIO_STEPPER, 
               SWOSIO_COUNTER, 
               SWOSIO_ROTARYENCODER, 
               SWOSIO_FREQUENCYMETER, 
               SWOSIO_LIDAR, 
               SWOSIO_CAM, 
               SWOSIO_SERVO,
               SWOSIO_PIXEL,
               SWOSIO_OLED,
               SWOSIO_I2C,
               SWOSIO_GYRO,
               SWOSIO_HC165,
               SWOSIO_POWER,
               SWOSIO_COLORSENSOR, 
               SWOSIO_TRAILSENSOR, 
               SWOSIO_ULTRASONIC, 
               SWOSIO_MAXIOTYPE } SwOSIOType_t;

// technologies to change IO type
const SwOSIOClass_t SWOSIOCLASS[SWOSIO_MAXIOTYPE ] = {
  SWOSIOCLASS_INPUT, // SWOSIO_DIGITAL
  SWOSIOCLASS_INPUT, // SWOSIO_SWITCH
  SWOSIOCLASS_INPUT, // SWOSIO_REEDSWITCH 
  SWOSIOCLASS_INPUT, // SWOSIO_LIGHTBARRIER
  SWOSIOCLASS_SINGULAR, // SWOSIO_BUTTON         
  SWOSIOCLASS_INPUT, // SWOSIO_ANALOG
  SWOSIOCLASS_INPUT, // SWOSIO_VOLTMETER
  SWOSIOCLASS_INPUT, // SWOSIO_OHMMETER
  SWOSIOCLASS_INPUT, // SWOSIO_THERMOMETER
  SWOSIOCLASS_INPUT, // SWOSIO_LDR
  SWOSIOCLASS_SINGULAR, // SWOSIO_JOYSTICK
  SWOSIOCLASS_MOTOR, // SWOSIO_MOTOR 
  SWOSIOCLASS_MOTOR, // SWOSIO_XMOTOR
  SWOSIOCLASS_MOTOR, // SWOSIO_XMMOTOR
  SWOSIOCLASS_MOTOR, // SWOSIO_TRACTOR  
  SWOSIOCLASS_MOTOR, // SWOSIO_ENCODER 
  SWOSIOCLASS_MOTOR, // SWOSIO_LAMP
  SWOSIOCLASS_MOTOR, // SWOSIO_VALVE 
  SWOSIOCLASS_MOTOR, // SWOSIO_COMPRESSOR
  SWOSIOCLASS_MOTOR, // SWOSIO_BUZZER
  SWOSIOCLASS_SINGULAR, // SWOSIO_STEPPER
  SWOSIOCLASS_INPUT, // SWOSIO_COUNTER 
  SWOSIOCLASS_INPUT, // SWOSIO_ROTARYENCODER
  SWOSIOCLASS_INPUT, // SWOSIO_FREQUENCYMETER
  SWOSIOCLASS_SINGULAR, // SWOSIO_LIDAR
  SWOSIOCLASS_SINGULAR, // SWOSIO_CAM 
  SWOSIOCLASS_SINGULAR, // SWOSIO_SERVO
  SWOSIOCLASS_SINGULAR, // SWOSIO_PIXEL
  SWOSIOCLASS_SINGULAR, // SWOSIO_OLED
  SWOSIOCLASS_SINGULAR, // SWOSIO_I2C
  SWOSIOCLASS_SINGULAR, // SWOSIO_GYRO
  SWOSIOCLASS_SINGULAR, // SWOSIO_HC165
  SWOSIOCLASS_SINGULAR, // SWOSIO_POWER
  SWOSIOCLASS_INPUT, // SWOSIO_COLORSENSOR
  SWOSIOCLASS_INPUT, // SWOSIO_TRAILSENSOR
  SWOSIOCLASS_INPUT // SWOSIO_ULTRASONIC
} ;  

// show via api?
const bool SHOWIOINAPI[SWOSIO_MAXIOTYPE ] = {
  true, // SWOSIO_DIGITAL
  true, // SWOSIO_SWITCH
  true, // SWOSIO_REEDSWITCH 
  true, // SWOSIO_LIGHTBARRIER
  true, // SWOSIO_BUTTON         
  true, // SWOSIO_ANALOG
  true, // SWOSIO_VOLTMETER
  true, // SWOSIO_OHMMETER
  true, // SWOSIO_THERMOMETER
  true, // SWOSIO_LDR
  true, // SWOSIO_JOYSTICK
  true, // SWOSIO_MOTOR 
  true, // SWOSIO_XMOTOR
  true, // SWOSIO_XMMOTOR
  true, // SWOSIO_TRACTOR  
  true, // SWOSIO_ENCODER 
  true, // SWOSIO_LAMP
  true, // SWOSIO_VALVE 
  true, // SWOSIO_COMPRESSOR
  true, // SWOSIO_BUZZER
  true, // SWOSIO_STEPPER
  true, // SWOSIO_COUNTER 
  true, // SWOSIO_ROTARYENCODER
  true, // SWOSIO_FREQUENCYMETER
  true, // SWOSIO_LIDAR
  false, // SWOSIO_CAM 
  true, // SWOSIO_SERVO
  true, // SWOSIO_PIXEL
  false, // SWOSIO_OLED
  false, // SWOSIO_I2C
  false, // SWOSIO_GYRO
  false, // SWOSIO_HC165
  true, // SWOSIO_POWER
  true, // SWOSIO_COLORSENSOR
  true, // SWOSIO_TRAILSENSOR
  true // SWOSIO_ULTRASONIC
} ;  

// show via api?
const char SWOSIOTYPE[SWOSIO_MAXIOTYPE][20] = {
  "DigitalInput",
  "Switch",
  "Reedswitch",
  "Lightbarrier",
  "Button",
  "Analog",
  "Voltmeter",
  "Ohmmeter",
  "Thermometer",
  "LDR",
  "Joystick",
  "Motor",
  "XMotor",
  "XMMotor",
  "Tractor",
  "Encoder",
  "Lamp",
  "Valve",
  "Compressor",
  "Buzzer",
  "Stepper",
  "Counter",
  "Rotaryencoder",
  "Frequencymeter",
  "Lidar",
  "Cam",
  "Servo",
  "Pixel",
  "OLED",
  "I2C",
  "Gyro",
  "HC165",
  "Power",
  "Colorsensor",
  "Trailsensor",
  "Ultrasonic"
} ;  

// HW versions
typedef enum { 
  FTSWARM_NOVERSION = -1, 
  FTSWARMJST_1V0, 
  FTSWARMCONTROL_1V3, 
  FTSWARMJST_1V15, 
  FTSWARMRS_2V0, 
  FTSWARMRS_2V1,
  FTSWARMCAM_3V12,
  FTSWARMDUINO_1V141,
  FTSWARMPWRDRIVE_1V141,
  FTSWARMXL_1V00,
  FTSWARMRC_1V140,
  FTSWARMMAXVERSION } FtSwarmVersion_t;

// how to move
typedef enum { FTSWARM_COAST, FTSWARM_BRAKE, FTSWARM_ON, FTSWARM_MAXMOTION } FtSwarmMotion_t;

// toggles
typedef enum { FTSWARM_NOTOGGLE, FTSWARM_TOGGLEUP, FTSWARM_TOGGLEDOWN } FtSwarmToggle_t;

// alignment
typedef enum { FTSWARM_ALIGNLEFT, FTSWARM_ALIGNCENTER, FTSWARM_ALIGNRIGHT } FtSwarmAlign_t;

// Gyro types
typedef enum { FTSWARM_GYRO_OFF, FTSWARM_GYRO_LSM, FTSWARM_GYRO_MPU } FtSwarmGyroMode_t;

// Ext Port modes
typedef enum { FTSWARM_EXT_OFF, FTSWARM_EXT_I2C_MASTER, FTSWARM_EXT_I2C_SLAVE, FTSWARM_EXT_OUTPUT, FTSWARM_EXT_SERVO, FTSWARM_EXT_LIDAR } FtSwarmExtMode_t;

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

#define MAXSPEED256  255 
#define MAXSPEED4096 4095 

// **** port definitions ****

// buttons
#define FTSWARM_S1 0
#define FTSWARM_S2 1
#define FTSWARM_S3 2
#define FTSWARM_S4 3
#define FTSWARM_F1 4
#define FTSWARM_F2 5
#define FTSWARM_J1 6
#define FTSWARM_J2 7

const char BUTTON[8][3] = { "S1", "S2", "S3", "S4", "F1", "F2", "J1", "J2" };

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
#define FTSWARM_M5 4
#define FTSWARM_M6 5 
#define FTSWARM_M7 6 
#define FTSWARM_M8 7 

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
#define FTSWARM_SERVO3 2
#define FTSWARM_SERVO4 3

// **** some internal types & classes, don't use them at all ****

class SwOSQuaternion {
  public:
      float w;
      float x;
      float y;
      float z;
      
      SwOSQuaternion() {
          w = 1.0f;
          x = 0.0f;
          y = 0.0f;
          z = 0.0f;
      }

      SwOSQuaternion(int16_t *data, const uint8_t* packet) {
        if (!packet) return;
        data[0] = (((int16_t)packet[0] << 8)  | (int16_t)packet[1]);
        data[1] = (((int16_t)packet[4] << 8)  | (int16_t)packet[5]);
        data[2] = (((int16_t)packet[8] << 8)  | (int16_t)packet[9]);
        data[3] = (((int16_t)packet[12] << 8) | (int16_t)packet[13]);
    }
      
      SwOSQuaternion(float nw, float nx, float ny, float nz) {
          w = nw;
          x = nx;
          y = ny;
          z = nz;
      }

      SwOSQuaternion getProduct(SwOSQuaternion q) {
          // Quaternion multiplication is defined by:
          //     (Q1 * Q2).w = (w1w2 - x1x2 - y1y2 - z1z2)
          //     (Q1 * Q2).x = (w1x2 + x1w2 + y1z2 - z1y2)
          //     (Q1 * Q2).y = (w1y2 - x1z2 + y1w2 + z1x2)
          //     (Q1 * Q2).z = (w1z2 + x1y2 - y1x2 + z1w2
          return SwOSQuaternion(
              w*q.w - x*q.x - y*q.y - z*q.z,  // new w
              w*q.x + x*q.w + y*q.z - z*q.y,  // new x
              w*q.y - x*q.z + y*q.w + z*q.x,  // new y
              w*q.z + x*q.y - y*q.x + z*q.w); // new z
      }

      SwOSQuaternion getConjugate() {
          return SwOSQuaternion(w, -x, -y, -z);
      }
      
      float getMagnitude() {
          return sqrt(w*w + x*x + y*y + z*z);
      }
      
      void normalize() {
          const float im = 1.0f / getMagnitude();
          w *= im;
          x *= im;
          y *= im;
          z *= im;
      }
      
      SwOSQuaternion getNormalized() {
        SwOSQuaternion r(w, x, y, z);
          r.normalize();
          return r;
      }
};

class SwOSPID {

  public:
  
    float kp;
    float ki;
    float kd;
    float max_integral;
    float min_integral;
    float max_output;
    float min_output;
    float integral = 0;
    float last_error = 0;

    SwOSPID( float kp, float ki, float kd, float min_integral, float max_integral, float min_output, float max_output );
    float solve( float desired_value, float sensor );
    void reset( float sensor ) { integral = 0; last_error = 0; };

};

// handle/pointer to a swarm IO, used by FtSwarmIO base class
typedef void* SwOSIOHandle_t;      

class FtSwarmIO {
  // base class for all ftSwarm interfaces, don't use this class at all
  
  protected:
    
    // constructors
    FtSwarmIO( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType);
    FtSwarmIO( FtSwarmSerialNumber_t serialNumber, SwOSIOType_t ioType);
    FtSwarmIO( const char *name, SwOSIOType_t ioType );

    // destructor
    ~FtSwarmIO();

  public:
    bool isOnline();          // check, if I'm online
    SwOSIOHandle_t me = NULL; // pointer to my swarm HW, don't use!
};

class FtSwarmSensor : public FtSwarmIO {
  // an input base class, don't use this class at all
  
  protected:
    FtSwarmSensor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType );
    FtSwarmSensor( const char *name, SwOSIOType_t ioType );

  public:
    void onTrigger( FtSwarmTrigger_t triggerEvent, FtSwarmIO *actor, int32_t p1 ); 
    void onTrigger( FtSwarmTrigger_t triggerEvent, FtSwarmIO *actor ); 
};

// **** input / actor classes to use in your sketch ****

class FtSwarmDigitalInput : public FtSwarmSensor {
  // digital inputs. ports A1..A4, all controller types

  protected:
    FtSwarmDigitalInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType, bool normallyOpen = true);
    FtSwarmDigitalInput( const char *name, SwOSIOType_t ioType, bool normallyOpen );
  
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
  // all kind of mechanical switches, A1..A4 all controllers
  // fischertechnik switches: 1-3 is normally open, 1-2 is normally closed
  public:
    FtSwarmSwitch( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen = true);
    FtSwarmSwitch( const char *name, bool normallyOpen = true );
};

class FtSwarmReedSwitch : public FtSwarmDigitalInput {
  // reed switches, A1..A4 all controllers
  public:
    FtSwarmReedSwitch( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen = true);
    FtSwarmReedSwitch( const char *name, bool normallyOpen = true);
};

class FtSwarmLightBarrier: public FtSwarmDigitalInput {
  // photo transistor as light barrier, A1..A4 all controllers
  public:
    FtSwarmLightBarrier( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen = true);
    FtSwarmLightBarrier( const char *name, bool normallyOpen = true);
};

class FtSwarmButton : public FtSwarmDigitalInput {
  // onboard buttons, FtSwarmControl only
  public:
    FtSwarmButton( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port );
    FtSwarmButton( const char *name );
};

class FtSwarmCounter : public FtSwarmSensor {
  // counter input is available at all ports
  protected:
    FtSwarmCounter( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType );
    FtSwarmCounter( const char *name, SwOSIOType_t ioType );
  public:
    FtSwarmCounter( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port );
    FtSwarmCounter( const char *name );

    int16_t getCounter( void );
    void resetCounter( void );

};

class FtSwarmRotaryEncoder : public FtSwarmSensor {
  // rotary input is available at all ports
  protected:
    FtSwarmRotaryEncoder( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType );
    FtSwarmRotaryEncoder( const char *name, SwOSIOType_t ioType );
  public:
    FtSwarmRotaryEncoder( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port );
    FtSwarmRotaryEncoder( const char *name );

    int16_t getCounter( void );
    void resetCounter( void );
};

class FtSwarmFrequencymeter : public FtSwarmSensor {
  // FtSwarmFrequencymeter is available at all input ports
  public:
    FtSwarmFrequencymeter( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port );
    FtSwarmFrequencymeter( const char *name );
    virtual int16_t getFrequency( void );

};

class FtSwarmAnalogInput : public FtSwarmSensor { 
  // general analog input, A1..A4 ftSwarm only
  protected:
    FtSwarmAnalogInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType);
    FtSwarmAnalogInput( const char *name, SwOSIOType_t ioType );

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

class FtSwarmMotor : public FtSwarmIO {
  // general motor class, use this class for (old) gray motors, mini motors, XS motors
  // M1..M2 all contollers - keep power budget in mind!
  protected:
    FtSwarmMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType, bool highResolution );
    FtSwarmMotor( const char *name, SwOSIOType_t ioType, bool highResolution );
  public:
    FtSwarmMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool highResolution = false ):FtSwarmMotor( serialNumber, port, SWOSIO_MOTOR, highResolution ) {};
    FtSwarmMotor( const char *name, bool highResolution = false ):FtSwarmMotor( name, SWOSIO_MOTOR, highResolution ) {};  
    void     setSpeed( int16_t speed );                                // speed +/- 255 or +/-4095 dependend on resolution, speed 0 motor stopss
    uint16_t getSpeed();                                               // actual speed
    void     setAcceleration( uint32_t rampUpT,  uint32_t rampUpY );  
    void     getAcceleration( uint32_t *rampUpT, uint32_t *rampUpY );
    
};

class FtSwarmTractorMotor : public FtSwarmMotor {
  // tractor & XM motor
  // M1..M2 all contollers - keep power budget in mind!
  protected:
    FtSwarmTractorMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType, bool highResolution );
    FtSwarmTractorMotor( const char * name, SwOSIOType_t ioType, bool highResolution );
      
  public:
    FtSwarmTractorMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool highResolution = false );
    FtSwarmTractorMotor( const char * name, bool highResolution = false );

    // FtSwarmTractor has different options for stopping motor:
    virtual void setMotionType( FtSwarmMotion_t motionType );
    FtSwarmMotion_t getMotionType();
    virtual void coast( void ) { setMotionType( FTSWARM_COAST ); };
    virtual void brake( void ) { setMotionType( FTSWARM_BRAKE ); };
    virtual void run(void) { setMotionType( FTSWARM_ON ); };
};

class FtSwarmXMMotor : public FtSwarmTractorMotor {
  // xm motor
  // M1..M2 all contollers - keep power budget in mind!
  public:
    FtSwarmXMMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool highResolution = false);
    FtSwarmXMMotor( const char * name, bool highResolution = false );
};

class FtSwarmEncoderMotor : public FtSwarmTractorMotor {
  // encoder motor
  // M1..M2 all contollers - keep power budget in mind!
  // TODO: implement encoder input
  public:
    FtSwarmEncoderMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool highResolution = false );
    FtSwarmEncoderMotor( const char * name, bool highResolution = false );
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


class FtSwarmOnOffActor : public FtSwarmMotor {
  // classic on/off devices
  // M1..M2 all contollers - keep power budget in mind!
  public:
    FtSwarmOnOffActor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType );
    FtSwarmOnOffActor( const char *name, SwOSIOType_t ioType );
    
    void on( int16_t power = MAXSPEED256 );
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

class FtSwarmGyro : public FtSwarmIO {
  // LSM6/MPU6050 Gyro
  public:
    FtSwarmGyro( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port );
    FtSwarmGyro( const char *name );

    void getAcceleration( float *x, float *y, float *z );
    void getQuaternion( float *w, float *x, float *y, float *z );
    void getYawPitchRoll(float *yaw, float *pitch, float *roll, bool radiants = false );
    void getEuler(float *alpha, float *beta, float *gamma, bool radiants = false );

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

    FtSwarmOLED(FtSwarmSerialNumber_t serialNumber);
    FtSwarmOLED( const char *name );
    
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
    FtSwarmCAM( FtSwarmSerialNumber_t serialNumber );
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
    void halt( void );                                     // stop all actors
    bool waitOnUserEvent( int parameter[10], TickType_t xTicksToWait = 512 );
    bool sendEventData( uint8_t *buffer, size_t size );
    bool IOAvaliable( const char *name ) { return false; };
};



// There is one only
extern FtSwarm ftSwarm;

extern void forever( char *prompt);

extern void forever( const char *prompt);
