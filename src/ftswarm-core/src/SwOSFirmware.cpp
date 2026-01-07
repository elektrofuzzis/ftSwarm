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
#include "easyMenu.h"
#include "SwOSCLI.h"
#include "SwOSLog.h"
#include "SwOSHW/SwOSHWHAL.h"

const char EXTMODE[7][14] = { "off", "I2C-Master", "I2C-Slave", "Outputs", "Servos", "Lidar", "" }; // "" just to avoid seg faults
const char GYRO[3][8]     = { "off", "LSM6", "MPU6050"};
const char ONOFF[2][5]    = { "off", "on" };
const char OFFM1M2[3][5]  = { "off", "M1", "M2" };
const char WIFI[3][12]    = { "off", "AP-Mode", "Client-Mode"};

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

  joy[0]->fb->deleteFilter( SWOS_FILTER_MULTIPLY );
  joy[1]->lr->deleteFilter( SWOS_FILTER_MULTIPLY );

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
      joy[0]->fb->addFilter( new SwOSMultiply( -1 ) );

      joy[1]->fb->addFilter( new SwOSFJoystick( calibration[2].minValue, calibration[2].midValue, calibration[2].maxValue ) );
      joy[1]->lr->addFilter( new SwOSFJoystick( calibration[3].minValue, calibration[3].midValue, calibration[3].maxValue ) );
      joy[1]->lr->addFilter( new SwOSMultiply( -1 ) );

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

#define MENUITEMSPERPAGE 20

class FirmwareIOMenu : protected Menu {

  protected:

    bool *anythingChanged = NULL;

    virtual void save( void );

  public:

    FirmwareIOMenu( uint8_t maxMenuItems ) : Menu( maxMenuItems ) {};

};

