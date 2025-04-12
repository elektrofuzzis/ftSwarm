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

#define DEACTIVATED 255

void keyboardEcho( bool on );
// set on to false to disable all keyboard echo

bool anyKey( void );
// true, if a char was sent

bool yesNo( const char *prompt, bool defaultValue = false );
// write prompt and check on y/n keys. true, if Y pressed

uint16_t enterNumber( const char *prompt, uint16_t defaultValue, uint16_t minValue = 0, uint16_t maxValue = 0xFFFF );
// write a prompt and get a uint16_t reading

int32_t enterNumberI32( const char *prompt, uint16_t defaultValue, int32_t minValue = 0, int32_t maxValue = 0xFFFFFF );
// write a prompt and get a int32_t reading

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
    char     prompt[40];

  public:
    ~Menu() { if (id) free(id); };
    void   start( const char *prompt, uint8_t spacer, uint16_t maxMenuItems = 40 );
    void   add( const char *item, const char *value, uint8_t id, bool staticDelimiter = false );
    void   add( const char *item, int value, uint8_t id );
    void   addF( const char *item, float value, uint8_t id );
    int8_t userChoice( void );
};
