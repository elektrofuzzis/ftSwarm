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

const char I2CMODE[7][12] = { "off", "Master", "Slave", "ftcSoundbar", "ftPwrDrive", "TXT", "ftDuino" };
const char ONOFF[2][5]    = { "off", "on" };

void I2CMenu() {

  bool    anythingChanged = false;
  char    prompt[255];

  while (1) {

    /*
    (1) Mode: Master/Slave/TXT/ftDuino
    (2) Gyro: LM6/MCU/OFF
    (3) Slave Address: x
    (0) exit
    */

   printf( "I2C mode: %d\ngyro: %d\n", nvs.I2CMode, nvs.gyro);

    printf("\n\nI2C Settings\n\n");
    sprintf(prompt, "(1) Mode: %s\n(2) Gyro: %s\n(3) I2C Address: %d\n\n(0) exit\nI2C>", I2CMODE[ nvs.I2CMode], ONOFF[nvs.gyro], nvs.I2CAddr );
    
    switch( enterNumber( prompt, 0, 0, 3) ) {
      
      case 0: // exit
        if ( ( anythingChanged) && ( yesNo( "To apply your changes, the device needs to be restarted.\nSave settings and restart now (Y/N)?") ) ) {
          // save config
          nvs.saveAndRestart();
        } else {
          return;
        }
        
      case 1: // I2cMode
        anythingChanged = true;
        nvs.I2CMode = (FtSwarmI2CMode_t) enterNumber( "(0) off (1) Master (2) Slave (3) ftcSoundBar (4) ftPwrDrive  (5) TXT (6) FTDUINO: ", nvs.I2CMode, 0, 6 );
        break;

      case 2: // Gyro
        anythingChanged = true;
        nvs.gyro = (bool) enterNumber( "(0) off (1) on: ", nvs.gyro, 0, 1 );
        break;

      case 3: // Gyro
        anythingChanged = true;
        nvs.I2CAddr = (uint8_t) enterNumber( "[16..127]: ", nvs.I2CAddr, 16, 127 );
        break;
        
    }
  }

}

