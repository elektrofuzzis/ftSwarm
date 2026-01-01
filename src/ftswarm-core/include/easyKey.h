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

#define MENU_EXIT        100
#define MENU_DEACTIVATED 101
#define MENU_NOKEY       1

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

void enterString( const char *prompt, char *d, char *s, uint16_t size, bool hidden = false );
// enterString with default value d

void enterString( const char *prompt, char *s, uint16_t size, int8_t maxItem, char *list[] );
// enterString and compate with a list of allowed values

void enterIdentifier( const char *prompt, char *s, uint16_t size );
// write a prompt and read an identifier from serial

