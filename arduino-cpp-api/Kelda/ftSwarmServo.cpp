//////////////////////////////////////////////////////////////////
//
// FtSwarmServo.h
//
// ftSwarm Servo Control
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
/////////////////////////////////////////////////////////////////

#include "FtSwarmServo.h"
#include "ftSwarm.h"

FtSwarmServo::FtSwarmServo( char* UUID ) : FtSwarmServo( UUID, 0x8000 ) {
  // fascade calling cobstructor with explicit offset
}

FtSwarmServo::FtSwarmServo( char* UUID, int16_t offset ) : FtSwarmObj( UUID, 0 ) {
  // constructor
    
  _offset = offset;

  if (isLocal()) {

    // Set pinMode, initialize PWM channel, attach PWM channel to SERVO
    pinMode( SERVO, OUTPUT );
    ledcSetup( SERVOPWM, 50, 16);
    ledcAttachPin( SERVO, SERVOPWM );

    // set mid postion
    setPosition(0);
  
  }
  
}

void FtSwarmServo::setPosition( int16_t position ) {
  // set position

  if (isLocal()) {
    // just a pwm command
    ledcWrite( SERVOPWM, position + _offset );
    
  } else {
    rpc( SETPOSITION, (uint32_t) position );
  }
}

int16_t FtSwarmServo::getPosition( void ) {
  // get position

  if (isLocal()) {
    // just a PWM command
    return ledcRead( SERVOPWM ) - _offset;
  } else {
    return (uint16_t) rpc( GETPOSITION );
  }
}

char* FtSwarmServo::classname() {
  return "FtSwarmServo";
}
