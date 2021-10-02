//////////////////////////////////////////////////////////////////
//
// FtSwarmObj.h
//
// ftSwarm basic class - all objects are derived from FtSwarmObj
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
/////////////////////////////////////////////////////////////////

#ifndef FtSwarmObj_H
#define FtSwarmObj_H

#include "ftSwarm.h"
#include <arduino.h>

// This is a general class representing all kinds of motors and sensors.
// Just used to defne the common sense, not intended to be used directly.

class FtSwarmObj {
  protected:
    char* _UUID;                // internal UUID of the CPU board
    uint8_t _port;              // port number, i.e. 3 for AIN4
    ftSwarmError _lastError;    // last error
    uint32_t _rpcSend( char *cmd ); // send cmd o remote device
  public:
    FtSwarmObj( char* UUID, uint8_t port );    // constructor
    ftSwarmError getLastError();               // get last communication error
    boolean isLocal();                         // check, if the object is local or if its adressed remotely
    virtual char* classname();                         // my class name as string
    uint32_t rpc( const char *function, uint32_t v1 ); // send a rpc cmd
    uint32_t rpc( const char *function, uint32_t v1, uint32_t v2 ); // send a rpc cmd
    uint32_t rpc( const char *function );
};


#endif
