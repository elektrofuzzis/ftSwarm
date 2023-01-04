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

#include "easykey.h"
#include <HardwareSerial.h>

bool enterSomething( const char *prompt, char *s, uint16_t size, bool hidden, int (*validChar)( int ch ) ) {

  char ch;
  char *str = (char *) calloc( size, sizeof(char) );
  uint8_t i = 0;
  
  Serial.write(prompt);

  while (1) {

    if ( Serial.available()>0 ) {
      ch = Serial.read();

      switch (ch) {
      
        case '\n': strcpy( s, str );
                   free(str);
                   Serial.write('\n');
                   return true;        
      
        case '\b': 
        case 127:  if (i>0) { 
                     str[--i] = '\0'; 
                     Serial.write( 127 ); 
                   }
                   break;
      
        case '\e': free(str);
                   Serial.write('\n');
                   return false;
      
        default:   if ( ( ch < 255 ) && ( validChar( ch ) ) && ( i<size-1) ) {
                     // add printable char
                     (hidden)?Serial.write( '*' ):Serial.write( ch );
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
      i = defaultValue;
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
    if ( ( !enterSomething( prompt, str, 10, false, isdigit ) ) || ( str[0] == '\0' ) ) {
      i = defaultValue;
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
