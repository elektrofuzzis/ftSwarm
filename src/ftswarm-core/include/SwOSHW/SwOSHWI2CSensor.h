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

/***************************************************
 *
 *   SwOSGyro
 *
 ***************************************************/

 class SwOSGyro : public SwOSIO {
   
  public:
    // constructor
	  SwOSGyro(const char *name, SwOSCtrl *ctrl);

    // administrative stuff
    virtual void recvState( SwOSCom *com ) {};

    // Test, if I'm an Sensor
    virtual bool isSensor( void ) { return true; }

    // interface
    virtual void getAcceleration( float *x, float *y, float *z ) {};
    virtual void getQuaternion( float *w, float *x, float *y, float *z ) {};
    virtual void getYawPitchRoll(float *yaw, float *pitch, float *roll, bool radiants) {};
    virtual void getEuler(float *alpha, float *beta, float *gamma, bool radiants ) {};
    
};

/***************************************************
 *
 *   SwOSLSM
 *
 ***************************************************/

 class SwOSGyroLSM : public SwOSGyro {
  protected:

    virtual void setupLocal(); 

  public:
    // constructor
	  SwOSGyroLSM(const char *name, SwOSCtrl *ctrl);
    ~SwOSGyroLSM();

    // read sensor
    virtual void read();
};

/***************************************************
 *
 *   SwOSMPU
 *
 ***************************************************/

 class SwOSGyroMPU : public SwOSGyro {
  protected:
    Quaternion  q;
    VectorInt16 aa;
    uint16_t packetSize;    // Expected MPU 6050 DMP packet size (default is 42 bytes)

    virtual void setupLocal(); 

  public:
    // constructor
	  SwOSGyroMPU(const char *name, SwOSCtrl *ctrl );
    ~SwOSGyroMPU();
    uint8_t popState( uint8_t *buffer );
    uint8_t pushState( uint8_t *buffer );
    virtual void jsonize( JSONize *json, uint8_t id);

    // read sensor
    virtual void read();

    // interface
    virtual void getAcceleration( float *x, float *y, float *z );
    virtual void getQuaternion( float *w, float *x, float *y, float *z );
    virtual void getYawPitchRoll(float *yaw, float *pitch, float *roll, bool radiants = false );
    virtual void getEuler(float *alpha, float *beta, float *gamma, bool radiants = false );
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
 
	  SwOSLidarInput(const char *name, SwOSCtrl *ctrl );
  
    // administrative stuff
    virtual void jsonize( JSONize *json, uint8_t id);

    // read sensor
	  virtual void read();
    virtual void setReading( int32_t newValue );

    // external commands
    virtual void setValue( int32_t value );                    // set value by an external call

};

/***************************************************
 *
 *   I2C Slave
 *
 ***************************************************/

 class SwOSI2C : public SwOSIO, public SwOSEventInput {
  protected:
    
    virtual void setupLocal(uint8_t I2CAddress); // initializes local HW
    virtual void setLocal( uint8_t reg, uint8_t value );
    virtual void setRemote(uint8_t reg, uint8_t value );

  public:

    uint8_t myRegister[MAXI2CREGISTERS];
    SwOSI2C( const char *name, SwOSCtrl *ctrl, uint8_t I2CAddress);
    virtual bool isI2C( void ) { return true; };
    virtual uint8_t pushState( uint8_t *buffer );
    virtual uint8_t popState( uint8_t *buffer );

    virtual void read();

    virtual void setRegister( uint8_t reg, uint8_t value );
    virtual uint8_t getRegister( uint8_t reg );

};