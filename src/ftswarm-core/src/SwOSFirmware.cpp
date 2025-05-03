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

const char EXTMODE[7][14] = { "off", "I2C-Master", "I2C-Slave", "Outputs", "Servos", "Lidar", "" }; // "" just to avoid seg faults
const char GYRO[3][8]     = { "off", "LSM6", "MPU6050"};
const char ONOFF[2][5]    = { "off", "on" };
const char OFFM1M2[3][5]  = { "off", "M1", "M2" };
const char WIFI[3][12]    = { "off", "AP-Mode", "Client-Mode"};

#define EXTMENUMODE 1
#define EXTMENUGYRO 2
#define EXTMENUI2C  3
#define EXTMENUINT  4
#define EXTMENUINT0 5
#define EXTMENUINT1 6
#define EXTMENUREG  7

void ExtensionMenu() {

  bool    anythingChanged = false;
  char    prompt[255];

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

    // printf( "I2C mode: %d\ngyro: %d\n", nvs.I2CMode, nvs.gyro);

    menu.start("Extension Port", 20);

    if ( myOSSwarm.Ctrl[0]->hasExtPort() ) menu.add("Mode", EXTMODE[ nvs.extensionPort] , EXTMENUMODE );

    // I2C Slave Mode. Options I2C Slave Address and Interrupt Line
    if ( nvs.extensionPort == FTSWARM_EXT_I2C_SLAVE ) {
      menu.add("I2C Slave Address", nvs.I2CAddr, EXTMENUI2C);
      menu.add("Interrupt Line", OFFM1M2[nvs.interruptLine], EXTMENUINT);
      menu.add("Interrupt Low Value",  nvs.interruptOnOff[0], EXTMENUINT0);
      menu.add("Interrupt High Value", nvs.interruptOnOff[1], EXTMENUINT1);
      menu.add("I2C Registers", nvs.I2CRegisters, EXTMENUREG);
    }

    // gyro if available
    if ( myOSSwarm.Ctrl[0]->hasGyro() ) menu.add("Gyro", ONOFF[nvs.gyro], EXTMENUGYRO );

    switch( menu.userChoice() ) {
      
      case 0: // exit
        if ( ( anythingChanged) && ( yesNo( "To apply your changes, the device needs to be restarted.\nSave settings and restart now (Y/N)?") ) ) {
          // save config
          nvs.saveAndRestart();
        } else {
          return;
        }
        
      case EXTMENUMODE: // ExtMode
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

      case EXTMENUGYRO: // Gyro
        anythingChanged = true;
        nvs.gyro = (FtSwarmGyroMode_t) enterNumber( "(0) off (1) on: ", nvs.gyro, 0, 1 );
        if ( ( nvs.gyro ) && ( nvs.CPU != FTSWARMRS_2V0 ) && ( nvs.CPU != FTSWARMRS_2V1 ) ) nvs.extensionPort = FTSWARM_EXT_I2C_MASTER;
        break;

      case EXTMENUI2C: // I2C Addr
        anythingChanged = true;
        nvs.I2CAddr = (uint8_t) enterNumber( "[16..127]: ", nvs.I2CAddr, 16, 127 );
        break;
        
      case EXTMENUINT: // Interrupt Line
        anythingChanged = true;
        nvs.interruptLine = (uint8_t) enterNumber( "(0) off, (1) M1, (2) M2:", nvs.interruptLine, 0, 2 );
        break;

      case EXTMENUINT0: // Interrupt Line Low
        anythingChanged = true;
        nvs.interruptOnOff[0] = (int16_t) enterNumberI32( "Low value [-255..255]", nvs.interruptOnOff[0], -255, 255 );
        break;

      case EXTMENUINT1: // Interrupt Line High
        anythingChanged = true;
        nvs.interruptOnOff[1] = (int16_t) enterNumberI32( "High Value [-255..255]", nvs.interruptOnOff[1], -255, 255 );
        break;

      case EXTMENUREG: // Max I2CRegisters
        anythingChanged = true;
        nvs.I2CRegisters = (uint8_t) enterNumber( "I2C Registers [1..8]", nvs.I2CRegisters, 1, MAXI2CREGISTERS);
        break;

    }
  }

}

