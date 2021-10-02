////////////////////////////////////////////////////
//
// FtSwarmServo.h
//
// ftSwarm Servo Control
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
///////////////////////////////////////////////////

#ifndef FtSwarmServo_H
#define FtSwarmServo_H

#include "FtSwarmObj.h"

class FtSwarmServo : public FtSwarmObj {
  protected:
    int16_t _offset;                                   // internal stored offset
  public:
    FtSwarmServo( char* UUID );                        // constructor with implicit offset 
    FtSwarmServo( char* UUID, int16_t offset );        // constructor with explicit offset
    virtual char* classname();                         // my class name as string
    void setPosition( int16_t position );              // set position
    int16_t getPosition( void );                       // get position
    
};

#endif
