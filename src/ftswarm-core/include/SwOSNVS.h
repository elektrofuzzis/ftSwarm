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

#define MAXNVSEVENTS    50
#define MAXEVENTCONFIGS 4
#define NVSVERSION      3

static const char* NVSNAMESPACE = "ftSwarm";

struct SwOSIOUID_t {
  FtSwarmSerialNumber_t serialNumber;
  SwOSIOType_t          ioType;
  uint8_t               port;
} __attribute__((packed));

struct SwOSJoyCalibration_t {

  int16_t minValue;
  int16_t midValue;
  int16_t maxValue; 

} __attribute__((packed));

struct SwOSNVSEvent_t {

  SwOSIOUID_t      sensor;
  SwOSIOUID_t      actor;
  FtSwarmTrigger_t trigger;
  int32_t          parameter;

} __attribute__((packed));

/* cmpEvent
    2 identical
    1 only parameter different
    0 else
*/
extern "C" bool cmpEvent( SwOSNVSEvent_t *a, SwOSNVSEvent_t *b );


// wifi types
typedef enum { wifiOFF, wifiAP, wifiClient } FtSwarmWifi_t;

class SwOSNVS {
  public:
    void initialSetup();   // ask user for HW details
	  int32_t                version = NVSVERSION;
	  FtSwarmController_t    controllerType;
	  FtSwarmVersion_t       CPU;
	  FtSwarmSerialNumber_t  serialNumber;
    uint8_t                channel;
	  char                   wifiSSID[64], wifiPwd[128];
    char                   swarmName[MAXIDENTIFIER];
    uint16_t               swarmSecret, swarmPIN;
    FtSwarmWifi_t          wifiMode;
    SwOSJoyCalibration_t   calibration[4];
    uint8_t                pixels;
    uint8_t                activeEventConfig;
    SwOSNVSEvent_t         events[MAXEVENTCONFIGS][MAXNVSEVENTS];
    bool                   webUI;
    bool                   IAmKelda;
    FtSwarmCommunication_t swarmCommunication;
    FtSwarmSerialNumber_t  swarmMember[MAXCTRL];
    uint8_t                swarmSpeed = 4;
    FtSwarmExtMode_t       extensionPort;
    bool                   gyro;
    uint8_t                I2CAddr;
    uint8_t                interruptLine; // 0 off, 1 M1, 2 M2
    int16_t                interruptOnOff[2];
    uint8_t                I2CRegisters;

	  SwOSNVS();                             // constructor
	  void begin();                          // read data from nvs & run an _initialSetup if needed
    bool load();                           // load data from nvs, return false if nvs contains invalid data
	  void save( bool writeAll = false );    // save config to flash
    void saveAndRestart();                 // save config & restart
    void saveEvents();                     // save events
    void loadEvents();                     // load events
    void createSwarm( char *name, uint16_t pin ); // create a new swarm
    bool addController( FtSwarmSerialNumber_t serialNumber );                       // add a controller
    bool deleteController( FtSwarmSerialNumber_t serialNumber );                    // delete a controller
    void deleteAllControllers( void );
    uint8_t swarmMembers( void ) ;         // number of swarm members
    void factorySettings( void );          // reset to factory settings
    bool RS485Available( void );           // true if board has RS485
    void printNVS();                       // print settings for debugging only  
    bool upgrade( void );                  // runs an nvs version upgrade, true if an upgrade took place
};

extern SwOSNVS nvs;
