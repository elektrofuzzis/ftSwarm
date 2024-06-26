/*
 * SwOSSNVS.h
 *
 * internal represenation of nvs values.
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include "SwOS.h"

#include <stdint.h>

#define MAXNVSEVENT 36
#define NVSVERSION  2

class NVSEvent {
  public:
    char sensor[ MAXIDENTIFIER];
    uint8_t LR; // 0=none, 1=LR, 2=FB
    char actor[ MAXIDENTIFIER];
    FtSwarmTrigger_t triggerEvent;
    bool             usePortValue;
    int32_t          parameter;
    NVSEvent();
    void reset( void );
};

class NVSEventList {
  public:
    NVSEvent event[MAXNVSEVENT];
    void reset( void );
};

// wifi types
typedef enum { wifiOFF, wifiAP, wifiClient } FtSwarmWifi_t;

class SwOSNVS {
  public:
    void initialSetup();   // ask user for HW details
	  int32_t               version = NVSVERSION;
	  FtSwarmController_t   controllerType;
	  FtSwarmVersion_t      CPU;
	  FtSwarmSerialNumber_t serialNumber;
    uint8_t               channel;
	  char                  wifiSSID[64], wifiPwd[128];
    char                  swarmName[MAXIDENTIFIER];
    uint16_t              swarmSecret, swarmPIN;
    FtSwarmWifi_t         wifiMode;
    int16_t               joyZero[2][2];
    uint8_t               RGBLeds;
    uint8_t               displayType = 1;
    NVSEventList          eventList;
    bool                  webUI;
    bool                  IAmKelda;
    FtSwarmCommunication_t swarmCommunication;
    FtSwarmSerialNumber_t swarmMember[MAXCTRL];
    uint8_t               swarmSpeed = 4;
    FtSwarmExtMode_t      extensionPort;
    bool                  gyro;
    uint8_t               I2CAddr;
    uint8_t               interruptLine; // 0 off, 1 M1, 2 M2
    int16_t               interruptOnOff[2];
    uint8_t               I2CRegisters;

	  SwOSNVS();                             // constructor
	  void begin();                          // read data from nvs & run an _initialSetup if needed
    bool load();                           // load data from nvs, return false if nvs contains invalid data
	  void save( bool writeAll = false );    // save config to flash
    void saveAndRestart();                 // save config & restart
    void createSwarm( char *name, uint16_t pin ); // create a new swarm
    bool addController( FtSwarmSerialNumber_t serialNumber );                       // add a controller
    bool deleteController( FtSwarmSerialNumber_t serialNumber );                    // delete a controller
    void deleteAllControllers( void );
    uint8_t swarmMembers( void ) ;         // number of swarm members
    void factorySettings( void );          // reset to factory settings
    bool RS485Available( void );           // true if board has RS485
    void printNVS();                       // print settings for debugging only  
};

extern SwOSNVS nvs;
