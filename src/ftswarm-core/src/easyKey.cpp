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
#include "SwOSLog.h"

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

  printf(prompt); 
  flushStdIO();
  // fflush(stdout); 

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
                      if ( easyKeyEcho ) printf("%c %c", 0x08, 0x08);
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
                        (hidden)?printf( "*" ):printf( "%c", ch );
                      }

                    }
                     
                   }
                   break;
      }

      flushStdIO();

    }

  }
  
}

int isdigitFloat( int ch ) {
  return isdigit( ch ) || ( ch == 'e') || ( ch == 'E') || ( ch == '.') || ( ch == ' ');
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

int isValidIdentifier( char *str ) {

  char *ptr = str;

  if (!isalpha( *ptr )) return false;

  while ( *ptr != '\0' ) {
    if ( !isalnum( *ptr ) ) return 0;
    ptr++;
  }

  return 1;

}

int isValidInteger( char *str ) {

  char *ptr = str;
  int  sign = 1;
  int  base = 10;

  switch (ptr[0]) {

    case '#': // RGB hex
              ptr++; 
              base = 16; 
              break;

    case '-': // Negative
              ptr++; 
              sign = -1; 
              break;

    case '+': // Positive
              ptr++; 
              break;

    case '0': // hex
              if ( ( ptr[1] == 'x' ) || ( ptr[1] == 'X' ) ) {
                ptr++;
                ptr++;
                base = 16;
              }
              break;
  }

  if ( base == 16 ) {
    while ( (*ptr != '\0') && ( isxdigit( *ptr ) ) ) ptr++;
  } else {
    while ( (*ptr != '\0') && ( isdigit( *ptr ) ) ) ptr++;
  }

  return (*ptr == '\0');

}

long xToL( char *str ) {

  char *ptr = str;
  int  sign = 1;
  int  base = 10;

  switch (ptr[0]) {

    case '#': // RGB hex
              ptr++; 
              base = 16; 
              break;

    case '-': // Negative
              ptr++; 
              sign = -1; 
              break;

    case '+': // Positive
              ptr++; 
              break;

    case '0': // hex
              if ( ( ptr[1] == 'x' ) || ( ptr[1] == 'X' ) ) {
                ptr++;
                ptr++;
                base = 16;
              }
              break;
  }

  // get number
  return strtol( ptr, NULL, base ) * sign;

}

void enterString( const char *prompt, char *s, uint16_t size, bool hidden ) {

  if (!enterSomething( prompt, s, size, hidden, isprint, NULL ) ) s[0] = '\0';
  
}

void enterString( const char *prompt, char *d, char *s, uint16_t size, bool hidden ) {

  char *input = (char *) calloc( 1, size );
  
  enterString( prompt, input, size, hidden );
  
  if (input[0] != '\0') strcpy( s, input );
  else                  strcpy( s, d );

  free(input);
  
}

void enterIdentifier( const char *prompt, char *s, uint16_t size ) {

  if (!enterSomething( prompt, s, size, false, isalnum, isValidIdentifier ) ) s[0] = '\0';
  
}

int isDigitExt( int ch ) {
  return ( isxdigit(ch) || ( ch == '+' ) || ( ch == '-' ) || ( ch == 'x' ) || ( ch == 'X' ) || ( ch == '#' ) );
}

int enterNumber( const char *prompt, int defaultValue, int minValue, int maxValue ) {

  char str[6];
  int i;

  while (1) {

    // get number and check on defaults
    if ( ( !enterSomething( prompt, str, 10, false, isDigitExt, isValidInteger ) ) || ( str[0] == '\0' ) ) {
      return defaultValue;
    } else {
      i = xToL( str );
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

char    validMenuChars[20];
uint8_t minValidMenu;
uint8_t maxValidMenu;

int isValidMenu( char *str ) {

  // empty?
  if ( str[0] == '\0' ) return 1;

  // number?
  if ( isdigit( str[0] ) ) {

    if (!isValidInteger(str)) return 0;

    // get value
    int v = atoi(str);

    // 0?
    if ( ( v == 0) && ( maxValidMenu != 0 ) ) return 0;

    // to big?
    if ( v > maxValidMenu ) return 0;

    // big enough?
    if ( v >= minValidMenu ) return 1;

    // already 2 digits?
    if (  v >= 10  ) return 0;

    // any chance?
    if ( ( ( v * 10 + minValidMenu % 10 ) >= minValidMenu ) &&
         ( ( v * 10 + maxValidMenu % 10 ) <= maxValidMenu ) ) return 1;

    // wrong number
    return 0;

  }

  // just one key!
  if (str[1] != '\0' ) return 0;

  // allowed?
  if (strchr( validMenuChars, str[0] ) ) return 1;

  // done
  return 0;
  

}

void Menu::enterString( const char *prompt, char *s, uint16_t size ) {

  // setup vaildMenuChars, minValidMenu and maxValidMenu
  minValidMenu = 255;
  maxValidMenu = 0;
  int maxChar = -1;

  for (int8_t i=0; i<=maxItem; i++ ) {
    
    if ( key[i] ) validMenuChars[++maxChar] = key[i];

    else {
      
      if ( id[i] < minValidMenu ) minValidMenu = id[i];
      if ( id[i] > maxValidMenu ) maxValidMenu = id[i];

    }

  }

  validMenuChars[++maxChar] = '\0';

  // ask user
  if (!enterSomething( prompt, s, size, false, isprint, isValidMenu ) ) s[0] = '\0';
  
}

void Menu::start( const char *prompt, uint8_t spacer, char delimiter ) { 

  maxItem = 0; 
  this->delimiter = delimiter;
  this->spacer = spacer;
  strcpy( this->prompt, (char *) prompt ); 

  // print Headline
  printf( "\n\n***** %s *****\n\n", this->prompt );

};

bool Menu::add( const char *item, int value, uint8_t id, char key ) {

  char dummy[40];
  
  sprintf( dummy, "%d", value );
  return add( item, dummy, id, key );

}

bool Menu::addF( const char *item, float value, uint8_t id, char key ) {

  char dummy[40];
  sprintf( dummy, "%f", value );

  return add( item, dummy, id, key );

}

bool Menu::add( const char *value, uint8_t id, char key ) {

  return add( "", value, id, key );

}

bool Menu::add( uint8_t id, char key ) {

  if ( maxItem >= MAXMENUITEMS ) false;

  maxItem++;
  this->id[maxItem]  = id;
  this->key[maxItem] = toupper( key );

  return true;

}

bool Menu::addExit( void ) {

  printf("\n");
  return add( "Exit", "", MENU_EXIT, 'x' );

}

bool Menu::add( const char *item, const char *value, uint8_t id, char key, bool staticDelimiter ) {

  if (!add( id, key)) return false;

  if      ( ( key != '\0' ) && ( id!=MENU_DEACTIVATED ) ) printf( "(%c)  %s", key, item );
  else if ( ( key != '\0' ) && ( id==MENU_DEACTIVATED ) ) printf( "     %s", item );
  else if ( ( key == '\0' ) && ( id==MENU_DEACTIVATED ) ) printf( "(--) %s", item );
  else                                                   printf( "(%2d) %s", maxItem, item );
  
  if ( ( value[0] != '\0' ) || ( staticDelimiter ) ) {
    printf("%c ", delimiter);
    for (uint8_t i=strlen( item ); i<spacer; i++)  printf( " " );
    printf( "%s\n", value );
  } else {
    printf("\n");
  }

  return true;

}

int8_t Menu::userChoice( void ) {

  char str[10];

  printf("\n");

  // ask user
  while (1) {

    printf(prompt);
    // ask user
    enterString( ">", str, 10 );

    // try, if it's a number
    int8_t choice = atoi( str );

    // don't accept negative user inputs
    if (choice < 0 ) continue;

    // count non-key entries
    uint8_t item = 0;

    // test all items
    for (uint8_t i=1; i<=maxItem; i++) {

      // test on a key
      if ( key[i] != '\0' ) {
        if ( ( key[i] == toupper( str[0] ) ) && ( str[1] == '\0') ) return id[i];

      // test on a number
      } else {
        item++;
        if ( choice == item ) return id[i];
      }

    }

  }

  return -1;

}

