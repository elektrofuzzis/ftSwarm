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

#include "SwOSHW.h"
#include "SwOSCLIParameter.h"

SwOSCLIParameter::~SwOSCLIParameter(){

    if (_value) free(_value);
    _value = NULL;

};

void SwOSCLIParameter::setValue( char *value ){

    if (_value) free(_value);

    _value = (char *) malloc( strlen(value)+1 );
    strcpy( _value, value );
    
};

void SwOSCLIParameter::setValue( SwOSIO *io, char *ioName ) {

    setValue( ioName);
    _io = io;
}

int SwOSCLIParameter::getValue( void ){

    if (!_value) return 0;

    return atoi(_value);

};


  bool SwOSCLIParameter::inRange( const char *name, int minValue, int maxValue ) {

  if (!isConstant()) {
    printf("Error: parameter %s is not a number.\n", name);
    return false;
  }

  if ( (getValue()<minValue) || (getValue()>maxValue) ) {
    printf("Error: parameter %s needs to be between %d and %d, but %d found.\n", name, minValue, maxValue, getValue());
    return false;
  }

  return true;

}