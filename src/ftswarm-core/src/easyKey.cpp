/*
 * easykey.cpp
 *
 * simple keyboard input commands
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#include <stdint.h>
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

bool enterSomething( const char *prompt, char *s, uint16_t size, bool hidden, int (*validChar)( int ch ) ) {

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
                    if ( easyKeyEcho ) Serial.write('\n');
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
                   if ( easyKeyEcho ) Serial.write('\n');
                   return false;
      
        default:   if ( ( ch < 255 ) && ( validChar( ch ) ) && ( i<size-1) ) {
                     // add printable char
                     if ( easyKeyEcho ) {
                      (hidden)?Serial.write( '*' ):Serial.write( ch );
                     }
                     str[i++] = ch;
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
  return isdigit( ch ) || ( ch == '-') ;
}

void enterString( const char *prompt, char *s, uint16_t size, bool hidden ) {

  if (!enterSomething( prompt, s, size, hidden, printable ) ) s[0] = '\0';
  
}

int identifier( int ch ) {
  return isdigit(ch) || isalpha(ch);
}

void enterIdentifier( const char *prompt, char *s, uint16_t size ) {

  if (!enterSomething( prompt, s, size, false, identifier ) ) s[0] = '\0';
  
}

uint16_t enterNumber( const char *prompt, uint16_t defaultValue, uint16_t minValue, uint16_t maxValue ) {

  char str[6];
  uint16_t i;

  while (1) {

    // get number and check on defaults
    if ( ( !enterSomething( prompt, str, 6, false, isdigit ) ) || ( str[0] == '\0' ) ) {
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
    if ( ( !enterSomething( prompt, str, 10, false, isdigitExt ) ) || ( str[0] == '\0' ) ) {
      return defaultValue;
    } else {
      i = atoi( str );
    }

    // in range?
    if ( ( i >= minValue ) && ( i <= maxValue ) ) return i;

  }

}

int YN( int ch ) {
  return ( ch == 'Y' ) || ( ch == 'y' ) || ( ch == 'N' ) || ( ch == 'n' ) ;
}

bool yesNo( const char *prompt, bool defaultValue ) {

  char str[2];
  if ( (!enterSomething( prompt, str, 2, false, YN ) ) || ( strlen( str ) == 0 ) ) return defaultValue;

  return ( str[0] == 'y' ) || ( str[0] == 'Y' ) ;

}


void Menu::start( const char *prompt, uint8_t spacer ) { 

  maxItem = 0; 
  id[0]   = 0;
  this->spacer = spacer;
  strcpy( this->prompt, (char *) prompt ); 

  // print Headline
  printf( "\n\n***** %s *****\n\n", this->prompt );

};

void Menu::add( const char *item, int value, uint8_t id ) {

  char dummy[40];
  sprintf( dummy, "%d", value );
  add( item, dummy, id );
}

void Menu::add( const char *item, const char *value, uint8_t id, bool staticDelimiter ){

  maxItem++;
  printf( "(%2d) %s", maxItem, item );
  
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

  printf("\n( 0) exit\n%s", prompt);
  
  // asking user
  uint16_t choice = enterNumber( ">", maxItem+1, 0, maxItem );
  if ( choice > maxItem ) 
    return -1; 
  else 
    return id[ choice ];

}