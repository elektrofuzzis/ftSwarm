////////////////////////////////////////////////////
//
// ftPwrDrive Arduino Interface
//
// 01.06.2025 V0.99
//
// (C) 2022 Christian Bergschneider & Stefan Fuss
//
// PLEASE USE AT LEAST FIRMWARE 0.98 !!!
//
///////////////////////////////////////////////////

#pragma once

#include <Arduino.h>
#include <Wire.h>

// some constant to make live easier:
 
// microstep modes
static const uint8_t FULLSTEP = 0, HALFSTEP = 4, QUARTERSTEP = 2, EIGTHSTEP = 6, SIXTEENTHSTEP = 7;
    
// servo numbers
static const uint8_t S1 = 0, S2 = 1, S3 = 2, S4 = 3;

// number of servos
static const uint8_t SERVOS = 4;

// enumeration of servo numbers
static const uint8_t S[ SERVOS ] = { S1, S2, S3, S4 }; // backward compatibility only

// motor numbers
static const uint8_t M1 = 1, M2 = 2, M3 = 4, M4 = 8;

// number of motors
static const uint8_t MOTORS = 4;
    
// enumeration of motor numbers
static const uint8_t M[ MOTORS ] = { M1, M2, M3, M4 };

// flags, i.e. used in getState
static const uint8_t ISMOVING = 1, ENDSTOP = 2, EMERCENCYSTOP = 4, HOMING = 8;

// gears
static const uint8_t Z10 = 10, Z12 = 12, Z15 = 15, Z20 = 20, Z30 = 30, Z40 = 40, Z58 = 58, WORMSCREW = 5;

// same constants for backward compatibility only
static const uint8_t FTPWRDRIVE_FULLSTEP = FULLSTEP, FTPWRDRIVE_HALFSTEP = HALFSTEP, FTPWRDRIVE_QUARTERSTEP = QUARTERSTEP, FTPWRDRIVE_EIGTHSTEP = EIGTHSTEP, FTPWRDRIVE_SIXTEENTHSTEP = SIXTEENTHSTEP; 
static const uint8_t FTPWRDRIVE_S1 = S1, FTPWRDRIVE_S2 = S2, FTPWRDRIVE_S3 = S3, FTPWRDRIVE_S4 = S4;
static const uint8_t FTPWRDRIVE_SERVOS = SERVOS;
static const uint8_t FTPWRDRIVE_M1 = M1, FTPWRDRIVE_M2 = M2, FTPWRDRIVE_M3 = M3, FTPWRDRIVE_M4 = M4;
static const uint8_t FTPWRDRIVE_MOTORS = MOTORS;
static const uint8_t FTPWRDRIVE_M[ MOTORS ] = { M1, M2, M3, M4 }; 
static const uint8_t FTPWRDRIVE_ISMOVING = ISMOVING, FTPWRDRIVE_ENDSTOP = ENDSTOP, FTPWRDRIVE_EMERCENCYSTOP = EMERCENCYSTOP, FTPWRDRIVE_HOMING = HOMING;

class FtPwrDrive {

  protected:
    uint8_t i2cAddress = 32;
    uint16_t error = 0;

    float gearFactor[ MOTORS ] = { 1,1,1,1 };

    uint8_t motorIndex( uint8_t motor );
     // returns the index (0..3) of a motor

  public:

    // readings from last read() cmd;
    uint8_t  lastState[4];
    int32_t     lastPosition[4];
    int32_t     lastDistance[4];
    
    FtPwrDrive( uint8_t myI2CAddress = 32, TwoWire *twi = &Wire );
      // constructor
      
    void Watchdog( int32_t w );
      // set watchog timer
      
    void setMicrostepMode( uint8_t mode );
      // set microstep mode
      // FILLSTEP, HALFSTEP, QUARTERSTEP, EIGTHSTEP, SIXTEENTHSTEP
      
    uint8_t getMicrostepMode( void );
      // get microstep mode
      // FILLSTEP, HALFSTEP, QUARTERSTEP, EIGTHSTEP, SIXTEENTHSTEP
      
    void setRelDistance( uint8_t motor, int32_t distance );
      // set a distance to go, relative to actual position
      
    void setRelDistanceAll( int32_t d1, int32_t d2, int32_t d3, int32_t d4 );
      // set a absolute distance to go for all motors

    void setAbsDistance( uint8_t motor, int32_t distance );
      // set a absolute distance to go
      
    void setAbsDistanceAll( int32_t d1, int32_t d2, int32_t d3, int32_t d4 );
      // set a absolute distance to go for all motors
      
    int32_t getStepsToGo( uint8_t motor );
      // number of needed steps to go to distance

    void getStepsToGoAll( int32_t *d1, int32_t *d2, int32_t *d3, int32_t *d4);
      // number of needed steps to go to distance

    void setMaxSpeed( uint8_t motor, int32_t speed );
      // set a max speed
      
    int32_t getMaxSpeed( uint8_t motor);
      // get max speed
      
