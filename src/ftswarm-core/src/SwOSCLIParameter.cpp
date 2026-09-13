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

    clear();

};

void SwOSCLIParameter::clear( void ) {

    if ( num ) free( num );
    if ( str ) free( str );
    // IO objects are borrowed from the swarm, not owned by the parameter.
    
    num = nullptr;
    str = nullptr;
    io  = nullptr;

};

void SwOSCLIParameter::setNumber( char *value ){

  clear();

  num = (char *) malloc( strlen(value)+1 );
  strcpy( num, value );
    
};

void SwOSCLIParameter::setString( char *value ){

  clear();

  int len = strlen( value );
  int pos = 0;

  // mark endorsing " to kill
  if ( ( value[0] == '"' ) && ( value[len-1] == '"' ) ) { pos=1; len-=2; }

  // copy content
  str = (char *) calloc( sizeof( char ), len+1 );
  strncpy( str, &(value[pos]), len );
     
}

void SwOSCLIParameter::setIO( SwOSIO *io ) {

  clear();
  this->io = io;

}

long SwOSCLIParameter::getNumber( void ) {

  if (!num) return 0;

  return xToL( num );

};

bool SwOSCLIParameter::inRange( const char *name, int minValue, int maxValue, char *error ) {

  if ( !num ) {
    if (error) sprintf( error, TRANSLATE( "Error: parameter %s is not a number.\n", "Fehler: Parameter %s ist keine Zahl.\n" ), name);
    return false;
  }

  int v = getNumber();

  if ( ( v<minValue ) || ( v>maxValue ) ) {
    if (error) sprintf( error, TRANSLATE( "Error: parameter %s needs to be between %d and %d, but %d found.\n", "Fehler: Parameter %s muss zwischen %d und %d sein, er hat aber den Wert %d.\n" ), name, minValue, maxValue, v );
    return false;
  }

  return true;

}