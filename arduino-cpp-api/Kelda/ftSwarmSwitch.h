//////////////////////////////////////////////////////////////////
//
// ftSwarmSwitch.h
//
// ftSwarm Switch Control
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
/////////////////////////////////////////////////////////////////

#ifndef FTSWARMSWITCH_H
#define FTSWARMSWITCH_H

#include "ftSwarmObj.h"
#include <arduino.h>

#define FTSWARM_S1 0
#define FTSWARM_S2 1
#define FTSWARM_S3 2
#define FTSWARM_S4 3
#define FTSWARM_F1 4
#define FTSWARM_F2 5
#define FTSWARM_J1 6
#define FTSWARM_J2 7

enum ftSwarmSwitchState  { RELEASED = 0, PRESSED = 1, UNKNOWN = 255};

class ftSwarmSwitch : public ftSwarmObj {
  protected:
    uint8_t getHC165();
  public:
    ftSwarmSwitch( char* UUID, uint8_t switchPort );
    ftSwarmSwitchState getState( );
    boolean isPressed( );
    boolean isReleased( );
};

#endif
