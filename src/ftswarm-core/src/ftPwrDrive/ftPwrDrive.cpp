////////////////////////////////////////////////////
//
// ftPwrDrive Arduino Interface
//
// 01.01.2022 V0.98 / latest version
//
// (C) 2022 Christian Bergschneider & Stefan Fuss
//
// PLEASE USE AT LEAST FIRMWARE 0.98 !!!
//
///////////////////////////////////////////////////

#include "ftPwrDrive/ftPwrDrive.h"
#include "ftPwrDrive/i2cBuffer.h"
#include <Wire.h>

// ftPwrDrive Commands
#define CMD_SETWATCHDOG         0  // void setWatchdog( int32_t interval )                                              set watchdog timer

#define CMD_SETMICROSTEPMODE    1  // void setMicrostepMode( mode )                                                     set microstep mode - FILLSTEP, HALFSTEP, QUARTERSTEP, EIGTHSTEP, SIXTEENTHSTEP
#define CMD_GETMICROSTEPMODE    2  // uint8_t getMicroStepMode( void )                                                  get microstep mode - FILLSTEP, HALFSTEP, QUARTERSTEP, EIGTHSTEP, SIXTEENTHSTEP

#define CMD_SETRELDISTANCE      3  // void setRelDistance( uint8_t motor, int32_t distance )                            set a distance to go, relative to actual position
#define CMD_SETABSDISTANCE      5  // void setAbsDistance( uint8_t motor, int32_t distance )                            set a absolute distance to go
#define CMD_GETSTEPSTOGO        7  // int32_t getSetpsToGo( uint8_t motor )                                             number of needed steps to go to distance

#define CMD_SETMAXSPEED         8  // void setMaxSpeed( uint8_t axis, int32_t maxSpeed )                                set max speed
#define CMD_GETMAXSPEED         9  // int32_t getMaxSpeed( uint8_t axis );                                              get max speed

#define CMD_STARTMOVING        10  // void startMoving( uint8_t motor, boolean disableOnStop )                          start motor moving, disableOnStop disables the motor driver at the end of the movement
#define CMD_STARTMOVINGALL     11  // void startMovingAll( uint8_t maskMotor, uint8_t maskDisableOnStop )               same as StartMoving, but using uint8_t masks
#define CMD_ISMOVING           12  // boolean isMoving( uint8_t motor )                                                 check, if a motor is moving
#define CMD_ISMOVINGALL        13  // uint8_t isMovingAll(  )                                                           return value is uint8_tmask, flag 1 is motor#1, flag2 is motor #2, ...

#define CMD_GETSTATE           14  // uint8_t getState( uint8_t motor )                                                 8754321  - flag 1 motor is running, flag 2 endstop, flag 3 EMS, flag 4 HOMING

#define CMD_SETPOSITION        15  // void setPosition( uint8_t motor, int32_t position )                               set position
#define CMD_SETPOSITIONALL     16  // void setPositionAll( int32_t p1, int32_t p2, int32_t p3, int32_t p4 )             set position of all motors
#define CMD_GETPOSITION        17  // int32_t getPosition( uint8_t motor )                                              get position
#define CMD_GETPOSITIONALL     18  // (int32_t,int32_t,int32_t,int32_t) getPositionAll( void )                          get position of all motors

#define CMD_SETACCELERATION    19  // void setAcceleration( uint8_t motor, int32_t acceleration )                       set acceleration
#define CMD_GETACCELERATION    20  // int32_t getAcceleration( uint8_t motor )                                          get acceleration
#define CMD_SETACCELERATIONALL 21  // void setAccelerationAll( int32_t acc1, int32_t acc2, int32_t acc3, int32_t acc4 ) set acceleration of all motors
#define CMD_GETACCELERATIONALL 22  // (int32_t,int32_t,int32_t,int32_t) getAccelerationAll( void )                      get acceleration of all motors

#define CMD_SETSERVO           23  // void setServo( uint8_t servo, int32_t position )                                  set servo position
#define CMD_GETSERVO           24  // int32_t getServo( uint8_t servo )                                                 get servo position
#define CMD_SETSERVOALL        25  // void setServoAll( int32_t p1, int32_t p2, int32_t p3, int32_t p4 )                set all servos positions
#define CMD_GETSERVOALL        26  // (int32_t, int32_t, int32_t, int32_t) getServoAll( void )                          get all servo positions
#define CMD_SETSERVOOFFSET     27  // void setServoOffset( uint8_t servo, int32_t Offset )                              set servo offset
#define CMD_GETSERVOOFFSET     28  // int32_t getServoOffset( void )                                                    get servo offset
#define CMD_SETSERVOOFFSETALL  29  // void serServoOffsetAll( int32_t o1, int32_t o2, int32_tnt o3, int32_t o4 )        set servo offset all
#define CMD_GETSERVOOFFSETALL  30  // (int32_t, int32_t, int32_t, int32_t) getServoOffsetAll( void )                    get all servo offset
#define CMD_SETSERVOONOFF      31  // void setServoOnOff( uint8_t servo, boolean OnOff )                                set servo pin On or Off without PWM

