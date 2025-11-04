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
  "controllers",        // SERIALIZE_LITERAL_CTRL
  "io",                 // SERIALIZE_LITERAL_IO
  "kelda",              // SERIALIZE_LITERAL_KELDA
  "sensor",             // SERIALIZE_LITERAL_SENSOR
  "actor",              // SERIALIZE_LITERAL_ACTOR
  "trigger",            // SERIALIZE_LITERAL_TRIGGER
  "state",              // SERIALIZE_LITERAL_STATE
  "speed",              // SERIALIZE_LITERAL_SPEED
  "UNUSED",             // SERIALIZE_LITERAL_UNUSED
  "offset",             // SERIALIZE_LITERAL_OFFSET
  "position",           // SERIALIZE_LITERAL_POSITION
  "value",              // SERIALIZE_LITERAL_VALUE
  "valueLR",            // SERIALIZE_LITERAL_VALUELR
  "valueFB",            // SERIALIZE_LITERAL_VALUEFB
  "name",               // SERIALIZE_LITERAL_NAME
  "id",                 // SERIALIZE_LITERAL_ID,
  "serialNumber",       // SERIALIZE_LITERAL_SERIALNUMBER
  "type",               // SERIALIZE_LITERAL_TYPE
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
  "v-Flip",             // SERIALIZE_LITERAL_VFLIP
  "activeConfig",       // SERIALIZE_LITERAL_ACTIVECONFIG
  "events",             // SERIALIZE_LITERAL_EVENTS
  "distance",           // SERIALIZE_LITERAL_DISTANCE
  "homing",             // SERIALIZE_LITERAL_HOMING
  "running"             // SERIALIZE_LITERAL_RUNNING
};

Serialize::Serialize( char *buffer, size_t bufSize, SerialFormat_t format ) {

  this->buffer  = buffer;
  this->bufSize = bufSize;
  this->format  = format;
  reset();

}

/**** Buffer Methods ****/

void Serialize::reset( void ) {

  if (!buffer) bzero( buffer, bufSize );
  ptr = 0;
  noSpacer = true;

}

void Serialize::write( const char *str ) {

  uint32_t len = strlen(str);

  if ( (ptr+len) > bufSize ) return;

  strcpy( &buffer[ptr], str );
  ptr += len;

}

void Serialize::writeBinary( uint8_t v ) {

  buffer[ptr++] = v;

}

void Serialize::write( SerialLiteral_t literal ) {

  if ( literal ) {

    if ( format == SERIALIZE_RAW ) { 
      writeBinary( literal+128 );
    } else {
      write("\"");
      write( SLITERAL[ literal ] );
      write("\":");
    }
  
  }

}

void Serialize::write( int value ) {
  
  char str[64];
  sprintf( str, "%d", value);
  write( str );
}

void Serialize::writeX( uint32_t value ) {
  
  char str[64];
  sprintf( str, "%06X", value);
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
  write( "\"" );
  write( value );
  write( "\"" );

}

void Serialize::item( SerialLiteral_t literal, int value ) {
  
  newObject( SERIALIZE_Item );
  write( literal);
  write( value );

}

void Serialize::itemX( SerialLiteral_t literal, uint32_t value ) {
  
  newObject( SERIALIZE_Item );
  write( literal);
  writeX( value );

}

void Serialize::item( SerialLiteral_t literal, float value, uint8_t decimalPlaces = 1, const char *unit ) {
  
  newObject( SERIALIZE_Item );
  
  // literal
  write( literal);
  
  // value
  char str[64];

  if (unit) 
    sprintf( str, "[%0.*f,\"%s\"]", decimalPlaces, value, unit);
  else
    sprintf( str, "%0.*f", decimalPlaces, value);

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
