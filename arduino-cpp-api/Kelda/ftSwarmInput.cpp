//////////////////////////////////////////////////////////////////
//
// ftSwarmInput.h
//
// ftSwarm Input Control
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
/////////////////////////////////////////////////////////////////

#include "ftSwarmInput.h"

#define X0 IO27
#define X1 IO26
#define X2 IO25
#define X3 IO33

ftSwarmDigitalInput::ftSwarmDigitalInput( char* UUID, uint8_t inputPort ) : ftSwarmObj( UUID, inputPort ) {

  if (isLocal()) {

    _lastError = FTSWARM_OK;
    
    switch ( _Port ) {
      case 0:
        _io = X3;
        break;
      case 1:
        _io = X2;
        break;
      case 2:
        _io = X1;
        break;
      case 3:
        _io = X0;
        break;
      default:
        _lastError = NO_PORT;
        break;
    }
    pinMode( _io, INPUT );
        
  } else {
    
  }
  
}

ftSwarmInputState ftSwarmDigitalInput::getState( ) {

  if (isLocal()) {

    if ( digitalRead( _io ) ) {
      return FTSWARM_HIGH;
    } else {
      return FTSWARM_LOW;
    }
        
  } else {

    return UNKNOWN;
    
  }
  
}

boolean ftSwarmDigitalInput::isHigh( ) {
  return (getState() == FTSWARM_HIGH );
}

boolean ftSwarmDigitalInput::isLow( ) {
  return (getState() == FTSWARM_LOW );
}
