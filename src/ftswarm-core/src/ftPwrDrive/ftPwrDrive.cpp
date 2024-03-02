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
#define CMD_SETWATCHDOG         0  // void setWatchdog( long interval )                                  set watchdog timer

#define CMD_SETMICROSTEPMODE    1  // void setMicrostepMode( mode )                                      set microstep mode - FILLSTEP, HALFSTEP, QUARTERSTEP, EIGTHSTEP, SIXTEENTHSTEP
#define CMD_GETMICROSTEPMODE    2  // uint8_t getMicroStepMode( void )                                   get microstep mode - FILLSTEP, HALFSTEP, QUARTERSTEP, EIGTHSTEP, SIXTEENTHSTEP

#define CMD_SETRELDISTANCE      3  // void setRelDistance( uint8_t motor, long distance )                set a distance to go, relative to actual position
#define CMD_SETABSDISTANCE      5  // void setAbsDistance( uint8_t motor, long distance )                set a absolute distance to go
#define CMD_GETSTEPSTOGO        7  // long getSetpsToGo( uint8_t motor )                                 number of needed steps to go to distance

#define CMD_SETMAXSPEED         8  // void setMaxSpeed( uint8_t axis, long maxSpeed )                    set max speed
#define CMD_GETMAXSPEED         9  // long getMaxSpeed( uint8_t axis );                                  get max speed

#define CMD_STARTMOVING        10  // void startMoving( uint8_t motor, boolean disableOnStop )           start motor moving, disableOnStop disables the motor driver at the end of the movement
#define CMD_STARTMOVINGALL     11  // void startMovingAll( uint8_t maskMotor, uint8_t maskDisableOnStop )   same as StartMoving, but using uint8_t masks
#define CMD_ISMOVING           12  // boolean isMoving( uint8_t motor )                                  check, if a motor is moving
#define CMD_ISMOVINGALL        13  // uint8_t isMovingAll(  )                                            return value is uint8_tmask, flag 1 is motor#1, flag2 is motor #2, ...

#define CMD_GETSTATE           14  // uint8_t getState( uint8_t motor )                                  8754321  - flag 1 motor is running, flag 2 endstop, flag 3 EMS, flag 4 HOMING

#define CMD_SETPOSITION        15  // void setPosition( uint8_t motor, long position )                   set position
#define CMD_SETPOSITIONALL     16  // void setPositionAll( long p1, long p2, long p3, long p4 )          set position of all motors
#define CMD_GETPOSITION        17  // long getPosition( uint8_t motor )                                  get position
#define CMD_GETPOSITIONALL     18  // (long,long,long,long) getPositionAll( void )                       get position of all motors

#define CMD_SETACCELERATION    19  // void setAcceleration( uint8_t motor, long acceleration )           set acceleration
#define CMD_GETACCELERATION    20  // long getAcceleration( uint8_t motor )                              get acceleration
#define CMD_SETACCELERATIONALL 21  // void setAccelerationAll( long acc1, long acc2, long acc3, long acc4 ) set acceleration of all motors
#define CMD_GETACCELERATIONALL 22  // (long,long,long,long) getAccelerationAll( void )                   get acceleration of all motors

#define CMD_SETSERVO           23  // void setServo( uint8_t servo, long position )                      set servo position
#define CMD_GETSERVO           24  // long getServo( uint8_t servo )                                     get servo position
#define CMD_SETSERVOALL        25  // void setServoAll( long p1, long p2, long p3, long p4 )             set all servos positions
#define CMD_GETSERVOALL        26  // (long, long, long, long) getServoAll( void )                       get all servo positions
#define CMD_SETSERVOOFFSET     27  // void setServoOffset( uint8_t servo, long Offset )                  set servo offset
#define CMD_GETSERVOOFFSET     28  // long getServoOffset( void )                                        get servo offset
#define CMD_SETSERVOOFFSETALL  29  // void serServoOffsetAll( long o1, long o2, longnt o3, long o4 )     set servo offset all
#define CMD_GETSERVOOFFSETALL  30  // (long, long, long, long) getServoOffsetAll( void )                 get all servo offset
#define CMD_SETSERVOONOFF      31  // void setServoOnOff( uint8_t servo, boolean OnOff )                 set servo pin On or Off without PWM

