/*
 * serialize.h
 *
 * simple framework to build a json reply for a REST based service
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "serialize.h"

const char SLITERAL[ SERIALIZE_LITERAL_MAX ][24] = {
  "",                   // SERIALIZE_LITERAL_NULL
  "ctrl",               // SERIALIZE_LITERAL_CTRL
  "io",                 // SERIALIZE_LITERAL_IO
  "kelda",              // SERIALIZE_LITERAL_KELDA
  "token",              // SERIALIZE_LITERAL_TOKEN
  "auth",               // SERIALIZE_LITERAL_AUTH
  "provided",           // SERIALIZE_LITERAL_PROVIDED
  "speed",              // SERIALIZE_LITERAL_SPEED
  "highResolution",     // SERIALIZE_LITERAL_HIGHRESOLUTION
  "offset",             // SERIALIZE_LITERAL_OFFSET
  "position",           // SERIALIZE_LITERAL_POSITION
  "value",              // SERIALIZE_LITERAL_VALUE
  "valueLR",            // SERIALIZE_LITERAL_VALUELR
  "valueFB",            // SERIALIZE_LITERAL_VALUEFB
  "name",               // SERIALIZE_LITERAL_NAME
  "id",                 // SERIALIZE_LITERAL_ID,
  "serialNumber",       // SERIALIZE_LITERAL_SERIALNUMBER
  "type",               // SERIALIZE_LITERAL_TYPE
  "state",              // SERIALIZE_LITERAL_STATE
  "icon",               // SERIALIZE_LITERAL_ICON
  "active",             // SERIALIZE_LITERAL_ACTIVE
  "brightness",         // SERIALIZE_LITERAL_BRIGHTNESS
  "color",              // SERIALIZE_LITERAL_COLOR
  "quaternion"          // SERIALIZE_LITERAL_QUATERNION
  "acceleration",       // SERIALIZE_LITERAL_ACCELERATION
  "url",                // SERIALIZE_LITERAL_URL
  "framesize",          // SERIALIZE_LITERAL_FRAMESIZE
  "quality",            // SERIALIZE_LITERAL_QUALITY
  "contrast",           // SERIALIZE_LITERAL_CONTRAST
  "saturation",         // SERIALIZE_LITERAL_SATURATION
  "h-Mirror",           // SERIALIZE_LITERAL_HMIRROR
  "v-Flip"              // SERIALIZE_LITERAL_VFLIP
};

Serialize::Serialize( SerialFormat_t format ) {

  this->format = format;
  reset();

}

/**** Buffer Methods ****/

void Serialize::reset( void ) {

  bzero( buffer, MAXJSONBUFFER );
  ptr = 0;

}

void Serialize::write( const char *str ) {

  uint32_t len = strlen(str);

  if ( (ptr+len) > MAXJSONBUFFER ) return;

  strcpy( &buffer[ptr], str );
  ptr += len;

}

void Serialize::writeBinary( uint8_t v ) {

  buffer[ptr++] = v;

}

void Serialize::write( SerialLiteral_t literal ) {

  if ( literal ) {

    if ( format == SERIALIZE_RAW ) { 
      writeBinary( literal );
    } else {
      write("\"");
      write( SLITERAL[ literal ] );
      write("\"");
    }

    write( ":" );
  
  }

}

void Serialize::write( int value ) {
  
  char str[64];
  sprintf( str, "%d", value);
  write( str );
}


/**** Objects *****/


void Serialize::newObject( SerialObject_t object ) {

  char line[50];

  // need a spacer?
  if ( ( lastObject == object ) ||
     ( ( lastObject == SERIALIZE_Item ) && ( object = SERIALIZE_Array ) )
     ) {

    // spacer is not needed, if supress Flag is set and last object is an object
    if ( !( (noSpacer) && ( object == SERIALIZE_Object) ) ) write( "," );
    noSpacer = false;
  }

  lastObject = object;
}

void Serialize::startObject( SerialLiteral_t literal ) {

  newObject( SERIALIZE_Object );

  if ( literal != SERIALIZE_LITERAL_NULL ) {
    write( literal );
    noSpacer = true;
  }

  write( "{" );
}

void Serialize::endObject( ) {

  write( "}" );
  lastObject = SERIALIZE_Object;

}

void Serialize::startArray( SerialLiteral_t literal ) {

  newObject( SERIALIZE_Array );
  write( literal );
  write( "[" );

}

