//////////////////////////////////////////////////////////////////
//
// FtSwarmButton.h
//
// ftSwarm Switch Control
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
/////////////////////////////////////////////////////////////////

#ifndef FtSwarmButton_H
#define FtSwarmButton_H

#include "FtSwarmObj.h"
#include <arduino.h>

// Port mappings

#define FTSWARM_S1 0
#define FTSWARM_S2 1
#define FTSWARM_S3 2
#define FTSWARM_S4 3
#define FTSWARM_F1 4
#define FTSWARM_F2 5
#define FTSWARM_J1 6
#define FTSWARM_J2 7

enum FtSwarmButtonState  { RELEASED = 0, PRESSED = 1, UNKNOWN = 255};

class FtSwarmButton : public FtSwarmObj {
  public:
    FtSwarmButton( char* UUID, uint8_t switchPort );
      // constructor
    virtual char* classname();
      // my class name as string
    FtSwarmButtonState getState( );
      // get switch state
    boolean isPressed( );
      // true, if button is pressed
    boolean isReleased( );
      // true, if button is released
};

#endif
