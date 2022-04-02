/*
 * jsonize.h
 *
 *  Created on: 14.11.2021
 *      Author: Stefan
 */

#pragma once

#include <esp_http_server.h>

typedef enum {
	JSONNone,
	JSONObject,
	JSONArray,
	JSONVariable
} JSONObject_t;

class JSONize {
protected:
	httpd_req_t *_req;
	char tabs[20] = "";
	JSONObject_t lastObject = JSONNone;
	void assign(); // :
	void text2string( char *t); // "t"
	void newObject( JSONObject_t object );
	void closeObject();
public:
	JSONize( httpd_req_t *req );
	void startArray( const char* identifier);
	void endArray();
	void startObject();
	void endObject();
	void variable( const char *identifier, char *value); // "identifier": "value"
	void variable( const char *identifier, const char *value); // "identifier": "value"
	void variable( const char *identifier, uint32_t i );
	void variableX( const char *identifier, uint32_t i );
  void variableVolt( const char *identifier, float f);
	void variableOhm( const char *identifier, float f);
  void variableCelcius( const char *identifier, float f);
};
