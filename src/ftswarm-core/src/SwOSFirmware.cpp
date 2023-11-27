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
const char WIFI[3][12]    = { "off", "AP-Mode", "Client-Mode"};

#define MAXMENUITEMS 40

class Menu {
  private:
    uint8_t maxItem = 0;
    uint8_t spacer = 0;
    uint8_t id[MAXMENUITEMS];
    char    prompt[40];

  public:
    void    start( const char *prompt, uint8_t spacer );
    void    add( const char *item, const char *value, uint8_t id );
    void    add( const char *item, int value, uint8_t id );
    uint8_t userChoice( void );
};

void Menu::start( const char *prompt, uint8_t spacer ) { 

  maxItem = 0; 
  id[0]   = 0;
  this->spacer = spacer;
  strcpy( this->prompt, (char *) prompt ); 

  // print Headline
  printf( "\n\n%s\n\n", this->prompt );

};

void Menu::add( const char *item, int value, uint8_t id ) {

  char dummy[40];
  sprintf( dummy, "%d", value );
  add( item, dummy, id );
}

void Menu::add( const char *item, const char *value, uint8_t id ){

  maxItem++;
  printf( "(%d) %s", maxItem, item );
  
  if ( value[0] != '\0' ) {
    printf(": ");
    for (uint8_t i=strlen( item ); i<spacer; i++)  printf( " " );
    printf( "%s\n", value );
  } else {
    printf("\n");
  }

  this->id[maxItem] = id;

}

uint8_t Menu::userChoice( void ) {

  printf("\n(0) exit\n%s", prompt);
  
  // asking user
  return id[ enterNumber( ">", 0, 0, maxItem ) ];

}

