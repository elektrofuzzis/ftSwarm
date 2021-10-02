//////////////////////////////////////////////////////////////////
//
// FtSwarmObj.cpp
//
// ftSwarm basic class - all objects are derived from FtSwarmObj
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
/////////////////////////////////////////////////////////////////

#include "FtSwarmObj.h"
#include <netinet/in.h>

FtSwarmObj::FtSwarmObj( char* UUID, uint8_t port ) {

  // reserve memory and copy UUID
  _UUID = (char *) calloc( strlen( UUID ) + 1, sizeof( char ) );
  strcpy( _UUID, UUID );

  // store port
  _port = port;

  // erverything was fine
  _lastError = FTSWARM_OK;
}

ftSwarmError FtSwarmObj::getLastError() {
  return _lastError;
}

boolean FtSwarmObj::isLocal() {
  return ( strcmp( _UUID, "" ) == 0 );
}

char* FtSwarmObj::classname() {
  return "FtSwarmObj";
}

uint32_t FtSwarmObj::_rpcSend( char *cmd ) {
  // ToDo BT implementation

  return 0;
  
}

uint32_t FtSwarmObj::rpc( const char* function, uint32_t v1 ) {

  // pretty print of cmd
  char cmd[250];
  sprintf( cmd, "%s,%d,%s,%lu", classname(), _port, function, htonl( v1) );

  // send cmd
  return _rpcSend( cmd );
}

uint32_t FtSwarmObj::rpc( const char* function, uint32_t v1, uint32_t v2 ) {

  // pretty print of cmd
  char cmd[250];
  sprintf( cmd, "%s,%d,%s,%lu,%lu", classname(), _port, function, htonl( v1), htonl( v2) );

  // send cmd
  return _rpcSend( cmd );
}

uint32_t FtSwarmObj::rpc( const char* function ) {

  // pretty print of cmd
  char cmd[250];
  sprintf( cmd, "%s,%d,%s", classname(), _port, function );

  // send cmd
  return _rpcSend( cmd );
}