#define CMD_HOMING             32  // void homing( uint8_t motor, int32_t maxDistance, boolean disableOnStop)           homing of motor "motor": run max. maxDistance or until endstop is reached. 

#define CMD_STOPMOVING         33  // void stopMoving( uint8_t motor )                                                  stop motor moving
#define CMD_STOPMOVINGALL      34  // void stopMovingAll( uint8_t maskMotor )                                           same as StartMoving, but using uint8_t masks

#define CMD_SETINSYNC          35  // void setInSync( unit8_t motor1, uint8_t motor2, boolean OnOff)                    set two motors running in sync

#define CMD_HOMINGOFFSET       36  // void homingOffset( unit8_t motor1, uint32_t offset )                              set offset to run during homing, after endstop is free again 
#define CMD_GETSTATEALL        37  // read all motor status
#define CMD_GETSTEPSTOGOALL    38  // read all motor status

i2cBuffer i2c;

FtPwrDrive::FtPwrDrive( uint8_t myI2CAddress, TwoWire *twi ) { 
  // // constructor
  
  i2cAddress = myI2CAddress;
  i2c.begin( twi );

}

void FtPwrDrive::Watchdog( int32_t wtime ) {
  // set wartchog timer
  i2c.sendData( i2cAddress, CMD_SETWATCHDOG, wtime );
}

void FtPwrDrive::setMicrostepMode( uint8_t mode ) {
  // set microstep mode - FILLSTEP, HALFSTEP, QUARTERSTEP, EIGTHSTEP, SIXTEENTHSTEP
  i2c.sendData( i2cAddress, (uint8_t)CMD_SETMICROSTEPMODE, mode );
}

uint8_t FtPwrDrive::getMicrostepMode( void ) {
  // get microstep mode - FILLSTEP, HALFSTEP, QUARTERSTEP, EIGTHSTEP, SIXTEENTHSTEP
  return i2c.receiveUint8_t( i2cAddress, CMD_GETMICROSTEPMODE );
}

void FtPwrDrive::setRelDistance( uint8_t motor, int32_t distance ) {
  // set a distance to go, relative to actual position
  i2c.sendData( i2cAddress, CMD_SETRELDISTANCE, motor, distance );
}

void FtPwrDrive::setRelDistanceAll( int32_t d1, int32_t d2, int32_t d3, int32_t d4 ) {
  // set a relative distance to go for all motors
  setRelDistance( M1, d1 );
  setRelDistance( M2, d2 );
  setRelDistance( M3, d3 );
  setRelDistance( M4, d4 );
}

void FtPwrDrive::setAbsDistance( uint8_t motor, int32_t distance ) {
  // set a absolute distance to go
  i2c.sendData( i2cAddress, CMD_SETABSDISTANCE, motor, distance );
}

void FtPwrDrive::setAbsDistanceAll( int32_t d1, int32_t d2, int32_t d3, int32_t d4 ) {
  // set a absolute distance to go for all motors
  setAbsDistance( M1, d1 );
  setAbsDistance( M2, d2 );
  setAbsDistance( M3, d3 );
  setAbsDistance( M4, d4 );
}

int32_t FtPwrDrive::getStepsToGo( uint8_t motor ) {
  // number of needed steps to go to distance
  return i2c.receiveInt32_t( i2cAddress, CMD_GETSTEPSTOGO, motor );
}

void FtPwrDrive::getStepsToGoAll( int32_t *d1, int32_t *d2, int32_t *d3, int32_t *d4) {
  // number of needed steps to go to distance
  i2c.receive4Int32_t( i2cAddress, CMD_GETSTEPSTOGOALL, d1, d2, d3, d4 );
}

void FtPwrDrive::setMaxSpeed( uint8_t motor, int32_t speed) {
  // set max speed
  i2c.sendData( i2cAddress, CMD_SETMAXSPEED, motor, speed );
}

int32_t FtPwrDrive::getMaxSpeed( uint8_t motor ) {
  // get max speed
  return i2c.receiveInt32_t( i2cAddress, CMD_GETMAXSPEED, motor );
}

void FtPwrDrive::startMoving( uint8_t motor, boolean disableOnStop ) {
  // start motor moving, disableOnStop disables the motor driver at the end of the movement
  i2c.sendData( i2cAddress, CMD_STARTMOVING, motor, (uint8_t) disableOnStop );
}

