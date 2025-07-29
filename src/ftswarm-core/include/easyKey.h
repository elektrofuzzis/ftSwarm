/*
 * easyKey.h
 *
 * simple keyboard input commands
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include <stdint.h>
#include <climits>

#define DEACTIVATED 255

int isValidFloat( char *str );
// test, if str is a valid float number

int isValidInteger( char *str );
// test, if str is a valid decimal or hex number

int isValidIdentifier( char *str );
// test if str is a valid identifier, starting with an alpha followed by alphnums

long xToL( char *str );
// convert a str (decimal or hex) to an long value

void keyboardEcho( bool on );
// set on to false to disable all keyboard echo

bool anyKey( void );
// true, if a char was sent

bool yesNo( const char *prompt, bool defaultValue = false );
// write prompt and check on y/n keys. true, if Y pressed

int enterNumber( const char *prompt, int defaultValue, int minValue = 0, int maxValue = INT_MAX );
// write a prompt and get a int  reading

float enterNumberF( const char *prompt, float defaultValue, float minValue, float maxValue );
// write a prompt and get a float reading

void enterString( const char *prompt, char *s, uint16_t size, bool hidden = false );
// write a prompt and read a string from serial

void enterIdentifier( const char *prompt, char *s, uint16_t size );
// write a prompt and read an identifier from serial

class Menu {
  private:
    uint8_t  maxItem = 0;
    uint8_t  spacer = 0;
    uint16_t maxMenuItems = 40;
    uint8_t  *id = NULL;
    char     delimiter = ' ';
    char     prompt[40];

  public:
    ~Menu() { if (id) free(id); };
    void   start( const char *prompt, uint8_t spacer, uint16_t maxMenuItems = 40, char delimiter = ':' );
    void   add( const char *item, const char *value, uint8_t id, bool staticDelimiter = false );
    void   add( const char *item, int value, uint8_t id );
    void   add( const char *value, uint8_t id);
    void   addF( const char *item, float value, uint8_t id );
    int8_t userChoice( void );
};
