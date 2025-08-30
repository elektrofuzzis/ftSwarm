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
 
  twi->beginTransmission( address );
  twi->write( data, len );
  error = twi->endTransmission();
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
  if ( twi->requestFrom( address, quantity) != quantity ) error = 4; 
  else error = 0;

  uint8_t x;
  // receive data
  while (twi->available()) { 
    x = twi->read();
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

void i2cBuffer::push( int32_t v ) {
  // writes a int32_t into the buffer
  memcpy( &data[len], &v, sizeof(v) );
  len += sizeof(v);
}

void i2cBuffer::push( int16_t v ) {
  // writes an int16_t into the buffer
  memcpy( &data[len], &v, sizeof(v) );
  len += sizeof(v);
}

int32_t i2cBuffer::popInt32_t( uint8_t pos ) {
  // reads a int32_t out of the buffer
  int32_t v;
  memcpy( &v, &data[pos], sizeof(v) );
  return v;
}

int16_t i2cBuffer::popInt16_t( uint8_t pos ) {
  // reads an int16_t out of the buffer
  int16_t v;
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
  // send a command with a int32_t value
  len = 0;
  push( cmd );
  push( v1 );
  sendBuffer( address );
}

void i2cBuffer::sendData( uint8_t address, uint8_t cmd, int32_t v1 ) {
  // send a command with a int32_t value
  len = 0;
  push( cmd );
  push( v1 );
  sendBuffer( address );
}

void i2cBuffer::sendData( uint8_t address, uint8_t cmd, int16_t v1 ) {
  // send a command with a int32_t value
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

void i2cBuffer::sendData( uint8_t address, uint8_t cmd, uint8_t v1, int32_t v2, uint8_t v3 ) {
  // send a command with a uint8_t, a int32_t value and another uint8_t
  len = 0;
  push( cmd );
  push( v1 );
  push( v2 );
  push( v3 );
  sendBuffer( address );
}

void i2cBuffer::sendData( uint8_t address, uint8_t cmd, uint8_t v1, int32_t v2 ) {
  // send a command with a uint8_t and a int32_t value
  len = 0;
  push( cmd );
  push( v1 );
  push( v2 );
  sendBuffer( address );
}


void i2cBuffer::sendData( uint8_t address, uint8_t cmd,int32_t v1, int32_t v2, int32_t v3, int32_t v4 ) {
  // send a command with 4 int32_t values
  
  len = 0;
  push( cmd );
  push( v1 );
  push( v2 );
  push( v3 );
  push( v4 );

  sendBuffer( address );
}

uint8_t i2cBuffer::receiveUint8_t( uint8_t address, uint8_t cmd ) {
  // receive a uint8_t value 
  sendData( address, cmd );
  receiveBuffer( address, 1 );
  return data[0];
}

uint8_t i2cBuffer::receiveUint8_t( uint8_t address, uint8_t cmd, uint8_t v1 ) {
  // receive a uint8_t value 
  sendData( address, cmd, v1 );
  receiveBuffer( address, 1 );
  return data[0];
}

void i2cBuffer::receive4Uint8_t( uint8_t address, uint8_t cmd, uint8_t *v1, uint8_t *v2, uint8_t *v3, uint8_t *v4 ) {
  // receive a uint8_t value 
  sendData( address, cmd );
  receiveBuffer( address, 4 );
  *v1 = data[0];
  *v2 = data[1];
  *v3 = data[2];
  *v4 = data[3];
   
}

int32_t i2cBuffer::receiveInt32_t( uint8_t address, uint8_t cmd, uint8_t v1 ) {
  // receive a int32_t value 
  sendData( address, cmd, v1 );
  receiveBuffer( address, 4 );
  return popInt32_t( 0 );
}

void i2cBuffer::receive4Int32_t( uint8_t address, uint8_t cmd, int32_t *v1, int32_t *v2, int32_t *v3, int32_t *v4 ) {
  // receive 4 int32_t values
  sendData( address, cmd );
  receiveBuffer( address, 16 );
  *v1 = popInt32_t( 0 );
  *v2 = popInt32_t( 4 );
  *v3 = popInt32_t( 8 );
  *v4 = popInt32_t( 12 );
} 

int16_t i2cBuffer::receiveInt16_t( uint8_t address, uint8_t cmd, uint8_t v1 ) {
  // receive an int16_t value 
  sendData( address, cmd, v1 );
  receiveBuffer( address, 2 );
  return popInt16_t( 0 );
}

void i2cBuffer::receive4Int16_t( uint8_t address, uint8_t cmd, int16_t &v1, int16_t &v2, int16_t &v3, int16_t &v4 ) {
  // receive 4 int16_t values
  sendData( address, cmd );
  receiveBuffer( address, 16 );
  v1 = popInt32_t( 0 );
  v2 = popInt32_t( 2 );
  v3 = popInt32_t( 4 );
  v4 = popInt32_t( 8 );
} 
