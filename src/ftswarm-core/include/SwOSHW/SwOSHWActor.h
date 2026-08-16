/*
 * SwOActor.h
 *
 * Actor hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include "SwOS.h"
#include "SwOSHW/SwOSHWBaseIO.h"
#include "SwOSHW/SwOSHWBaseCtrl.h"
#include "SwOSHW/SwOSHWAnalog.h"
#include "SwOSFilter.h"
#include "SwOSEffect.h"

// only to feed that silly compiler
class SwOSAnalogInput;

/***************************************************
 *
 *   SwOSMotor
 *
 ***************************************************/

class SwOSMotor : public SwOSIO {

  protected:

    // generics
    FtSwarmMotion_t motionType = FTSWARM_COAST;
    int16_t         speed = 0;

    // local HW procedures
    virtual void setLocal( ) = 0;
    virtual bool autoCoast( ) { return true; }

    // remote HW procedures
    virtual void setRemote() { }

  public:

    SwOSMotor(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType, uint8_t flags);

    // administrative stuff
    virtual void            setMotionType( FtSwarmMotion_t motionType );
    virtual FtSwarmMotion_t getMotionType() { return motionType; }; 
    virtual void            serialize( Serialize *serialize ); // serialize object to JSON
    virtual void            onTrigger( SwOSTriggerMath triggerMath, int32_t sensor, int32_t delta, FtSwarmTriggerParameter parameter );
    virtual void            operate( void );
    virtual bool            isMotor( void ) { return true; };
    virtual bool            isActor( void ) { return true; };
    virtual int16_t         getMaxSpeed( void );
    
    // get my raw value
    virtual int32_t getValueI32( void ) { return speed; };

    // commands
    virtual void    setSpeed( int16_t speed );
    virtual int16_t getSpeed() { return speed; };
    virtual void    apply( void );
    virtual void    halt( void ) { setSpeed(0); apply(); };
    virtual void    setValue( FtSwarmMotion_t motionType, int16_t speed ) { this->motionType = motionType; this->speed = speed; };  // set values                             // get speed
    virtual void    setAcceleration( uint32_t rampUpT,  uint32_t rampUpY ) {};
    virtual void    getAcceleration( uint32_t *rampUpT, uint32_t *rampUpY ) {};

};

/***************************************************
 *
 *   SwOSDCMotor
 *
 ***************************************************/

class SwOSDCMotor : public SwOSMotor {
  protected:
  
    // DC motors
    gpio_num_t             IN1 = GPIO_NUM_NC;
    gpio_num_t             IN2 = GPIO_NUM_NC;
    ledc_channel_config_t *ledc_channel = NULL;
    uint32_t               rampUpT = 0;
    uint32_t               rampUpY = 0;
  
    // local HW procedures
    virtual void setupLocal( void ) override;
    virtual void setLocal( void ) override;
    virtual int16_t duty( void );
    virtual void setPWM( int16_t xin1, int16_t xin2, gpio_num_t pwm, uint32_t duty );
  
    // remote HW procedures
    virtual void setRemote() override;
  
    virtual bool autoCoast( void ) { return ( ( ioType != SWOSIO_XMMOTOR ) && ( ioType != SWOSIO_TRACTOR ) && ( ioType != SWOSIO_ENCODER ) ); };

  public:
    
    // Constructors
    SwOSDCMotor(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType, uint8_t flags );
    virtual ~SwOSDCMotor( );
  
    // commands
    virtual void setAcceleration( uint32_t rampUpT,  uint32_t rampUpY );    // set acceleration ramp
    virtual void getAcceleration( uint32_t *rampUpT, uint32_t *rampUpY );   // get acceleration ramp
    
};

/***************************************************
 *
 *   SwOSLamp
 *
 ***************************************************/

class SwOSLamp : public SwOSDCMotor {

  protected:

    SwOSLampBlink *blink = nullptr;

  public:

    SwOSLamp(const char *name, uint8_t port, SwOSCtrl *ctrl, uint8_t flags ):SwOSDCMotor(name, port, ctrl, SWOSIO_LAMP, flags ) {};

    virtual void onTrigger( SwOSTriggerMath triggerMath, int32_t sensor, int32_t delta, FtSwarmTriggerParameter parameter );
    virtual void operate( void );
    virtual void serialize( Serialize *serialize );
    virtual bool isLamp( void ) { return true; }; 

    virtual void setEffect( FtSwarmTriggerParameter effect );

};

