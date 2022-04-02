#pragma once

#include <stdint.h>

char simpleSelect( const char *prompt, char minChar, char maxChar );

uint8_t simpleSelectUI8( const char *prompt, char minChar, char maxChar );

bool yesNo( const char *prompt );

uint16_t enterNumber( const char *prompt );

void enterString( const char *prompt, char *s, uint8_t maxChars, bool hidden = false );