void SwarmControlMenu() {

  bool    anythingChanged = false;
  char    prompt[255];

  while (1) {
    
    printf("\n\nftSwarmControl settings\n\n");
    sprintf( prompt, "(1) Display:  type %d\n (2) Calibrate Joysticks\n\n(0) exit\nftSwarmControl>", nvs.displayType );
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

void WebServerMenu() {

  uint8_t maxChoice;
  bool    anythingChanged = false;
  char    prompt[255];
  
  while(1) {

    printf("\n\nweb server settings\n\n");

    if ( ( myOSSwarm.Ctrl[0]->getType() == FTSWARM ) && ( nvs.webUI ) ) {
      sprintf(prompt, "(1) WebUI: %s\n(2) Show %d ftPixels in UI\n\n(0) exit\nweb server>", ONOFF[nvs.webUI], nvs.RGBLeds );
      maxChoice = 2;
    } else {
      sprintf(prompt, "(1) WebUI: %s\n\n(0) exit\nweb server>", ONOFF[nvs.webUI] );
      maxChoice = 1;
    }

    switch( enterNumber( prompt, 0, 0, maxChoice) ) {
      
      case 0: // exit
        if ( ( anythingChanged) && ( yesNo( "To apply your changes, the device needs to be restarted.\nSave settings and restart now (Y/N)?") ) ) {
          // save config
          nvs.saveAndRestart();
        } else {
          return;
        }
        
      case 1: // WebServer on/off
        anythingChanged = true;
        nvs.webUI = !nvs.webUI;
        break;
        
      case 2: // # of ftPixel
        anythingChanged = true;
        nvs.RGBLeds = enterNumber( "enter number of ftPixel in WebUI [2..18]: ", nvs.RGBLeds, 2, MAXLED );
        break;
    }

  }
  
}

void wifiMenu( void ) {

  char          prompt[250];
  bool          anythingChanged = false;
  uint8_t       maxChoice;
  FtSwarmWifi_t wifiMode;
  
  while(1) {

    printf("\n\nWifi Settings\n\n");

    switch( nvs.wifiMode ) {

      case wifiOFF:
        sprintf( prompt, "(1) wifi: off\n\n(0) exit\nwifi>" );
        maxChoice = 1;
        break;
        
      case wifiAP:
        sprintf( prompt, "(1) wifi:     AP-MODE\n(2) SSID:     %s\n(X) Password: NOPASSWORD\n(4) Channel:  %d\n\n(0) exit\nwifi>", nvs.wifiSSID, nvs.channel );
        maxChoice = 4;
        break;
        
      case wifiClient:
        sprintf( prompt, "(1) wifi:     CLIENT-MODE\n(2) SSID:     %s\n(3) Password: ******\n\n(0) exit\nwifi>", nvs.wifiSSID );
        maxChoice = 3;
        break;

      default:
        printf( "ERROR: invalid wifi mode %d\n", nvs.wifiMode );
        return;
    }
    
    switch( enterNumber( prompt, 0, 0, maxChoice) ) {
      
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
    }
  }
}

void joinSwarm( bool createNewSwarm ) {

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
    sprintf( prompt, "Do you really want to quit swarm \"%s\" and create new swarm \"%s\" with pin %d (Y/N) ?", nvs.swarmName, name, pin );
  } else {
    sprintf( prompt, "Do you really want to quit swarm \"%s\" and join swarm \"%s\" with pin %d (Y/N) ?", nvs.swarmName, name, pin );
  }

  // stop, if NO
  if ( !yesNo( prompt ) ) return;

  // now build/join swarm
  myOSSwarm.lock();
  myOSSwarm.leaveSwarm( );
  myOSSwarm.allowPairing = true;
  myOSSwarm.joinSwarm( createNewSwarm, name, pin );
  myOSSwarm.unlock();

  // wait some ticks to catch a response
  vTaskDelay( 250 / portTICK_PERIOD_MS );

  // stop pairing
  myOSSwarm.allowPairing = false;

  // now I need to register again
  myOSSwarm.lock();
  myOSSwarm.registerMe();
  myOSSwarm.unlock();

  // wait some ticks to catch a response
  vTaskDelay( 250 / portTICK_PERIOD_MS );

  // in case of a new swarm save nvs & quit.
  if ( ( createNewSwarm ) && ( myOSSwarm.members() <= 1 ) ) {
    nvs.save();
    printf("Swarm \"%s\" created sucessfully.\n", name );
    return;
  }

  // in case of joining a swarm, check on other devices and get new swarm's secret
  if ( myOSSwarm.members() <= 1 ) { // no other device out there
    // restore old settings
    myOSSwarm.lock();
    myOSSwarm.reload();
    myOSSwarm.unlock();
    printf("ERROR: swarm \"%s\" not found. Rejoined old swarm %s\n", name, nvs.swarmName );
    return;
  }

  // the CMD_GOTYOU-Message catched the new SECRET
  // so just store all changes
  nvs.save();
  
  printf("Swarm \"%s\" joined sucessfully.\n", name );

}

const char SWARMCOMMUNICATION[4][13] = { "none", "wifi", "RS485", "wifi & RS485" };

void swarmMenu( void ) {

  char prompt[250];
  char item1[2][2] = { "X", "1" };
  FtSwarmCommunication_t swarmCommunication;
  
  while (1) {

    printf( "\n\nswarm settings\n\nThis device is connected to swarm \"%s\" with %d member(s) online.\nSwarm PIN is %d.\n", nvs.swarmName, myOSSwarm.members(), nvs.swarmPIN );
    sprintf( prompt, "(%s) swarm communication: %s\n(2) create a new swarm\n(3) join another swarm\n(4) list swarm members\n\n(0) main\nswarm>", item1[nvs.RS485Available()], SWARMCOMMUNICATION[nvs.swarmCommunication]);
    
    switch( enterNumber(prompt, 0, 0, 4) ) {
      case 0: // main
        return;

      case 1: // swarm communication
        if ( nvs.RS485Available() ) {
          swarmCommunication = (FtSwarmCommunication_t) enterNumber( "enter swarm communication [1-wifi, 2-RS485, 3-both]:", nvs.swarmCommunication, 1, 3 );
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
        
      case 2: // create new swarm
        joinSwarm( true ); 
        break;
        
      case 3: // join a swarm
        joinSwarm( false ); 
        break;
        
      case 4: // list swarm members
        printf("\nSwarm members:\n" );
        for (uint8_t i=0; i<=myOSSwarm.maxCtrl; i++ ) {
          if ( myOSSwarm.Ctrl[i] )
            printf("#%d %s\n", i, myOSSwarm.Ctrl[i]->getHostname() );
        }
        break;
    }
    
  }

}

void aliasMenu( void ) {

  SwOSObj *OSObj[20];
  bool    anythingChanged = false;

  while (1) {

    uint8_t item = 0;
    printf("\n\nalias controller menu:\n\n");
    
    OSObj[item++] = myOSSwarm.Ctrl[0]; 
    printf("(%2d) hostname %s - %s\n", item, myOSSwarm.Ctrl[0]->getName(), myOSSwarm.Ctrl[0]->getAlias() );

    // list inputs
    for (uint8_t i=0; i<myOSSwarm.Ctrl[0]->inputs; i++ ) { 
      OSObj[item++] = myOSSwarm.Ctrl[0]->input[i];
      printf("(%2d) %-4s - %-32s\n", 
              item, myOSSwarm.Ctrl[0]->input[i]->getName(), myOSSwarm.Ctrl[0]->input[i]->getAlias());
    }

    // list actors
    for (uint8_t i=0; i<myOSSwarm.Ctrl[0]->actors; i++ ) {
      OSObj[item++] = myOSSwarm.Ctrl[0]->actor[i];
      printf("(%2d) %-4s - %-32s\n", 
             item, myOSSwarm.Ctrl[0]->actor[i]->getName(), myOSSwarm.Ctrl[0]->actor[i]->getAlias());
    }

    // FTSWARM special HW
    if ( myOSSwarm.Ctrl[0]->getType() == FTSWARM ) {
      
      SwOSSwarmJST *ftSwarm = static_cast<SwOSSwarmJST *>(myOSSwarm.Ctrl[0]);

      // list LEDs
      for (uint8_t i=0; i<nvs.RGBLeds; i++ ) {
        if (ftSwarm->led[i]) {
          OSObj[item++] = ftSwarm->led[i];
          printf("(%2d) %-4s - %-32s\n", 
                 item, ftSwarm->led[i]->getName(), ftSwarm->led[i]->getAlias());
        }
      }
      
      // list servo
      for (uint8_t i=0; i<ftSwarm->servos; i++ ) {
        if (ftSwarm->servo[i]) {
          OSObj[item++] = ftSwarm->servo[i];
          printf("(%2d) %-4s - %-32s\n", 
                 item, ftSwarm->servo[i]->getName(), ftSwarm->servo[i]->getAlias());
        }
      }

      // list gyro
      if (ftSwarm->gyro) {
        OSObj[item++] = ftSwarm->gyro;
        printf("(%2d) %-4s - %-32s\n", item, ftSwarm->gyro->getName(), ftSwarm->gyro->getAlias() );
      }
    }

    // FTSWARMCONTROL special HW
    if ( myOSSwarm.Ctrl[0]->getType() == FTSWARMCONTROL ) {
      
      SwOSSwarmControl *ftSwarmControl = static_cast<SwOSSwarmControl *>(myOSSwarm.Ctrl[0]);

      // buttons
      for (uint8_t i=0; i<8; i++ ) {
        OSObj[item++] = ftSwarmControl->button[i];
        printf("(%2d) %-4s - %-32s\n", 
                item, ftSwarmControl->button[i]->getName(),   ftSwarmControl->button[i]->getAlias() );
      }

      // joysticks
      for (uint8_t i=0; i<2; i++ ) {
        OSObj[item++] = ftSwarmControl->joystick[i];
        printf("(%2d) %-4s - %-32s\n", 
               item, ftSwarmControl->joystick[i]->getName(), ftSwarmControl->joystick[i]->getAlias());
      }
      
      if (ftSwarmControl->oled) {
        OSObj[item++] = ftSwarmControl->oled;
        printf("(%2d) %-4s - %-32s\n", item, ftSwarmControl->oled->getName(), ftSwarmControl->oled->getAlias() );
      }
            
    }

    // User's choice
    uint8_t choice = enterNumber("\n(0) exit\nalias>", 0, 0, item );

    // exit?
    if ( choice == 0 ) {
      if ( ( anythingChanged )  &&  yesNo( "Save changes? (Y/N)?" ) ) {

        // Open
        nvs_handle_t my_handle;
        ESP_ERROR_CHECK( nvs_open("ftSwarm", NVS_READWRITE, &my_handle) );

        // save
        myOSSwarm.Ctrl[0]->saveAliasToNVS( my_handle );

        // commit
        ESP_ERROR_CHECK( nvs_commit( my_handle ) );
          
      }
      return;

    // set name
    } else {
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
    newSensor = myOSSwarm.getIO( sensor );
    
    if (sensor[0] == '\0' ) 
      return false;
    
    else if (!newSensor) 
      printf("sensor %s doesn't exist in the swarm.\n", sensor);
    
    else if ( !( (newSensor->getIOType() == FTSWARM_INPUT ) || (newSensor->getIOType() == FTSWARM_BUTTON ) || (newSensor->getIOType() == FTSWARM_JOYSTICK ) ) )
      printf("%s needs to be an input, a button or a joystick.\n", sensor);

    else
      // now, I'm fine
      break;
  }

  // which poti?
  if ( newSensor->getIOType() == FTSWARM_JOYSTICK ) 
    LR = enterNumber( "joystick direction: (1) left/right (2) forward/backward", 1, 1, 2 );
  else
    LR = 0;
  
  // enter trigger event
  trigger = (FtSwarmTrigger_t) enterNumber( "Trigger event: (0) TriggerUp (1) TriggerDown (2) ChangeValue: ", 0, 0, 2 );
  
  // enter actor
  while (true) {
    enterString( "actor: ", actor, sizeof(actor) );
    newActor = myOSSwarm.getIO( actor);
    
    if (actor[0] == '\0' ) 
      return false;
      
    else if (!newActor) 
      printf("actor %s doesn't exist in the swarm.\n", actor);

    else if ( !( (newActor->getIOType() == FTSWARM_ACTOR ) || (newActor->getIOType() == FTSWARM_LED ) || (newActor->getIOType() == FTSWARM_SERVO ) ) )
      printf("%s needs to be an actor, a LED or a servo.\n", actor);
    
    else
      // now, I'm fine
      break;
  }

  // enter parameter
  usePortValue = enterNumber( "(0) set a static value OR (1) use the sensors value? ", 0, 0, 1 );
  if (!usePortValue) parameter = enterNumberI32( "Which value should be set? ", 0, 0, 0xFFFFFF );

  // delete old trigger
  SwOSIO *oldSensor = myOSSwarm.getIO( event->sensor );
  
  if ( oldSensor != NULL ) {
  
    switch ( oldSensor->getIOType() ) {
    
      case FTSWARM_INPUT: 
        static_cast<SwOSInput *>(oldSensor)->unregisterEvent( trigger ); 
        break;
      
      case FTSWARM_BUTTON: 
        static_cast<SwOSButton *>(oldSensor)->unregisterEvent( trigger ); 
        break;
        
      case FTSWARM_JOYSTICK: 
        if ( LR == 1 ) static_cast<SwOSJoystick *>(oldSensor)->triggerLR.unregisterEvent( trigger );
        else           static_cast<SwOSJoystick *>(oldSensor)->triggerFB.unregisterEvent( trigger );
        break; 
        
      default: 
        printf("error saving your event.\n"); 
        return false;

    }

  }

  // modify trigger
  switch ( newSensor->getIOType() ) {
    
    case FTSWARM_INPUT: 
      static_cast<SwOSInput *>(newSensor)->registerEvent( trigger, newActor, usePortValue, parameter ); 
      break;
    
    case FTSWARM_BUTTON: 
      static_cast<SwOSButton *>(newSensor)->registerEvent( trigger, newActor, usePortValue, parameter ); 
      break;
    
    case FTSWARM_JOYSTICK: 
      if ( LR == 1 ) static_cast<SwOSJoystick *>(newSensor)->triggerLR.registerEvent( trigger, newActor, usePortValue, parameter );
      else           static_cast<SwOSJoystick *>(newSensor)->triggerFB.registerEvent( trigger, newActor, usePortValue, parameter );
      break;

    default: 
      printf("error saving your event.\n"); 
      return false;
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
    printf("remote control menu:\n\n");
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

void mainMenu( void ) {

  uint8_t choice, maxChoice;
  char prompt[255];
  
  // FTSWARMCONTROL special HW
  if ( myOSSwarm.Ctrl[0]->getType() == FTSWARMCONTROL ) {
    sprintf( prompt, "\nMain Menu\n\n(1) wifi settings\n(2) webserver settings\n(3) swarm settings\n(4) alias names\n(5) factory settings\n(6) I2C\n(7) remoteControl\n(8) ftSwarmControl\n\n(0) exit\nmain>" );
    maxChoice = 8;
  } else {
    sprintf( prompt, "\nMain Menu\n\n(1) wifi settings\n(2) webserver settings\n(3) swarm settings\n(4) alias names\n(5) factory settings\n(6) I2C\n(7) remoteControl\n\n(0) exit\nmain>" );
    maxChoice = 7;
  }
 
  while (1) {

    choice = enterNumber( prompt, 0, 0, maxChoice );

    switch( choice  ) {
      case 0: return;
      case 1: wifiMenu(); break;
      case 2: WebServerMenu(); break;
      case 3: swarmMenu(); break;
      case 4: aliasMenu(); break;
      case 5: factorySettings(); break;
      case 6: I2CMenu(); break;
      case 7: remoteControl(); break;
      case 8: SwarmControlMenu(); break;
    }
    
  }

}

/*----------------------------------------*/

#define CLIMAXLINE 255
#define MAXPARAM   20

typedef enum { 
  EVAL_INVALIDCMD = -2,
  EVAL_SYNTAXERROR = -1, 
  EVAL_OK, 
  EVAL_EOL, 
  EVAL_DOT, 
  EVAL_COMMA, 
  EVAL_LPARANTHESIS, 
  EVAL_RPARANTHESIS, 
  EVAL_NUMBER, 
  EVAL_LITERAL
} EvalResult_t;

typedef enum { 
  CMD_ERROR = -2, 
  CMD_UNKONWN = -1, 
  CMD_HELP, 
  CMD_SETUP, 
  CMD_EXIT 
} Cmd_t;

typedef enum { 
  ERROR_OK = -1, 
  ERROR_SYNTAXERROR, 
  ERROR_DOTEXPECTED, 
  ERROR_LITERALEXPECTED, 
  ERROR_IOEXPECTED,
  ERROR_NUMBEREXPECTED,
  ERROR_LPARANTHESISEXPECTED,
  ERROR_RPARANTHESISEXPECTED,
  ERROR_UNKOWNCMD,
  ERROR_WRONGNUMBEROFARGUMENTS
} Error_t;

const char IOCMD[IOCMD_MAX][20] = {
  "subscribe",
  "getIOType",
  "getSensorType",
  "setSensorType",
  "getValueUI32",
  "getValueF",
  "getVoltage",
  "getResistance",
  "getKelvin",
  "getCelcius",
  "getFahrenheit",
  "getToggle",
  "setActorType",
  "getActorType",
  "setSpeed",
  "getSpeed",
  "setMotionType",
  "getMotionType"
};

const uint8_t IOCMD_Parameters[IOCMD_MAX] = {
  1, // subscribe
  0, // getIOType
  0, // getSensorType
  2, // setSensorType
  0, // getValueUI32
  0, // getValueF
  0, // getVoltage
  0, // getResistance
  0, // getKelvin
  0, // getCelcius
  0, // getFahrenheit
  0, // getToggle  
  1, // setActorType  
  0, // getActorType
  1, // setSpeed
  0, // getSpeed
  1, // setMotionType
  0 // getMotionType
};

class CLI {
  protected:

    // user input    
    char _line[CLIMAXLINE];

    char   *_evalPtr;
    char   *_start;
    IOCmd_t _cmd;
    int     _maxParam;
    int     _param[MAXPARAM];

    void Error( Error_t error, int expected = 0, int found = 0 );

    // some simple char tests    
    bool isDigit(char ch);
    bool isAlpha(char ch);
    bool isLiteral(char ch);

    EvalResult_t getNumber( void );
    EvalResult_t getLiteral( void );
    EvalResult_t getNextToken( char* token );

    bool tokenizeCmd( char *cmd );
    Cmd_t evalSimpleCommand( char *token );  // tests, if token is a simple command
    void evalIOCommand( char *token );       // evals an IO command, token is already first token  
    void executeIOCommand( void );
    bool eval( void );

  public:
    CLI();
    void run(void);
};

CLI::CLI() {
  _line[0] = '\0';
  _evalPtr = NULL;
  _start   = NULL;
}

void CLI::Error( Error_t error, int expected, int found ) {

  int pos = 0;

  // calc error postion
  if ( (_start) && (_line) && ( _start > _line ) ) pos = _start - _line; 

  // print marker
  for (int i=0; i<pos; i++) printf(" ");
  printf( "^ Error: " );

  // print error
  switch ( error ) {
    case ERROR_SYNTAXERROR:             printf("Syntax error.\n"); break;
    case ERROR_DOTEXPECTED:             printf("\".\" expected.\n"); break;  
    case ERROR_LITERALEXPECTED:         printf("Literal expected.\n"); break; 
    case ERROR_IOEXPECTED:              printf("Not a valid IO port or IO port is offline.\n"); break;
    case ERROR_NUMBEREXPECTED:          printf("Number expected.\n"); break;
    case ERROR_LPARANTHESISEXPECTED:    printf("\"(\" expected.\n"); break;
    case ERROR_RPARANTHESISEXPECTED:    printf("\")\" expected.\n"); break;
    case ERROR_UNKOWNCMD:               printf("unkown command\n"); break;
    case ERROR_WRONGNUMBEROFARGUMENTS:  printf("%d parameters expected, %d found.\n", expected, found ); break;
    default:                          printf("Syntax error.\n"); break;
  }
  
}

bool CLI::isDigit( char ch) {
  return ( (ch >= '0') &&  (ch <= '9' ) );
}

bool CLI::isAlpha( char ch ) {
  return ( (ch >= 'A') && (ch <= 'Z' ) ) ||
         ( (ch >= 'a') && (ch <= 'z' ) ) ||
         (ch == '_');
}

bool CLI::isLiteral( char ch) {
  return (isDigit(ch) ) || isAlpha(ch);
}

EvalResult_t CLI::getNumber( void ) {
  // _evalPtr is already on the first char

  // if the number starts with + or -, the next char needs to be a digit
  if ( ( (*_evalPtr=='+') || (*_evalPtr=='-') ) && 
       ( !isDigit(_evalPtr[1]) ) )
    return EVAL_SYNTAXERROR;

  // now loop until end of digits
  while (isDigit(*++_evalPtr));

  return EVAL_NUMBER;

}

EvalResult_t CLI::getLiteral( void ) {
  // _evalPtr is already on the first char

  // now loop until end of literals
  while (isLiteral(*_evalPtr)) _evalPtr++;

  return EVAL_LITERAL;

}

EvalResult_t CLI::getNextToken( char *token ) {

  // kill white spaces
  while (*_evalPtr==' ') _evalPtr++;

  // remember where we started
  _start = _evalPtr;  

  // different behaviour based on first char
  EvalResult_t eval = EVAL_OK;
  switch (*_evalPtr) {
    case '\0':        eval = EVAL_EOL; break; // no _evalPtr++, so we can't skip EOL
    
    case '.':         eval = EVAL_DOT; _evalPtr++; break;
    
    case ',':         eval = EVAL_COMMA; _evalPtr++; break;

    case '(':         eval = EVAL_LPARANTHESIS; _evalPtr++; break;
    
    case ')':         eval = EVAL_RPARANTHESIS; _evalPtr++; break;
    
    case '+':
    case '-':  
    case '0' ... '9': eval = getNumber( ); break;
    
    case 'a' ... 'z':
    case 'A' ... 'Z': eval = getLiteral( ); break;
    
    default:          eval = EVAL_SYNTAXERROR; break;

  }

   // valid data? copy token or clear it
  if ( eval >= EVAL_OK ) {
    strncpy( token, _start, (_evalPtr - _start) );
    token[_evalPtr - _start]='\0';
    
  } else {
    token[0] = '\0';
  }

  // return evaluation result
  return eval;
}

Cmd_t CLI::evalSimpleCommand( char *token ) {

  Cmd_t cmd = CMD_UNKONWN;

   // did I found a command?
  if      ( strcmp( token, "help"  ) == 0 ) cmd = CMD_HELP;
  else if ( strcmp( token, "setup" ) == 0 ) cmd = CMD_SETUP;
  else if ( strcmp( token, "exit" ) == 0 )  cmd = CMD_EXIT;
  
  // command found?
  if ( cmd >= 0 ) {

    // all commands don't have parameters
    if ( getNextToken( token ) != EVAL_EOL ) {
      Error( ERROR_SYNTAXERROR );
      return CMD_ERROR;
    }

    // execute 
    switch (cmd) {
      case CMD_HELP:  printf("help\n"); break;
      case CMD_SETUP: mainMenu();       break;
    }

  }

  return cmd;

}

bool CLI::eval( void ) {
  // returns false on command exit

  char token[CLIMAXLINE];
  
  // rest variables to run the evaluation
  _evalPtr = _line;

  // every line must start with a literal
  switch ( getNextToken( token ) ) {
    case EVAL_LITERAL: break;         // everything ok
    case EVAL_EOL:     return true;   // just an emoty line, ignore
    default:           Error( ERROR_LITERALEXPECTED ); return true;
  }

  // interpret as a simple command
  switch ( evalSimpleCommand( token ) ) {
    case CMD_EXIT:    return false;   // exit: stop CLI
    case CMD_UNKONWN: break;          // not a command: continue
    default:          return true;    // error, setup, help: stop evaluation
  }

  // if it's not a command, it should be a host or an io.
  evalIOCommand( token );
  return true;

}

void CLI::executeIOCommand( void ) {

}

bool CLI::tokenizeCmd( char *cmd ) {

  for (uint8_t i=0; i<=IOCMD_MAX; i++) {
    if ( strcmp( IOCMD[i], cmd ) == 0 ) {
      _cmd = (IOCmd_t) i;
      return true;
    }
  }

  return false;

}

void CLI::evalIOCommand( char *token ) {
  // token is already the first literal. Could be a host or an IO

  SwOSCtrl *ctrl;
  SwOSIO   *io;
  char     cmd[CLIMAXLINE];
  char     IOName[CLIMAXLINE];
  
  // check, if the token is a controller or an io
  ctrl = (SwOSCtrl *)myOSSwarm.getControler( token );
  if (ctrl) { 
    // it's a controller, now we need the io port
    strcpy( IOName, token);
    if ( getNextToken( token ) != EVAL_DOT ) { Error( ERROR_DOTEXPECTED ); return; }
    if ( getNextToken( token ) != EVAL_LITERAL ) { Error( ERROR_LITERALEXPECTED ); return; }
    io = ctrl->getIO( token );
    strcat(IOName, ".");
    strcat(IOName, token);
  } else {
    io = myOSSwarm.getIO( IOName );
  }

  // check, if it's an IO
  if (!io) { Error( ERROR_IOEXPECTED ); return; }

  // now we need another "." and a method
  if ( getNextToken( token ) != EVAL_DOT ) { Error( ERROR_DOTEXPECTED ); return; }

  // next token should be a command
  if ( getNextToken( cmd ) != EVAL_LITERAL ) { Error( ERROR_LITERALEXPECTED ); return; }

  // let's tokenize the cmd
  if ( !tokenizeCmd( cmd ) ) { Error( ERROR_UNKOWNCMD ); return; }

  // optional parameters
  _maxParam = -1;
  switch ( getNextToken( token ) ) {
    case EVAL_EOL: break;
    case EVAL_LPARANTHESIS: {
      bool cont = true;
      while (cont) {
        
        // get a parameter
        switch ( getNextToken( token ) ) {
          case EVAL_NUMBER:         _maxParam++;
                                    _param[_maxParam] = atoi(token);
                                    break;
          case EVAL_RPARANTHESIS:   cont=false;
                                    break;
          default:                  Error( ERROR_NUMBEREXPECTED ); 
                                    return;
        }
        
        // if first token was ), stop the loop
        if (!cont) break;

        // now we're looking for , and )
        switch ( getNextToken( token ) ) {
          case EVAL_RPARANTHESIS: cont = false;
                                  break;
          case EVAL_COMMA:        break;
          default:                Error( ERROR_RPARANTHESISEXPECTED ); return;
        }
      }
      break; }
    default:
      Error( ERROR_LPARANTHESISEXPECTED );
      return;
  }

  if ( _maxParam+1 != IOCMD_Parameters[ _cmd ] ) { Error( ERROR_WRONGNUMBEROFARGUMENTS, IOCMD_Parameters[ _cmd ], _maxParam+1 ); return; } 

  // nothing should be left
  if ( getNextToken( token ) != EVAL_EOL ) { Error( ERROR_SYNTAXERROR ); return; }

  // subscribe needs non-int-parameters
  if (_cmd==IOCMD_subscribe) {
    io->subscribe( IOName, _param[0] );
  } else 
    io->execute( _cmd, _maxParam, _param );

  return;

}

void CLI::run( void ) {

  printf("\n\nftSwarmOS %s\n\n(C) Christian Bergschneider & Stefan Fuss\n", SWOSVERSION );
  printf("\n\nsetup - start configuration menus\nexit - end CLI mode\nhelp - show commands\n\n");

  while (true) {

    // wait on user
    enterString(">", _line, CLIMAXLINE-1 );

    // eval string
    if (!eval() ) break;
  
  }

}


void firmware( void ) {

  CLI cli;

  cli.run();

}