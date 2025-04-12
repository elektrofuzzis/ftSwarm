/*
 * easykey.cpp
 *
 * simple keyboard input commands
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "easyKey.h"
#include <HardwareSerial.h>

bool easyKeyEcho = true;

void keyboardEcho( bool on ) {
  easyKeyEcho = false;
}

bool anyKey( void ) {

  bool result = Serial.available();
  
  while( Serial.available() ) { Serial.read(); delay(25); }

  return result;
  
}

bool enterSomething( const char *prompt, char *s, uint16_t size, bool hidden, int (*validChar)( int ch ), int (*validString)( char *str) ) {

  char ch;
  char *str = (char *) calloc( size, sizeof(char) );
  uint8_t i = 0;

  printf(prompt); fflush(stdout); 

  while (1) {

    // I'm alive
    delay(1);

    if ( Serial.available()>0 ) {
      ch = Serial.read();
      
      switch (ch) {
        case '\n':  break;
        case '\r':  strcpy( s, str );
                    free(str);
                    if ( easyKeyEcho ) printf("\n");
                    return true;
        case '\b': 
        case 127:   if (i>0) { 
                      str[--i] = '\0';
                      if ( easyKeyEcho ) {
                        Serial.write(0x8); 
                        Serial.write(' '); 
                        Serial.write(0x8);
                      } 
                    }
                    break;
      
        case '\e': free(str);
                   if ( easyKeyEcho ) printf("\n");
                   return false;
      
        default:   if ( ( ch < 255 ) && ( validChar( ch ) ) && ( i<size-1) ) {

  	                // add new char
                     str[i++] = ch;

                     // is the whole string ok?
                     if ( ( validString ) && ( !validString(str) ) ) {
                      // not ok: revoke char
                      str[i--] = '\0';

                    } else {
                      // ok: print char
                      if ( easyKeyEcho ) {
                        (hidden)?Serial.write( '*' ):Serial.write( ch );
                      }

                    }
                     
                   }
                   break;
      }

    }

  }
  
}

int printable( int ch ) {
  return (ch>=32) && (ch <= 126);
}

int isdigitExt( int ch ) {
  return isdigit( ch ) || ( ch == '-') || ( ch == '+');
}

int isdigitFloat( int ch ) {
  return isdigitExt( ch ) || ( ch == 'e') || ( ch == 'E') || ( ch == '.') || ( ch == ' ');
}

void skip( char **str, int (*validChar)( int ch ) ) {

  while ( **str )  {
    if ( !validChar( **str ) ) return;
    (*str)++;
  }

}

int isValidFloat( char *str ) {
  // test if the str could be later a valid float

  char *ptr = str;

  // skip optional leading chars
  skip( &ptr, isblank );

  // skip optional sign
  if ( ( *ptr ) && ( ( *ptr == '-') || ( *ptr == '+' ) ) ) ptr++;

  // skip first block iof digits
  skip( &ptr, isdigit );

  // end of string?
  if (!*ptr ) return 1;

  // comma found? continue with a second block of digits
  if (*ptr == '.') {
    ptr++;
    skip( &ptr, isdigit );
  }

  // end of string?
  if (!*ptr ) return 1;

  // optional exponent
  if ( (*ptr == 'e') || (*ptr == 'E') ) {
    ptr++;
    // sign?
    if ( (*ptr == '+') || (*ptr == '-') ) ptr++;
    // digits?
    skip( &ptr, isdigit );  
  }

  // nothing left? 
  if (!*ptr) return 1;

  // skip trailing blanks
  skip( &ptr, isblank );

  // nothing left? 
  if (!*ptr) return 1;
  
  // not a float
  return 0; 

}

void enterString( const char *prompt, char *s, uint16_t size, bool hidden ) {

  if (!enterSomething( prompt, s, size, hidden, printable, NULL ) ) s[0] = '\0';
  
}

int identifier( int ch ) {
  return isdigit(ch) || isalpha(ch);
}

void enterIdentifier( const char *prompt, char *s, uint16_t size ) {

  if (!enterSomething( prompt, s, size, false, identifier, NULL ) ) s[0] = '\0';
  
}

uint16_t enterNumber( const char *prompt, uint16_t defaultValue, uint16_t minValue, uint16_t maxValue ) {

  char str[6];
  uint16_t i;

  while (1) {

    // get number and check on defaults
    if ( ( !enterSomething( prompt, str, 6, false, isdigit, NULL ) ) || ( str[0] == '\0' ) ) {
      return defaultValue;
    } else {
      i = atoi( str );
    }

    // in range?
    if ( ( i >= minValue ) && ( i <= maxValue ) ) return i;

  }

}

int32_t enterNumberI32( const char *prompt, uint16_t defaultValue, int32_t minValue, int32_t maxValue ) {

  char str[6];
  int32_t i;

  while (1) {

    // get number and check on defaults
    if ( ( !enterSomething( prompt, str, 10, false, isdigitExt, NULL ) ) || ( str[0] == '\0' ) ) {
      return defaultValue;
    } else {
      i = atoi( str );
    }

    // in range?
    if ( ( i >= minValue ) && ( i <= maxValue ) ) return i;

  }

}

float enterNumberF( const char *prompt, float defaultValue, float minValue, float maxValue ) {

  char str[20];
  float f;

  while (1) {

    // get number and check on defaults
    if ( ( !enterSomething( prompt, str, 20, false, isdigitFloat, isValidFloat ) ) || ( str[0] == '\0' ) ) {
      return defaultValue;
    } else {
      f = atof( str );
    }

    // in range?
    if ( ( f >= minValue ) && ( f <= maxValue ) ) return f;

  }

}

int YN( int ch ) {
  return ( ch == 'Y' ) || ( ch == 'y' ) || ( ch == 'N' ) || ( ch == 'n' ) ;
}

bool yesNo( const char *prompt, bool defaultValue ) {

  char str[2];
  if ( (!enterSomething( prompt, str, 2, false, YN, NULL ) ) || ( strlen( str ) == 0 ) ) return defaultValue;

  return ( str[0] == 'y' ) || ( str[0] == 'Y' ) ;

}


void Menu::start( const char *prompt, uint8_t spacer, uint16_t maxMenuItems ) { 

  if (id) free(id);
  id = (uint8_t *) malloc( maxMenuItems );

  maxItem = 0; 
  id[0]   = 0;
  this->spacer = spacer;
  this->maxMenuItems = maxMenuItems;
  strcpy( this->prompt, (char *) prompt ); 

  // print Headline
  printf( "\n\n***** %s *****\n\n", this->prompt );

};

void Menu::add( const char *item, int value, uint8_t id ) {

  char dummy[40];
  sprintf( dummy, "%d", value );
  add( item, dummy, id );
}

void Menu::addF( const char *item, float value, uint8_t id ) {

  char dummy[40];
  sprintf( dummy, "%f", value );
  add( item, dummy, id );
}

void Menu::add( const char *item, const char *value, uint8_t id, bool staticDelimiter ){

  if ( maxItem >= maxMenuItems-2 ) return;

  maxItem++;
  if ( id==DEACTIVATED ) printf( "(--) %s", item );
  else if (maxMenuItems < 100 ) printf( "(%2d) %s", maxItem, item );
  else printf( "(%3d) %s", maxItem, item );
  
  if ( ( value[0] != '\0' ) || ( staticDelimiter ) ) {
    printf(": ");
    for (uint8_t i=strlen( item ); i<spacer; i++)  printf( " " );
    printf( "%s\n", value );
  } else {
    printf("\n");
  }

  this->id[maxItem] = id;

}

int8_t Menu::userChoice( void ) {

  if (maxMenuItems < 100 ) printf("\n( 0) exit\n%s", prompt);
  else printf("\n(  0) exit\n%s", prompt);
  
  // asking user
  uint16_t choice = enterNumber( ">", maxItem+1, 0, maxItem );
  if ( choice > maxItem ) 
    return -1; 
  else 
    return id[ choice ];

}