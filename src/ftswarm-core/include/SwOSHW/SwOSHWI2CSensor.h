/*
 * SwOSHWI2CSensor.h
 *
 * I2C based sensor hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include "SwOSHWBaseIO.h"

#include <VL53L0X.h>
#include <MPU6050_6Axis_MotionApps20.h>
#include "SwOSHW/SwOSHWActor.h"

class SwOSMotor;

/***************************************************
 *
 *   SwOSGyro
 *
 ***************************************************/

 class SwOSGyro : public SwOSInput {

  protected:
    float ypr[3];
   
  public:
    // constructor
	  SwOSGyro(const char *name, SwOSCtrl *ctrl, uint8_t flags);

    // administrative stuff
    virtual void recvState( SwOSCom *com ) {};
    virtual bool isGyro( void ) { return true; };
    virtual void serialize( Serialize *serialize );
    virtual uint8_t popState( uint8_t *buffer );
    virtual uint8_t pushState( uint8_t *buffer );

    // interface
    virtual void getYawPitchRoll(float *yaw, float *pitch, float *roll, bool radiants);
    
};

/***************************************************
 *
 *   SwOSLSM
 *
 ***************************************************/

 class SwOSGyroLSM : public SwOSGyro {

  protected:

    unsigned long lastMicros = micros();  
    float         rollGyro   = 0;
    float         pitchGyro  = 0;
    const float   alpha      = 0.98f;

    virtual void setupLocal(); 

  public:

    // constructor
	  SwOSGyroLSM(const char *name, SwOSCtrl *ctrl, uint8_t flags );
    ~SwOSGyroLSM();

    // read sensor
    virtual void operate();
};

/***************************************************
 *
 *   SwOSMPU
 *
 ***************************************************/

 class SwOSGyroMPU : public SwOSGyro {
  protected:

    uint16_t packetSize;    // Expected MPU 6050 DMP packet size (default is 42 bytes)

    virtual void setupLocal(); 

  public:
    // constructor
	  SwOSGyroMPU(const char *name, SwOSCtrl *ctrl, uint8_t flags );
    ~SwOSGyroMPU();

    // read sensor
    virtual void operate();

};

/***************************************************
 *
 *   SwOSLidarInput
 *
 ***************************************************/

 class SwOSLidarInput : public SwOSInput {

  protected:

    virtual void setupLocal();

  public:
 
	  SwOSLidarInput(const char *name, SwOSCtrl *ctrl, uint8_t flags );
  
    // administrative stuff
    virtual void serialize( Serialize *serialize );

    // read sensor
	  virtual void operate();

};

/***************************************************
 *
 *   I2C Slave
 *
 ***************************************************/

 class SwOSI2C : public SwOSIO, public SwOSEventInput {
  
  protected:
    SwOSMotor *intIO = NULL;
    
    virtual void setupLocal(uint8_t I2CAddress); // initializes local HW
    virtual void setLocal( uint8_t reg, uint8_t value );
    virtual void setRemote(uint8_t reg, uint8_t value );

  public:

    uint8_t myRegister[MAXI2CREGISTERS];
    SwOSI2C( const char *name, SwOSCtrl *ctrl, uint8_t flags, uint8_t I2CAddress);
    
    virtual bool isI2C( void ) { return true; };

    // check, if state has changed to send by data to kelda
    virtual bool isDirty( void ) { return true; };

    virtual uint8_t pushState( uint8_t *buffer );
    virtual uint8_t popState( uint8_t *buffer );

    virtual void operate();

    virtual void setRegister( uint8_t reg, uint8_t value );
    virtual uint8_t getRegister( uint8_t reg );

};

/***************************************************
 *
 *   CAN (TWAI)
 *
 ***************************************************/

// last CAN datagram sent/received - ID & up to MAXCANPAYLOAD payload bytes
struct SwOSCANMsg_t {
  uint32_t id     = 0;
  uint8_t  length = 0;
  uint8_t  payload[MAXCANPAYLOAD] = { 0 };
};

class SwOSCAN : public SwOSIO {

  protected:
    SwOSCANMsg_t lastMsg;

    virtual void setupLocal( void );          // initializes local TWAI HW, TX=SDA, RX=SCL, GPIO OC mode
    virtual void transmitLocal( uint32_t id, const uint8_t *payload, uint8_t length ); // transmit a datagram on the local CAN bus
    virtual void sendToMember( uint32_t id, const uint8_t *payload, uint8_t length );  // forward a send request to a remote member
    virtual void sendToKelda( uint32_t id, const uint8_t *payload, uint8_t length );   // report a received datagram to Kelda
    void printCSV( uint32_t id, const uint8_t *payload, uint8_t length );              // print id & payload as csv, if subscribed

  public:

    SwOSCAN( const char *name, SwOSCtrl *ctrl, uint8_t flags );

    virtual bool isCAN( void ) { return true; };

    // check, if state has changed to send by data to kelda
    virtual bool isDirty( void ) { return true; };

    virtual uint8_t pushState( uint8_t *buffer );
    virtual uint8_t popState( uint8_t *buffer );

    // receive CAN datagrams from the local bus
    virtual void operate();

    // send a CAN datagram - id & up to MAXCANPAYLOAD payload bytes
    virtual void sendCAN( uint32_t id, const uint8_t *payload, uint8_t length );

    // a remote member reported a datagram received from its local bus
    virtual void recvRemote( uint32_t id, const uint8_t *payload, uint8_t length );

};