////////////////////////////////////////////////////
//
// ftSwarmMotor.h
//
// ftSwarm Motor Class
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
///////////////////////////////////////////////////

#ifndef FTSWARMMOTOR_H
#define FTSWARMMOTOR_H

#include "ftSwarmObj.h"
#include <arduino.h>

#define FTSWARM_M1 0
#define FTSWARM_M2 1

enum ftSwarmMCmd  { SLEEP, LEFT, RIGHT, BRAKE, COAST };

class ftSwarmMotor : public ftSwarmObj {
  protected:
    uint8_t _sleep;
    uint8_t _in1;
    uint8_t _in2;
    uint8_t _pwmChannel;
  public:
    ftSwarmMotor( char* UUID, uint8_t motor);
    ftSwarmError cmd( ftSwarmMCmd cmd, uint8_t speed );
    ftSwarmError left( uint8_t speed );
    ftSwarmError right( uint8_t speed );
    ftSwarmError coast( );
    ftSwarmError brake( );
    ftSwarmError sleep( );
};


#endif
