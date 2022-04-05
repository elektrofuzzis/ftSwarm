/*
 * SwOSNVS.h
 *
 *  Created on: 20.10.2021
 *      Author: Stefan
 */

#pragma once

#include <stdint.h>

#include "ftSwarm.h"

class SwOSNVS {
  protected:
    void _initialSetup();   // ask user for HW details
public:
	  int32_t               version;
	  FtSwarmControler_t    controlerType;
	  FtSwarmVersion_t      CPU;
	  FtSwarmVersion_t      HAT;
	  FtSwarmSerialNumber_t serialNumber;
    uint8_t               channel;
	  char                  wifiSSID[64], wifiPwd[128];
    char                  swarmName[MAXIDENTIFIER];
    uint16_t              swarmSecret, swarmPIN;
    bool                  APMode;
    int16_t               joyZero[2][2];

	  SwOSNVS();                             // constructor
	  void begin();                          // read data from nvs & run an _initialSetup if needed
    bool load();                           // load data from nvs, return false if nvs contains invalid data
	  void save( bool writeAll = false );    // save config to flash
    void saveAndRestart();                 // save config & restart
    void createSwarm( char *name, uint16_t pin ); // create a new swarm
    void printNVS();                       // print settings for debugging only  
};
