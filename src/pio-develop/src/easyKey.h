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

void enterString( const char *prompt, char *s, uint16_t size, bool hidden = false );
// write a prompt and read a string from serial

void enterIdentifier( const char *prompt, char *s, uint16_t size );
// write a prompt and read an identifier from serial