void FirmwareIOMenu::save( void ) {

  uint8_t i;
  bool    changes = false;

  // any changes?
  for ( i=0; i<MAXCTRL; i++ ) { if (anythingChanged[i]) changes = true; }

  if ( ( changes ) && yesNo( "Save changes? (Y/N)?" ) ) {

    // local changes
    if ( anythingChanged[0] ) {

       // save in local nvs
       myOSSwarm.Ctrl[0]->save(2);
       nvs.saveEvents();

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

}

class MenuLocalSettings : private Menu {

  static const int8_t MENU_WIFI       = -1;
  static const int8_t MENU_SSID       = -2;
  static const int8_t MENU_PASSWORD   = -3;
  static const int8_t MENU_CHANNEL    = -4;
  static const int8_t MENU_WEBUI      = -5;
  static const int8_t MENU_PIXELS     = -7;
  static const int8_t MENU_EXT        = -8;
  static const int8_t MENU_I2CADDR    = -9;
  static const int8_t MENU_I2CINT     = -10;
  static const int8_t MENU_I2CLOW     = -11;
  static const int8_t MENU_I2CHIGH    = -12;
  static const int8_t MENU_I2CREGS    = -13;
  static const int8_t MENU_GYRO       = -14;
  static const int8_t MENU_CALIBRATE  = -15;

  bool anythingChanged = false;
  void wifiMode( void );

  public:

    MenuLocalSettings( char *basePrompt = NULL ) : Menu(  basePrompt, "wifi", "Wifi & Local Settings", 14 ) {};
    void run( void );

};

void MenuLocalSettings::wifiMode( void ) {

  FtSwarmWifi_t wifiMode = (FtSwarmWifi_t) enterNumber( "enter wifi mode [ 0-off , 1-AP-Mode, 2-Client-Mode]: ", nvs.wifiMode, 0, 2 );

  if ( nvs.wifiMode != wifiMode ) {
    
    if ( ( wifiMode == wifiOFF ) && ( nvs.swarmCommunication & 0x1 ) ) {
      printf("\e[0;31mError: please deactivate wifi in swarm communication first.\e[0m\n");

    } else {

      nvs.wifiMode = wifiMode;
      
      if ( ( nvs.wifiMode == wifiAP ) && ( ( nvs.channel < 1 ) || ( nvs.channel > 13 ) ) ) nvs.channel = 1; // to avoid invalid channel settings
      
      anythingChanged = true;

    } 

  }

}

void MenuLocalSettings::run( void ) {

  char info[250];
  
  while(1) {

    switch ( nvs.wifiMode ) {
    case wifiAP:      sprintf(info, "hostname:            %s\nip-address:          %d.%d.%d.%d\n\n", myOSSwarm.Ctrl[0]->getHostname(), WiFi.softAPIP()[0], WiFi.softAPIP()[1], WiFi.softAPIP()[2], WiFi.softAPIP()[3]);
                      break;
    case wifiClient:  sprintf(info, "hostname:            %s\nip-address:          %d.%d.%d.%d\n\n", myOSSwarm.Ctrl[0]->getHostname(), WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
                      break;
    default:          sprintf(info, "hostname:            %s\nip-address:          none\n\n",myOSSwarm.Ctrl[0]->getHostname());
    }

    // build menu
    start( );
    printf(info);
    add( "wifi mode", WIFI[nvs.wifiMode], MENU_WIFI, 'w');

    if (nvs.wifiMode != wifiOFF ) {
      
      add( "SSID", nvs.wifiSSID, MENU_SSID, 's');
      
      if (nvs.wifiMode != wifiAP) {
        add( "Password", "*****", MENU_PASSWORD, 'p' );
      } else {
        add( "channel", nvs.channel, MENU_CHANNEL, 'c' );
      } 
      
      add( "Web UI", ONOFF[nvs.webUI], MENU_WEBUI, 'u' );
      
      if ( ( nvs.webUI ) && ( myOSSwarm.Ctrl[0]->getType() != FTSWARMCONTROL ) ) add( "ftPixels in UI", nvs.pixels, MENU_PIXELS, 'f' );

    }
     
    if ( myOSSwarm.Ctrl[0]->hasExtPort() ) { 
      add("Extension Port", EXTMODE[ nvs.extensionPort] , MENU_EXT, 'e' ); 
    }

    // I2C Slave Mode. Options I2C Slave Address and Interrupt Line
    if ( nvs.extensionPort == FTSWARM_EXT_I2C_SLAVE ) {
      add("I2C Slave Address", nvs.I2CAddr, MENU_I2CADDR, 'a');
      add("Interrupt Line", OFFM1M2[nvs.interruptLine], MENU_I2CINT, 'i' );
      add("Interrupt Low Value",  nvs.interruptOnOff[0], MENU_I2CLOW, 'l' );
      add("Interrupt High Value", nvs.interruptOnOff[1], MENU_I2CHIGH, 'h' );
      add("I2C Registers", nvs.I2CRegisters, MENU_I2CREGS, 'r' );
    }

    // gyro if available
    if ( myOSSwarm.Ctrl[0]->hasGyro() ) { 
      add("Gyro", ONOFF[nvs.gyro], MENU_GYRO, 'g' ); 
    }

    if ( myOSSwarm.Ctrl[0]->getType() == FTSWARMCONTROL ) {
      add("Calibrate Joysticks", "", MENU_CALIBRATE, 'j', false );
    }

    addExit();

    char line[100];

    switch ( userChoice(  ) ) {

      case MENU_EXIT:       if ( ( anythingChanged) && ( yesNo( "To apply your changes, the device needs to be restarted.\nSave settings and restart now (Y/N)?") ) ) {
                              // save config
                              nvs.saveAndRestart();
                            } else {
                              return;
                            }
        
      case MENU_WIFI:       wifiMode( );
                            break;
        
      case MENU_SSID:       anythingChanged = true;
                            sprintf( line, "Please enter new SSID [%s]: ", nvs.wifiSSID );
                            enterString( line, nvs.wifiSSID, nvs.wifiSSID, 64);
                            break;
        
      case MENU_PASSWORD:   anythingChanged = true;
                            enterString("Please enter new Password: ", nvs.wifiPwd, 64, true);
                            break;

      case MENU_CHANNEL:    anythingChanged = true;
                            nvs.channel = enterNumber( "enter channel [1..13] - use 1,6 or 11 if possible: ", nvs.channel, 1, 13 );
                            break;

      case MENU_WEBUI:      anythingChanged = true;
                            nvs.webUI = !nvs.webUI;
                            break;
        
      case MENU_PIXELS:     anythingChanged = true;
                            nvs.pixels = enterNumber( "enter number of ftPixel in WebUI [2..18]: ", nvs.pixels, 2, MAXLEDS );
                            break;

      case MENU_CALIBRATE:  if  ( yesNo( "\nStart calibration (Y/N)?" ) ) {
                              anythingChanged = true;
                              calibrateJoysticks( nvs.calibration );
                            }
                            break;

      case MENU_GYRO:       anythingChanged = true;
                            nvs.gyro = (FtSwarmGyroMode_t) enterNumber( "(0) off (1) on: ", nvs.gyro, 0, 1 );
                            if ( ( nvs.gyro ) && ( nvs.CPU != FTSWARMRS_2V0 ) && ( nvs.CPU != FTSWARMRS_2V1 ) ) nvs.extensionPort = FTSWARM_EXT_I2C_MASTER;
                            break;

      case MENU_I2CADDR:    anythingChanged = true;
                            nvs.I2CAddr = (uint8_t) enterNumber( "[16..127]: ", nvs.I2CAddr, 16, 127 );
                            break;

      case MENU_I2CHIGH:    anythingChanged = true;
                            nvs.interruptOnOff[1] = (int16_t) enterNumber( "High Value [-255..255]", nvs.interruptOnOff[1], -255, 255 );
                            break;

      case MENU_I2CLOW:     anythingChanged = true;
                            nvs.interruptOnOff[1] = (int16_t) enterNumber( "High Value [-255..255]", nvs.interruptOnOff[1], -255, 255 );
                            break;

      case MENU_I2CINT:     anythingChanged = true;
                            nvs.interruptLine = (uint8_t) enterNumber( "motor (1 for M1, 2 for M2, ...) or 0 to skip: ", nvs.interruptLine, 0, MAXIOS[nvs.CPU].motors );
                            break;

      case MENU_I2CREGS:    anythingChanged = true;
                            nvs.I2CRegisters = (uint8_t) enterNumber( "I2C Registers [1..8]", nvs.I2CRegisters, 1, MAXI2CREGISTERS);
                            break;

    }

  }

}

class MenuIOConfig : protected FirmwareIOMenu {

  protected:

    static const int8_t MENU_ALIAS    = -1;
    static const int8_t MENU_TYPE     = -2;
    static const int8_t MENU_LABEL    = -3;
    static const int8_t MENU_ADD      = -4;
    static const int8_t MENU_DEL      = -5;
    static const int8_t MENU_CFG      = -6;
    static const int8_t MENU_PREVIOUS = -7;
    static const int8_t MENU_NEXT     = -8;

    SwOSIO* io;
    bool    selfSave = false;
    uint8_t event[MENUITEMSPERPAGE + 1];
    int8_t  maxEvent = -1;

    int     pageOffset = 0;
    bool    morePages  = false;

    void changeType( void );
    void changeAlias( void );

    void fillEventList( void );
    void enterIO( const char* prompt, SwOSIOUID_t *uio, bool input );
    bool enterEvent( SwOSNVSEvent_t *event );

    bool changeEvent( SwOSNVSEvent_t *event );

    void addEvent( void );
    void deleteEvent( void );

    void printEventParameter( FtSwarmOperand_t op, SwOSIO *sensor, SwOSIO *actor, char *doing, int32_t parameter );
    void printEvent( SwOSNVSEvent_t event, uint8_t details = 0 );

    void changeConfig( void );

  public:

    MenuIOConfig( char *basePrompt = NULL, bool *anythingChanged = NULL, SwOSIO* io = NULL );
    ~MenuIOConfig() { if ( selfSave ) free( anythingChanged ); };
    void run( void );

};

MenuIOConfig::MenuIOConfig( char *basePrompt, bool *anythingChanged, SwOSIO* io ):FirmwareIOMenu( MENUITEMSPERPAGE + 10 ) {

  this->io = io;

  if (anythingChanged) 
    this->anythingChanged = anythingChanged;
  else {
    this->anythingChanged = (bool *) calloc( sizeof(bool), MAXCTRL );
    selfSave = true;
  }

  if (io) begin( basePrompt, io->getAliasOrName(),    io->getAliasOrName(),  10, ':' );
  else    begin( basePrompt, "Remote Configuration", "Remote Configuration", 10, ':' );

}

void MenuIOConfig::fillEventList( void ) {

  uint8_t item = 0;
  maxEvent = -1;

  for (uint8_t i=0; i<MAXNVSEVENTS; i++) {

    // end of list?
    if ( nvs.events[nvs.activeEventConfig][i].sensor.serialNumber == 0) break;

    // io not specified OR event is about my io
    if ( (!io) || 
         ( io == myOSSwarm.getIO( nvs.events[nvs.activeEventConfig][i].sensor ) ) ||
         ( io == myOSSwarm.getIO( nvs.events[nvs.activeEventConfig][i].actor  ) ) 
       ) {

      // end of list?
      if ( maxEvent >= MENUITEMSPERPAGE-1 ) {
        morePages = true;
        return;
      }

      // store?
      if ( ++item >= pageOffset ) event[++maxEvent] = i;

    }

  }

}

void MenuIOConfig::changeAlias( void ) {
  
  char prompt[250];
  char alias[MAXIDENTIFIER];
  
  // ask user for new alias
  sprintf( prompt, "%s - please enter new alias: ", io->getAliasOrName() );
  enterIdentifier( prompt, alias, MAXIDENTIFIER );

  // nothing changed
  if ( strcmp( alias, io->getAlias() ) == 0 ) return;
                
  // test on duplicates
  SwOSIO *testIO = myOSSwarm.getIO( alias );
  if ( (testIO) && ( testIO != io ) ) {
    printf("\e[0;31mERROR: This alias is already used in the swarm.\n\e[0m");
    return;
  }

  // change name
  io->setAlias( alias );
  anythingChanged[ myOSSwarm.getIndex( io->getCtrl()->serialNumber ) ] = true;

}

void MenuIOConfig::changeType( void ) {
  
  char alias[MAXIDENTIFIER];
  char prompt[250];

  // change type?
  SwOSIOType_t ioType = io->getIOType();

  // singular class -> done
  if ( SWOSIOCLASS[ioType] == SWOSIOCLASS_SINGULAR ) {
    printf( "\e[0;31mERROR: IO type %s could not be changed to another IO type.\n\e[0m\n", SWOSIOTYPE[ioType] );
    return;
  }
  
  // list compatible types and ask user
  int8_t       maxType = -1;
  SwOSIOType_t defaultValue, type[SWOSIO_MAXIOTYPE];

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

  sprintf( prompt, "Choose new IO Type [%s]:", SWOSIOTYPE[defaultValue]);
  SwOSIOType_t newIOType = type[enterNumber( prompt, defaultValue, 0, maxType )];

  // change type?
  if ( ioType != newIOType ) { 

    // need my controller
    SwOSCtrl *ctrl = io->getCtrl();

    // my index within the io list
    uint8_t index = ctrl->getIndex(io);

    // change it
    if ( ctrl->changeIOType( ctrl->getIndex(io), newIOType ) ) {

      // since I changed my type, io was deleted. Need to refresh io.
      io = ctrl->io[index];

      // yes I did it
      anythingChanged[ myOSSwarm.getIndex( io->getCtrl()->serialNumber ) ] = true;

    }

  }

}

void MenuIOConfig::enterIO( const char* prompt, SwOSIOUID_t *uio, bool input ) {

  char   alias[MAXIDENTIFIER] = "";
  SwOSIO *io;

  // default
  io = myOSSwarm.getIO( *uio );
  if (io) strcpy( alias, io->getAliasOrName() );
  
  while (1) {

    enterString( prompt, alias, alias, sizeof(alias) );

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

}

bool MenuIOConfig::enterEvent( SwOSNVSEvent_t *event ) {

  char   prompt[128];
  SwOSIO *eventIO;

  // sensor
  if ( (io) && ( io->isInput() ) ) {

    printf("sensor's name: %s\n", io->getAliasOrName() );
    io->getUID( &event->sensor );

  } else {

    eventIO = myOSSwarm.getIO( event->sensor );
    
    if (eventIO) sprintf( prompt, "Enter sensor's name [%s]: ", eventIO->getAliasOrName() );
    else         sprintf( prompt, "Enter sensor's name: " );
    
    enterIO( prompt, &event->sensor, true  );

  }

  // trigger
  if ( !myOSSwarm.getIO( event->sensor )->isDigitalInput() ) {
    printf( "Enter trigger event: change value.\n");
    event->triggerMath.bits.trigger = FTSWARM_TRIGGERVALUE;
  
  } else {
    FtSwarmTrigger_t trigger = event->triggerMath.bits.trigger;
    sprintf( prompt, "Enter trigger event - (0) trigger down  (1) trigger up  (2) change value [%d]: ", trigger );
    event->triggerMath.bits.trigger = (FtSwarmTrigger_t) enterNumber( prompt, trigger, 0, 2 );
    printEvent( *event, 1 );
  }


  // actor
  if ( (io) && ( io->isActor() ) ) {

    printf("actors's name: %s\n", io->getAliasOrName() );
    io->getUID( &event->actor );

  } else {

    eventIO = myOSSwarm.getIO( event->actor );
    
    if (eventIO) sprintf( prompt, "Enter actor's name [%s]: ", eventIO->getAliasOrName() );
    else         sprintf( prompt, "Enter actor's name: " );
  
    enterIO( prompt,  &event->actor,  false );

  }

  printEvent( *event, 2 );

  // V1
  sprintf( prompt, "Use - (0) fixed value  (1) sensor's value  (2) actor's value [%d]: ", event->triggerMath.bits.v1 );
  event->triggerMath.bits.v1 = (FtSwarmOperand_t) enterNumber( prompt, event->triggerMath.bits.v1, 0, 2 );

  // constant?
  if ( event->triggerMath.bits.v1 == FTSWARM_CONSTANT ) {
    sprintf( prompt, "Enter fixed value [%d]: ", event->parameter );
    event->parameter = enterNumber( prompt, event->parameter, -4096, 0xFFFFFF );
  }

  printEvent( *event, 3 );

  // operator
  sprintf( prompt, "(1) add or  (2) muliply another value - (0) done [%d]: ", event->triggerMath.bits.op );
  event->triggerMath.bits.op = (FtSwarmOperator_t) enterNumber( prompt, event->triggerMath.bits.op, 0, 2 );
  
  printEvent( *event, 4 );

  // V2
  if ( event->triggerMath.bits.op != FTSWARM_ASSIGN ) {

    if ( event->triggerMath.bits.v1==FTSWARM_CONSTANT ) {
      
      if ( event->triggerMath.bits.v2 == FTSWARM_CONSTANT ) event->triggerMath.bits.v2 = FTSWARM_SENSORVALUE;
      
      sprintf( prompt, "Use - (1) sensor's value  (2) actor's value [%d]: ", event->triggerMath.bits.v2 );
      event->triggerMath.bits.v2 = (FtSwarmOperand_t) enterNumber( prompt, event->triggerMath.bits.v2, 1, 2 );

    } else {
      
      sprintf( prompt, "Use - (0) fixed value  (1) sensor's value  (2) actor's value [%d]: ", event->triggerMath.bits.v2 );
      event->triggerMath.bits.v2 = (FtSwarmOperand_t) enterNumber( prompt, event->triggerMath.bits.v2, 0, 2 );

    }

    // constant?
    if ( event->triggerMath.bits.v2 == FTSWARM_CONSTANT ) {
      sprintf( prompt, "Enter fixed value [%d]: ", event->parameter );
      event->parameter = enterNumber( prompt, event->parameter, -4096, 0xFFFFFF );
    
    }

  }

  printEvent( *event );

  return true;

}

bool MenuIOConfig::changeEvent(SwOSNVSEvent_t *event ) {

  // create a copy of the event
  SwOSNVSEvent_t newEvent;
  memcpy( &newEvent, event, sizeof(SwOSNVSEvent_t) );

  // ask user
  if ( !enterEvent( &newEvent ) ) return false;

  // nothing changed?
  if ( cmpEvent( &newEvent, event ) == 2 ) return false;

  // duplicates?
  for (uint8_t i=0; i< MAXNVSEVENTS; i++ ) {

    if ( ( cmpEvent( &newEvent, &nvs.events[nvs.activeEventConfig][i] ) >0 ) &&
         ( event != &nvs.events[nvs.activeEventConfig][i])
       ) {

      printf("ERROR: This event already exists.");
      return false;
    
    }

  }

  // change event
  myOSSwarm.deleteEvent( event );
  memcpy( event, &newEvent, sizeof( SwOSNVSEvent_t ) );
  myOSSwarm.addEvent( event );

  // events are stored locally only
  anythingChanged[0] = true;

  return true;

}

void MenuIOConfig::addEvent( void ) {

  for ( uint8_t i=0; i<MAXNVSEVENTS; i++ ) {

    // free space found?
    if ( nvs.events[nvs.activeEventConfig][i].sensor.serialNumber == 0) {

      changeEvent( &nvs.events[nvs.activeEventConfig][i] );

      return;

    }

  }

}

void MenuIOConfig::deleteEvent( void ) {

  char prompt[255];
  sprintf( prompt, "Which event should be deleted? [0 - abort, 1..%d]:", maxEvent+1 );
  uint8_t selected = enterNumber( prompt, 0, 0, maxEvent+1 );

  // abort
  if ( selected==0 ) return;

  // adjust selected to event list index
  selected = selected -1;

  // delete event
  myOSSwarm.deleteEvent( &nvs.events[nvs.activeEventConfig][event[selected]] );

  // move all successors
  if ( selected+1 < MAXNVSEVENTS ) memcpy( &nvs.events[nvs.activeEventConfig][event[selected]], &nvs.events[nvs.activeEventConfig][event[selected]+1], ( MAXNVSEVENTS - selected -1 ) * sizeof( SwOSNVSEvent_t ) );

  // cleanup last event
  bzero( &nvs.events[nvs.activeEventConfig][MAXNVSEVENTS-1], sizeof( SwOSNVSEvent_t) );

  // events are stored locally only
  anythingChanged[0] = true;

}


void MenuIOConfig::printEventParameter( FtSwarmOperand_t op, SwOSIO *sensor, SwOSIO *actor, char *doing, int32_t parameter ) {

  switch ( op ) {
    case FTSWARM_CONSTANT:    if ( actor->isPixel() ) printf( "#%06X", parameter );
                              else                    printf( "%d", parameter ); 
                              break;

    case FTSWARM_SENSORVALUE: printf( "%s.getValue()", sensor->getAliasOrName() ); 
                              break;

    case FTSWARM_ACTORVALUE:  printf( "%s.get%s()", actor->getAliasOrName(), doing );
                              break;
  }

}

void MenuIOConfig::printEvent( SwOSNVSEvent_t event, uint8_t details ) {

  SwOSIO *sensor = dynamic_cast<SwOSIO*>( myOSSwarm.getIO( event.sensor ) );
  SwOSIO *actor  = dynamic_cast<SwOSIO*>( myOSSwarm.getIO( event.actor ) );

  if (details) printf("\n");

  printf("%s.%s", sensor->getAliasOrName(), FTSWARMTRIGGER[ event.triggerMath.bits.trigger ] );
  if (details == 1 ) { printf("\n"); return; }

  char doing[15];
  if      ( actor->isMotor() ) strcpy( doing, "Speed");
  else if ( actor->isServo() ) strcpy( doing, "Position");
  else if ( actor->isPixel() ) strcpy( doing, "Color" );
  else                         strcpy( doing, "???" );

  printf(" -> %s.set%s( ", actor->getAliasOrName(), doing );
  if (details == 2 ) { printf("\n"); return; }

  printEventParameter( event.triggerMath.bits.v1, sensor, actor, doing, event.parameter );
  if (details == 3 ) { printf("\n"); return; }

  if ( event.triggerMath.bits.op != FTSWARM_ASSIGN ) {
  
    printf(" %s ", FTSWARMOPERATOR[event.triggerMath.bits.op] );
    if (details == 4 ) { printf("\n"); return; }
  
    printEventParameter( event.triggerMath.bits.v2, sensor, actor, doing, event.parameter );
  } 
  
  printf(" )\n");

}

void MenuIOConfig::changeConfig( void ) {

  char line[80];
  uint8_t newConfig;

  sprintf( line, "Switch to configuration [1..%d]", MAXEVENTCONFIGS );
  newConfig = enterNumber( line, nvs.activeEventConfig+1, 1, MAXEVENTCONFIGS ) -1;
  
  if ( newConfig != nvs.activeEventConfig ) {
      anythingChanged[0] = true;
      nvs.activeEventConfig = newConfig;
      myOSSwarm.deleteEvents();
      myOSSwarm.addEvents( newConfig );
  }

}

void MenuIOConfig::run( void ) {

  while (1) {

    start();
    fillEventList();

    if (io) {
    
      add( "name", io->getName(), MENU_DEACTIVATED, MENU_NOKEY );
      add( "IO type", SWOSIOTYPE[ io->getIOType() ], MENU_TYPE, 't' );
      add( "alias",   io->getAlias(), MENU_ALIAS, 'a' );
      add( "label",  "", MENU_LABEL, 'l' );

      if ( maxEvent >= 0 ) printf("\n     Events:\n");

    }

    for (uint8_t i=0; i<=maxEvent; i++) {

      printf("(%2d) ", i+1 );
      printEvent( nvs.events[nvs.activeEventConfig][event[i]] );

      add( i );

    }

    printf("\n");

    if (morePages)      add( "next page", "", MENU_NEXT, '>' );
    if (pageOffset > 0) add( "previous page", "", MENU_PREVIOUS, '<' );
    if ( ( morePages ) || (pageOffset > 0) ) printf("\n");

    if ( maxEvent < MAXNVSEVENTS ) add( "add event", "", MENU_ADD, '+' );
    if ( maxEvent >= 0           ) add( "delete event", "", MENU_DEL, '-' );
    add( "switch configuration", "", MENU_CFG, 's' );
    
    addExit();

    int8_t choice = userChoice();

    switch ( choice ) {

      case MENU_EXIT:     if ( selfSave ) save();
                          return;

      case MENU_ALIAS:    changeAlias( );
                          break;

      case MENU_TYPE:     changeType( );
                          break;

      case MENU_ADD:      printf("\n" ); 
                          addEvent( );
                          break;

      case MENU_DEL:      printf("\n");
                          deleteEvent( );
                          break;

      case MENU_CFG:      printf("\n");
                          changeConfig();
                          break;

      case MENU_NEXT:     pageOffset += MENUITEMSPERPAGE;
                          break;

      case MENU_PREVIOUS: pageOffset -= MENUITEMSPERPAGE;
                          if ( pageOffset < 0 ) pageOffset = 0;
                          break;

      default:            printf("\n"); 
                          changeEvent( &nvs.events[nvs.activeEventConfig][event[choice-1]] );
                          break;

    }

  }

}

class MenuIOList : private FirmwareIOMenu {

  private:

    // Menu constansts
    static const int8_t MENU_PIXEL    = -1;
    static const int8_t MENU_INPUT    = -2;
    static const int8_t MENU_ACTOR    = -3;
    static const int8_t MENU_NEXT     = -4;
    static const int8_t MENU_PREVIOUS = -5;

    // controller
    SwOSCtrl *controller = NULL;

    // IOs shown in the menu
    SwOSIO *io[MENUITEMSPERPAGE+1];
    int8_t maxItem    = -1;
    int8_t selected   = -1;
    int    pageOffset = 0;
    bool   morePages  = false;

    // types of IOs to show
    bool listInputs = true;
    bool listActors = false;
    bool listPixels = false;

    void fillIOList( void );

  public:

    MenuIOList( char *basePrompt = NULL, SwOSCtrl *controller = NULL );
    ~MenuIOList() { if ( anythingChanged ) free( anythingChanged ); };
    void run( void );

};

MenuIOList::MenuIOList( char *basePrompt, SwOSCtrl *controller ):FirmwareIOMenu( MENUITEMSPERPAGE + 10 ) {

  anythingChanged = (bool*) calloc( sizeof( bool ), MAXCTRL );

  const char ioconfig[] = "IO configuration";

  // if <0 show all controllers, if >=0 show this one only
  this->controller = controller;

  // initialize anythingChanged
  for (uint8_t i=0; i<MAXCTRL; i++) anythingChanged[i] = false;

  if ( controller )  begin( basePrompt, controller->getName(), controller->getName(), 10, ' ' );
  else               begin( basePrompt, ioconfig, ioconfig, MENUITEMSPERPAGE+10, ' ' );

}

void MenuIOList::fillIOList( void ) {

  int item  = -1;
  maxItem   = -1;
  morePages = false;

  // all controllers
  for ( int8_t c=0; c<=myOSSwarm.maxCtrl; c++ ) {
    
    // check, if the controller needs to be listed
    if ( ( myOSSwarm.Ctrl[c] ) && ( ( !controller ) || ( myOSSwarm.Ctrl[c] == controller ) ) ) {

      // all IOs
      for (uint8_t i=0; i<myOSSwarm.Ctrl[c]->IOs; i++ ) {

        if ( ( myOSSwarm.Ctrl[c]->io[i] ) && 
             ( ( listInputs && myOSSwarm.Ctrl[c]->io[i]->isInput() ) ||
               ( listActors && myOSSwarm.Ctrl[c]->io[i]->isActor() && !myOSSwarm.Ctrl[c]->io[i]->isPixel() ) ||
               ( listPixels && myOSSwarm.Ctrl[c]->io[i]->isPixel() ) ) ) {

                if ( maxItem >= MENUITEMSPERPAGE-1 ) {
                  morePages = true;
                  return;
                }

                if ( ++item >= pageOffset ) io[++maxItem] = myOSSwarm.Ctrl[c]->io[i];

         }

      }

    }

  }

}

void MenuIOList::run( void ) {

  while (1) {

    start();

    fillIOList();
  
    printf("     Name               Type            Events Alias\n");

    // list IOs
    char data[80];
    char name[MAXIDENTIFIER+10];

    // list IOs
    for (int8_t i=0; i<=maxItem; i++ ) {
         
      // different ways to print the IO name
      if ( controller >= 0 ) sprintf( name, "%s",    io[i]->getName() );
      else                   sprintf( name, "%s.%s", io[i]->getCtrl()->getName(), io[i]->getName() );

      // count events
      uint8_t events=0;
      for (uint8_t e=0; i<MAXNVSEVENTS; e++) {

        // end of list?
        if ( nvs.events[nvs.activeEventConfig][e].sensor.serialNumber == 0) break;

        // my event?
        if ( ( io[i] == myOSSwarm.getIO( nvs.events[nvs.activeEventConfig][e].sensor ) ) ||
             ( io[i] == myOSSwarm.getIO( nvs.events[nvs.activeEventConfig][e].actor  ) ) 
           ) 
          events++;

      }

      sprintf( data, "%-18s %-15s %-3d    %s", name, SWOSIOTYPE[ io[i]->getIOType() ], events, io[i]->getAlias() );

      add( data, "", i+1, '\0', true );

    }

    printf("\n");

    if (morePages)      add( "next page", "", MENU_NEXT, '>' );
    if (pageOffset > 0) add( "previous page", "", MENU_PREVIOUS, '<' );
    if ( ( morePages ) || (pageOffset > 0) ) printf("\n");

    if (!listInputs) add( "show inputs", "", MENU_INPUT, 'i' );
    if (!listActors) add( "show actors", "", MENU_ACTOR, 'a' );
    if (!listPixels) add( "show pixels", "", MENU_PIXEL, 'p' );

    addExit();
  
    // User's choice
    int8_t choice = userChoice( );
    MenuIOConfig *menuIOConfig;

    switch (choice) {

      case MENU_EXIT:     save();
                          return;

      case MENU_PIXEL:    listInputs = false;
                          listActors = false;
                          listPixels = true;
                          break;

      case MENU_ACTOR:    listInputs = false;
                          listActors = true;
                          listPixels = false;
                          break;

      case MENU_INPUT:    listInputs = true;
                          listActors = false;
                          listPixels = false;
                          break;

      case MENU_NEXT:     pageOffset += MENUITEMSPERPAGE;
                          break;

      case MENU_PREVIOUS: pageOffset -= MENUITEMSPERPAGE;
                          if ( pageOffset < 0 ) pageOffset = 0;
                          break;

      default:            // changeEvents( io[choice-1] );
                          menuIOConfig = new MenuIOConfig( this->prompt, anythingChanged, io[choice-1] );
                          menuIOConfig->run();
                          delete( menuIOConfig );
                          break;

    }

  }

}

// ---------------------- menuSwarmConfig stuff ----------------------

class MenuSwarmConfig : Menu {

  private:
    static const int8_t MENU_NEW           = -1;
    static const int8_t MENU_ADD           = -2;
    static const int8_t MENU_DELETE        = -3;
    static const int8_t MENU_COMMUNICATION = -4;
    static const int8_t MENU_SPEED         = -5;
    static const int8_t MENU_PIN           = -6;
    static const int8_t MENU_ALIAS         = -7;

    SwOSCtrl *ctrl[MAXCTRL];
    int8_t   maxCtrl = -1;

    void fillCtrlList( void );
    void changeAlias( void );
    void newSwarm( void );
    void addController( void );
    void deleteController( void );

  public:
    MenuSwarmConfig( char *basePrompt = NULL );
    void run( void );
};

MenuSwarmConfig::MenuSwarmConfig( char *basePrompt ):Menu( basePrompt, "Swarm Configuration", "Swarm Configuration", MAXCTRL + 10 ) {
};


void MenuSwarmConfig::fillCtrlList( void ) {

  maxCtrl = -1;
  for (int8_t i=0; i<=myOSSwarm.maxCtrl; i++) {
    if (myOSSwarm.Ctrl[i]) ctrl[++maxCtrl] = myOSSwarm.Ctrl[i];
  }

}

void MenuSwarmConfig::changeAlias(void ) {
  
  char prompt[250];
  char alias[MAXIDENTIFIER];

  // Which one to change?
  sprintf( prompt, "Please select the controller to be changed [1..%d]:", maxCtrl+1 );
  int8_t selected = enterNumber( prompt, 0, 1, maxCtrl+1 ) -1;

  // nothing selected
  if (selected < 0 ) return;

  // controlle isn't online
  if (ctrl[selected]->getComState() != COMSTATE_ONLINE ) {
    printf("\e[0;31mERROR: %s is not online.\n\e[0m\n", ctrl[selected]->getAliasOrName() );
    return;
  }
  
  // ask user for new alias
  sprintf( prompt, "%s - please enter new alias: ", ctrl[selected]->getAliasOrName() );
  enterIdentifier( prompt, alias, MAXIDENTIFIER );

  // no changes
  if ( strcmp( alias, ctrl[selected]->getAliasOrName() ) == 0 ) return;
                
  // test on duplicates
  SwOSCtrl *testCtrl = (SwOSCtrl *)myOSSwarm.getController( alias );
  if ( (testCtrl) && ( testCtrl != ctrl[selected] ) ) {
    printf("\e[0;31mERROR: This alias is already used in the swarm.\n\e[0m");
    return;
  }

  // change name
  ctrl[selected]->setAlias( alias );
  ctrl[selected]->save(2);

}

void MenuSwarmConfig::newSwarm( void ) {

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

void MenuSwarmConfig::addController( void ) {

  FtSwarmSerialNumber_t serialNumber = (FtSwarmSerialNumber_t) enterNumber("Enter new swarm members serial number [1..9999]: ", -1, 1, 9999 );

  if ( myOSSwarm.isMember( serialNumber ) ) { printf("\e[0;31mERROR: This controller is already part of this swarm.\e[0m\n"); return; }
  
  if ( !myOSSwarm.addController( serialNumber ) ) {
    // no space left
    printf("\e[0;31mERROR: No space left in swarm. Controller #%d was declined.\e[0m\n", serialNumber );
    return;
  }

  printf("Controller SN %d was added to the swarm.\n", serialNumber );

  // wait max 1.5 minutes to get the controller connected
  uint8_t i=0;
  while ( !myOSSwarm.isOnline( serialNumber ) ) {
    i++;
    delay(100);
    if (i > 15 ) break;
  }
  
  if ( !myOSSwarm.isOnline( serialNumber ) ) printf("\e[0;31mWARNING: Please switch controller #%d on.\e[0m\n", serialNumber);

  nvs.save( );

}

void MenuSwarmConfig::deleteController( void ) {

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

void MenuSwarmConfig::run( void ) {

  FtSwarmCommunication_t swarmCommunication;

  while (1) {
    
    fillCtrlList();
    start();

    add("Swarm Name", nvs.swarmName, MENU_DEACTIVATED, MENU_NOKEY );
    if ( myOSSwarm.Kelda ) add("Kelda", myOSSwarm.Kelda->getAliasOrName(), MENU_DEACTIVATED, MENU_NOKEY );

    if ( nvs.RS485Available() ) {    
      add( "Communication", FTSWARMCOMMUNICATION[nvs.swarmCommunication], MENU_COMMUNICATION, 'c' );
      if ( nvs.swarmCommunication != SWARMCOM_WIFI ) {
        add( "Swarm speed", nvs.swarmSpeed, MENU_SPEED, 's' );
      }
    }
    else
      add( "Communication", FTSWARMCOMMUNICATION[nvs.swarmCommunication], MENU_DEACTIVATED, MENU_NOKEY );

    add("Pin", nvs.swarmPIN, MENU_DEACTIVATED, MENU_NOKEY );
    
    printf("\n");

    printf("     Name        Status   NW-Age    Alias\n" );

    for ( int8_t i=0; i<=maxCtrl; i++ ) {
      
      if ( ctrl[i]->isOnline() ) { 
        printf("(%2d) ", i+1); 
        add( i ); 
      
      } else { 
        printf("     "); 
      }

      printf( "%10s  %-7s  [%.6lu]  %s\n", ctrl[i]->getName(), SWOSCOMSTATE[ctrl[i]->getComState()], ctrl[i]->networkAge(), ctrl[i]->getAlias() );
    }

    printf("\n\n");
    add( "create new swarm", "", MENU_NEW,    'n' );

    if (myOSSwarm.Ctrl[0]->IAmKelda) {
      add( "add a controller to my swarm", "", MENU_ADD,    '+' );
      if (myOSSwarm.maxCtrl > 0) add( "revoke a controller from my swarm", "", MENU_DELETE, '-' );
    }

    add( "set alias name", "", MENU_ALIAS,  'a' );
    addExit();
  
    int8_t choice = userChoice();

    switch( choice ) {
      case MENU_EXIT: // main
        return;

      case MENU_COMMUNICATION: 
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
        break;

      case MENU_SPEED:  nvs.swarmSpeed = enterNumber( "(0) low ... (4) highspeed (max. 50m)>", nvs.swarmSpeed, 0, 4 );
                        if ( yesNo( "To apply your changes, the device needs to be restarted.\nSave settings and restart now (Y/N)?") ) nvs.saveAndRestart();
                        break;

      case MENU_NEW:    newSwarm();
                        break;
        
      case MENU_ADD:    addController();
                        break;

      case MENU_DELETE: deleteController();
                        break;

      case MENU_ALIAS:  changeAlias();
                        break;

      default:          if (ctrl[choice]->getComState() != COMSTATE_ONLINE ) {
                          printf("\e[0;31mERROR: %s is not online.\n\e[0m\n", ctrl[choice]->getAliasOrName() );
                        } else {
                          MenuIOList MenuIOList( prompt, ctrl[choice] );
                          MenuIOList.run();
                        }
                        break;

      }

  }

}

class MainMenu : private Menu {

  private:
    static const int8_t MENU_WEB      = -1;
    static const int8_t MENU_SWARM    = -2;
    static const int8_t MENU_IOCONFIG = -3;
    static const int8_t MENU_FACTORY  = -4;
    static const int8_t MENU_REMOTE   = -5;

  public:
    MainMenu( ):Menu( NULL, "setup", "Main Menu", 14) {};
    void factorySettings( void );
    void run( void );

};

void MainMenu::factorySettings( void ) {
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

void MainMenu::run( void ) {

  while (1) {

    start( );
    add("Wifi & Local Settings", "", MENU_WEB, 'w' );
    if ( ( WiFi.status() == WL_CONNECTED ) || ( nvs.wifiMode == wifiAP ) || ( nvs.RS485Available() ) ) {
      add("Swarm Configuration", "", MENU_SWARM, 's' );
    } else {
      add("Swarm Configuration - activate WiFi", "", MENU_DEACTIVATED );
    }

    add("IO Configuration", "", MENU_IOCONFIG, 'i' );
    add("Remote/Event Configuration", "", MENU_REMOTE, 'r' );

    add("Factory Reset", "", MENU_FACTORY, 'f' );
    addExit();

    MenuLocalSettings *menuLocalSettings;
    MenuSwarmConfig   *menuSwarmConfig;
    MenuIOList        *menuIOList;
    MenuIOConfig      *menuEvent;

    switch( userChoice(  )  ) {
      case MENU_EXIT:         return;
      
      case MENU_WEB:        menuLocalSettings = new MenuLocalSettings();
                            menuLocalSettings->run( );
                            delete menuLocalSettings;
                            break;

      case MENU_SWARM:      menuSwarmConfig = new MenuSwarmConfig();
                            menuSwarmConfig->run();
                            delete menuSwarmConfig;
                            break;

      case MENU_IOCONFIG:   menuIOList = new MenuIOList();
                            menuIOList->run();
                            delete menuIOList;
                            break;

/*
      case MENU_REMOTE:     menuEvent = new MenuEvent();
                            menuEvent->run();
                            delete menuEvent;
                            break;
*/
      case MENU_REMOTE:     menuEvent = new MenuIOConfig();
                            menuEvent->run();
                            delete menuEvent;
                            break;

      case MENU_FACTORY:    factorySettings();
                            break;

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

void mainMenu( void ) {
  
  MainMenu main;
  main.run();

}