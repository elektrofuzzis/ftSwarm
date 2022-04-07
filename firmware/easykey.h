/*
 * easykey.h
 *
 * simple keyboard input commands
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include <stdint.h>

char simpleSelect( const char *prompt, char minChar, char maxChar );
// write prompt, test stdin on a key between minChar and maxChar.

uint8_t simpleSelectUI8( const char *prompt, char minChar, char maxChar );
// get a uint8_t via keyboard/serial

bool yesNo( const char *prompt );
// write prompt and check on y/n keys. true, if Y pressed

uint16_t enterNumber( const char *prompt );
// write a prompt and get a uint16_t reading

void enterString( const char *prompt, char *s, uint8_t maxChars, bool hidden = false );
// write a prompt and read a string from serial