/***************************************************
 *
 *   SwOSStepper
 *
 ***************************************************/

 class SwOSStepper : public SwOSMotor {
  protected:
  
    // stepper motors
    int32_t distance = 0;
    int32_t position = 0;
    uint8_t pwrDriveMotor;
    bool    motorIsHoming;
    bool    motorIsRunning;
  
    // local HW procedures
    virtual void setLocal( void ) override;
  
    // remote HW procedures
    virtual void setRemote() override; 
  
  public:
  
    // Constructors
    SwOSStepper(const char *name, uint8_t port, SwOSCtrl *ctrl, uint8_t flags);
    virtual ~SwOSStepper( );
    virtual bool isStepper( void ) { return true; };
    virtual void operate();
    virtual void serialize( Serialize *serialize );

    // check, if state has changed to send by data to kelda
    virtual bool isDirty( void ) { return true; };

    virtual uint8_t  pushState( uint8_t *buffer );
    virtual uint8_t  popState( uint8_t *buffer );
   
    // commands
    virtual void setValue( int32_t distance, int32_t position, bool isHoming, bool isRunning );
    virtual void setDistance( int32_t distance, bool relative  );  // set a distance to go
    virtual int32_t getDistance( void );                           // get distance
    virtual void startStop( bool start );                          // start/stop motor
    virtual void setPosition( int32_t position );                  // set absolute motor position
    virtual int32_t getPosition( void );                           // get motor position
    virtual void homing( int32_t maxDistance );                    // start homing
    virtual void setHomingOffset( int32_t offset );                // set homing offset
    virtual bool isHoming( void );                                 // check if motor is in homing procedure
    virtual bool isRunning( void );                                // check if motor is running
    virtual void setIsHoming( bool isHoming );                     // used by controller during operate() to set local info
    virtual void setIsRunning( bool isRunning );                   // used by controller during operate() to set local info
    virtual void halt( void ) { startStop(false); };
    
    /*virtual void setAbsDistance(int32_t distance );              // set a absolute distance to go
    virtual int32_t getStepsToGo( void );                          // number of needed steps to go to distance
    virtual void setMaxSpeed( int32_t speed );                     // set a max speed
    virtual int32_t getMaxSpeed(void );                            // get max speed
    virtual void startMoving( boolean disableOnStop = true );      // start motor moving, disableOnStop disables the motor driver at the end of the movement
    virtual void stopMoving( void );                               // stop motor moving immediately
    virtual boolean isMoving( void );                              // check, if a motor is moving
    */
  
  };

/***************************************************
 *
 *   SwOSServo - virtual servo class
 *
 ***************************************************/

 class SwOSServo : public SwOSIO {
  protected:
    int16_t position = 0;
    int16_t offset   = 45;

    // local HW procedures
    virtual void setLocal( void ) {};   // set position locally

    // remote HW procedures
    virtual void setRemote( void );  // setPosition remotely 

  public:
    // constructor
	  SwOSServo(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType, uint8_t flags ) : SwOSIO( name, port, ctrl, ioType, flags ) {};
    
    // administrative stuff
    virtual void serialize( Serialize *serialize );
    virtual void onTrigger( SwOSTriggerMath triggerMath, int32_t sensor, int32_t delta, FtSwarmTriggerParameter parameter );
    virtual void adjust( void ) {};
    virtual bool isServo( void ) override { return true; };
    virtual bool isActor( void ) override { return true; };
    virtual int16_t getMaxPosition( void ) { return 90 - offset; };
    virtual int16_t getMinPosition( void ) { return 0 - offset; };

    // get my raw value
    virtual int32_t getValueI32( void ) { return position; };

    // commands
	  virtual int16_t getOffset( )   { return offset; };
	  virtual int16_t getPosition( ) { return position; };
	  virtual void setOffset( int16_t offset );
	  virtual void setPosition( int16_t position );

    // getNVSParameter to send nvs parameters like servo offset via createIO
    virtual uint8_t *getNVSParameter( uint8_t *size );
    
    // setNVSParameter to receive nvs parameters like servo offset via createIO
    virtual void setNVSParameter( uint8_t parameter[], uint8_t *size );
  };
  
/***************************************************
 *
 *   SwOSDigitalSERVO
 *
 ***************************************************/

class SwOSDigitalServo : public SwOSServo {
  protected:
    gpio_num_t      SERVO;
	  ledc_channel_t  channelSERVO;
    
    // local HW procedures
    virtual void setupLocal( void ) override; // initializes local HW
    virtual void setLocal( void ) override;   // set position locally
  
  public:
    // constructor
	  SwOSDigitalServo(const char *name, uint8_t port, SwOSCtrl *ctrl, uint8_t flags);

};

/***************************************************
 *
 *   SwOSRCServo
 *
 ***************************************************/

 class SwOSRCServo : public SwOSServo {
  protected:

    int16_t         minValue = 490;             // poti's value at min position
    int16_t         maxValue = 910;             // poti's value at max position  

    SwOSAnalogInput *poti   = NULL;     
    SwOSMotor       *motor  = NULL;
    SwOSPID         *pid    = NULL;
    
    int16_t         target  = FILTER_INVALID;   // FILTER_INVALID -> don't regulate
    bool            calibration = false;        // calibration ongoing? Stop regulation during calibration
    
    // local HW procedures

    // initialize local HW
    virtual void setupLocal( void ) override; 

    // set position locally
    virtual void setLocal( void ) override;       

    // get position from poti
    virtual void poti2position();           

    // calibrate local HW by setting a speed and measuring poti values
    virtual void calibrateLocal( uint8_t speed ); 

    // calibrate remote HW by sending a command to the controller
    virtual void calibrateRemote( uint8_t speed ); 

  public:
    // constructor
	  SwOSRCServo(const char *name, uint8_t port, SwOSCtrl *ctrl, uint8_t flags );
    ~SwOSRCServo();

    virtual int16_t getMaxPosition( void );

    virtual void operate( void ) override;

    virtual void calibrate( uint8_t speed );

};
