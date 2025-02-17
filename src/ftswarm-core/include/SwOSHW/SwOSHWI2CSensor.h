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
  protected:
    
    LSM6DSRSensor *lsm = NULL;
    MPU6050       *mpu = NULL;

    int32_t _accelerometer[3];
    int32_t _gyroscope[3];

    virtual void _setupLocalLSM(); 
    virtual void _setupLocalMPU(); 

    virtual void _readLSM();
    virtual void _readMPU();
    
  public:
    // constructor
	  SwOSGyro(const char *name, SwOSCtrl *ctrl, FtSwarmGyroMode_t gyroMode );
    ~SwOSGyro();

    // administrative stuff
	  virtual FtSwarmIOType_t getIOType() { return FTSWARM_GYRO; };
    virtual char *getIcon() { return (char *) "25_gyro.svg"; };
    virtual void jsonize( JSONize *json, uint8_t id);

    // Test, if I'm an Sensor
    virtual bool isSensor( void ) { return true; }

    // read sensor
    virtual void read();
};

/***************************************************
 *
 *   SwOSLidarInput
 *
 ***************************************************/

 class SwOSLidarInput : public SwOSInput {

  protected:

    virtual void _setupLocal();
    virtual void setSensorTypeLocal( FtSwarmSensor_t sensorType );

  public:
 
	  SwOSLidarInput(const char *name, uint8_t port, SwOSCtrl *ctrl );
  
    // administrative stuff
	  virtual FtSwarmIOType_t getIOType() { return FTSWARM_DIGITALINPUT; };
    virtual void jsonize( JSONize *json, uint8_t id);

    // read sensor
	  virtual void read();
    virtual void setReading( int32_t newValue );

    // external commands
    virtual void            setSensorType( FtSwarmSensor_t sensorType );  // set sensor type
    virtual void            setValue( int32_t value );                    // set value by an external call

};

/***************************************************
 *
 *   I2C Slave
 *
 ***************************************************/

 class SwOSI2C : public SwOSIO, public SwOSEventInput {
  protected:
    
    virtual void _setupLocal(uint8_t I2CAddress); // initializes local HW
    virtual void _setLocal( uint8_t reg, uint8_t value );
    virtual void _setRemote(uint8_t reg, uint8_t value );

  public:

    uint8_t myRegister[MAXI2CREGISTERS];
    SwOSI2C( const char *name, SwOSCtrl *ctrl, uint8_t I2CAddress);

    virtual void read();
    virtual FtSwarmIOType_t getIOType() { return FTSWARM_I2C; };

    virtual void setRegister( uint8_t reg, uint8_t value );
    virtual uint8_t getRegister( uint8_t reg );

};