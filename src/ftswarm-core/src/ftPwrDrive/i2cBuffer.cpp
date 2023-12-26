#include "ftPwrDrive/i2cBuffer.h"
#include <Wire.h>

// #define DEBUG_COM

void i2cBuffer::sendBuffer( uint8_t address  ) {
  // send data

  #ifdef DEBUG_COM
    Serial.print("sendBuffer "); Serial.print( address ); Serial.print(" ");
  
    for (int i=0; i<len; i++ ) {
      Serial.print( data[i], HEX );
      Serial.print(" ");
    }
  
    Serial.println();
  #endif
 
  Wire.beginTransmission( address );
  Wire.write( data, len );
  error = Wire.endTransmission();
}

void i2cBuffer::receiveBuffer( uint8_t address, uint8_t quantity ) {
  // receive data

  #ifdef DEBUG_COM
    Serial.print( "receiveBuffer( " ); Serial.print( address ), Serial.print(","); Serial.print( quantity ); Serial.println(")");
  #endif

  // there is always a write before reading data.
  // We need some microseconds to avoid getting struggled
  // delayMicroseconds(1);

  len = 0;

  // request quantity uint8_ts
  if ( Wire.requestFrom( address, quantity) != quantity ) error = 4; 
  else error = 0;

  uint8_t x;
  // receive data
  while (Wire.available()) { 
    x = Wire.read();
    data[len++] = x;

    #ifdef DEBUG_COM
      Serial.print( x, HEX ); 
      Serial.print( " " );
    #endif
    
  }

  #ifdef DEBUG_COM
    Serial.println();
  #endif
  
  // fillup buffer
  for (uint8_t i=len;i<16;i++) {
    data[i] = 0;
  }

}

void i2cBuffer::push( uint8_t v ) {
  // writes a uint8_t into the buffer
  data[len++] = v;
}

void i2cBuffer::push( long v ) {
  // writes a long into the buffer
  memcpy( &data[len], &v, sizeof(v) );
  len += sizeof(v);
}

void i2cBuffer::push( int v ) {
  // writes an int into the buffer
  memcpy( &data[len], &v, sizeof(v) );
  len += sizeof(v);
}

long i2cBuffer::popLong( uint8_t pos ) {
  // reads a long out of the buffer
  long v;
  memcpy( &v, &data[pos], sizeof(v) );
  return v;
}

int i2cBuffer::popInt( uint8_t pos ) {
  // reads an int out of the buffer
  int v;
  memcpy( &v, &data[pos], sizeof(v) );
  return v;
}

void i2cBuffer::sendData( uint8_t address, uint8_t cmd ) {
  // send a command
  len = 0;
  push( cmd );
  sendBuffer( address );
}

void i2cBuffer::sendData( uint8_t address, uint8_t cmd, uint8_t v1 ) {
  // send a command with a long value
  len = 0;
  push( cmd );
  push( v1 );
  sendBuffer( address );
}

void i2cBuffer::sendData( uint8_t address, uint8_t cmd, long v1 ) {
  // send a command with a long value
  len = 0;
  push( cmd );
  push( v1 );
  sendBuffer( address );
}

void i2cBuffer::sendData( uint8_t address, uint8_t cmd, int v1 ) {
  // send a command with a long value
  len = 0;
  push( cmd );
  push( v1 );
  sendBuffer( address );
}

void i2cBuffer::sendData( uint8_t address, uint8_t cmd, uint8_t v1, uint8_t v2 ) {
  // send a command with 2 uint8_t
  len = 0;
  push( cmd );
  push( v1 );
  push( v2 );
  sendBuffer( address );
}

void i2cBuffer::sendData( uint8_t address, uint8_t cmd, uint8_t v1, uint8_t v2, uint8_t v3 ) {
  // send a command with 3 uint8_t
  len = 0;
  push( cmd );
  push( v1 );
  push( v2 );
  push( v3 );
  sendBuffer( address );
}

void i2cBuffer::sendData( uint8_t address, uint8_t cmd, uint8_t v1, long v2, uint8_t v3 ) {
  // send a command with a uint8_t, a long value and another uint8_t
  len = 0;
  push( cmd );
  push( v1 );
  push( v2 );
  push( v3 );
  sendBuffer( address );
}

void i2cBuffer::sendData( uint8_t address, uint8_t cmd, uint8_t v1, long v2 ) {
  // send a command with a uint8_t and a long value
  len = 0;
  push( cmd );
  push( v1 );
  push( v2 );
  sendBuffer( address );
}


void i2cBuffer::sendData( uint8_t address, uint8_t cmd,long v1, long v2, long v3, long v4 ) {
  // send a command with 4 long values
  
  len = 0;
  push( cmd );
  push( v1 );
  push( v2 );
  push( v3 );
  push( v4 );

  sendBuffer( address );
}

uint8_t i2cBuffer::receiveuint8_t( uint8_t address, uint8_t cmd ) {
  // receive a uint8_t value 
  sendData( address, cmd );
  receiveBuffer( address, 1 );
  return data[0];
}

uint8_t i2cBuffer::receiveuint8_t( uint8_t address, uint8_t cmd, uint8_t v1 ) {
  // receive a uint8_t value 
  sendData( address, cmd, v1 );
  receiveBuffer( address, 1 );
  return data[0];
}

void i2cBuffer::receive4uint8_t( uint8_t address, uint8_t cmd, uint8_t *v1, uint8_t *v2, uint8_t *v3, uint8_t *v4 ) {
  // receive a uint8_t value 
  sendData( address, cmd );
  receiveBuffer( address, 4 );
  *v1 = data[0];
  *v2 = data[1];
  *v3 = data[2];
  *v4 = data[3];
   
}

long i2cBuffer::receiveLong( uint8_t address, uint8_t cmd, uint8_t v1 ) {
  // receive a long value 
  sendData( address, cmd, v1 );
  receiveBuffer( address, 4 );
  return popLong( 0 );
}

void i2cBuffer::receive4Long( uint8_t address, uint8_t cmd, long *v1, long *v2, long *v3, long *v4 ) {
  // receive 4 long values
  sendData( address, cmd );
  receiveBuffer( address, 16 );
  *v1 = popLong( 0 );
  *v2 = popLong( 4 );
  *v3 = popLong( 8 );
  *v4 = popLong( 12 );
} 

int i2cBuffer::receiveInt( uint8_t address, uint8_t cmd, uint8_t v1 ) {
  // receive an int value 
  sendData( address, cmd, v1 );
  receiveBuffer( address, 2 );
  return popInt( 0 );
}

void i2cBuffer::receive4Int( uint8_t address, uint8_t cmd, int &v1, int &v2, int &v3, int &v4 ) {
  // receive 4 int values
  sendData( address, cmd );
  receiveBuffer( address, 16 );
  v1 = popInt( 0 );
  v2 = popInt( 2 );
  v3 = popInt( 4 );
  v4 = popInt( 8 );
} 
