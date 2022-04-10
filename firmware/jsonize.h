/*
 * jsonize.h
 *
 * simple framework to build a json reply for a REST based service
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */

#pragma once

#include <esp_http_server.h>

// known JSON objects
typedef enum {
	JSONNone,
	JSONObject,
	JSONArray,
	JSONVariable
} JSONObject_t;

// simple class to build a json string and send it immediately
class JSONize {
  
protected:
	httpd_req_t *_req;                      // esp-idf's http-request
	JSONObject_t lastObject = JSONNone;     // which type I'm processing
	void assign();                          // write ":"
	void text2string( char *t);             // write a sting in quotes: "t"
	void newObject( JSONObject_t object );  // start a new object and decide to write a "," 
  
public:
	JSONize( httpd_req_t *req );

  // start and end a new object
  void startObject();
  void endObject();

  // start and end a ne array
	void startArray( const char* identifier);
	void endArray();

  // write variables
	void variable( const char *identifier, char *value);
  void variableB( const char *identifier, bool b );
  void variableUI8( const char *identifier, uint8_t i );
  void variableI16( const char *identifier, int16_t i );
  void variableUI16( const char *identifier, uint16_t i );
	void variableUI32( const char *identifier, uint32_t i );
  void variableUI32X( const char *identifier, uint32_t i );
  void variableVolt( const char *identifier, float f);
	void variableOhm( const char *identifier, float f);
  void variableCelcius( const char *identifier, float f);
  
};
