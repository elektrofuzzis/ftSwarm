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
#include "SwOSHW/SwOSHWBaseIO.h"
#include "SwOSHW/SwOSHWBaseCtrl.h"
#include "SwOSHW/SwOSHWAnalog.h"
#include "SwOSFilter.h"

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
    virtual void setupLocal( void ) {};
    virtual void setLocal( void ) {};
    virtual bool autoCoast( void ) { return true; };

    // remote HW procedures
    virtual void setRemote() {};  

  public:

    bool highResolution = false;

    SwOSMotor(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType );

    // administrative stuff
    virtual bool            isActor( void ) { return true; };
    virtual void            setMotionType( FtSwarmMotion_t motionType );
    virtual FtSwarmMotion_t getMotionType() { return motionType; }; 
    virtual void            jsonize( JSONize *json, uint8_t id); // serialize object to JSON
    virtual void            onTrigger( int32_t value );
    virtual void            read( void );

    // commands
    virtual void    setSpeed( int16_t speed );
    virtual int16_t getSpeed() { return speed; };
    virtual void    apply( void );
    virtual void    halt( void ) { setSpeed(0); apply(); };
    virtual void    setValue( FtSwarmMotion_t motionType, int16_t speed ) { this->motionType = motionType; this->speed = speed; };  // set values                             // get speed
    virtual void    setAcceleration( uint32_t rampUpT,  uint32_t rampUpY ) {};
    virtual void    getAcceleration( uint32_t *rampUpT, uint32_t *rampUpY ) {};
    virtual void    setParameter( int32_t parameter );

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
    virtual void setupLocal( void );
    virtual void setLocal( void );
    virtual void setPWM( int16_t xin1, int16_t xin2, gpio_num_t pwm, uint32_t duty );
  
    // remote HW procedures
    virtual void setRemote();
  
    virtual bool autoCoast( void ) { return ( ( ioType != SWOSIO_XMMOTOR ) && ( ioType != SWOSIO_TRACTOR ) && ( ioType != SWOSIO_ENCODER ) ); };

  public:
    
    // Constructors
    SwOSDCMotor(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType );
    virtual ~SwOSDCMotor( );
  
    // commands
    virtual void setAcceleration( uint32_t rampUpT,  uint32_t rampUpY );    // set acceleration ramp
    virtual void getAcceleration( uint32_t *rampUpT, uint32_t *rampUpY );   // get acceleration ramp
    
  };

/***************************************************
 *
 *   SwOSStepper
 *
 ***************************************************/

 class SwOSStepper : public SwOSMotor {
  protected:
  
    // stepper motors
    long    distance;
    long    position;
    uint8_t pwrDriveMotor;
    bool    motorIsHoming;
    bool    motorIsRunning;
  
    // local HW procedures
    virtual void setLocal( void );
  
    // remote HW procedures
    virtual void setRemote(); 
  
  public:
    bool highResolution = false;
  
    // Constructors
    SwOSStepper(const char *name, uint8_t port, SwOSCtrl *ctrl);
    virtual ~SwOSStepper( );
    virtual bool isStepper( void ) { return true; };
   
    // commands
    virtual void setValue( long distance, long position, bool isHoming, bool isRunning );
    virtual void setDistance( long distance, bool relative  );  // set a distance to go
    virtual long getDistance( void );                          // get distance
    virtual void startStop( bool start );                      // start/stop motor
    virtual void setPosition( long position );                 // set absolute motor position
    virtual long getPosition( void );                          // get motor position
    virtual void homing( long maxDistance );                   // start homing
    virtual void setHomingOffset( long offset );               // set homing offset
    virtual bool isHoming( void );                             // check if motor is in homing procedure
    virtual bool isRunning( void );                            // check if motor is running
    virtual void setIsHoming( bool isHoming );                 // used by controller during read() to set local info
    virtual void setIsRunning( bool isRunning );               // used by controller during read() to set local info
    
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
 *   SwOSServo - virtual servo class
 *
 ***************************************************/

 class SwOSServo : public SwOSIO {
  protected:
    int16_t position = 0;
    int16_t offset   = 128;

    // local HW procedures
    virtual void setLocal() {};   // set position locally

    // remote HW procedures
    virtual void setRemote() {};  // setPosition remotely 

  public:
    // constructor
	  SwOSServo(const char *name, uint8_t port, SwOSCtrl *ctrl ) : SwOSIO( name, port, ctrl, SWOSIO_SERVO ) {};

    // Test, if I'm an Actor
    virtual bool isActor( void ) { return true; }
    
    // administrative stuff
    virtual void jsonize( JSONize *json, uint8_t id);
    virtual void onTrigger( int32_t value );
    virtual void adjust( void ) {};
    virtual bool isServo( void ) { return true; };

    // commands
	  virtual int16_t getOffset( )   { return offset; };
	  virtual int16_t getPosition( ) { return position; };
	  virtual void setOffset( int16_t offset );
	  virtual void setPosition( int16_t position );
 
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
    virtual void setupLocal(); // initializes local HW
    virtual void setLocal();   // set position locally

    // remote HW procedures
    virtual void setRemote();  // setPosition remotely 
  
  public:
    // constructor
	  SwOSDigitalServo(const char *name, uint8_t port, SwOSCtrl *ctrl);

};

/***************************************************
 *
 *   SwOSRCServo
 *
 ***************************************************/

 class SwOSRCServo : public SwOSServo {
  protected:

    SwOSAnalogInput *poti   = NULL;
    SwOSMotor       *motor  = NULL;
    SwOSPID         *pid    = new SwOSPID( 2.0, 1, 0, 0, 100, -512, 512);
    int16_t         target = FILTER_INVALID; // FILTER_INVALID -> don't regulate
    
    virtual void setLocal();       // set position locally

  public:
    // constructor
	  SwOSRCServo(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSAnalogInput *poti, SwOSMotor *motor );
    ~SwOSRCServo();

    virtual void adjust( void );

};
