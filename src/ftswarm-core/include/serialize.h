/*
 * serialize.h
 *
 * simple framework to build a json reply for a REST based service
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */

#pragma once

#include <esp_http_server.h>

typedef enum {
  SERIALIZE_RAW,
  SERIALIZE_JSON
} SerialFormat_t;

typedef enum {
	SERIALIZE_None,
	SERIALIZE_Object,
	SERIALIZE_Array,
	SERIALIZE_Item
} SerialObject_t;

typedef enum {
  SERIALIZE_LITERAL_NULL,
  SERIALIZE_LITERAL_CTRL,
  SERIALIZE_LITERAL_IO,
  SERIALIZE_LITERAL_KELDA,
  SERIALIZE_LITERAL_TOKEN,
  SERIALIZE_LITERAL_AUTH,
  SERIALIZE_LITERAL_PROVIDED,
  SERIALIZE_LITERAL_STATE,
  SERIALIZE_LITERAL_SPEED,
  SERIALIZE_LITERAL_HIGHRESOLUTION,
  SERIALIZE_LITERAL_OFFSET,
  SERIALIZE_LITERAL_POSITION,
  SERIALIZE_LITERAL_VALUE,
  SERIALIZE_LITERAL_VALUELR,
  SERIALIZE_LITERAL_VALUEFB,
  SERIALIZE_LITERAL_NAME,
  SERIALIZE_LITERAL_ID,
  SERIALIZE_LITERAL_SERIALNUMBER,
  SERIALIZE_LITERAL_TYPE,
  SERIALIZE_LITERAL_ICON,
  SERIALIZE_LITERAL_ACTIVE,
  SERIALIZE_LITERAL_BRIGHTNESS,
  SERIALIZE_LITERAL_COLOR,
  SERIALIZE_LITERAL_QUATERNION,
  SERIALIZE_LITERAL_ACCELERATION,
  // cam only
  SERIALIZE_LITERAL_URL,
  SERIALIZE_LITERAL_FRAMESIZE,
  SERIALIZE_LITERAL_QUALITY,
  SERIALIZE_LITERAL_CONTRAST,
  SERIALIZE_LITERAL_SATURATION,
  SERIALIZE_LITERAL_HMIRROR,
  SERIALIZE_LITERAL_VFLIP,
  SERIALIZE_LITERAL_MAX
} SerialLiteral_t; 

// simple class to build a json string and send it immediately
class Serialize {
  
protected:
	SerialObject_t lastObject = SERIALIZE_None;  // which type I'm processing
  bool           noSpacer   = true;            // supress first ,
  uint32_t       ptr        = 0;
  SerialFormat_t format     = SERIALIZE_RAW;
  size_t         bufSize    = 0;

  void write( SerialLiteral_t literal );
  void write( int value );
  void writeX( uint32_t value );
  void writeBinary( uint8_t v );

public:

  char *buffer;

	Serialize( char *buffer, size_t bufSize, SerialFormat_t format );

  // reset the buffer
  void reset( void );

  void write( const char *str );

  // start and end a new object
  void startObject( SerialLiteral_t literal = SERIALIZE_LITERAL_NULL );
  void newObject( SerialObject_t object );  // start a new object and decide to write a "," 
  void endObject();

  // start and end a new array
	void startArray( SerialLiteral_t literal = SERIALIZE_LITERAL_NULL );
	void endArray();

  // items
  void item( SerialLiteral_t literal, const char *value );
  void item( SerialLiteral_t literal, int value);
  void itemX( SerialLiteral_t literal, uint32_t value);
  void item( SerialLiteral_t literal, float value, uint8_t decimalPlaces, const char *unit = NULL );
  void item( SerialLiteral_t literal, float v1, float v2, float v3, float v4 );
  void item( SerialLiteral_t literal, float v1, float v2, float v3 );
    
};