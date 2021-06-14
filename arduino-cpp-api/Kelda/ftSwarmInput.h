//////////////////////////////////////////////////////////////////
//
// ftSwarmInput.h
//
// ftSwarm Input Control
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
/////////////////////////////////////////////////////////////////

#ifndef FTSWARMINPUT_H
#define FTSWARMINPUT_H

#include "ftSwarmObj.h"
#include <arduino.h>

#define FTSWARM_A1 0
#define FTSWARM_A2 1
#define FTSWARM_A3 2
#define FTSWARM_A4 3

enum ftSwarmInputState  { FTSWARM_LOW = 0, FTSWARM_HIGH = 1, UNKNOWN = 255};

class ftSwarmDigitalInput : public ftSwarmObj {
  protected:
    uint8_t _io;
  public:
    ftSwarmDigitalInput( char* UUID, uint8_t inputPort );
    ftSwarmInputState getState( );
    boolean isHigh( );
    boolean isLow( );
};

#endif
