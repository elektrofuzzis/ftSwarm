/*
 * SwOSFirmware.cpp
 *
 * Firmware menues
 * 
 * (C) 2021-23 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOSFirmware.h"

#include <WiFi.h>

#include "SwOS.h"
#include "SwOSSwarm.h"
#include "SwOSNVS.h"
#include "easyKey.h"
#include "SwOSCLI.h"
#include "SwOSLog.h"
#include "SwOSHW/SwOSHWHAL.h"

const char EXTMODE[7][14] = { "off", "I2C-Master", "I2C-Slave", "Outputs", "Servos", "Lidar", "" }; // "" just to avoid seg faults
const char GYRO[3][8]     = { "off", "LSM6", "MPU6050"};
const char ONOFF[2][5]    = { "off", "on" };
const char OFFM1M2[3][5]  = { "off", "M1", "M2" };
const char WIFI[3][12]    = { "off", "AP-Mode", "Client-Mode"};

#define MISCMENUMODE  1
#define MISCMENUGYRO  2
#define MISCMENUI2C   3
#define MISCMENUINT   4
#define MISCMENUINT0  5
#define MISCMENUINT1  6
#define MISCMENUREG   7
#define MISCCALIBRATE 8

void initCalibration( SwOSJoyCalibration_t *calibration ) {
  calibration->minValue = 500;
  calibration->maxValue = 3500;
}

bool testCalibration( int32_t value, SwOSJoyCalibration_t *calibration, char visualizer[], uint8_t p1, uint8_t p2 ) {

  if ( value == FILTER_INVALID ) return false;

  bool change = false;

  if (value < calibration->minValue ) { change = true; calibration->minValue = value; visualizer[p1] = '+'; }
  if (value > calibration->maxValue ) { change = true; calibration->maxValue = value; visualizer[p2] = '+'; }

  return change;

}

bool calibrateJoysticks( SwOSJoyCalibration_t calibration[4] ) {

  SwOSDigitalInput*    s1 = (SwOSDigitalInput*)myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_S1 );
  SwOSDigitalInput*    s4 = (SwOSDigitalInput*)myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_S4 );
  SwOSJoystick*        joy[2];
  char                 visualizer[15];
  SwOSJoyCalibration_t newCalibration[4];

  // get direct readings
  joy[0] = (SwOSJoystick *)myOSSwarm.Ctrl[0]->getIO("JOY1");
  joy[1] = (SwOSJoystick *)myOSSwarm.Ctrl[0]->getIO("JOY2");

  // drop joystick filters
  joy[0]->fb->deleteFilter( SWOS_FILTER_JOYSTICK );
  joy[0]->lr->deleteFilter( SWOS_FILTER_JOYSTICK );
  joy[1]->fb->deleteFilter( SWOS_FILTER_JOYSTICK );
  joy[1]->lr->deleteFilter( SWOS_FILTER_JOYSTICK );
  
  // init calibration
  for ( uint8_t i=0; i<4; i++ ) initCalibration( &newCalibration[i] );

  // since some filters are dropped, we need to wait for new values
  delay(100);

  // 1st step: rotate the stick to get min/max values

  strcpy( visualizer, "---- ----" );
  printf("\nPlease rotate both joysticks.\nClick S1 when all - changed to + or S4 to abort. %s", visualizer ); flushStdIO();

  while ( true ) {

    // abort?
    if ( s4->getToggle() == FTSWARM_TOGGLEUP ) { 
      joy[0]->fb->addFilter( new SwOSFJoystick( calibration[0].minValue, calibration[0].midValue, calibration[0].maxValue ) );
      joy[0]->lr->addFilter( new SwOSFJoystick( calibration[1].minValue, calibration[1].midValue, calibration[1].maxValue ) );
      joy[1]->fb->addFilter( new SwOSFJoystick( calibration[2].minValue, calibration[2].midValue, calibration[2].maxValue ) );
      joy[1]->lr->addFilter( new SwOSFJoystick( calibration[3].minValue, calibration[3].midValue, calibration[3].maxValue ) );
      return false; 
    }

    // finish?
    if ( ( s1->getToggle() == FTSWARM_TOGGLEUP ) && ( strcmp( visualizer, "++++ ++++" ) == 0 ) ) { break; }

    if ( testCalibration( joy[0]->lr->getValueI32(), &newCalibration[0], visualizer, 0, 3 ) ||
         testCalibration( joy[0]->fb->getValueI32(), &newCalibration[1], visualizer, 1, 2 ) ||
         testCalibration( joy[1]->lr->getValueI32(), &newCalibration[2], visualizer, 5, 8 ) ||
         testCalibration( joy[1]->fb->getValueI32(), &newCalibration[3], visualizer, 6, 7 ) ) {
      printf("\b\b\b\b\b\b\b\b\b%s", visualizer); flushStdIO();
    }
    
    // wait for new values
    delay(25);

  }

  // 2nd step get mid / released positions

  printf("\nPlease release both joysticks or press S4 to abort.\n");

  int32_t lastValue[4] = { FILTER_INVALID, FILTER_INVALID, FILTER_INVALID, FILTER_INVALID };
  int32_t newValue[4]  = { FILTER_INVALID, FILTER_INVALID, FILTER_INVALID, FILTER_INVALID };
  uint8_t nTimes = 0;

  while (nTimes < 3) {

    // abort?
    if ( s4->getToggle() == FTSWARM_TOGGLEUP ) { 
      joy[0]->fb->addFilter( new SwOSFJoystick( calibration[0].minValue, calibration[0].midValue, calibration[0].maxValue ) );
      joy[0]->lr->addFilter( new SwOSFJoystick( calibration[1].minValue, calibration[1].midValue, calibration[1].maxValue ) );
      joy[1]->fb->addFilter( new SwOSFJoystick( calibration[2].minValue, calibration[2].midValue, calibration[2].maxValue ) );
      joy[1]->lr->addFilter( new SwOSFJoystick( calibration[3].minValue, calibration[3].midValue, calibration[3].maxValue ) );
      return false; 
    }

    // catch new values
    newValue[0] = joy[0]->fb->getValueI32();
    newValue[1] = joy[0]->lr->getValueI32();
    newValue[2] = joy[1]->fb->getValueI32();
    newValue[3] = joy[1]->lr->getValueI32();

    // are the new values the same values as last time?
    bool stable = true;
    for (uint8_t i=0; i<4; i++) {
      stable == stable || ( lastValue[i] != FILTER_INVALID ) || ( newValue[i] != FILTER_INVALID ) || ( lastValue[i] == newValue[i] );
      lastValue[i] = newValue[i];
    }

    // if all is clear, increment counter, otherwise reset it
    if ( stable ) nTimes++; else nTimes = 0;

    // wait for new values
    delay(25);
    
  }

  // all done, copy values 
  for (uint8_t i=0; i<4; i++) {
    newCalibration[i].midValue = newValue[i];
  }
  memcpy( calibration, newCalibration, 4 * sizeof( SwOSJoyCalibration_t ) );

  // set filters  
  joy[0]->fb->addFilter( new SwOSFJoystick( calibration[0].minValue, calibration[0].midValue, calibration[0].maxValue ) );
  joy[0]->lr->addFilter( new SwOSFJoystick( calibration[1].minValue, calibration[1].midValue, calibration[1].maxValue ) );
  joy[1]->fb->addFilter( new SwOSFJoystick( calibration[2].minValue, calibration[2].midValue, calibration[2].maxValue ) );
  joy[1]->lr->addFilter( new SwOSFJoystick( calibration[3].minValue, calibration[3].midValue, calibration[3].maxValue ) );

  return true;

}

void miscSettingsMenu() {

  bool          anythingChanged = false;
  char          prompt[255];
  SwOSJoystick* joystick = NULL;

  Menu menu;

  while (1) {

    /*
    (1) Mode: Master/Slave/TXT
    (2) Gyro: LM6/MCU/OFF
    (3) Slave Address: x
    (0) exit
    */

    /* Mode:
        I2C Master
        I2C Slave
        MCU Gyro
        Motor-IO 
    */

    menu.start("Misc Settings", 20);

    if ( myOSSwarm.Ctrl[0]->hasExtPort() ) menu.add("Mode", EXTMODE[ nvs.extensionPort] , MISCMENUMODE );

    // I2C Slave Mode. Options I2C Slave Address and Interrupt Line
    if ( nvs.extensionPort == FTSWARM_EXT_I2C_SLAVE ) {
      menu.add("I2C Slave Address", nvs.I2CAddr, MISCMENUI2C);
      menu.add("Interrupt Line", OFFM1M2[nvs.interruptLine], MISCMENUINT);
      menu.add("Interrupt Low Value",  nvs.interruptOnOff[0], MISCMENUINT0);
      menu.add("Interrupt High Value", nvs.interruptOnOff[1], MISCMENUINT1);
      menu.add("I2C Registers", nvs.I2CRegisters, MISCMENUREG);
    }

    // gyro if available
    if ( myOSSwarm.Ctrl[0]->hasGyro() ) menu.add("Gyro", ONOFF[nvs.gyro], MISCMENUGYRO );

    if ( myOSSwarm.Ctrl[0]->getType() == FTSWARMCONTROL ) {
      menu.add("Calibrate Joysticks", "", MISCCALIBRATE, false );

    }

    switch( menu.userChoice() ) {
      
      case 0: // exit
        if ( ( anythingChanged) && ( yesNo( "To apply your changes, the device needs to be restarted.\nSave settings and restart now (Y/N)?") ) ) {
          // save config
          nvs.saveAndRestart();
        } else {
          return;
        }
        
      case MISCMENUMODE: // ExtMode
        anythingChanged = true;
        if ( myOSSwarm.Ctrl[0]->getType() == FTSWARMCONTROL ) {
          FtSwarmExtMode_t newMode =  (FtSwarmExtMode_t) enterNumber( "(-) off (1) I2C-Master (-) I2C-Slave (-) Outputs (-) Servos (5) Lidar: ", nvs.extensionPort, 0, 5 );
          if ( ( newMode == FTSWARM_EXT_I2C_MASTER ) ||
               ( newMode == FTSWARM_EXT_LIDAR ) ) {
               nvs.extensionPort = newMode;
          }
        } else {
          nvs.extensionPort = (FtSwarmExtMode_t) enterNumber( "(0) off (1) I2C-Master (2) I2C-Slave (3) Outputs (4) Servos (5) Lidar: ", nvs.extensionPort, 0, 5 );
        }
        break;

      case MISCMENUGYRO: // Gyro
        anythingChanged = true;
        nvs.gyro = (FtSwarmGyroMode_t) enterNumber( "(0) off (1) on: ", nvs.gyro, 0, 1 );
        if ( ( nvs.gyro ) && ( nvs.CPU != FTSWARMRS_2V0 ) && ( nvs.CPU != FTSWARMRS_2V1 ) ) nvs.extensionPort = FTSWARM_EXT_I2C_MASTER;
        break;

      case MISCMENUI2C: // I2C Addr
        anythingChanged = true;
        nvs.I2CAddr = (uint8_t) enterNumber( "[16..127]: ", nvs.I2CAddr, 16, 127 );
        break;
        
      case MISCMENUINT: // Interrupt Line
        anythingChanged = true;
        nvs.interruptLine = (uint8_t) enterNumber( "motor (1 for M1, 2 for M2, ...) or 0 to skip: ", nvs.interruptLine, 0, MAXIOS[nvs.CPU].motors );
        break;

      case MISCMENUINT0: // Interrupt Line Low
        anythingChanged = true;
        nvs.interruptOnOff[0] = (int16_t) enterNumber( "Low value [-255..255]", nvs.interruptOnOff[0], -255, 255 );
        break;

      case MISCMENUINT1: // Interrupt Line High
        anythingChanged = true;
        nvs.interruptOnOff[1] = (int16_t) enterNumber( "High Value [-255..255]", nvs.interruptOnOff[1], -255, 255 );
        break;

      case MISCMENUREG: // Max I2CRegisters
        anythingChanged = true;
        nvs.I2CRegisters = (uint8_t) enterNumber( "I2C Registers [1..8]", nvs.I2CRegisters, 1, MAXI2CREGISTERS);
        break;

      case MISCCALIBRATE: // calibrate joysticks
        if  ( yesNo( "\nStart calibration (Y/N)?" ) ) {
          anythingChanged = true;
          calibrateJoysticks( nvs.calibration );
        }
        break;

    }
  }

}

