/*
 * SwOSHWI2CCtrl.h
 *
 * I2C based controller (ftSwarmDuino & ftSwarmPwrDrive) implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include "SwOSHWBaseCtrl.h"
 
/***************************************************
 *
 *   SwOSSwarmI2CCtrl
 *
 ***************************************************/

 class SwOSSwarmI2CCtrl : public SwOSCtrl {
  public:

    // constructor, destructor
    SwOSSwarmI2CCtrl( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmKelda );
    ~SwOSSwarmI2CCtrl();

};

/***************************************************
 *
 *   SwOSSwarmPwrDrive - ftPwrDrive using FtSwarmI2C-Board
 *
 ***************************************************/

class SwOSSwarmPwrDrive : public SwOSSwarmI2CCtrl {
  private:
    uint8_t _microstepMode = 0;
  public:
    // constructor, destructor
    SwOSSwarmPwrDrive( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmKelda );
    SwOSSwarmPwrDrive( SwOSCom *com ); 
    ~SwOSSwarmPwrDrive();

    virtual char*              myType();                                   // what I am?
	  virtual FtSwarmController_t getType();                                  // what I am?
    virtual SwOSCom *          state2Com( MacAddr destination );
    virtual bool               recvState( SwOSCom *com );                  // receive state from another ftSwarmXX
    virtual void               read( void );
  
    // Communications
    virtual bool OnDataRecv( SwOSCom *com );     // data via espnow revceived
  
    virtual void setMicrostepMode( uint8_t mode, bool dontSendToRemote );
      // set microstep mode
      // FILLSTEP, HALFSTEP, QUARTERSTEP, EIGTHSTEP, SIXTEENTHSTEP
      
    virtual uint8_t getMicrostepMode( void );
      // get microstep mode
      // FILLSTEP, HALFSTEP, QUARTERSTEP, EIGTHSTEP, SIXTEENTHSTEP

};

/***************************************************
 *
 *   SwOSSwarmDuino - ftDuino using FtSwarmI2C-Board
 *
 ***************************************************/

class SwOSSwarmDuino : public SwOSSwarmI2CCtrl {

  private:
    uint8_t i2cerror = 0; 

  public:
    // constructor, destructor
    SwOSSwarmDuino( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmKelda );
    SwOSSwarmDuino( SwOSCom *com ); 
    ~SwOSSwarmDuino();

    virtual char* myType();                                                // what I am?
	  virtual FtSwarmController_t getType();                                  // what I am?
	  virtual void read();                                                   // run measurements

};