#define CMD_HOMING             32  // void homing( uint8_t motor, long maxDistance, boolean disableOnStop) homing of motor "motor": run max. maxDistance or until endstop is reached. 

#define CMD_STOPMOVING         33  // void stopMoving( uint8_t motor )                                  stop motor moving
#define CMD_STOPMOVINGALL      34  // void stopMovingAll( uint8_t maskMotor )                           same as StartMoving, but using uint8_t masks

#define CMD_SETINSYNC          35  // void setInSync( unit8_t motor1, uint8_t motor2, boolean OnOff)    set two motors running in sync

#define CMD_HOMINGOFFSET       36  // void homingOffset( unit8_t motor1, ulong offset )                 set offset to run during homing, after endstop is free again 
#define CMD_GETSTATEALL        37  // read all motor status
#define CMD_GETSTEPSTOGOALL    38  // read all motor status

i2cBuffer i2c;

ftPwrDrive::ftPwrDrive( uint8_t myI2CAddress, int sda, int scl ) { 
  // // constructor
  i2cAddress = myI2CAddress;
  Wire.begin( sda, scl );
}

void ftPwrDrive::Watchdog( long wtime ) {
  // set wartchog timer
  i2c.sendData( i2cAddress, CMD_SETWATCHDOG, wtime );
}

void ftPwrDrive::setMicrostepMode( uint8_t mode ) {
  // set microstep mode - FILLSTEP, HALFSTEP, QUARTERSTEP, EIGTHSTEP, SIXTEENTHSTEP
  i2c.sendData( i2cAddress, (uint8_t)CMD_SETMICROSTEPMODE, mode );
}

uint8_t ftPwrDrive::getMicrostepMode( void ) {
  // get microstep mode - FILLSTEP, HALFSTEP, QUARTERSTEP, EIGTHSTEP, SIXTEENTHSTEP
  return i2c.receiveuint8_t( i2cAddress, CMD_GETMICROSTEPMODE );
}

void ftPwrDrive::setRelDistance( uint8_t motor, long distance ) {
  // set a distance to go, relative to actual position
  i2c.sendData( i2cAddress, CMD_SETRELDISTANCE, motor, distance );
}

void ftPwrDrive::setRelDistanceAll( long d1, long d2, long d3, long d4 ) {
  // set a relative distance to go for all motors
  setRelDistance( M1, d1 );
  setRelDistance( M2, d2 );
  setRelDistance( M3, d3 );
  setRelDistance( M4, d4 );
}

void ftPwrDrive::setAbsDistance( uint8_t motor, long distance ) {
  // set a absolute distance to go
  i2c.sendData( i2cAddress, CMD_SETABSDISTANCE, motor, distance );
}

void ftPwrDrive::setAbsDistanceAll( long d1, long d2, long d3, long d4 ) {
  // set a absolute distance to go for all motors
  setAbsDistance( M1, d1 );
  setAbsDistance( M2, d2 );
  setAbsDistance( M3, d3 );
  setAbsDistance( M4, d4 );
}

long ftPwrDrive::getStepsToGo( uint8_t motor ) {
  // number of needed steps to go to distance
  return i2c.receiveLong( i2cAddress, CMD_GETSTEPSTOGO, motor );
}

void ftPwrDrive::getStepsToGoAll( long *d1, long *d2, long *d3, long *d4) {
  // number of needed steps to go to distance
  i2c.receive4Long( i2cAddress, CMD_GETSTEPSTOGOALL, d1, d2, d3, d4 );
}

void ftPwrDrive::setMaxSpeed( uint8_t motor, long speed) {
  // set max speed
  i2c.sendData( i2cAddress, CMD_SETMAXSPEED, motor, speed );
}

long ftPwrDrive::getMaxSpeed( uint8_t motor ) {
  // get max speed
  return i2c.receiveLong( i2cAddress, CMD_GETMAXSPEED, motor );
}

