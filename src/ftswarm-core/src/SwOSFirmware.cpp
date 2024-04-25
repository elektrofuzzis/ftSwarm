/*
 * SwOSFirmware.cpp
 *
 * Firmware menues
 * 
 * (C) 2021-23 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOSFirmware.h"
#include "SwOS.h"
#include "SwOSSwarm.h"
#include "SwOSNVS.h"
#include "easyKey.h"
#include "SwOSCLI.h"

const char EXTMODE[5][13] = { "off", "I2C-Master", "I2C-Slave", "Gyro MCU6040", "Outputs" };
const char ONOFF[2][5]    = { "off", "on" };
const char OFFM1M2[3][5]  = { "off", "M1", "M2" };
const char WIFI[3][12]    = { "off", "AP-Mode", "Client-Mode"};

#define EXTMENUMODE 1
#define EXTMENUGYRO 2
#define EXTMENUI2C  3
#define EXTMENUINT  4
#define EXTMENUINT0 5
#define EXTMENUINT1 6

void ExtentionMenu() {

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

    menu.start("Extention Port", 20);

    menu.add("Mode", EXTMODE[ nvs.extentionPort] , EXTMENUMODE );

    // I2C Slave Mode. Options I2C Slave Address and Interrupt Line
    if ( nvs.extentionPort == FTSWARM_EXT_I2C_SLAVE ) {
      menu.add("I2C Slave Address", nvs.I2CAddr, EXTMENUI2C);
      menu.add("Interrupt Line", OFFM1M2[nvs.interruptLine], EXTMENUINT);
      menu.add("Interrupt Low Value",  nvs.interruptOnOff[0], EXTMENUINT0);
      menu.add("Interrupt High Value", nvs.interruptOnOff[1], EXTMENUINT1);
    }

    // internal gyro is only available at ftSwarmRS
    if ( ( nvs.CPU == FTSWARMRS_2V0 ) || ( nvs.CPU == FTSWARMRS_2V1 ) ) menu.add("internal Gyro", ONOFF[nvs.gyro], EXTMENUGYRO );

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
        nvs.extentionPort = (FtSwarmExtMode_t) enterNumber( "(0) off (1) I2C-Master (2) I2C-Slave (3) Gyro MCU6040 (4) Outputs: ", nvs.extentionPort, 0, 4 );
        break;

      case EXTMENUGYRO: // Gyro
        anythingChanged = true;
        nvs.gyro = (bool) enterNumber( "(0) off (1) on: ", nvs.gyro, 0, 1 );
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
        nvs.interruptOnOff[0] = (int16_t) enterNumberI32( "[-255..255]", nvs.interruptOnOff[0], -255, 255 );
        break;

      case EXTMENUINT1: // Interrupt Line High
        anythingChanged = true;
        nvs.interruptOnOff[1] = (int16_t) enterNumberI32( "[-255..255]", nvs.interruptOnOff[1], -255, 255 );
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
            printf("Error: please deactivate wifi in swarm communication first.\n");
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

void joinSwarm( boolean createNewSwarm ) {

  // create a new swarm
  char name[64];
  char prompt[256];

  // get new swarm's name
  while (1) {
    
    enterString( "Please enter new swarm's name [minimum 5 chars]: ", name, MAXIDENTIFIER);

    // different names, at least 5 chars
    if ( ( strcmp( nvs.swarmName, name ) != 0 ) && ( strlen( name ) > 4 ) ) {
      break;
    }

  }

  // get new swarm's pin
  uint16_t pin = enterNumber("Please enter new swarm's PIN [1..9999]: ", 0, 1, 9999 );

  // build swarm
  if ( createNewSwarm ) {
    sprintf( prompt, "Do you really want to quit swarm \"%s\", create new swarm \"%s\" with pin %d and reboot (Y/N) ?", nvs.swarmName, name, pin );
  } else {
    sprintf( prompt, "Do you really want to quit swarm \"%s\" and join swarm \"%s\" with pin %d (Y/N) ?", nvs.swarmName, name, pin );
  }

  // stop, if NO
  if ( !yesNo( prompt ) ) return;

  // leave old swarm
  SwOSError_t err = myOSSwarm.leaveSwarm();
  if ( ( err == SWOS_TIMEOUT ) && (!yesNo( "I couldn't inform all swarm members. Continue anyway [Y/N]?" ) ) ) return;
 
  // new swarm?
  if ( createNewSwarm ) {
    err = myOSSwarm.createSwarm( name, pin );
    if ( err != SWOS_OK ) { printf("Internal error %d during createSwarm.\n", err ); return; }
    nvs.deleteAllControllers();
    nvs.save();
    printf("%s successfully created.\n", name );
    printf("Press any key to reboot\n");
    anyKey();
    ESP.restart();

  } else {

    err = myOSSwarm.joinSwarm( name, pin );
    switch ( err ) {
      case SWOS_OK:       printf("%s successfully joined.\n", name ); 
                          nvs.save();
                          return;

      case SWOS_DENY:     printf("Kelda refused you.\n"); 
                          break;

      case SWOS_TIMEOUT:  printf("Couldn't connect to Kelda.\n");
                          break;

      default:            printf("Internal error %d during joinSwarm.\n", err );
                          break;
    }

    if ( yesNo( "Reload swarm settings and reboot [Y/N]?" ) ) { 
      ESP.restart();
    } 

  }

}

void inviteControllerToSwarm( void ) {

  uint16_t SN = enterNumber("Add Controller: Please enter the controllers serial number [1..999]: ", 0, 1, 999 );

  if ( myOSSwarm.getController( SN ) ) {
    printf("This controller is already a swarm member\n");
    return;
  }

  SwOSError_t err = myOSSwarm.inviteToSwarm( SN );
  
  switch ( err ) {
    case SWOS_OK:       printf("Device %d successfully joined.\n", SN ); 
                        nvs.save(); 
                        myOSSwarm.registerMe( MacAddr( broadcast ), SN );
                        longDelay();
                        break;
    case SWOS_DENY:     printf("Controller %d refused.\n", SN); break;
    case SWOS_TIMEOUT:  printf("Couldn't connect to controller %d.\n", SN); break;
    default:            printf("Internal error %d during joinSwarm.\n", err ); break;
  }

}

void rejectControllerFromSwarm( void ) {

  uint16_t SN = enterNumber("Delete Controller: Please enter the controllers serial number [1..999]: ", 0, 1, 999 );

  if ( !myOSSwarm.getController( SN ) ) {
    printf("This controller isn't a swarm member\n");
    return;
  }

  bool force = false;

  while (1) {

    switch ( myOSSwarm.rejectController( SN, force ) ) {

      case SWOS_OK:       printf( "Device %d left the swarm.\n", SN);
                          nvs.deleteController( SN );
                          nvs.save();
                          return;
      
      case SWOS_TIMEOUT:  printf( "Couldn't connect to device %d,\n", SN);
                          if (!yesNo("Delete it anyway [Y/N]?" ) ) return;
                          force = true;
                          break;

      case SWOS_DENY:     printf( "This device is used actively. Stop yout program first.\n");
                          return;

      default:            printf( "Error: unexpected swarm behaviour.\n");
                          return;

    }
    
  } 

}

const char SWARMCOMMUNICATION[4][13] = { "none", "wifi", "RS485", "wifi & RS485" };

// swam menu identifiers

#define MENUCREATESWARM        1
#define MENUJOINSWARM          2
#define MENUSWARMCOMMUNICATION 3
#define MENUINVITECONTROLLER   4
#define MENUREJECTCONTROLLER   5
#define MENUSWARMSPEED         6

void swarmMenu( void ) {

  char kelda[MAXIDENTIFIER];
  FtSwarmCommunication_t swarmCommunication;
  Menu menu;

  while (1) {

    menu.start("swarm configuration", 19 );

    if (nvs.IAmKelda) {

      printf("%s is Kelda running swarm \"%s\" using Pin %d:\n\nSN  NW Age Hostname \n", myOSSwarm.Ctrl[0]->getHostname(), nvs.swarmName, nvs.swarmPIN );
      for ( int8_t i=0; i<=myOSSwarm.maxCtrl; i++ ) {
        if ( myOSSwarm.Ctrl[i] ) {
          printf("%3d %.6lu %s\n", myOSSwarm.Ctrl[i]->serialNumber, myOSSwarm.Ctrl[i]->networkAge(), myOSSwarm.Ctrl[i]->getHostname() );
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
    
    menu.add( "create a new swarm", "", MENUCREATESWARM );
    menu.add( "join another swarm", "", MENUJOINSWARM );

    if (nvs.IAmKelda) {
      menu.add( "invite a controller to my swarm", "", MENUINVITECONTROLLER );
      menu.add( "reject a controller from my swarm", "", MENUREJECTCONTROLLER );
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
              printf("Error: please activate wifi first.\n");
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

      case MENUCREATESWARM:
        joinSwarm( true ); 
        break;
        
      case MENUJOINSWARM:
        joinSwarm( false ); 
        break;

      case MENUINVITECONTROLLER:
        inviteControllerToSwarm( );
        break;

      case MENUREJECTCONTROLLER:
        rejectControllerFromSwarm( );
        break;

    }
    
  }

}

void aliasMenu( void ) {

  SwOSObj *OSObj[99];
  bool    anythingChanged = false;

  Menu    menu;

  while (1) {

    uint8_t item = 0;
    menu.start( "alias controller menu:", 10);
    
    OSObj[item++] = myOSSwarm.Ctrl[0]; 
    menu.add( myOSSwarm.Ctrl[0]->getName(), myOSSwarm.Ctrl[0]->getAlias(), item, true );

    // list inputs
    for (uint8_t i=0; i<myOSSwarm.Ctrl[0]->inputs; i++ ) { 
      OSObj[item++] = myOSSwarm.Ctrl[0]->input[i];
      menu.add( myOSSwarm.Ctrl[0]->input[i]->getName(), myOSSwarm.Ctrl[0]->input[i]->getAlias(), item, true );
    }

    // list actors
    for (uint8_t i=0; i<myOSSwarm.Ctrl[0]->actors; i++ ) {
      OSObj[item++] = myOSSwarm.Ctrl[0]->actor[i];
      menu.add( myOSSwarm.Ctrl[0]->actor[i]->getName(), myOSSwarm.Ctrl[0]->actor[i]->getAlias(), item, true );
    }

    // list LEDs
    for (uint8_t i=0; i<nvs.RGBLeds; i++ ) {
      if ( myOSSwarm.Ctrl[0]->led[i]) {
        OSObj[item++] =  myOSSwarm.Ctrl[0]->led[i];
        menu.add( myOSSwarm.Ctrl[0]->led[i]->getName(),  myOSSwarm.Ctrl[0]->led[i]->getAlias(), item, true );
      }
    }

    // special HW
    SwOSSwarmJST *ftSwarm;
    SwOSSwarmControl *ftSwarmControl;
    SwOSSwarmCAM *ftSwarmCAM;

    switch ( myOSSwarm.Ctrl[0]->getType() ) {

      case FTSWARM:         ftSwarm = static_cast<SwOSSwarmJST *>(myOSSwarm.Ctrl[0]);
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

      case FTSWARMCONTROL:  ftSwarmControl = static_cast<SwOSSwarmControl *>(myOSSwarm.Ctrl[0]);
                            // buttons
                            for (uint8_t i=0; i<8; i++ ) {
                              OSObj[item++] = ftSwarmControl->button[i];
                             menu.add( ftSwarmControl->button[i]->getName(),   ftSwarmControl->button[i]->getAlias(), item, true );
                            }
                            // joysticks
                            for (uint8_t i=0; i<2; i++ ) {
                              OSObj[item++] = ftSwarmControl->joystick[i];
                              menu.add( ftSwarmControl->joystick[i]->getName(), ftSwarmControl->joystick[i]->getAlias(), item , true);
                            }
                            if (ftSwarmControl->oled) {
                              OSObj[item++] = ftSwarmControl->oled;
                              menu.add( ftSwarmControl->oled->getName(), ftSwarmControl->oled->getAlias(), item, true );
                            }
                            break;

      case FTSWARMCAM:      ftSwarmCAM = static_cast<SwOSSwarmCAM *>(myOSSwarm.Ctrl[0]);
                            if (ftSwarmCAM->cam) {
                              OSObj[item++] = ftSwarmCAM->cam;
                              menu.add( ftSwarmCAM->cam->getName(), ftSwarmCAM->cam->getAlias(), item, true );
                            }
                            break;

      default:              break;
    }

    // User's choice
    uint8_t choice = menu.userChoice( );

    // exit?
    if ( choice == 0 ) {

      if ( ( anythingChanged )  &&  yesNo( "Save changes? (Y/N)?" ) ) {

        // Open
        nvs_handle_t my_handle;
        ESP_ERROR_CHECK( nvs_open("ftSwarm", NVS_READWRITE, &my_handle) );

        // save
        myOSSwarm.Ctrl[0]->saveAliasToNVS( my_handle );

        // send new config to Kelda
        if ( ( myOSSwarm.Kelda ) && ( myOSSwarm.Kelda != myOSSwarm.Ctrl[0] ) ) myOSSwarm.Ctrl[0]->sendAlias( myOSSwarm.Kelda->macAddr );

        // commit
        ESP_ERROR_CHECK( nvs_commit( my_handle ) );
          
      }
      return;

    // set name
    } else if ( choice > 0 ) {

      char alias[MAXIDENTIFIER];
      char prompt[250];
      sprintf( prompt, "%s - please enter new alias: ", OSObj[choice-1]->getName() );
      enterIdentifier( prompt, alias, MAXIDENTIFIER );
      OSObj[choice-1]->setAlias( alias );
      anythingChanged = true;
    
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
#define MAINMENUEXTENTION 6
#define MAINMENUSWARMCTRL 7

void mainMenu( void ) {

  Menu menu;

  while (1) {

    menu.start( "Main Menu", 14 );
    menu.add("Wifi & Web UI", "", MAINMENUWEB );
    menu.add("Swarm Configuration", "", MAINMENUSWARM );
    menu.add("Alias Names", "", MAINMENUALIAS );
    menu.add("Factory Reset", "", MAINMENUFACTORY );

    if (myOSSwarm.Ctrl[0]->IAmKelda) menu.add("Remote Control", "", MAINMEUREMOTE );

    // special HW
    switch (myOSSwarm.Ctrl[0]->getType()) {

      case FTSWARM:         menu.add("Extention Port", "", MAINMENUEXTENTION );
                            break;

      case FTSWARMCONTROL:  menu.add("Extention Port", "", MAINMENUEXTENTION );
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
      case MAINMENUEXTENTION: ExtentionMenu();    break;
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