/*
 * easyMenu.h
 *
 * simple menu system
 * 
 * (C) 2026 Christian Bergschneider & Stefan Fuss
 * 
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "easyKey.h"
#include "easyMenu.h"

int Menu::isValid( char *str ) {

  // accept empty strings
  if ( strcmp( str, "" ) == 0 ) return true;

  // is there an option, which starts with str?
  for (int8_t i=0; i<=maxOption; i++ ) {
    if ( strcmp( option[i], str ) >= 0 ) return true;
  }

  // noway
  return false;

}

Menu::Menu( const char *basePrompt, const char *newPrompt, const char *header, uint8_t maxMenuItems, uint8_t spacer, char delimiter )  {

  // allocate id + option space
  this->maxMenuItems = maxMenuItems;
  id     = (int8_t *)  calloc( maxMenuItems, sizeof( int8_t ) );
  option = (char   **) calloc( maxMenuItems, sizeof( char *) );

  // go on
  begin( basePrompt, newPrompt, header, spacer, delimiter );

}

Menu::~Menu(){ 

  resetPrompt(); 
  resetOption();

  if ( id )     free( id );
  if ( option ) free( option );

};

void Menu::begin( const char *basePrompt, const char *newPrompt, const char *header, uint8_t spacer, char delimiter ) { 

  // free old stuff
  resetPrompt();
  resetOption();

  // copy parameters
  this->delimiter = delimiter;
  this->spacer = spacer;

  // allocate prompt
  uint16_t len = 3;
  if ( basePrompt ) len += strlen(basePrompt);
  if ( newPrompt )  len += strlen(newPrompt);
  this->prompt = (char *) calloc( len, sizeof( char ) );

  // build prompt
  if ( basePrompt ) {
    strcpy( this->prompt, (char *) basePrompt ); 
    strcat( this->prompt, "/" );
  }
  if ( newPrompt != NULL )strcat( this->prompt, newPrompt );

  // allocate header
  len = 2;
  if (header) len += strlen( header );
  this->header = (char *) calloc( len, sizeof( char ) );

  // build header
  if ( header ) strcpy( this->header, header );

}

void Menu::resetPrompt( void ) {

  if (prompt) free( prompt );
  if (header) free( header );

}

void Menu::resetOption( void ) {

  if (option) {
    for (int8_t i=0; i<=maxOption; i++) if ( option[i] ) free( option[i] );
  }

  maxOption = -1;

}

void Menu::start( void ) {

  resetOption();

  // print Headline
  printf( "\n\n***** %s *****\n\n", this->header );

};

bool Menu::add( const char *item, int value, int8_t id, char key ) {

  char dummy[40];
  
  sprintf( dummy, "%d", value );
  return add( item, dummy, id, key );

}

bool Menu::addF( const char *item, float value, int8_t id, char key ) {

  char dummy[40];
  sprintf( dummy, "%f", value );

  return add( item, dummy, id, key );

}

bool Menu::add( const char *value, int8_t id, char key ) {

  return add( "", value, id, key );

}

bool Menu::add( int8_t id, char key ) {

  if ( maxOption >= MAXMENUITEMS ) false;

  maxOption++;
  this->id[maxOption] = id;

  char line[80];

  // key
  if ( key != '\0' ) {
    sprintf(line, "%c", toupper( key ) );

  // number
  } else {
    sprintf( line, "%d", maxOption+1 );
  }

  option[maxOption] = (char *) calloc( strlen(line) + 1, sizeof(char) );
  strcpy( option[maxOption], line );

  return true;

}

bool Menu::addExit( void ) {

  printf("\n");
  return add( "Exit", "", MENU_EXIT, 'x' );

}

bool Menu::add( const char *item, const char *value, int8_t id, char key, bool staticDelimiter ) {

  if ( ( id != MENU_DEACTIVATED ) && ( !add( id, key ) ) ) return false;

  if      ( ( key != '\0' ) && ( id!=MENU_DEACTIVATED ) ) printf( "(%c)  %s", key, item );
  else if ( ( key != '\0' ) && ( id==MENU_DEACTIVATED ) ) printf( "     %s",  item );
  else if ( ( key == '\0' ) && ( id==MENU_DEACTIVATED ) ) printf( "(--) %s",  item );
  else                                                    printf( "(%2d) %s", maxOption+1, item );
  
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
    // enter( ">", str, 10 );
    // if (!enterSomething( ">", str, 10, false, isprint, [this](char *str) { return isValid(str); } ) ) str[0] = '\0';
    enterString( ">", str, 10, maxOption, option );

    // test all items
    for (int8_t i=0; i<=maxOption; i++) {

      if ( strcmp( option[i], str ) == 0 ) return id[i];

    }

  }

  // happy compiler
  return -1;

}