void ftPwrDrive::startMoving( uint8_t motor, boolean disableOnStop ) {
  // start motor moving, disableOnStop disables the motor driver at the end of the movement
  i2c.sendData( i2cAddress, CMD_STARTMOVING, motor, (uint8_t) disableOnStop );
}

void ftPwrDrive::startMovingAll( uint8_t maskMotor, uint8_t maskDisableOnStop  ) {
  // same as StartMoving, but using uint8_t masks
  i2c.sendData( i2cAddress, CMD_STARTMOVINGALL, maskMotor, maskDisableOnStop );
}

void ftPwrDrive::stopMoving( uint8_t motor ) {
  // stop motor moving immediately
  i2c.sendData( i2cAddress, CMD_STOPMOVING, motor );
}

void ftPwrDrive::stopMovingAll( uint8_t maskMotor  ) {
  // same as stopMoving, but using uint8_t masks
  i2c.sendData( i2cAddress, CMD_STOPMOVINGALL, maskMotor );
}

boolean ftPwrDrive::isMoving( uint8_t motor ) {
  // check, if a motor is moving
  return i2c.receiveuint8_t( i2cAddress, CMD_ISMOVING, motor );
}
  
uint8_t ftPwrDrive::isMovingAll( void ) {
  // return value is uint8_tmask, flag 1 is motoris#1, flag2 is motor #2, 
  return i2c.receiveuint8_t( i2cAddress, CMD_ISMOVINGALL );
}

uint8_t ftPwrDrive::getState( uint8_t motor ) {
  // 8754321  - flag 1 motor is running, flag 2 endstop, flag 3 EMS, flag 4 homing
  
  uint8_t x = i2c.receiveuint8_t( i2cAddress, CMD_GETSTATE, motor );
  return x;
}

void ftPwrDrive::getStateAll( uint8_t *state1, uint8_t *state2, uint8_t *state3, uint8_t *state4 ) {
  // request all states
  i2c.receive4uint8_t( i2cAddress, CMD_GETSTATEALL, state1, state2, state3, state4 );
  
}

boolean ftPwrDrive::endStopActive( uint8_t motor ) {
  // check, if end stop is pressed
  return i2c.receiveuint8_t( i2cAddress, CMD_GETSTATE, motor ) & 0x02;
}

boolean ftPwrDrive::emergencyStopActive( void ) {
  // check, if emergeny stop is pressed
  return i2c.receiveuint8_t( i2cAddress, CMD_GETSTATE, M1 ) & 0x04;
}

void ftPwrDrive::setPosition( uint8_t motor, long position ) {
  // set position
  i2c.sendData( i2cAddress, CMD_SETPOSITION, motor, position );
}

void ftPwrDrive::setPositionAll( long p1, long p2, long p3, long p4 ) {
  // set position of all motors
  i2c.sendData( i2cAddress, CMD_SETPOSITIONALL, p1, p2, p3, p4 );
}

long ftPwrDrive::getPosition( uint8_t motor ) {
  // get position
  return i2c.receiveLong( i2cAddress, CMD_GETPOSITION, motor );
}

void ftPwrDrive::getPositionAll( long *p1, long *p2, long *p3, long *p4 ) {
  // get position of all motors
  i2c.receive4Long( i2cAddress, CMD_GETPOSITIONALL, p1, p2, p3, p4 );
}

void ftPwrDrive::setAcceleration( uint8_t motor, long acceleration ) {
  // set acceleration
  i2c.sendData( i2cAddress, CMD_SETACCELERATION, motor, acceleration );
}

void ftPwrDrive::setAccelerationAll( long a1, long a2, long a3, long a4 ) {
  // set acceleration of all motors
  i2c.sendData( i2cAddress, CMD_SETACCELERATIONALL, a1, a2, a3, a4 );
}

long ftPwrDrive::getAcceleration( uint8_t motor ) {
  // get acceleration
  return i2c.receiveLong( i2cAddress, CMD_GETACCELERATION, motor );
}

void ftPwrDrive::getAccelerationAll( long *a1, long *a2, long *a3, long *a4 ) {
   // get acceleration of all motors

  i2c.receive4Long( i2cAddress, CMD_GETACCELERATIONALL, a1, a2, a3, a4 );
}

