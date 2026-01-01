/*
 * SwOSCLIParameter.cpp
 *
 * describe parameters from CLI for execute-Commands! 
 * 
 * (C) 2023 Christian Bergschneider & Stefan Fuss
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "easyKey.h"
#include "SwOSCLIParameter.h"

SwOSCLIParameter::~SwOSCLIParameter(){

    if ( num ) free( num );
    if ( str ) free( str );
    if ( io  ) free( io  );
    
    num = NULL;
    str = NULL;
    io  = NULL;

};

void SwOSCLIParameter::setNumber( char *value ){

  if ( num ) free( num );

  num = (char *) malloc( strlen(value)+1 );
  strcpy( num, value );
    
};

void SwOSCLIParameter::setString( char *value ){

  if ( str ) free( str );

  int len = strlen( value );
  int pos = 0;

  // mark endorsing " to kill
  if ( ( value[0] == '"' ) && ( value[len-1] == '"' ) ) { pos=1; len-=2; }

  // copy content
  str = (char *) calloc( sizeof( char ), len+1 );
  strncpy( str, &(value[pos]), len );
     
}

void SwOSCLIParameter::setIO( SwOSIO *io ) {

  this->io = io;

}

long SwOSCLIParameter::getNumber( void ) {

  if (!num) return 0;

  return xToL( num );

};

bool SwOSCLIParameter::inRange( const char *name, int minValue, int maxValue, char *error ) {

  if ( !num ) {
    if (error) sprintf( error, "Error: parameter %s is not a number.\n", name);
    return false;
  }

  int v = getNumber();

  if ( ( v<minValue ) || ( v>maxValue ) ) {
    if (error) sprintf( error, "Error: parameter %s needs to be between %d and %d, but %d found.\n", name, minValue, maxValue, v );
    return false;
  }

  return true;

}