//////////////////////////////////////////////////////////////////
//
// ftSwarmObj.cpp
//
// ftSwarm basic class - all objects are derived from ftSwarmObj
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
/////////////////////////////////////////////////////////////////

#include "ftSwarmObj.h"

ftSwarmObj::ftSwarmObj( char* UUID, uint8_t Port ) {

  // reserve memory and copy UUID
  _UUID = (char *) calloc( strlen( UUID ) + 1, sizeof( char ) );
  strcpy( _UUID, UUID );

  // store Port
  _Port = Port;

  // erverything was fine
  _lastError = FTSWARM_OK;
}

ftSwarmError ftSwarmObj::getLastError() {
  return _lastError;
}

boolean ftSwarmObj::isLocal() {
  return ( strcmp( _UUID, "" ) == 0 );
}
