////////////////////////////////////////////////////
//
// FtSwarmMotor.h
//
// ftSwarm Motor Class
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
///////////////////////////////////////////////////

#ifndef FtSwarmMotor_H
#define FtSwarmMotor_H

#include "FtSwarmObj.h"
#include <arduino.h>

// port numbers
#define FTSWARM_M1 0
#define FTSWARM_M2 1

// speed constants
#define MAXSPEED   255

// motor commands
enum ftSwarmMCmd  { LEFT, RIGHT, BRAKE, COAST };

class FtSwarmMotor : public FtSwarmObj {
  protected:
    uint8_t _in1;             // DRV8837 IN1
    uint8_t _in2;             // DRV8837 IN2
    uint8_t _in1Pwm, _in2Pwm; // used PWM channels
  public:
    FtSwarmMotor( char* UUID, uint8_t motor);
      // constructor
      
    ftSwarmError cmd( ftSwarmMCmd cmd, uint8_t speed );
      // run a motor cmd, i.e. LEFT 128 means to turn left with half speed
      
    ftSwarmError left( uint8_t speed );
      // turn left
      
    ftSwarmError right( uint8_t speed );
      // turn right
      
    ftSwarmError coast( );
      // set coast mode: motor isn't controlled any more
      
    ftSwarmError brake( );
      // motor stops and blocks

    char* classname();
      // my class name as string
};


#endif
