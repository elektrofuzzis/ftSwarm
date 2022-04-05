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

#include "easykey.h"


char simpleSelect( const char *prompt, char minChar, char maxChar ) {

  printf(prompt);

  char c = '\0';
  while ( (c < minChar) || ( c > maxChar) ) {
    c = getchar();
  }

  printf( "%c\n", c);

  return c;

}

uint8_t simpleSelectUI8( const char *prompt, char minChar, char maxChar ) {

  char c = simpleSelect( prompt, minChar, maxChar );
  return (c - minChar)+1;
  
}

bool yesNo( const char *prompt ) {

  printf(prompt);

  char c = '\0';
  while ( (c != 'Y') &&  ( c != 'N' ) ) {
    c = toupper( getchar() );
  }

  printf( "%c\n", c);

  return ( c == 'Y' );

}

uint16_t enterNumber( const char *prompt ) {

  uint16_t i = 0;
  printf(prompt);

  char c = '0';
  while ( (c != '\n') || ( i == 0 ) ) {
    c = getchar();
    if ( isdigit(c) ) {
      putchar(c);
      i=i*10+(c-'0');
    }
  }
  printf("\n");

  return i;

}

void enterString( const char *prompt, char *s, uint8_t maxChars, bool hidden ) {

  char    ch;
  uint8_t i = 0;
  
  printf(prompt);

  while (1) {
    
    ch = getchar();

    if (ch == '\n') {
      // got "enter"
      s[i]='\0';
      putchar(ch);
      return;
      
    } else if ( ( ch >=32 ) && ( ch < 128 ) && ( i<maxChars ) ) {
      // add printable char

      if ( hidden )
        putchar( '*' );
      else
        putchar( ch );

      s[i++] = ch;
      
    }
    
  }
  
}
