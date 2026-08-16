#ifndef i2cBuffer_h
#define i2cBuffer_h

#include <Arduino.h>
#include <Wire.h>

class i2cBuffer {
  private:
    TwoWire *twi = NULL;
  public:
    uint8_t data[64];
    uint8_t len = 0;
    uint8_t error = 0;
    void push( uint8_t v );
      // writes a uint8_t into the buffer
    void push( int16_t v );
      // writes an int16_t into the buffer
    void push( int32_t v );
      // writes a int32_t into the buffer
    int16_t popInt16_t( uint8_t pos );
      // reads an int16_t out of the buffer
    int32_t popInt32_t( uint8_t pos );
      // reads a int32_t out of the buffer
    
    void sendData( uint8_t address, uint8_t cmd );
      // send a command 

    void sendData( uint8_t address, uint8_t cmd, uint8_t v1 );
      // send a command with a uint8_t 
    void sendData( uint8_t address, uint8_t cmd, int16_t v1 );
      // send a command with a int
    void sendData( uint8_t address, uint8_t cmd, int32_t v1 );
      // send a command with a int32_t value
    
    void sendData( uint8_t address, uint8_t cmd, uint8_t v1, uint8_t v2 );
      // send a command with 2 uint8_t
    void sendData( uint8_t address, uint8_t cmd, uint8_t v1, int32_t v2 );
      // send a command with a uint8_t and a int32_t value
    
    void sendData( uint8_t address, uint8_t cmd, uint8_t v1, uint8_t v2, uint8_t v3 ) ;
      // send a command with 3 uint8_t
    void sendData( uint8_t address, uint8_t cmd, uint8_t v1, int32_t v2, uint8_t v3 ) ;
      // send a command with a uint8_t, a int32_t value and another uint8_t
        void sendData( uint8_t address, uint8_t cmd, int32_t v1, int32_t v2, int32_t v3, int32_t v4 );
      // send a command with 4 int32_t values
    
    uint8_t receiveUint8_t( uint8_t address, uint8_t cmd );
      // receive a uint8_t value 
    uint8_t receiveUint8_t( uint8_t address, uint8_t cmd, uint8_t v1 );
      // receive a uint8_t value 
    void receive4Uint8_t( uint8_t address, uint8_t cmd, uint8_t *v1, uint8_t *v2, uint8_t *v3, uint8_t *v4 );
      // receive 4 uint8_t values 
    int32_t receiveInt32_t( uint8_t address, uint8_t cmd, uint8_t v1 );
      // receive a int32_t value 
    void receive4Int32_t( uint8_t address, uint8_t cmd, int32_t *v1, int32_t *v2, int32_t *v3, int32_t *v4 );
      // receive 4 int32_t values
    int16_t receiveInt16_t( uint8_t address, uint8_t cmd, uint8_t v1 );
      // receive an int16_t value 
    void receive4Int16_t( uint8_t address, uint8_t cmd, int16_t &v1, int16_t &v2, int16_t &v3, int16_t &v4 );
      // receive 4 int16_t values
    void sendBuffer( uint8_t address );
      // send data
    void receiveBuffer( uint8_t address, uint8_t quantity );

    void begin( TwoWire *twi ) { this->twi = twi; };
};

#endif