void FtPwrDrive::startMovingAll( uint8_t maskMotor, uint8_t maskDisableOnStop  ) {
  // same as StartMoving, but using uint8_t masks
  i2c.sendData( i2cAddress, CMD_STARTMOVINGALL, maskMotor, maskDisableOnStop );
}

void FtPwrDrive::stopMoving( uint8_t motor ) {
  // stop motor moving immediately
  i2c.sendData( i2cAddress, CMD_STOPMOVING, motor );
}

void FtPwrDrive::stopMovingAll( uint8_t maskMotor  ) {
  // same as stopMoving, but using uint8_t masks
  i2c.sendData( i2cAddress, CMD_STOPMOVINGALL, maskMotor );
}

boolean FtPwrDrive::isMoving( uint8_t motor ) {
  // check, if a motor is moving
  return i2c.receiveUint8_t( i2cAddress, CMD_ISMOVING, motor );
}
  
uint8_t FtPwrDrive::isMovingAll( void ) {
  // return value is uint8_tmask, flag 1 is motoris#1, flag2 is motor #2, 
  return i2c.receiveUint8_t( i2cAddress, CMD_ISMOVINGALL );
}

uint8_t FtPwrDrive::getState( uint8_t motor ) {
  // 8754321  - flag 1 motor is running, flag 2 endstop, flag 3 EMS, flag 4 homing
  
  uint8_t x = i2c.receiveUint8_t( i2cAddress, CMD_GETSTATE, motor );
  return x;
}

void FtPwrDrive::getStateAll( uint8_t *state1, uint8_t *state2, uint8_t *state3, uint8_t *state4 ) {
  // request all states
  i2c.receive4Uint8_t( i2cAddress, CMD_GETSTATEALL, state1, state2, state3, state4 );
  
}

boolean FtPwrDrive::endStopActive( uint8_t motor ) {
  // check, if end stop is pressed
  return i2c.receiveUint8_t( i2cAddress, CMD_GETSTATE, motor ) & 0x02;
}

boolean FtPwrDrive::emergencyStopActive( void ) {
  // check, if emergeny stop is pressed
  return i2c.receiveUint8_t( i2cAddress, CMD_GETSTATE, M1 ) & 0x04;
}

void FtPwrDrive::setPosition( uint8_t motor, int32_t position ) {
  // set position
  i2c.sendData( i2cAddress, CMD_SETPOSITION, motor, position );
}

void FtPwrDrive::setPositionAll( int32_t p1, int32_t p2, int32_t p3, int32_t p4 ) {
  // set position of all motors
  i2c.sendData( i2cAddress, CMD_SETPOSITIONALL, p1, p2, p3, p4 );
}

int32_t FtPwrDrive::getPosition( uint8_t motor ) {
  // get position
  return i2c.receiveInt32_t( i2cAddress, CMD_GETPOSITION, motor );
}

void FtPwrDrive::getPositionAll( int32_t *p1, int32_t *p2, int32_t *p3, int32_t *p4 ) {
  // get position of all motors
  i2c.receive4Int32_t( i2cAddress, CMD_GETPOSITIONALL, p1, p2, p3, p4 );
}

void FtPwrDrive::setAcceleration( uint8_t motor, int32_t acceleration ) {
  // set acceleration
  i2c.sendData( i2cAddress, CMD_SETACCELERATION, motor, acceleration );
}

void FtPwrDrive::setAccelerationAll( int32_t a1, int32_t a2, int32_t a3, int32_t a4 ) {
  // set acceleration of all motors
  i2c.sendData( i2cAddress, CMD_SETACCELERATIONALL, a1, a2, a3, a4 );
}

int32_t FtPwrDrive::getAcceleration( uint8_t motor ) {
  // get acceleration
  return i2c.receiveInt32_t( i2cAddress, CMD_GETACCELERATION, motor );
}

void FtPwrDrive::getAccelerationAll( int32_t *a1, int32_t *a2, int32_t *a3, int32_t *a4 ) {
   // get acceleration of all motors

  i2c.receive4Int32_t( i2cAddress, CMD_GETACCELERATIONALL, a1, a2, a3, a4 );
}

void FtPwrDrive::setServo( uint8_t servo, int32_t position ) {
  // set servo position
  i2c.sendData( i2cAddress, CMD_SETSERVO, servo, position );
}

int32_t FtPwrDrive::getServo( uint8_t servo ) {
  // get servo position
  return i2c.receiveInt32_t( i2cAddress, CMD_GETSERVO, servo );
}

