#ifndef i2cBuffer_h
#define i2cBuffer_h

#include <Arduino.h>

class i2cBuffer {
  public:
    uint8_t data[64];
    uint8_t len = 0;
    void push( uint8_t v );
      // writes a uint8_t into the buffer
    void push( long v );
      // writes a long into the buffer
    void push( int v );
      // writes an int into the buffer
    long popLong( uint8_t pos );
      // reads a long out of the buffer
    int popInt( uint8_t pos );
      // reads an int out of the buffer
    void sendData( uint8_t address, uint8_t cmd );
      // send a command 
    void sendData( uint8_t address, uint8_t cmd, long v1 );
      // send a command with a long value
    void sendData( uint8_t address, uint8_t cmd, uint8_t v1 );
      // send a command with a uint8_t 
    void sendData( uint8_t address, uint8_t cmd, int v1 );
      // send a command with a int
    void sendData( uint8_t address, uint8_t cmd, uint8_t v1, long v2, uint8_t v3 ) ;
      // send a command with a uint8_t, a long value and another uint8_t
    void sendData( uint8_t address, uint8_t cmd, uint8_t v1, long v2 );
      // send a command with a uint8_t and a long value
    void sendData( uint8_t address, uint8_t cmd, long v1, long v2, long v3, long v4 );
      // send a command with 4 long values
    uint8_t receiveuint8_t( uint8_t address, uint8_t cmd );
      // receive a uint8_t value 
    uint8_t receiveuint8_t( uint8_t address, uint8_t cmd, uint8_t v1 );
      // receive a uint8_t value 
    void receive4uint8_t( uint8_t address, uint8_t cmd, uint8_t *v1, uint8_t *v2, uint8_t *v3, uint8_t *v4 );
      // receive 4 uint8_t values 
    long receiveLong( uint8_t address, uint8_t cmd, uint8_t v1 );
      // receive a long value 
    void receive4Long( uint8_t address, uint8_t cmd, long *v1, long *v2, long *v3, long *v4 );
      // receive 4 long values
    int receiveInt( uint8_t address, uint8_t cmd, uint8_t v1 );
      // receive an int value 
    void receive4Int( uint8_t address, uint8_t cmd, int &v1, int &v2, int &v3, int &v4 );
      // receive 4 int values
    void sendBuffer( uint8_t address );
      // send data
    void receiveBuffer( uint8_t address, uint8_t quantity );
};

#endif