void ExtentionMenu() {

  bool    anythingChanged = false;
  char    prompt[255];

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

    printf("\n\nExtention Port Settings\n\n");
    sprintf(prompt, "(1) Mode: %s\n(2) Gyro: %s\n(3) I2C Address: %d\n\n(0) exit\nExtention>", EXTMODE[ nvs.extentionPort], ONOFF[nvs.gyro], nvs.I2CAddr );
    
    switch( enterNumber( prompt, 0, 0, 3) ) {
      
      case 0: // exit
        if ( ( anythingChanged) && ( yesNo( "To apply your changes, the device needs to be restarted.\nSave settings and restart now (Y/N)?") ) ) {
          // save config
          nvs.saveAndRestart();
        } else {
          return;
        }
        
      case 1: // ExtMode
        anythingChanged = true;
        nvs.extentionPort = (FtSwarmExtMode_t) enterNumber( "(0) off (1) I2C-Master (2) I2C-Slave (3) Gyro MCU6040 (4) Outputs: ", nvs.extentionPort, 0, 4 );
        break;

      case 2: // Gyro
        anythingChanged = true;
        nvs.gyro = (bool) enterNumber( "(0) off (1) on: ", nvs.gyro, 0, 1 );
        break;

      case 3: // I2C Addr
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

  char          prompt[250];
  bool          anythingChanged = false;
  uint8_t       maxChoice;
  FtSwarmWifi_t wifiMode;

  Menu menu;
  
  while(1) {

    // build menu
    menu.start( "Wifi & WebUI", 14 );
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
    sprintf( prompt, "Do you really want to quit swarm \"%s\" and create new swarm \"%s\" with pin %d (Y/N) ?", nvs.swarmName, name, pin );
  } else {
    sprintf( prompt, "Do you really want to quit swarm \"%s\" and join swarm \"%s\" with pin %d (Y/N) ?", nvs.swarmName, name, pin );
  }

  // stop, if NO
  if ( !yesNo( prompt ) ) return;

  // now build/join swarm
  myOSSwarm.lock();
  myOSSwarm.leaveSwarm( );
  myOSSwarm.unlock();

  // new swarm?
  if ( createNewSwarm ) {
    nvs.createSwarm( name, pin );
    myOSNetwork.setSecret( nvs.swarmSecret, nvs.swarmPIN );
    nvs.save();
    printf("Swarm \"%s\" created sucessfully.\n", name );
    return;
  }

  // existing swarm?
  myOSSwarm.lock();
  myOSSwarm.allowPairing = true;
  myOSSwarm.unlock();

  // try to join the swarm 3 times
  for (uint8_t i=0; i<3; i++) {

    myOSSwarm.lock();
    myOSSwarm.joinSwarm( createNewSwarm, name, pin );
    myOSSwarm.unlock();

    // wait some ticks to catch a response
    for (uint8_t j=0; j<10; j++) {
      vTaskDelay( 25 / portTICK_PERIOD_MS );
    
      // got an ack? -> break inner loop;
      if ( !myOSNetwork.hasJoinedASwarm() ) break;

    }

    // got an ack? -> break outer loop;
    if ( !myOSNetwork.hasJoinedASwarm() ) break;

  }

  // stop pairing
  myOSSwarm.lock();
  myOSSwarm.allowPairing = false;
  myOSSwarm.unlock();

  // Now check, if a swarm was found
  if ( myOSNetwork.hasJoinedASwarm() ) {

    // no swarm found, restore old settings
    myOSSwarm.lock();
    myOSSwarm.reload();
    myOSSwarm.unlock();
    printf("ERROR: swarm \"%s\" not found. Rejoined old swarm %s\n", name, nvs.swarmName );
    return;

  } else {

    // swarm found

    // send my configuration to the swarm
    myOSSwarm.lock();
    myOSSwarm.registerMe();
    myOSSwarm.unlock();

    // some delay to get all swarm members
    vTaskDelay( 250 / portTICK_PERIOD_MS );

    // save new secret, pin & swarm name
    nvs.save();
    
    printf("Swarm \"%s\" joined sucessfully.\n", name );

  }

}

const char SWARMCOMMUNICATION[4][13] = { "none", "wifi", "RS485", "wifi & RS485" };

void swarmMenu( void ) {

  char kelda[MAXIDENTIFIER];
  FtSwarmCommunication_t swarmCommunication;
  Menu menu;

  while (1) {

    menu.start("swarm configuration", 19 );

    printf( "This device is connected to swarm \"%s\" with %d member(s) online.\nSwarm PIN is %d.\n", nvs.swarmName, myOSSwarm.members(), nvs.swarmPIN );

    if (nvs.IAmKelda) {
      strcpy( kelda, "this controler");
    } else if ( myOSSwarm.Kelda) {
      strcpy( kelda, myOSSwarm.Kelda->getHostname() );
    } else {
      strcpy( kelda, "none" );
    }
    menu.add( "Kelda", kelda, 1);

    if ( nvs.RS485Available() ) menu.add( "swarm communication", SWARMCOMMUNICATION[nvs.swarmCommunication], 2 );

    if (nvs.IAmKelda) {
      menu.add( "create a new swarm", "", 3 );
      menu.add( "list swarm members", "", 4 );
    } else {
      menu.add( "join another swarm", "", 3 );
    }

    switch( menu.userChoice() ) {
      case 0: // main
        return;

      case 1: // kelda
        if (nvs.IAmKelda) printf("Downgrading this controler from Kelda to swarm member.\n");
        else              printf("Upgrading this controler from swarm member to Kelda.\n");
        if ( yesNo( "To apply your changes, the device needs to be restarted.\nSave settings and restart now (Y/N)?") ) {
          nvs.IAmKelda = !nvs.IAmKelda;
          nvs.saveAndRestart();
        }
        break;

      case 2: // swarm communication
        if ( !nvs.RS485Available() ) {
          printf("This controler just supports wifi.\n");
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
        
      case 3: // create or join a swarm
        joinSwarm( nvs.IAmKelda ); 
        break;
        
      case 4: // list swarm members
        printf("\nSwarm members:\n" );
        for (uint8_t i=0; i<=myOSSwarm.maxCtrl; i++ ) {
          if ( myOSSwarm.Ctrl[i] )
            printf("#%d %s/%s - last contact %lu ms ago\n", i, myOSSwarm.Ctrl[i]->getName(), myOSSwarm.Ctrl[i]->getAlias(), myOSSwarm.Ctrl[i]->networkAge() );
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

    // list LEDs
    for (uint8_t i=0; i<nvs.RGBLeds; i++ ) {
      if ( myOSSwarm.Ctrl[0]->led[i]) {
        OSObj[item++] =  myOSSwarm.Ctrl[0]->led[i];
        printf("(%2d) %-4s - %-32s\n", 
               item,  myOSSwarm.Ctrl[0]->led[i]->getName(),  myOSSwarm.Ctrl[0]->led[i]->getAlias());
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
                                printf("(%2d) %-4s - %-32s\n", item, ftSwarm->servo[i]->getName(), ftSwarm->servo[i]->getAlias());
                              }
                            }
                            // list gyro
                            if (ftSwarm->gyro) {
                              OSObj[item++] = ftSwarm->gyro;
                              printf("(%2d) %-4s - %-32s\n", item, ftSwarm->gyro->getName(), ftSwarm->gyro->getAlias() );
                            }
                            break;

      case FTSWARMCONTROL:  ftSwarmControl = static_cast<SwOSSwarmControl *>(myOSSwarm.Ctrl[0]);
                            // buttons
                            for (uint8_t i=0; i<8; i++ ) {
                              OSObj[item++] = ftSwarmControl->button[i];
                              printf("(%2d) %-4s - %-32s\n", item, ftSwarmControl->button[i]->getName(),   ftSwarmControl->button[i]->getAlias() );
                            }
                            // joysticks
                            for (uint8_t i=0; i<2; i++ ) {
                              OSObj[item++] = ftSwarmControl->joystick[i];
                              printf("(%2d) %-4s - %-32s\n", item, ftSwarmControl->joystick[i]->getName(), ftSwarmControl->joystick[i]->getAlias());
                            }
                            if (ftSwarmControl->oled) {
                              OSObj[item++] = ftSwarmControl->oled;
                              printf("(%2d) %-4s - %-32s\n", item, ftSwarmControl->oled->getName(), ftSwarmControl->oled->getAlias() );
                            }
                            break;

      case FTSWARMCAM:      ftSwarmCAM = static_cast<SwOSSwarmCAM *>(myOSSwarm.Ctrl[0]);
                            if (ftSwarmCAM->cam) {
                              OSObj[item++] = ftSwarmCAM->cam;
                              printf("(%2d) %-4s - %-32s\n", item, ftSwarmCAM->cam->getName(), ftSwarmCAM->cam->getAlias() );
                            }
                            break;

      default:              break;
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

        // send new config to Kelda
        if ( ( myOSSwarm.Kelda ) && ( myOSSwarm.Kelda != myOSSwarm.Ctrl[0] ) ) myOSSwarm.Ctrl[0]->sendAlias( myOSSwarm.Kelda->macAddr );

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
    newSensor = myOSSwarm.getIO( sensor, FTSWARM_INPUT );
    
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
    newActor = myOSSwarm.getIO( actor, FTSWARM_ACTOR );
    
    if (actor[0] == '\0' ) 
      return false;
      
    else if (!newActor) 
      printf("actor %s doesn't exist in the swarm.\n", actor);

    else if ( !( (newActor->getIOType() == FTSWARM_ACTOR ) || (newActor->getIOType() == FTSWARM_PIXEL ) || (newActor->getIOType() == FTSWARM_SERVO ) ) )
      printf("%s needs to be an actor, a LED or a servo.\n", actor);
    
    else
      // now, I'm fine
      break;
  }

  // enter parameter
  usePortValue = enterNumber( "(0) set a static value OR (1) use the sensors value? ", 0, 0, 1 );
  if (!usePortValue) parameter = enterNumberI32( "Which value should be set? ", 0, 0, 0xFFFFFF );

  // delete old trigger
  SwOSIO *oldSensor = myOSSwarm.getIO( event->sensor, FTSWARM_INPUT );
  
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
  switch (myOSSwarm.Ctrl[0]->getType()) {

    case FTSWARM:         sprintf( prompt, "\nMain Menu\n\n(1) wifi & Web UI\n(2) swarm configuration\n(3) alias names\n(4) factory reset\n(5) remoteControl\n(6) extention port\n\n(0) exit\nmain>" );
                          maxChoice = 6;
                          break;

    case FTSWARMCONTROL:  sprintf( prompt, "\nMain Menu\n\n(1) wifi & Web UI\n(2) swarm configuration\n(3) alias names\n(4) factory reset\n(5) remoteControl\n(6) extention port\n(7) ftSwarmControl\n\n(0) exit\nmain>" );
                          maxChoice = 7;
                          break;

    case FTSWARMCAM:    
    case FTSWARMPWRDRIVE:
    case FTSWARMDUINO:    sprintf( prompt, "\nMain Menu\n\n(1) wifi & Web UI\n(2)swarm configuration\n(3) alias names\n(4) factory reset\n(5) remoteControl\n\n(0) exit\nmain>" );
                          maxChoice = 5;
                          break;

  }

  while (1) {

    choice = enterNumber( prompt, 0, 0, maxChoice );

    switch( choice  ) {
      case 0: return;
      case 1: wifiMenu(); break;
      case 2: swarmMenu(); break;
      case 3: aliasMenu(); break;
      case 4: factorySettings(); break;
      case 5: remoteControl(); break;
      case 6: ExtentionMenu(); break;
      case 7: SwarmControlMenu(); break;
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