void FtPwrDrive::setServoAll( int32_t p1, int32_t p2, int32_t p3, int32_t p4 ) {
  // set all servos positions
  i2c.sendData( i2cAddress, CMD_SETSERVOALL, p1, p2, p3, p4 );
}

void FtPwrDrive::getServoAll( int32_t *p1, int32_t *p2, int32_t *p3, int32_t *p4 ) {
  // get all servo positions
  i2c.receive4Int32_t( i2cAddress, CMD_GETSERVOALL, p1, p2, p3, p4 );
}
      
void FtPwrDrive::setServoOffset( uint8_t servo, int32_t offset ) {
  // set servo offset
  i2c.sendData( i2cAddress, CMD_SETSERVOOFFSET, servo, offset );
}

int32_t FtPwrDrive::getServoOffset( uint8_t servo ) {
  // get servo offset
  return i2c.receiveInt32_t( i2cAddress, CMD_GETSERVOOFFSET, servo );
}

void FtPwrDrive::setServoOffsetAll( int32_t o1, int32_t o2, int32_t o3, int32_t o4 ) {
  // set servo offset all
  i2c.sendData( i2cAddress, CMD_SETSERVOOFFSETALL, o1, o2, o3, o4 );
}

void FtPwrDrive::getServoOffsetAll( int32_t *o1, int32_t *o2, int32_t *o3, int32_t *o4 ) {
  // get all servo offset
  i2c.receive4Int32_t( i2cAddress, CMD_GETSERVOOFFSETALL, o1, o2, o3, o4 );
}

void FtPwrDrive::setServoOnOff( uint8_t servo, boolean on ) {
  // set servo pin On or Off without PWM
  i2c.sendData( i2cAddress, CMD_SETSERVOONOFF, servo, (uint8_t) on );
}

void FtPwrDrive::homing( uint8_t motor, int32_t maxDistance, boolean disableOnStop ) {
  // homing of motor using end stop
  i2c.sendData( i2cAddress, CMD_HOMING, motor, maxDistance, disableOnStop );
}

boolean FtPwrDrive::isHoming( uint8_t motor ) {
  // check, homing is active
  return getState( motor ) & HOMING;
}

void FtPwrDrive::homingOffset( uint8_t motor, int32_t offset ) {
  // set Offset to run in homing, after endstop is free again
  i2c.sendData( i2cAddress, CMD_HOMINGOFFSET, motor, offset );
}

void FtPwrDrive::wait( uint8_t motor_mask, uint16_t interval) {
  // wait until all motors in motor_mask completed their work

  while ( isMovingAll() & motor_mask ) {
    delay( interval );
  }

}

float FtPwrDrive::setGearFactor( uint8_t motor, int32_t gear1, int32_t gear2 ) {
  // Sets the gear factor. Please read setRelDistanceR for details.
  return setGearFactor( motor, (float) gear1, (float) gear2 );
}

float FtPwrDrive::setGearFactor( uint8_t motor, float gear1, float gear2 ) {
  // Sets the gear factor. Please read setRelDistanceR for details.
 
  return gearFactor[ motorIndex(motor) ] = gear1 / gear2;
 
}

void FtPwrDrive::setRelDistanceR( uint8_t motor, float distance ) {
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

  setRelDistance( motor, gearFactor[ motorIndex( motor ) ] * 200 * distance );

}

void FtPwrDrive::setAbsDistanceR( uint8_t motor, float distance ) {
  // Sets the absolute distance in R - real units. Please read setRelDistanceR for details.

  setAbsDistance( motor, gearFactor[ motorIndex( motor ) ] * 200 * distance );

}

void FtPwrDrive::setInSync( uint8_t motor1, uint8_t motor2, boolean OnOff) {
  // set two motors running in sync

  i2c.sendData( i2cAddress, CMD_SETINSYNC, motor1, motor2, OnOff);
}

uint8_t FtPwrDrive::motorIndex( uint8_t motor ) {
  // returns the index (0..3) of a motor
  switch (motor) {
    case M1: 
      return 0;
    case M2: 
      return 1;
    case M3: 
      return 2;
    case M4: 
      return 3;
    default:
      return 0;   
  }
  
}

void FtPwrDrive::read( void ) {
  
  // I'm alive
  Watchdog( 500 );

  // get input & motor states
  getStateAll( &lastState[0], &lastState[1], &lastState[2], &lastState[3] );
  getPositionAll( &lastPosition[0], &lastPosition[1], &lastPosition[2], &lastPosition[3] );
  getStepsToGoAll( &lastDistance[0], &lastDistance[1], &lastDistance[2], &lastDistance[3] );

  error = i2c.error;
  
}

uint8_t FtPwrDrive::getError( void ) {
    return error;
}