void SwarmControlMenu() {

  bool    anythingChanged = false;
  char    prompt[255];

  while (1) {
    
    printf("\n\nftSwarmControl settings\n\n");
    sprintf( prompt, "(1) Display:  type %d\n(2) Calibrate Joysticks\n\n(0) exit\nftSwarmControl>", nvs.displayType );
    switch( enterNumber( prompt, 0, 0, 2) ) {
      
      case 0: // exit
        if ( ( anythingChanged) && ( yesNo( "To apply your changes, the device needs to be restarted.\nSave settings and restart now (Y/N)?") ) ) {
          // save config
          nvs.saveAndRestart();
        } else {
          return;
        }
        
      case 1: // DisplayType
        anythingChanged = true;
        nvs.displayType = 1 + ( !( nvs.displayType - 1 ) );
        break;

      case 2: // calibrate joysticks
        if  ( yesNo( "Start calibration (Y/N)?" ) ) {
          anythingChanged = true;
          static_cast<SwOSSwarmControl *>(myOSSwarm.Ctrl[0])->joystick[0]->calibrate( &nvs.joyZero[0][0], &nvs.joyZero[0][1] );
          static_cast<SwOSSwarmControl *>(myOSSwarm.Ctrl[0])->joystick[1]->calibrate( &nvs.joyZero[1][0], &nvs.joyZero[1][1] );
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
      
      if ( nvs.webUI ) menu.add( "ftPixels in UI", nvs.RGBLeds, 6);
    
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
        nvs.RGBLeds = enterNumber( "enter number of ftPixel in WebUI [2..18]: ", nvs.RGBLeds, 2, MAXLEDS );
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
const char COMSTATE[5][30] = { "\e[1;31mOFFLINE\e[0m", "\e[1;33mPHASE1\e[0m", "\e[1;33mPHASE2\e[0m", "\e[1;32mONLINE\e[0m", "\e[1;31mERROR\e[0m\e[0m" };

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

    if (myOSSwarm.Ctrl[0]->IAmKelda) {

      printf("%s is Kelda running swarm \"%s\" using Pin %d:\n\nSN  NW Age State  Hostname \n", myOSSwarm.Ctrl[0]->getHostname(), nvs.swarmName, nvs.swarmPIN );
      for ( int8_t i=0; i<=myOSSwarm.maxCtrl; i++ ) {
        if ( myOSSwarm.Ctrl[i] ) {
          printf("%3d %.6lu %-11s %s\n", myOSSwarm.Ctrl[i]->serialNumber, myOSSwarm.Ctrl[i]->networkAge(), COMSTATE[myOSSwarm.Ctrl[i]->getComState()], myOSSwarm.Ctrl[i]->getHostname() );
        }
      }

    } else {

      printf( "%s is connected to swarm \"%s\".\nSwarm PIN is %d.\n", myOSSwarm.Ctrl[0]->getHostname(), nvs.swarmName, nvs.swarmPIN );
      if (myOSSwarm.Kelda) {
        printf("Kelda SN %d is online\n", myOSSwarm.Kelda->serialNumber );
      } else {
        printf("Kelda is offline\n");
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

void aliasMenu( void ) {

  SwOSObj *OSObj[99];
  bool    anythingChanged[MAXCTRL];
  uint8_t controller = 0;
  Menu    menu;

  // initialize anythingChanged
  for (uint8_t i=0; i<MAXCTRL; i++) anythingChanged[i] = false;

  while (1) {

    uint8_t item = 0;
    menu.start( "alias menu:", 10, 999 );

    // Kelda only: option to select another controller in the swarm
    if ( myOSSwarm.Ctrl[0]->IAmKelda ) {
      menu.add( "Controller", myOSSwarm.Ctrl[controller]->getName(), 99 );
      printf("\n");
    }

    // show existing alias
    OSObj[item++] = myOSSwarm.Ctrl[controller]; 
    menu.add( myOSSwarm.Ctrl[controller]->getName(), myOSSwarm.Ctrl[controller]->getAlias(), item, true );
  
    // list inputs
    for (uint8_t i=0; i<myOSSwarm.Ctrl[controller]->inputs; i++ ) {
      if ( myOSSwarm.Ctrl[controller]->input[i] ) { 
        OSObj[item++] = myOSSwarm.Ctrl[controller]->input[i];
        menu.add( myOSSwarm.Ctrl[controller]->input[i]->getName(), myOSSwarm.Ctrl[controller]->input[i]->getAlias(), item, true );
      }
    }
  
    // list actors
    for (uint8_t i=0; i<myOSSwarm.Ctrl[controller]->actors; i++ ) {
      if (myOSSwarm.Ctrl[controller]->actor[i]) {
        OSObj[item++] = myOSSwarm.Ctrl[controller]->actor[i];
        menu.add( myOSSwarm.Ctrl[controller]->actor[i]->getName(), myOSSwarm.Ctrl[controller]->actor[i]->getAlias(), item, true );
      }
    }
  
    // list LEDs
    for (uint8_t i=0; i<nvs.RGBLeds; i++ ) {
      if ( myOSSwarm.Ctrl[controller]->led[i]) {
        if (myOSSwarm.Ctrl[controller]->led[i]) {
          OSObj[item++] = myOSSwarm.Ctrl[controller]->led[i];
          menu.add( myOSSwarm.Ctrl[controller]->led[i]->getName(),  myOSSwarm.Ctrl[controller]->led[i]->getAlias(), item, true );
        }
      }
    }
  
    // special HW
    SwOSSwarmJST *ftSwarm;
    SwOSSwarmControl *ftSwarmControl;
    SwOSSwarmCAM *ftSwarmCAM;
  
    switch ( myOSSwarm.Ctrl[controller]->getType() ) {
  
      case FTSWARM:         ftSwarm = static_cast<SwOSSwarmJST *>(myOSSwarm.Ctrl[controller]);
                            // list servos
                            for (uint8_t i=0; i<ftSwarm->servos; i++ ) {
                              if (ftSwarm->servo[i]) {
                                OSObj[item++] = ftSwarm->servo[i];
                                menu.add( ftSwarm->servo[i]->getName(), ftSwarm->servo[i]->getAlias(), item, true );
                              }
                            }
                            // list gyro
                            if (ftSwarm->gyro) {
                              OSObj[item++] = ftSwarm->gyro;
                              menu.add( ftSwarm->gyro->getName(), ftSwarm->gyro->getAlias(), item, true );
                            }
                            // list i2c
                            if (ftSwarm->I2C) {
                              OSObj[item++] = ftSwarm->I2C;
                              menu.add( ftSwarm->I2C->getName(), ftSwarm->I2C->getAlias(), item, true );
                            }
                            break;
                            // list gyro
                            if (ftSwarm->gyro) {
                              OSObj[item++] = ftSwarm->gyro;
                              menu.add( ftSwarm->gyro->getName(), ftSwarm->gyro->getAlias(), item, true );
                            }
                            break;
  
      case FTSWARMCONTROL:  ftSwarmControl = static_cast<SwOSSwarmControl *>(myOSSwarm.Ctrl[controller]);
                            // buttons
                            for (uint8_t i=0; i<8; i++ ) {
                              if (ftSwarmControl->button[i]) {
                                OSObj[item++] = ftSwarmControl->button[i];
                                menu.add( ftSwarmControl->button[i]->getName(),   ftSwarmControl->button[i]->getAlias(), item, true );
                              }
                            }
                            // joysticks
                            for (uint8_t i=0; i<2; i++ ) {
                              if (ftSwarmControl->joystick[i]) {
                                OSObj[item++] = ftSwarmControl->joystick[i];
                                menu.add( ftSwarmControl->joystick[i]->getName(), ftSwarmControl->joystick[i]->getAlias(), item , true);
                              }
                            }
                            if (ftSwarmControl->oled) {
                              OSObj[item++] = ftSwarmControl->oled;
                              menu.add( ftSwarmControl->oled->getName(), ftSwarmControl->oled->getAlias(), item, true );
                            }
                            // list gyro
                            if (ftSwarmControl->gyro) {
                              OSObj[item++] = ftSwarmControl->gyro;
                              menu.add( ftSwarmControl->gyro->getName(), ftSwarmControl->gyro->getAlias(), item, true );
                            }
                            break;
  
      case FTSWARMCAM:      ftSwarmCAM = static_cast<SwOSSwarmCAM *>(myOSSwarm.Ctrl[controller]);
                            if (ftSwarmCAM->cam) {
                              OSObj[item++] = ftSwarmCAM->cam;
                              menu.add( ftSwarmCAM->cam->getName(), ftSwarmCAM->cam->getAlias(), item, true );
                            }
                            break;
  
      default:              break;
    }
  
    // User's choice
    uint8_t choice = menu.userChoice( );
    
    uint8_t i;
    bool changes = false;
    SwOSCom *alias2nvs = NULL;

    switch (choice) {

      case  0:  // exit: save nvs?

                // any changes?
                for ( i=0; i<MAXCTRL; i++ ) { if (anythingChanged[i]) changes = true; }

                if ( ( changes )  &&  yesNo( "Save changes? (Y/N)?" ) ) {

                  // local changes
                  if ( anythingChanged[0] ) {

                    // save in local nvs
                    nvs_handle_t my_handle;
                    ESP_ERROR_CHECK( nvs_open("ftSwarm", NVS_READWRITE, &my_handle) );
                    myOSSwarm.Ctrl[0]->saveAliasToNVS( my_handle );
                    ESP_ERROR_CHECK( nvs_commit( my_handle ) );

                    // send new config to Kelda
                    if ( ( myOSSwarm.Kelda ) && ( myOSSwarm.Kelda != myOSSwarm.Ctrl[0] ) ) myOSSwarm.Ctrl[0]->sendAlias( myOSSwarm.Kelda->macAddr );

                  }

                  // remote changes
                  for ( i=1; i<MAXCTRL; i++ ) {
                    if ( anythingChanged[i] ) {
                      myOSSwarm.Ctrl[i]->sendAlias( myOSSwarm.Ctrl[i]->macAddr );
                      alias2nvs = new SwOSCom( myOSSwarm.Ctrl[i]->macAddr, myOSSwarm.Ctrl[i]->serialNumber, CMD_SAVEALIAS2NVS );
                      alias2nvs->send( );
                      delete alias2nvs;
                      alias2nvs = NULL;
                    }
                  }
          
                }
                return;
      
      case 99:  controller = selectController( controller );
                printf("controller: %d\n", controller);
                break;

      case 255: break;

      default:  // set name
                char alias[MAXIDENTIFIER];
                char prompt[250];
                SwOSIO* testIO;

                if (OSObj[choice-1]) {

                  // ask user
                  sprintf( prompt, "%s - please enter new alias: ", OSObj[choice-1]->getName() );
                  enterIdentifier( prompt, alias, MAXIDENTIFIER );
                
                  // duplicates?
                  testIO = myOSSwarm.getIO( alias, FTSWARM_UNDEF );

                  if ( (testIO) && ( testIO != OSObj[choice-1] ) ) {
                    // duplicate alias name
                    printf("\e[0;31mERROR: This alias is already used in the swarm.\n\e[0m");

                  } else {
                    // change alias
                    OSObj[choice-1]->setAlias( alias );
                    anythingChanged[controller]= true;
                  }
                }
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
    nvs_handle_t my_handle;
    ESP_ERROR_CHECK( nvs_open("ftSwarm", NVS_READWRITE, &my_handle) );
    myOSSwarm.Ctrl[0]->saveAliasToNVS( my_handle );
    ESP_ERROR_CHECK( nvs_commit( my_handle ) );

    nvs.saveAndRestart();

  }

}

bool changeEvent( NVSEvent *event ) {

  char actor[MAXIDENTIFIER];
  char sensor[MAXIDENTIFIER];

  SwOSIO *newSensor = NULL;
  SwOSIO *newActor = NULL;

  FtSwarmTrigger_t trigger;
  int32_t parameter;
  bool    usePortValue;
  uint8_t LR;

  // enter sensor
  while (true) {
    enterString( "sensor: ", sensor, sizeof(sensor) );
    newSensor = myOSSwarm.getIO( sensor, FTSWARM_UNDEF );
    
    if (sensor[0] == '\0' ) 
      return false;

    else if (!newSensor) 
      printf("sensor %s doesn't exist in the swarm.\n", sensor);
    
    else if ( !newSensor->isSensor() )
      printf("%s needs to be a sensor.\n", sensor);

    else
      // now, I'm fine
      break;
  }

  // which poti?
  if ( newSensor->getIOType() == FTSWARM_JOYSTICK ) 
    LR = enterNumber( "joystick direction: (1) left/right (2) forward/backward: ", 1, 1, 2 );
  else
    LR = 0;
  
  // enter trigger event
  trigger = (FtSwarmTrigger_t) enterNumber( "Trigger event: (0) TriggerUp (1) TriggerDown (2) ChangeValue: ", 0, 0, 2 );
  
  // enter actor
  while (true) {
    enterString( "actor: ", actor, sizeof(actor) );
    newActor = myOSSwarm.getIO( actor, FTSWARM_UNDEF );
    
    if (actor[0] == '\0' ) 
      return false;
      
    else if (!newActor) 
      printf("actor %s doesn't exist in the swarm.\n", actor);

    else if ( !newActor->isActor() )
      printf("%s needs to be an actor, a LED or a servo.\n", actor);
    
    else
      // now, I'm fine
      break;
  }

  // enter parameter
  usePortValue = enterNumber( "(0) set a static value OR (1) use the sensors value? ", 0, 0, 1 );
  if (!usePortValue) parameter = enterNumberI32( "Which value should be set? ", 0, 0, 0xFFFFFF );

  // delete old trigger
  SwOSIO *oldSensor = myOSSwarm.getIO( event->sensor, FTSWARM_UNDEF );
  
  if ( oldSensor != NULL ) {
  
    switch ( oldSensor->getIOType() ) {
    
      case FTSWARM_JOYSTICK: 
        if ( LR == 1 ) static_cast<SwOSJoystick *>(oldSensor)->triggerLR.unregisterEvent( trigger );
        else           static_cast<SwOSJoystick *>(oldSensor)->triggerFB.unregisterEvent( trigger );
        break; 
        
      default: 
        static_cast<SwOSInput *>(oldSensor)->unregisterEvent( trigger ); 
        break;
        
    }

  }

  // modify trigger
  switch ( newSensor->getIOType() ) {
    
    case FTSWARM_JOYSTICK: 
      if ( LR == 1 ) static_cast<SwOSJoystick *>(newSensor)->triggerLR.registerEvent( trigger, newActor, usePortValue, parameter );
      else           static_cast<SwOSJoystick *>(newSensor)->triggerFB.registerEvent( trigger, newActor, usePortValue, parameter );
      break;

    default: 
      static_cast<SwOSInput *>(newSensor)->registerEvent( trigger, newActor, usePortValue, parameter ); 
      break;
  }

  // change event
  strcpy( event->sensor, sensor ); 
  event->LR = LR; 
  strcpy( event->actor, actor);  
  event->triggerEvent = trigger;
  event->usePortValue = usePortValue;
  event->parameter = parameter;

  return true;
  
}

void printX( char *str, uint8_t fill ) {

  char line[255];

  // copy string with max fill chars
  strncpy( line, str, fill );

  // fill training blanks
  for (uint8_t i=strlen(line); i<fill; i++) line[i] = ' ';

  // print
  printf( "%s ", line );
  
}

void remoteControl( void ) {

  uint8_t choice, maxChoice;
  uint8_t item;
  uint8_t eventPtr[MAXNVSEVENT+1];
  bool anythingChanged = false;

  while ( 1 ) {

    // clear eventPtr
    memset( eventPtr, 255, sizeof( eventPtr ) );

    // reset item
    item = 0;
    eventPtr[0] = 255;

    // list events
    printf("\n\n***** Remote Control *****\n\n");
    printf("     sensor             event       actor           value\n");

    for ( uint8_t i=0; i<MAXNVSEVENT; i++ ) {
      
      if ( nvs.eventList.event[i].sensor[0] != '\0' ) {
        // used event
        item++;
        eventPtr[item] = i;

        printf("(%2d) ", item);
        
        printX( nvs.eventList.event[i].sensor, 15 );

        switch ( nvs.eventList.event[i].LR ) {
          case 0: printf( "   "); break;
          case 1: printf( "LR "); break;
          case 2: printf( "FB "); break;          
        }
        
        switch( nvs.eventList.event[i].triggerEvent ) {
          case FTSWARM_TRIGGERUP:    printf("TriggerUp   "); break;
          case FTSWARM_TRIGGERDOWN:  printf("TriggerDown "); break;
          case FTSWARM_TRIGGERVALUE: printf("ChangeValue "); break;
          default:                   printf("?           "); break;
        }
        
        printX( nvs.eventList.event[i].actor, 15 );

        if (nvs.eventList.event[i].usePortValue)
          printf("SENSORVALUE\n");
        else
          printf("%" PRId32 "\n", nvs.eventList.event[i].parameter );
        
      } else if ( eventPtr[0] == 255 ){
        eventPtr[0] = i;
      }
    }

    // additional commands
    printf("\n(%d) add event\n", item + 1 );
    maxChoice = item + 1;

    if ( item > 0 ) {
      printf("(%d) delete event\n", item + 2 );
      maxChoice = item + 2;
    }
    
    printf("\n(%d) exit\n", 0 );

    // get user's choice
    choice = enterNumber("\nremote control>", 0, 0, maxChoice );    

    // do what the user wants
    if ( choice == 0 ) {
     if ( ( anythingChanged ) && yesNo( "Save changes to nvs [Y/N]? " ) ) nvs.save();
     return;
      
    } else if ( choice == ( item + 1 ) ) {
      // add
      if ( changeEvent( &nvs.eventList.event[eventPtr[0]] ) ) anythingChanged = true;
      
    } else if ( choice == ( item + 2 ) ) {
      // delete
      choice = enterNumber( "Which event should be deleted? ", 1, 1, item );
      nvs.eventList.event[eventPtr[choice]].actor[0] = '\0';
      nvs.eventList.event[eventPtr[choice]].sensor[0] = '\0';
      anythingChanged = true;
      
    } else {
      // modify item
      if ( changeEvent( &nvs.eventList.event[eventPtr[choice]] ) ) anythingChanged = true;
    }

  }

}

#define MAINMENUWEB       1
#define MAINMENUSWARM     2
#define MAINMENUALIAS     3
#define MAINMENUFACTORY   4
#define MAINMEUREMOTE     5
#define MAINMENUEXTENSION 6
#define MAINMENUSWARMCTRL 7

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
    menu.add("Alias Names", "", MAINMENUALIAS );
    menu.add("Factory Reset", "", MAINMENUFACTORY );

    if (myOSSwarm.Ctrl[0]->IAmKelda) menu.add("Remote Control", "", MAINMEUREMOTE );

    // special HW
    switch (myOSSwarm.Ctrl[0]->getType()) {

      case FTSWARM:         menu.add("Extension Port", "", MAINMENUEXTENSION );
                            break;

      case FTSWARMCONTROL:  menu.add("Extension Port", "", MAINMENUEXTENSION );
                            menu.add("ftSwarmControl", "", MAINMENUSWARMCTRL );
                            break;

    }

    switch( menu.userChoice(  )  ) {
      case 0:                 return;
      case MAINMENUWEB:       wifiMenu();         break;
      case MAINMENUSWARM:     swarmMenu();        break;
      case MAINMENUALIAS:     aliasMenu();        break;
      case MAINMENUFACTORY:   factorySettings();  break;
      case MAINMEUREMOTE:     remoteControl();    break;
      case MAINMENUEXTENSION: ExtensionMenu();    break;
      case MAINMENUSWARMCTRL: SwarmControlMenu(); break;
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