void Serialize::endArray( ) {

  write( "]" );

}

void Serialize::item( SerialLiteral_t literal, const char* value ) {
  
  newObject( SERIALIZE_Item );
  write( literal);
  if ( format == SERIALIZE_JSON ) write( "\"" );
  write( value );
  if ( format == SERIALIZE_JSON ) write( "\"" );

}

void Serialize::item( SerialLiteral_t literal, int value ) {
  
  newObject( SERIALIZE_Item );
  write( literal);
  write( value );

}

void Serialize::item( SerialLiteral_t literal, float value, uint8_t decimalPlaces = 1, const char *unit ) {
  
  newObject( SERIALIZE_Item );
  
  // literal
  write( literal);
  
  // value
  char str[64];
  sprintf( str, "%0.*f", decimalPlaces, value);
  write( str );

  // unit if given
  if (unit) { write(" "); write( unit ); }

}

void Serialize::item( SerialLiteral_t literal, float v1, float v2, float v3, float v4 ) {

  newObject( SERIALIZE_Item );
  
  write( literal);

  char str[255];
  sprintf( str, "(%f,%f,%f,%f)", v1, v2, v3, v4 );
  write( str );

}

void Serialize::item( SerialLiteral_t literal, float v1, float v2, float v3 ) {

  newObject( SERIALIZE_Item );
  
  write( literal);

  char str[255];
  sprintf( str, "(%f,%f,%f)", v1, v2, v3 );
  write( str );

}

/*

void JSONize::variable( const char *identifier, char *value) {

  newObject(JSONVariable);

  text2string( (char *) identifier );
  assign();
  text2string( value);

}

void JSONize::variableB( const char *identifier, bool b) {
  variableI16( identifier, (int16_t) b );
}

void JSONize::variableUI8( const char *identifier, uint8_t i) {
  variableI16( identifier, (int16_t) i );
}

void JSONize::variableI16( const char *identifier, int16_t i ) {

  char value[10];

  newObject(JSONVariable);

  text2string( (char *) identifier );
  assign();

  sprintf(value, "%d", i );
  write( value );

}

void JSONize::variableUI16( const char *identifier, uint16_t i ) {

  char value[10];

  newObject(JSONVariable);

  text2string( (char *) identifier );
  assign();

  sprintf(value, "%d", i );
  write( value );

}

void JSONize::variableUI32( const char *identifier, uint32_t i ) {

  char value[10];

  newObject(JSONVariable);

  text2string( (char *) identifier );
  assign();

  sprintf(value, "%d", i );
  write( value );

}

void JSONize::variableI32( const char *identifier, int32_t i ) {

  char value[10];

  newObject(JSONVariable);

  text2string( (char *) identifier );
  assign();

  sprintf(value, "%d", i );
  write( value );

}

void JSONize::variableUI32X( const char *identifier, uint32_t i ) {

  char value[10];

  newObject(JSONVariable);

  text2string( (char *) identifier );
  assign();

  sprintf(value, "\"#%06lx\"", (long) i );
  write( value );

}

void JSONize::variableVolt( const char *identifier, float f ) {

  char value[20];

  newObject(JSONVariable);

  text2string( (char *) identifier );
  assign();

  sprintf(value, "%0.3f V", f );
  text2string( value );

}

void JSONize::variableOhm( const char *identifier, float f ) {

  char value[20];

  newObject(JSONVariable);

  text2string( (char *) identifier );
  assign();

  uint x = f;

  sprintf(value, "%u Ohm", x );
  text2string( value );

}

void JSONize::variableCelcius( const char *identifier, float f ) {

  char value[20];

  newObject(JSONVariable);

  text2string( (char *) identifier );
  assign();

  sprintf(value, "%0.1f °C", f );
  text2string( value );

}

void JSONize::variable4F( const char *identifier, float f1, float f2, float f3, float f4 ) {

  char value[100];

  newObject(JSONVariable);

  text2string( (char *) identifier );
  assign();

  sprintf(value, "%f %f %f %f", f1, f2, f3, f4 );
  text2string( value );

}

void JSONize::variable3I16( const char *identifier, int16_t i1, int16_t i2, int16_t i3 )  {

  char value[100];

  newObject(JSONVariable);

  text2string( (char *) identifier );
  assign();

  sprintf(value, "%d %d %d", i1, i2, i3 );
  text2string( value );

}

*/