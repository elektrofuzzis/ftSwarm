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

#define MAXJSONBUFFER 10240

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
  SERIALIZE_LITERAL_STATE,
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

  void write( const char *str );
  void write( SerialLiteral_t literal );
  void write( int value );
  void writeBinary( uint8_t v );

public:

  char buffer[ MAXJSONBUFFER ];

	Serialize( SerialFormat_t format = SERIALIZE_RAW );

  // reset the buffer
  void reset( void );

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
  void item( SerialLiteral_t literal, float value, uint8_t decimalPlaces, const char *unit = NULL );
  void item( SerialLiteral_t literal, float v1, float v2, float v3, float v4 );
  void item( SerialLiteral_t literal, float v1, float v2, float v3 );
  
  // write variables 
  /*
	void variable( SerialLiteral_t literal, char *value);
  void variableB( SerialLiteral_t literal, bool b );
  void variableUI8( SerialLiteral_t literal, uint8_t i );
  void variableI16( SerialLiteral_t literal, int16_t i );
  void variableUI16( SerialLiteral_t literal, uint16_t i );
	void variableUI32( SerialLiteral_t literal, uint32_t i );
	void variableI32( const char *identifier, int32_t i );
  void variableUI32X( const char *identifier, uint32_t i );
  void variableVolt( const char *identifier, float f);
	void variableOhm( const char *identifier, float f);
  void variableCelcius( const char *identifier, float f);
  void variable4F( const char *identifier, float f1, float f2, float f3, float f4 );
  void variable3I16( const char *identifier, int16_t i1, int16_t i2, int16_t i3 );
  */
    
};