void ftPwrDrive::setServo( uint8_t servo, long position ) {
  // set servo position
  i2c.sendData( i2cAddress, CMD_SETSERVO, servo, position );
}

long ftPwrDrive::getServo( uint8_t servo ) {
  // get servo position
  return i2c.receiveLong( i2cAddress, CMD_GETSERVO, servo );
}

void ftPwrDrive::setServoAll( long p1, long p2, long p3, long p4 ) {
  // set all servos positions
  i2c.sendData( i2cAddress, CMD_SETSERVOALL, p1, p2, p3, p4 );
}

void ftPwrDrive::getServoAll( long *p1, long *p2, long *p3, long *p4 ) {
  // get all servo positions
  i2c.receive4Long( i2cAddress, CMD_GETSERVOALL, p1, p2, p3, p4 );
}
      
void ftPwrDrive::setServoOffset( uint8_t servo, long offset ) {
  // set servo offset
  i2c.sendData( i2cAddress, CMD_SETSERVOOFFSET, servo, offset );
}

long ftPwrDrive::getServoOffset( uint8_t servo ) {
  // get servo offset
  return i2c.receiveLong( i2cAddress, CMD_GETSERVOOFFSET, servo );
}

void ftPwrDrive::setServoOffsetAll( long o1, long o2, long o3, long o4 ) {
  // set servo offset all
  i2c.sendData( i2cAddress, CMD_SETSERVOOFFSETALL, o1, o2, o3, o4 );
}

void ftPwrDrive::getServoOffsetAll( long *o1, long *o2, long *o3, long *o4 ) {
  // get all servo offset
  i2c.receive4Long( i2cAddress, CMD_GETSERVOOFFSETALL, o1, o2, o3, o4 );
}

void ftPwrDrive::setServoOnOff( uint8_t servo, boolean on ) {
  // set servo pin On or Off without PWM
  i2c.sendData( i2cAddress, CMD_SETSERVOONOFF, servo, (uint8_t) on );
}

void ftPwrDrive::homing( uint8_t motor, long maxDistance, boolean disableOnStop ) {
  // homing of motor using end stop
  i2c.sendData( i2cAddress, CMD_HOMING, motor, maxDistance, disableOnStop );
}

boolean ftPwrDrive::isHoming( uint8_t motor ) {
  // check, homing is active
  return getState( motor ) & HOMING;
}

void ftPwrDrive::homingOffset( uint8_t motor, long offset ) {
  // set Offset to run in homing, after endstop is free again
  i2c.sendData( i2cAddress, CMD_HOMINGOFFSET, motor, offset );
}

void ftPwrDrive::wait( uint8_t motor_mask, uint16_t interval) {
  // wait until all motors in motor_mask completed their work

  while ( isMovingAll() & motor_mask ) {
    delay( interval );
  }

}

float ftPwrDrive::setGearFactor( uint8_t motor, long gear1, long gear2 ) {
  // Sets the gear factor. Please read setRelDistanceR for details.
  return setGearFactor( motor, (float) gear1, (float) gear2 );
}

float ftPwrDrive::setGearFactor( uint8_t motor, float gear1, float gear2 ) {
  // Sets the gear factor. Please read setRelDistanceR for details.
 
  return gearFactor[ motorIndex(motor) ] = gear1 / gear2;
 
}

void ftPwrDrive::setRelDistanceR( uint8_t motor, float distance ) {
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

void ftPwrDrive::setAbsDistanceR( uint8_t motor, float distance ) {
  // Sets the absolute distance in R - real units. Please read setRelDistanceR for details.

  setAbsDistance( motor, gearFactor[ motorIndex( motor ) ] * 200 * distance );

}

void ftPwrDrive::setInSync( uint8_t motor1, uint8_t motor2, boolean OnOff) {
  // set two motors running in sync

  i2c.sendData( i2cAddress, CMD_SETINSYNC, motor1, motor2, OnOff);
}

uint8_t ftPwrDrive::motorIndex( uint8_t motor ) {
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
