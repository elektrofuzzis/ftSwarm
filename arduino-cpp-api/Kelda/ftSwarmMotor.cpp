////////////////////////////////////////////////////
//
// ftSwarmMotor.h
//
// ftSwarm Motor Class
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
///////////////////////////////////////////////////

#include "ftSwarmMotor.h"

#define PWM_M1 0
#define PWM_M2 1

ftSwarmMotor::ftSwarmMotor( char* UUID, uint8_t motor ) : ftSwarmObj( UUID, motor ) {

  if ( isLocal() ) {

    _sleep = IO5;
      
    switch (_Port) {
      case 0:
        _in1 = IO23;
        _in2 = IO4;
        _pwmChannel = PWM_M1;
        break;    
      case 1:
        _in1 = IO2;
        _in2 = IO0;
        _pwmChannel = PWM_M2;
        break;    
      default:
        _lastError = NO_PORT; 
        return;
        break;
    }

    // set pinModes
    pinMode( _sleep, OUTPUT );
    pinMode( _in1, OUTPUT );
    pinMode( _in2, OUTPUT );
  
    // set pwm
    ledcSetup( _pwmChannel, 5000, 8);

    // set sleepmode
    sleep();

  } else {

  }
  
}

ftSwarmError ftSwarmMotor::cmd( ftSwarmMCmd cmd, uint8_t speed ) {
  
  if (isLocal()) {

    _lastError = FTSWARM_OK;

    switch (cmd) {
      
      case SLEEP:
        // SLEEP LOW, IN1 X, IN2 X
        digitalWrite( _sleep, LOW );
        break;
      
      case LEFT:
        // SLEEP HIGH, IN1 PWM, IN2 HIGH
        digitalWrite( _sleep, HIGH );
        ledcAttachPin( _in1, _pwmChannel );
        ledcWrite( _pwmChannel, speed );
        digitalWrite( _in2, HIGH );
        break;
      
      case RIGHT:
        // SLEEP HIGH, IN1 HIGH, IN2 PWM
        digitalWrite( _sleep, HIGH );
        ledcAttachPin( _in2, _pwmChannel );
        ledcWrite( _pwmChannel, speed );
        digitalWrite( _in1, HIGH );
        break;
      
      case BRAKE:
        // SLEEP HIGH, IN1 HIGH, IN2 HIGH
        digitalWrite( _sleep, HIGH );
        digitalWrite( _in1, HIGH );
        digitalWrite( _in2, HIGH );
        break;        
      
      case COAST:
        // SLEEP HIGH, IN1 LOW, IN2 LOW
        digitalWrite( _sleep, HIGH );
        digitalWrite( _in1, LOW );
        digitalWrite( _in2, LOW );
        break;

      default:
        _lastError = UNKNOWN_CMD;
        break;
    }

    return _lastError;
    
  } else {
    return FTSWARM_OK;
  }
  
}

ftSwarmError ftSwarmMotor::left( uint8_t speed ) {
  return cmd( LEFT, speed );
}

ftSwarmError ftSwarmMotor::right( uint8_t speed ) {
  return cmd( RIGHT, speed );
}

ftSwarmError ftSwarmMotor::brake( ) {
  return cmd( BRAKE, 0 );
}

ftSwarmError ftSwarmMotor::coast( ) {
  return cmd( COAST, 0 );
}

ftSwarmError ftSwarmMotor::sleep( ) {
  return cmd( SLEEP, 0 );
}
