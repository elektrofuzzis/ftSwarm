////////////////////////////////////////////////////
//
// FtSwarmMotor.h
//
// ftSwarm Motor Class
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
///////////////////////////////////////////////////

#include "FtSwarmMotor.h"

FtSwarmMotor::FtSwarmMotor( char* UUID, uint8_t motor ) : FtSwarmObj( UUID, motor ) {

  if ( isLocal() ) {

    // define IO ports
    switch (_port) {
      case 0:
        _in1 = M1IN1;
        _in2 = M1IN2;
        break;    
      case 1:
        _in1 = M2IN1;
        _in2 = M2IN2;
        break;    
      default:
        _lastError = NO_PORT; 
        return;
        break;
    }

    // set pinModes
    pinMode( _in1, OUTPUT );
    pinMode( _in2, OUTPUT );

    // define 2 pwm channels to work with the DRV8837, base frequency is 5kHz
    _in1Pwm = _port*2;
    _in2Pwm = _port*2+1;
    ledcSetup( _in1Pwm, 5000, 8);
    ledcSetup( _in2Pwm, 5000, 8);
    ledcAttachPin( _in1, _in1Pwm );
    ledcAttachPin( _in2, _in2Pwm );
    ledcWrite( _in1Pwm, 0 );
    ledcWrite( _in2Pwm, 0 );

  } else {

  }
  
}

ftSwarmError FtSwarmMotor::cmd( ftSwarmMCmd cmd, uint8_t speed ) {
  
  if (isLocal()) {

    _lastError = FTSWARM_OK;

    switch (cmd) {
      
      case LEFT:
        // SLEEP HIGH, IN1 PWM, IN2 HIGH
        ledcWrite( _in1Pwm, speed );
        ledcWrite( _in2Pwm, 0 );
        break;
      
      case RIGHT:
        // SLEEP HIGH, IN1 HIGH, IN2 PWM
        ledcWrite( _in1Pwm, 0 );
        ledcWrite( _in2Pwm, speed );
        break;
      
      case BRAKE:
        // SLEEP HIGH, IN1 HIGH, IN2 HIGH
        ledcWrite( _in1Pwm, MAXSPEED );
        ledcWrite( _in2Pwm, MAXSPEED );
        break;        
      
      case COAST:
        // SLEEP HIGH, IN1 LOW, IN2 LOW
        ledcWrite( _in1Pwm, 0 );
        ledcWrite( _in2Pwm, 0 );
        break;

      default:
        _lastError = UNKNOWN_CMD;
        break;
    }

    return _lastError;
    
  } else {
    return (ftSwarmError) rpc( MOTORCMD, cmd, speed );
  }
  
}

ftSwarmError FtSwarmMotor::left( uint8_t speed ) {
  return cmd( LEFT, speed );
}

ftSwarmError FtSwarmMotor::right( uint8_t speed ) {
  return cmd( RIGHT, speed );
}

ftSwarmError FtSwarmMotor::brake( ) {
  return cmd( BRAKE, 0 );
}

ftSwarmError FtSwarmMotor::coast( ) {
  return cmd( COAST, 0 );
}

char* FtSwarmMotor::classname() {
  return "FtSwarmMotor";
}