    void startMoving( uint8_t motor, boolean disableOnStop = true );
      // start motor moving, disableOnStop disables the motor driver at the end of the movement
      
    void startMovingAll( uint8_t maskMotor, uint8_t maskDisableOnStop = M1|M2|M3|M4 );
      // same as StartMoving, but using uint8_t masks
      
    void stopMoving( uint8_t motor );
      // stop motor moving immediately
      
    void stopMovingAll( uint8_t maskMotor = M1|M2|M3|M4 );
      // same as stopMoving, but using uint8_t masks
      
    boolean isMoving( uint8_t motor );
      // check, if a motor is moving
      
    uint8_t isMovingAll( );
      // return value is uint8_tmask, flag 1 is motor#1, flag2 is motor#2, ...

    void wait( uint8_t motor_mask, uint16_t interval = 100 );
      // wait until all motors in motor_mask completed their work
      
    uint8_t getState( uint8_t motor );
      // 8754321  - flag 1 motor is running, flag 2 endstop, flag 3 EMS, flag 4 homing

    void getStateAll( uint8_t *state1, uint8_t *state2, uint8_t *state3, uint8_t *state4 );
      // request all states

    boolean endStopActive( uint8_t motor );
      // check, if end stop is pressed

    boolean emergencyStopActive( void );
      // check, if emergeny stop is pressed
      
    void setPosition( uint8_t motor, int32_t position );
      // set position
      
    void setPositionAll( int32_t p1, int32_t p2, int32_t p3, int32_t p4 );
      // set position of all motors
      
    int32_t getPosition( uint8_t motor );
      // get position
      
    void getPositionAll( int32_t *p1, int32_t *p2, int32_t *p3, int32_t *p4 );
      // get position of all motors

    // don't use acceleration yet - just stub implementation
      
    void setAcceleration( uint8_t motor, int32_t acceleration );
      // set acceleration
      
    void setAccelerationAll( int32_t a1, int32_t a2, int32_t a3, int32_t a4 );
      // set accelerationof all motors
      
    int32_t getAcceleration( uint8_t motor );
      // get acceleration

    void getAccelerationAll( int32_t *a1, int32_t *a2, int32_t *a3, int32_t *a4 );
      // get acceleration of all motors

    void setServo( uint8_t servo, int32_t position );
      // set servo position

    int32_t getServo( uint8_t servo );
      // get servo position

    void setServoAll( int32_t p1, int32_t p2, int32_t p3, int32_t p4 );
      // set all servos positions

    void getServoAll( int32_t *p1, int32_t *p2, int32_t *p3, int32_t *p4 );
      // get all servo positions
      
    void setServoOffset( uint8_t servo, int32_t offset );
      // set servo offset

    int32_t getServoOffset( uint8_t servo );
      // get servo offset

    void setServoOffsetAll( int32_t o1, int32_t o2, int32_t o3, int32_t o4 );
      // set servo offset all

    void getServoOffsetAll( int32_t *o1, int32_t *o2, int32_t *o3, int32_t *o4 );
      // get all servo offset

    void setServoOnOff( uint8_t servo, boolean on );
      // set servo pin On or Off without PWM

    void homing( uint8_t motor, int32_t maxDistance, boolean disableOnStop = true );
      // homing of motor using end stop

    boolean isHoming( uint8_t motor );
      // check, homing is active
	  
	  void homingOffset( uint8_t motor, int32_t offset );
	  // set Offset to run in homing, after endstop is free again

    float setGearFactor( uint8_t motor, int32_t gear1, int32_t gear2 );
      // Sets the gear factor. Please read setRelDistanceR for details.
      
    float setGearFactor( uint8_t motor, float gear1, float gear2 );
      // Sets the gear factor. Please read setRelDistanceR for details.

    void setRelDistanceR( uint8_t motor, float distance );
      // Sets the relative distance in R - real units.
      // To use this function, you should det your gear factor by setGearFactor, first.
      // motor - M1..M4
      // distance - relative distance in real units.
      // Example1:
      //   setGearFactor( Z10, Z40)  sets your gear to motor -> Z10 -> Z40. Every turn of your motor will devided by 4.
      //   setRelDistanceR( M1, 10 ) will move your gear 10 times. The motor will run 10 * 40 * 200 steps. (200 steps are one turn of your motor).
      // Example2:
      //   setGearFactor( 1, WORMSCREW )  sets your gear to motor -> Wormscrew. Every turn of your motor will move your linear system by 5 mm.
      //   setRelDistanceR( M1, 10 ) will move linear system by 10 mm. The motor will run 10 * 5 * 200 steps. (200 steps are one turn of your motor).

    void setAbsDistanceR( uint8_t motor, float distance );
      // Sets the absolute distance in R - real units. Please read setRelDistanceR for details.

    void setInSync( uint8_t motor1, uint8_t motor2, boolean OnOff);
      // set two motors running in sync

    void read( void );
    uint8_t getError( void );

};