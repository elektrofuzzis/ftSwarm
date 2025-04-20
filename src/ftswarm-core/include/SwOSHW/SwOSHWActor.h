/*
 * SwOActor.h
 *
 * Actor hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include <esp_adc_cal.h>

#include "SwOS.h"
#include "SwOSHWBaseIO.h"
#include "SwOSHWBaseCtrl.h"
#include "SwOSHWAnalog.h"

/***************************************************
 *
 *   SwOSActor
 *
 ***************************************************/

class SwOSActor : public SwOSIO {
  protected:
  
    // DC motors
    gpio_num_t     _IN1 = GPIO_NUM_NC;
    gpio_num_t     _IN2 = GPIO_NUM_NC;
    ledc_channel_config_t *ledc_channel = NULL;
    uint32_t       _rampUpT = 0;
    uint32_t       _rampUpY = 0;
  
    // stepper motors
    long           _distance;
    long           _position;
    uint8_t        _pwrDriveMotor;
    bool           _isHoming;
    bool           _isRunning;
  
    // generics
    FtSwarmActor_t  _actorType = FTSWARM_MOTOR;
    FtSwarmMotion_t _motionType = FTSWARM_COAST;
    int16_t         _speed = 0;
  
    // local HW procedures
    virtual void _setupI2C(); // initializes local HW
    virtual void _setupLocal(); // initializes local HW
    virtual void _setLocalI2C();   // start moving locally
    virtual void _setLocalLHW();   // start moving locally
    virtual void setPWM( int16_t in1, int16_t in2, gpio_num_t pwm, uint32_t duty );
  
    // remote HW procedures
    virtual void _setRemote();  // start moving remotely 
  
  public:
    bool           _highResolution = false;
  
    // Constructors
    SwOSActor(const char *name, uint8_t port, SwOSCtrl *ctrl );
    ~SwOSActor( );
  
    // adminstrative stuff
    virtual FtSwarmIOType_t getIOType()  { return FTSWARM_ACTOR; };
    virtual FtSwarmActor_t  getActorType() { return _actorType; };
    virtual FtSwarmIcon_t   getIcon();
    virtual void            jsonize( JSONize *json, uint8_t id); // serialize object to JSON
    virtual void            onTrigger( int32_t value );
    virtual void            read( void );
  
    // Test, if I'm an Actor
    virtual bool            isActor( void ) { return true; };
  
    // commands
    virtual void            setActorType( FtSwarmActor_t actorType, bool highResolution, bool dontSendToRemote );    // set actor type
    virtual void            setValue( FtSwarmMotion_t motionType, int16_t speed ) { _motionType = motionType; _speed = speed; };  // set values
    virtual void            setSpeed( int16_t speed );                                 // set speed
    virtual void            apply( void );                                             // apply speed/setAcceleration/setMotionType
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
 *   SwOSBaseServo - virtual servo class
 *
 ***************************************************/

 class SwOSBaseServo : public SwOSIO {
  protected:
    int16_t _position = 0;
    int16_t _offset   = 128;

    // local HW procedures
    virtual void _setLocal() {};   // set position locally

    // remote HW procedures
    virtual void _setRemote() {};  // setPosition remotely 

  public:
    // constructor
	  SwOSBaseServo(const char *name, uint8_t port, SwOSCtrl *ctrl);

    // Test, if I'm an Actor
    virtual bool isActor( void ) { return true; }
    
    // administrative stuff
    virtual void jsonize( JSONize *json, uint8_t id);
    virtual void onTrigger( int32_t value );
	  virtual FtSwarmIOType_t getIOType() { return FTSWARM_SERVO; };
    virtual FtSwarmIcon_t getIcon() { return FTSWARM_14_SERVO; };    
    virtual void adjust( void ) {};

    // commands
	  virtual int16_t getOffset( )   { return _offset; };
	  virtual int16_t getPosition( ) { return _position; };
	  virtual void setOffset( int16_t offset, bool dontSendToRemote );
	  virtual void setPosition( int16_t position, bool dontSendToRemote );
 
  };
  
/***************************************************
 *
 *   SwOSSERVO
 *
 ***************************************************/

class SwOSServo : public SwOSBaseServo {
  protected:
    gpio_num_t      _SERVO;
	  ledc_channel_t  _channelSERVO;
    
    // local HW procedures
    virtual void _setupLocal(); // initializes local HW
    virtual void _setLocal();   // set position locally

    // remote HW procedures
    virtual void _setRemote();  // setPosition remotely 
  
  public:
    // constructor
	  SwOSServo(const char *name, uint8_t port, SwOSCtrl *ctrl);

};

/***************************************************
 *
 *   SwOSRCServo
 *
 ***************************************************/

 class SwOSRCServo : public SwOSBaseServo {
  protected:

    SwOSAnalogInput *poti  = NULL;
    SwOSActor       *motor = NULL;
    SwOSPID         *pid   = new SwOSPID( 2.0, 1, 0, 0, 100, -512, 512);
    float           target;
    
    virtual void _setLocal();       // set position locally

  public:
    // constructor
	  SwOSRCServo(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSAnalogInput *poti, SwOSActor *actor );
    ~SwOSRCServo();

    virtual void adjust( void );

};