void wifiMenu( void ) {

  char          info[250];
  bool          anythingChanged = false;
  uint8_t       maxChoice;
  FtSwarmWifi_t wifiMode;

  Menu menu;
  
  while(1) {

    switch ( nvs.wifiMode ) {
    case wifiAP:      sprintf(info, "hostname:           %s\nip-address:         %d.%d.%d.%d\n\n", myOSSwarm.Ctrl[0]->getHostname(), WiFi.softAPIP()[0], WiFi.softAPIP()[1], WiFi.softAPIP()[2], WiFi.softAPIP()[3]);
                      break;
    case wifiClient:  sprintf(info, "hostname:           %s\nip-address:         %d.%d.%d.%d\n\n", myOSSwarm.Ctrl[0]->getHostname(), WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
                      break;
    default:          sprintf(info, "hostname:           %s\nip-address:         none\n\n",myOSSwarm.Ctrl[0]->getHostname());
    }

    // build menu
    menu.start( "Wifi & WebUI", 14 );
    printf(info);
    menu.add( "wifi", WIFI[nvs.wifiMode], 1);

    if (nvs.wifiMode != wifiOFF ) {
      
      menu.add( "SSID", nvs.wifiSSID, 2);
      
      if (nvs.wifiMode != wifiAP) {
        menu.add( "Password", "*****", 3 );
      } else {
        menu.add( "channel", nvs.channel, 4 );
      } 
      
      menu.add( "Web UI", ONOFF[nvs.webUI], 5);
      
      if ( nvs.webUI ) menu.add( "ftPixels in UI", nvs.pixels, 6);
    
    }

    switch ( menu.userChoice(  ) ) {

      case 0: // exit
        if ( ( anythingChanged) && ( yesNo( "To apply your changes, the device needs to be restarted.\nSave settings and restart now (Y/N)?") ) ) {
          // save config
          nvs.saveAndRestart();
        } else {
          return;
        }
        
      case 1: // AP-Mode/Client-Mode
        wifiMode = (FtSwarmWifi_t) enterNumber( "enter wifi mode [ 0-off , 1-AP-Mode, 2-Client-Mode]: ", nvs.wifiMode, 0, 2 );
        if ( nvs.wifiMode != wifiMode ) {
          if ( ( wifiMode == wifiOFF ) && ( nvs.swarmCommunication & 0x1 ) ) {
            printf("\e[0;31mError: please deactivate wifi in swarm communication first.\e[0m\n");
          } else {
            nvs.wifiMode = wifiMode;
            if ( ( nvs.wifiMode == wifiAP ) && ( ( nvs.channel < 1 ) || ( nvs.channel > 13 ) ) ) nvs.channel = 1; // to avoid invalid channel settings
            anythingChanged = true;
          } 
        }
        break;
        
      case 2: // SSID
        anythingChanged = true;
        enterString("Please enter new SSID: ", nvs.wifiSSID, 64);
        break;
        
      case 3: // Password
        anythingChanged = true;
        enterString("Please enter new Password: ", nvs.wifiPwd, 64, true);
        break;

      case 4: // Channel
        anythingChanged = true;
        nvs.channel = enterNumber( "enter channel [1..13] - use 1,6 or 11 if possible: ", nvs.channel, 1, 13 );
        break;

      case 5: // WebServer on/off
        anythingChanged = true;
        nvs.webUI = !nvs.webUI;
        break;
        
      case 6: // # of ftPixel
        anythingChanged = true;
        nvs.pixels = enterNumber( "enter number of ftPixel in WebUI [2..18]: ", nvs.pixels, 2, MAXLEDS );
        break;
    }
  }
}

void newSwarm( void ) {

  char name[MAXIDENTIFIER];
  name[0] = '\0';
  while (strlen(name) < 5) enterString("New Swarm Name [min. 5 chars]: ", name, MAXIDENTIFIER );
  uint16_t pin = enterNumber("New Swarm Pin [1..9999]: ", -1, 1, 9999 );

  if ( myOSSwarm.Ctrl[0]->IAmKelda ) {
    if (!yesNo( "Destroy the existing swarm and create a new one? [Y/N] " ) ) return;
  } else {
    if (!yesNo( "Leave the existing swarm and create a new one? [Y/N] " ) ) return;
  }

  strcpy( nvs.swarmName, name );
  nvs.swarmPIN = pin;

  myOSSwarm.newSwarm(  );

  nvs.save( );

}

void addController( void ) {

  FtSwarmSerialNumber_t serialNumber = (FtSwarmSerialNumber_t) enterNumber("Enter new swarm members serial number [1..9999]: ", -1, 1, 9999 );

  if ( myOSSwarm.isMember( serialNumber ) ) { printf("\e[0;31mERROR: This controller is already part of this swarm.\e[0m\n"); return; }
  
  if ( !myOSSwarm.addController( serialNumber ) ) {
    // no space left
    printf("\e[0;31mERROR: No space left in swarm. Controller #%d was declined.\e[0m\n", serialNumber );
    return;
  }

  printf("Controller SN %d was added to the swarm.\n", serialNumber );
  if ( !myOSSwarm.isOnline( serialNumber ) ) printf("\e[0;31mWARNING: Please switch controller #%d on.\e[0m\n", serialNumber);

  nvs.save( );

}

void deleteController( void ) {

  FtSwarmSerialNumber_t serialNumber = (FtSwarmSerialNumber_t) enterNumber("Enter serial number to be revoked [1..9999]: ", -1, 1, 9999 );

  if ( !myOSSwarm.isMember( serialNumber ) ) { printf("\e[0;31mERROR: This controller isn't part of this swarm.\e[0m\n"); return; }
  
  if ( !myOSSwarm.deleteController( serialNumber ) ) {
    // not found
    printf("\e[0;31mERROR: This controller isn't part of this swarm.\e[0m\n");
    return;
  }

  printf("Controller SN %d was revoked from the swarm.\n", serialNumber );

  nvs.save( );

}

const char SWARMCOMMUNICATION[4][13] = { "none", "wifi", "RS485", "wifi & RS485" };
const char COMSTATE[5][30] = { "\e[1;31mOFFLINE!\e[0m", "\e[1;31mOFFLINE\e[0m", "\e[1;33mCONNECTING\e[0m", "\e[1;32mONLINE\e[0m", "\e[1;31mERROR\e[0m\e[0m" };

// swam menu identifiers

#define MENUSWARMCREATE        1
#define MENUSWARMADD           2
#define MENUSWARMDELETE        3
#define MENUSWARMCOMMUNICATION 4
#define MENUSWARMSPEED         5

void swarmMenu( void ) {

  char kelda[MAXIDENTIFIER];
  FtSwarmCommunication_t swarmCommunication;
  Menu menu;

  while (1) {

    menu.start("swarm configuration", 19 );

    if (myOSSwarm.Ctrl[0]->IAmKelda) 
      printf("%s is Kelda running swarm \"%s\" using Pin %d:\n\nSN  NW Age State  Hostname \n", myOSSwarm.Ctrl[0]->getHostname(), nvs.swarmName, nvs.swarmPIN );
    else
      printf( "%s is connected to swarm \"%s\" uising Swarm PIN %d.\n", myOSSwarm.Ctrl[0]->getHostname(), nvs.swarmName, nvs.swarmPIN );

    for ( int8_t i=0; i<=myOSSwarm.maxCtrl; i++ ) {
      if ( myOSSwarm.Ctrl[i] ) {
        printf("%3d %.6lu %-11s %s\n", myOSSwarm.Ctrl[i]->serialNumber, myOSSwarm.Ctrl[i]->networkAge(), COMSTATE[myOSSwarm.Ctrl[i]->getComState()], myOSSwarm.Ctrl[i]->getHostname() );
      }
    }

    printf("\n");

    if ( nvs.RS485Available() ) {
      menu.add( "swarm communication", SWARMCOMMUNICATION[nvs.swarmCommunication], MENUSWARMCOMMUNICATION );
      if ( nvs.swarmCommunication != swarmComWifi ) menu.add( "swarm speed", nvs.swarmSpeed, MENUSWARMSPEED);
    }
    
    menu.add( "create a new swarm", "", MENUSWARMCREATE );

    if (nvs.IAmKelda) {
      menu.add( "add a controller", "", MENUSWARMADD );
      menu.add( "revoke a controller", "", MENUSWARMDELETE );
    }

    switch( menu.userChoice() ) {
      case 0: // main
        return;

      case MENUSWARMCOMMUNICATION: 
        if ( !nvs.RS485Available() ) {
          printf("This controller just supports wifi.\n");
        } else {
          if ( nvs.IAmKelda ) swarmCommunication = (FtSwarmCommunication_t) enterNumber( "enter swarm communication [1-wifi, 2-RS485, 3-both]:", nvs.swarmCommunication, 1, 3 );
          else                swarmCommunication = (FtSwarmCommunication_t) enterNumber( "enter swarm communication [1-wifi, 2-RS485]:", nvs.swarmCommunication, 1, 2 );
          if (nvs.swarmCommunication != swarmCommunication) {
            // test if wifiMode is OFF and swarm should use wifi
            if ( ( nvs.wifiMode == wifiOFF ) && ( swarmCommunication & 0x1 ) ) {
              printf("\e[0;31mError: please activate wifi first.\e[0m\n");
            } else {
              // let's save data
              nvs.swarmCommunication = swarmCommunication;
              if ( yesNo( "To apply your changes, the device needs to be restarted.\nSave settings and restart now (Y/N)?") ) nvs.saveAndRestart();
            }
          }
        }
        break;

      case MENUSWARMSPEED:
        nvs.swarmSpeed = enterNumber( "(0) low ... (4) highspeed (max. 50m)>", nvs.swarmSpeed, 0, 4 );
        if ( yesNo( "To apply your changes, the device needs to be restarted.\nSave settings and restart now (Y/N)?") ) nvs.saveAndRestart();
        break;

      case MENUSWARMCREATE:
        newSwarm();
        break;
        
      case MENUSWARMADD:
        addController();
        break;

      case MENUSWARMDELETE:
        deleteController();
        break;

      }

  }

}

uint8_t selectController( uint8_t controller ) {

  Menu menu;

  menu.start("Please select a controller", 10);

  for (uint8_t i=0; i<=myOSSwarm.maxCtrl; i++) {
    if ( myOSSwarm.Ctrl[i] ) menu.add( myOSSwarm.Ctrl[i]->getName(), "", i );
  }

  uint8_t choice = menu.userChoice( );

  printf( "choice: %d\n", choice);

  // nothing selected -> default controller
  if ( choice == 255 ) return controller;

  // index number of controller
  return choice;

}

bool setAliasAndType( SwOSObj *selected, SwOSCtrl *ctrl ) {
  
  char         alias[MAXIDENTIFIER];
  char         prompt[250];

  // null -> done!
  if (!selected) return false;
  
  // ask user for new alias
  sprintf( prompt, "%s - please enter new alias: ", selected->getName() );
  enterIdentifier( prompt, alias, MAXIDENTIFIER );
                
  // test on duplicates
  SwOSIO *testIO = myOSSwarm.getIO( alias );
  if ( (testIO) && ( testIO != selected ) ) {
    printf("\e[0;31mERROR: This alias is already used in the swarm.\n\e[0m");
    return false;
  }

  // change name
  bool anythingChanged = strcmp( alias, selected->getAlias() );
  if (anythingChanged ) selected->setAlias( alias );

  // if it's the controller, we're done
  if (selected == ctrl) return anythingChanged;

  // change type?
  SwOSIO*      io     = (SwOSIO*)selected;
  SwOSIOType_t ioType = io->getIOType();

  // singular class -> done
  if ( SWOSIOCLASS[ioType] == SWOSIOCLASS_SINGULAR ) return anythingChanged;

  // ftPwrDrive -> done
  if ( io->getCtrl()->getCPU() == FTSWARMPWRDRIVE_1V141 ) return anythingChanged;

  // list compatible types and ask user
  int8_t       maxType = -1;
  SwOSIOType_t defaultValue, type[99];

  for (uint8_t i=0; i<SWOSIO_MAXIOTYPE; i++) {

    // compatible type?
    if ( SWOSIOCLASS[ioType] == SWOSIOCLASS[i] ) {

      maxType++;

      // default?
      if ( ioType == (SwOSIOType_t) i ) defaultValue = (SwOSIOType_t)i;

      // menu entry
      type[maxType] = (SwOSIOType_t) i;
      printf( "(%2d) %s\n", maxType, SWOSIOTYPE[i] );

    }

  }

  sprintf( prompt, "Choose new IO Type - default %s:", SWOSIOTYPE[defaultValue]);
  SwOSIOType_t newIOType = type[enterNumber( prompt, defaultValue, 0, maxType )];

  if ( ioType != newIOType ) { 
    if ( ctrl->changeIOType( ctrl->getIndex( io ), newIOType ) ) anythingChanged = true;
  }

  return anythingChanged;
               
}

void ioTypesMenu( void ) {

  SwOSObj      *OSObj[99];
  bool         anythingChanged[MAXCTRL];
  uint8_t      controller = 0;
  Menu         menu;
  char         data[80];
  bool         showPixel = false;

  // initialize anythingChanged
  for (uint8_t i=0; i<MAXCTRL; i++) anythingChanged[i] = false;

  while (1) {

    uint8_t item = 0;
    menu.start( "io types menu:", 10, 255, ' ' );

    printf("      Name        Type            Alias\n");

    // show existing alias
    OSObj[item++] = myOSSwarm.Ctrl[controller]; 
    sprintf( data, "Controller      %s", myOSSwarm.Ctrl[controller]->getAlias() );
    menu.add( myOSSwarm.Ctrl[controller]->getName(), data, item, true );
  
    // list IOs
    for (uint8_t i=0; i<myOSSwarm.Ctrl[controller]->IOs; i++ ) {

      if ( myOSSwarm.Ctrl[controller]->io[i] ) { 

        SwOSIOType_t ioType = myOSSwarm.Ctrl[controller]->io[i]->getIOType();

        if ( ( showPixel && ( ioType == SWOSIO_PIXEL ) ) || ( !showPixel && ( ioType != SWOSIO_PIXEL ) ) ) {

          OSObj[item++] = myOSSwarm.Ctrl[controller]->io[i];
          sprintf( data, "%-15s %s", SWOSIOTYPE[ioType], myOSSwarm.Ctrl[controller]->io[i]->getAlias() );
          menu.add( myOSSwarm.Ctrl[controller]->io[i]->getName(), data, item, true );
        }

      }

    }

    printf("\n");
    if ( showPixel ) menu.add( "Standard IO", "", 98 );
    else             menu.add( "ftPixel",     "", 98 );
  
    // Kelda only: option to select another controller in the swarm
    if ( myOSSwarm.Ctrl[0]->IAmKelda ) {
      printf("\n");
      menu.add( "Controller", myOSSwarm.Ctrl[controller]->getName(), 99 );
    }

    // User's choice
    uint8_t choice = menu.userChoice( );
    
    uint8_t i;
    bool changes = false;

    switch (choice) {

      case  0:  // exit: save nvs?

                // any changes?
                for ( i=0; i<MAXCTRL; i++ ) { if (anythingChanged[i]) changes = true; }

                if ( ( changes )  &&  yesNo( "Save changes? (Y/N)?" ) ) {

                  // local changes
                  if ( anythingChanged[0] ) {

                    // save in local nvs
                    myOSSwarm.Ctrl[0]->save(2);

                    // send new config to Kelda
                    if ( ( myOSSwarm.Kelda ) && ( myOSSwarm.Kelda != myOSSwarm.Ctrl[0] ) ) myOSSwarm.Ctrl[0]->sendIOConfig( myOSSwarm.Kelda->macAddr );

                  }

                  // remote changes
                  for ( i=1; i<MAXCTRL; i++ ) {
                    if ( anythingChanged[i] ) {
                      myOSSwarm.Ctrl[i]->sendIOConfig( myOSSwarm.Ctrl[i]->macAddr );
                      myOSSwarm.Ctrl[i]->save( 2 );
                    }
                  }
          
                }
                return;

      case 98:  showPixel = !showPixel;
                break;
      
      case 99:  controller = selectController( controller );
                printf("controller: %d\n", controller);
                break;

      case 255: break;

      default:  // set alias & type
                anythingChanged[controller] = setAliasAndType( OSObj[choice-1], myOSSwarm.Ctrl[controller] );
                break;
      }
    
  }
  
}

void factorySettings( void ) {
  // reset controller to factory settings

  if (yesNo("Do you want to reset this device to it's factory settings (Y/N)?" ) ) {

    nvs.factorySettings();

    myOSSwarm.Ctrl[0]->factorySettings();

    printf("device will restart now.\n");

    delay(2000);

    // Alias names
    myOSSwarm.Ctrl[0]->saveToNVS( );

    nvs.saveAndRestart();

  }

}

const char FTSWARMTRIGGER[FTSWARM_MAXTRIGGER][12] = {
  "TriggerDown",
  "TriggerUp",
  "ChangeValue",
  "I2CRead",
  "I2CWrite"
};

bool enterIO( const char* prompt, SwOSIOUID_t *uio, bool input ) {

  char   alias[MAXIDENTIFIER];
  SwOSIO *io;

  while (1) {

    enterString( prompt, alias, sizeof(alias) );

    // default
    if ( alias[0] == '\0' ) {
      io = myOSSwarm.getIO( *uio );
      if (io) strcpy( alias, io->getAlias() );
    }

    // user abort
    if ( alias[0] == '\0' ) return false;

    // get IO
    io = myOSSwarm.getIO( alias );

    // error handling;
    if      ( !io )                              printf("Error: %s doesn't exists.\n", alias); 
    else if ( ( input  ) && ( !io->isInput() ) ) printf("Error: %s is not a sensor\n", alias);
    else if ( ( !input ) && ( !io->isActor() ) ) printf("Error: %s is not a actor\n", alias);
    // all good
    else break;
    
  }

  uio->serialNumber = io->getCtrl()->serialNumber;
  uio->ioType       = io->getIOType();
  uio->port         = io->getPort();

  return true;

}

const char* getAction( SwOSIOUID_t uio ) {

  SwOSIO *io = myOSSwarm.getIO( uio );

  if (!io) return "";
  if (io->isMotor()) return "setSpeed";
  if (io->isPixel()) return "setColor";
  if (io->isServo()) return "setPosition";

  return "";

}

bool enterEvent( SwOSNVSEvent_t *event ) {

  char           prompt[128];
  SwOSIO         *io;

  // sensor
  io = myOSSwarm.getIO( event->sensor );
  if (io) sprintf( prompt, "Enter sensor's name [%s]: ", io->getAlias() );
  else    sprintf( prompt, "Enter sensor's name: " );
  if (! enterIO( prompt, &event->sensor, true  ) ) { return false; }

  // trigger
  sprintf( prompt, "Enter trigger event (0) trigger down (1) trigger up (2) use sensor value [%d]: ", event->trigger );
  event->trigger = (FtSwarmTrigger_t) enterNumber( prompt, event->trigger, 0, 2 );

  // actor
  io = myOSSwarm.getIO( event->actor );
  if (io) sprintf( prompt, "Enter actor's name [%s]: ", io->getAlias() );
  else    sprintf( prompt, "Enter actor's name: " );
  if (! enterIO( prompt,  &event->actor,  false ) ) { return false; }
  
  // constant value
  if (event->trigger != FTSWARM_TRIGGERVALUE ) {
    sprintf( prompt, "Enter value to apply to %s.%s() [%d]: ", myOSSwarm.getIO( event->actor )->getAlias(), getAction( event->actor ), event->parameter );
    event->parameter = enterNumber( prompt, event->parameter, -4096, 0xFFFFFF );
  }

  return true;

}

bool changeEvent( uint8_t config, SwOSNVSEvent_t *event ) {

  // create a copy of the event
  SwOSNVSEvent_t newEvent;
  memcpy( &newEvent, event, sizeof(SwOSNVSEvent_t) );

  // ask user
  if ( !enterEvent( &newEvent ) ) return false;

  // nothing changed?
  if ( cmpEvent( &newEvent, event ) == 2 ) return false;

  // duplicates?
  for (uint8_t i=0; i< MAXNVSEVENTS; i++ ) {

    if ( ( cmpEvent( &newEvent, &nvs.events[config][i] ) >0 ) &&
         ( event != &nvs.events[config][i])
       ) {

      printf("ERROR: This event already exists.");
      return false;
    
    }

  }

  // change event
  myOSSwarm.deleteEvent( event );
  memcpy( event, &newEvent, sizeof( SwOSNVSEvent_t ) );
  myOSSwarm.addEvent( event );

  return true;

}

bool deleteEvent( uint8_t config, uint8_t events ) {

  char prompt[255];
  sprintf( prompt, "Which event should be deleted? [0 - abort, 1..%d]:", events );
  uint8_t event = enterNumber( prompt, 0, 0, events );

  // abort
  if ( event==0 ) return false;

  // delete event
  myOSSwarm.deleteEvent( &nvs.events[config][event-1] );

  // move all successors
  if ( event < MAXNVSEVENTS ) memcpy( &nvs.events[config][event-1], &nvs.events[config][event], ( MAXNVSEVENTS - event ) * sizeof( SwOSNVSEvent_t ) );

  // cleanup last event
  bzero( &nvs.events[config][MAXNVSEVENTS-1], sizeof( SwOSNVSEvent_t) );

  return true;

}

#define REMOTECTRL_ADD MAXNVSEVENTS + 2
#define REMOTECTRL_DEL MAXNVSEVENTS + 3
#define REMOTECTRL_CFG MAXNVSEVENTS + 4

void remoteControl( void ) {

  Menu    menu;
  char    line[128];
  char    value[MAXIDENTIFIER];
  char    sensor[MAXIDENTIFIER];
  char    actor[MAXIDENTIFIER];
  uint8_t events = 0;
  bool    anythingChanged = false;
  uint8_t newConfig;

  while (1) {

    sprintf( line, "Remote Control #%d", nvs.activeEventConfig +1 );
    menu.start( line, 0 );

    events = 0;
  
    for (uint8_t i=0; i<MAXNVSEVENTS; i++) {

      // end of list?
      if ( nvs.events[nvs.activeEventConfig][i].sensor.serialNumber == 0) break;

      // increase events counter
      events++;
        
      // add menu item
      myOSSwarm.getAlias( nvs.events[nvs.activeEventConfig][i].sensor, sensor );
      myOSSwarm.getAlias( nvs.events[nvs.activeEventConfig][i].actor,  actor  );

      if ( nvs.events[nvs.activeEventConfig][i].trigger == FTSWARM_TRIGGERVALUE ) 
        sprintf( value, "%s", sensor );
      else
        sprintf( value, "%d", nvs.events[nvs.activeEventConfig][i].parameter );

      sprintf(line, "%s.%s -> %s.%s(%s)", sensor, FTSWARMTRIGGER[nvs.events[nvs.activeEventConfig][i].trigger], actor, getAction(nvs.events[nvs.activeEventConfig][i].actor), value );
      menu.add( line, "", i+1 );

    }

    printf("\n");

    if ( events < MAXNVSEVENTS ) menu.add( "add event", "", REMOTECTRL_ADD );
    menu.add( "delete event", "", REMOTECTRL_DEL );
    menu.add( "switch configuration", "", REMOTECTRL_CFG );

    uint8_t choice = menu.userChoice( );
    
    switch (choice) {

      case 0:               if ( ( anythingChanged ) && ( yesNo("Save configuration [Y/N]?") ) ) nvs.saveEvents();
                            return;

      case REMOTECTRL_ADD:  printf("\n" ); 
                            if ( changeEvent( nvs.activeEventConfig, &nvs.events[nvs.activeEventConfig][events] ) ) anythingChanged = true;
                            break;

      case REMOTECTRL_DEL:  printf("\n");
                            if ( deleteEvent( nvs.activeEventConfig, events ) ) anythingChanged = true;
                            break;

      case REMOTECTRL_CFG:  sprintf( line, "Switch to configuration [1..%d]", MAXEVENTCONFIGS );
                            newConfig = enterNumber( line, nvs.activeEventConfig+1, 1, MAXEVENTCONFIGS ) -1;
                            if ( newConfig != nvs.activeEventConfig ) {
                              anythingChanged = true;
                              nvs.activeEventConfig = newConfig;
                              myOSSwarm.deleteEvents();
                              myOSSwarm.addEvents( newConfig );
                            }
                            break;

      default:              printf("\n"); 
                            if ( changeEvent( nvs.activeEventConfig, &nvs.events[nvs.activeEventConfig][choice -1] ) ) anythingChanged = true;
                            break;

    }

  }

}

#define MAINMENUWEB       1
#define MAINMENUSWARM     2
#define MAINMENUIOTYPES   3
#define MAINMENUFACTORY   4
#define MAINMENUREMOTE    5
#define MAINMENUMISC      6

void mainMenu( void ) {

  Menu menu;

  while (1) {

    menu.start( "Main Menu", 14 );
    menu.add("Wifi & Web UI", "", MAINMENUWEB );
    if ( ( WiFi.status() == WL_CONNECTED ) || ( nvs.wifiMode == wifiAP ) || ( nvs.RS485Available() ) ) {
      menu.add("Swarm Configuration", "", MAINMENUSWARM );
    } else {
      menu.add("Swarm Configuration - activate WiFi", "", DEACTIVATED );
    }
    menu.add("IO Types & Alias Names", "", MAINMENUIOTYPES );

    if (myOSSwarm.Ctrl[0]->IAmKelda) menu.add("Remote Control", "", MAINMENUREMOTE );

    // special HW
    switch (myOSSwarm.Ctrl[0]->getType()) {

      case FTSWARMCONTROL:
      case FTSWARM:         menu.add("Misc Settings", "", MAINMENUMISC );
                            break;

    }

    menu.add("Factory Reset", "", MAINMENUFACTORY );

    switch( menu.userChoice(  )  ) {
      case 0:                 return;
      case MAINMENUWEB:       wifiMenu();         break;
      case MAINMENUSWARM:     swarmMenu();        break;
      case MAINMENUIOTYPES:   ioTypesMenu();      break;
      case MAINMENUFACTORY:   factorySettings();  break;
      case MAINMENUREMOTE:    remoteControl();    break;
      case MAINMENUMISC:      miscSettingsMenu(); break;
    }
    
  }

}

/*----------------------------------------*/

void firmware( void ) {

  myOSSwarm.begin( true );

  if ( nvs.IAmKelda ) {
    
    // only Keldas use CLI
    SwOSCLI cli;
    cli.run();

  } else {

    mainMenu();

  }

}