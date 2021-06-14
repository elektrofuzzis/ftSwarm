//////////////////////////////////////////////////////////////////
//
// ftSwarmObj.h
//
// ftSwarm basic class - all objects are derived from ftSwarmObj
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
/////////////////////////////////////////////////////////////////

#ifndef FTSWARMOBJ_H
#define FTSWARMOBJ_H

#include "ftSwarm.h"
#include <arduino.h>

class ftSwarmObj {
  protected:
    char* _UUID;
    uint8_t _Port;
    ftSwarmError _lastError;
  public:
    ftSwarmObj( char* UUID, uint8_t Port );
    ftSwarmError getLastError();
    boolean isLocal();
};


#endif
