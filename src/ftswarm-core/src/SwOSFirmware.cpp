/*
 * SwOSFirmware.cpp
 *
 * Firmware menues
 * 
 * (C) 2021-23 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "esp_netif.h"
#include "esp_wifi.h"

#include "SwOSFirmware.h"
#include "SwOS.h"
#include "SwOSSwarm.h"
#include "SwOSNVS.h"
#include "easyKey.h"
#include "easyMenu.h"
#include "SwOSCLI.h"
#include "SwOSLog.h"
#include "SwOSHW/SwOSHWLocal.h"

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

  if ( ( changes ) && yesNo( TRANSLATE( "Save changes? (Y/N)?", "Änderungen speichern? (J/N)?" ) ) ) {

    // local changes
    if ( anythingChanged[0] ) {

       // save in local nvs
       myOSSwarm.Ctrl[0]->save( FTSWARM_NVSSCOPE_ALIASSERVO, SWOS_NOPORT );
       nvs.saveEvents();

       // send new config to Kelda
       if ( ( myOSSwarm.Kelda ) && ( myOSSwarm.Kelda != myOSSwarm.Ctrl[0] ) ) myOSSwarm.Ctrl[0]->sendIOConfig( myOSSwarm.Kelda->macAddr );

    }

    // remote changes
    for ( i=1; i<MAXCTRL; i++ ) {
      if ( anythingChanged[i] ) {
        myOSSwarm.Ctrl[i]->sendIOConfig( myOSSwarm.Ctrl[i]->macAddr );
        myOSSwarm.Ctrl[i]->save( FTSWARM_NVSSCOPE_ALIASSERVO, SWOS_NOPORT );
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

  bool anythingChanged = false;
  bool setPassword( void );
  void wifiMode( void );

  public:

    MenuLocalSettings( char *basePrompt = NULL ) : Menu(  basePrompt, TRANSLATE( "wifi", "WLAN" ), TRANSLATE( "Wifi & Local Settings", "WLAN & Lokale Einstellungen" ), 14 ) {};
    void run( void );

};

void MenuLocalSettings::wifiMode( void ) {

  FtSwarmWifi_t wifiMode = (FtSwarmWifi_t) enterNumber( TRANSLATE( "Enter wifi mode [ 0-off , 1-AP-Mode, 2-Client-Mode]: ", "WLAN-Modus [ 0-aus , 1-AP-Modus, 2-Client-Modus]: " ), nvs.wifi.mode, 0, 2 );

  if ( nvs.wifi.mode != wifiMode ) {
    
    nvs.wifi.mode = wifiMode;   
    if ( ( nvs.wifi.mode == wifiAP ) && ( ( nvs.wifi.channel < 1 ) || ( nvs.wifi.channel > 13 ) ) ) nvs.wifi.channel = 1; // to avoid invalid channel settings
    anythingChanged = true;

  }

}

bool MenuLocalSettings::setPassword( void ) {

  char pwd[64];

  while (1) {

    enterString( TRANSLATE( "Please enter new Password - 8-64 chars: ", "Bitte geben Sie ein neues Passwort ein - 8-64 Zeichen: " ), pwd, 64, true );

    if ( strlen( pwd ) == 0) {
      printf( TRANSLATE( "Keep old password.\n", "Altes Passwort wird beibehalten.\n" ) );
      return false;

    } else if ( strlen( pwd ) < 8 ) {
      printf( TRANSLATE( "Please use at minimum 8 chars.\n", "Bitte verwenden Sie mindestens 8 Zeichen.\n" ) );

    } else {
      strcpy( nvs.wifi.Password, pwd );
      return true;
    }

  }

  return false;

}

void MenuLocalSettings::run( void ) {

  char info[250];
  uint32_t scope = FTSWARM_NVSSCOPE_NONE;
  
  // get IP
  esp_netif_ip_info_t ip_info;
  esp_netif_t* netif = NULL;
  if      (nvs.wifi.mode == wifiAP)     netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
  else if (nvs.wifi.mode == wifiClient) netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
  if      ( netif ) esp_netif_get_ip_info( netif, &ip_info );

  while(1) {

    switch ( nvs.wifi.mode ) {
      case wifiAP:      
      case wifiClient:  sprintf(info, "hostname:            %s\nip-address:          %d.%d.%d.%d\n\n", myOSSwarm.Ctrl[0]->getHostname(), IP2STR(&ip_info.ip) );
                        break;
      default:          sprintf(info, "hostname:            %s\nip-address:          none\n\n",myOSSwarm.Ctrl[0]->getHostname());
                        break;
    }

    // build menu
    start( );
    printf(info);
    add( TRANSLATE( "wifi mode", "WLAN-Modus" ), WIFI[nvs.wifi.mode], MENU_WIFI, 'w');

    if (nvs.wifi.mode != wifiOFF ) {
      
      add( "SSID", nvs.wifi.SSID, MENU_SSID, 's');
      add( TRANSLATE( "Password", "Passwort" ), "*****", MENU_PASSWORD, 'p' );
      
      if (nvs.wifi.mode == wifiAP) {
        add( "channel", nvs.wifi.channel, MENU_CHANNEL, 'c' );
      } 
      
      add( "Web UI", ONOFF[nvs.wifi.webUI], MENU_WEBUI, 'u' );
      
      if ( ( nvs.wifi.webUI ) && ( FTSWARM_HAL_PIXELS ) ) add( TRANSLATE( "ftPixels in UI", "ftPixel in UI" ), nvs.pixels, MENU_PIXELS, 'f' );

    }
     
    if ( FTSWARM_HAL_EXT_PORT ) { 
      add( TRANSLATE( "Extension Port", "Erweiterungsport" ), EXTMODE[ nvs.extensionPort.mode] , MENU_EXT, 'e' ); 
    }

    // I2C Slave Mode. Options I2C Slave Address and Interrupt Line
    if ( nvs.extensionPort.mode == FTSWARM_EXT_I2C_SLAVE ) {
      add( TRANSLATE( "I2C Slave Address", "I2C-Slave-Adresse" ), nvs.extensionPort.I2CAddr, MENU_I2CADDR, 'a');
      add( TRANSLATE( "Interrupt Line", "Interrupt-Ausgang" ), OFFM1M2[nvs.extensionPort.interruptLine], MENU_I2CINT, 'i' );
      add( TRANSLATE( "Interrupt Low Value", "Interrupt Wert für 0" ), nvs.extensionPort.interruptOnOff[0], MENU_I2CLOW, 'l' );
      add( TRANSLATE( "Interrupt High Value", "Interrupt Wert für 1" ), nvs.extensionPort.interruptOnOff[1], MENU_I2CHIGH, 'h' );
      add( TRANSLATE( "I2C Registers", "I2C-Register" ), nvs.extensionPort.I2CRegisters, MENU_I2CREGS, 'r' );
    }

    // gyro if available
    if ( myOSSwarm.Ctrl[0]->hasGyro() ) { 
      add("Gyro", ONOFF[nvs.extensionPort.gyro], MENU_GYRO, 'g' ); 
    }

    addExit();

    if ( ( nvs.wifi.mode == wifiOFF ) && ( nvs.swarm.communication.wifi ) ) printf( TRANSLATE( "\nHINT: Check wifi settings vs. swarm communication settings\n", "\nHINWEIS: Überprüfen Sie die WLAN- und Swarm-Kommunikationseinstellungen\n" ) );

    char line[100];
    FtSwarmVersion_t cpu;

    switch ( userChoice(  ) ) {

      case MENU_EXIT:       if ( ( anythingChanged) && ( yesNo( TRANSLATE( "To apply your changes, the device needs to be restarted.\nSave settings and restart now (Y/N)?", "Um Ihre Änderungen anzuwenden, muss das Gerät neu gestartet werden.\nEinstellungen speichern und neu starten (J/N)?" ) ) ) ) {
                              // save config
                              nvs.saveAndRestart( (FtSwarmNVSScope_t) scope );
                            } else {
                              return;
                            }
        
      case MENU_WIFI:       wifiMode( );
                            scope = scope | FTSWARM_NVSSCOPE_WIFI;
                            break;
        
      case MENU_SSID:       anythingChanged = true;
                            sprintf( line, TRANSLATE( "Please enter new SSID [%s]: ", "Bitte neue SSID eingeben [%s]: " ), nvs.wifi.SSID );
                            enterString( line, nvs.wifi.SSID, nvs.wifi.SSID, 64);
                            scope = scope | FTSWARM_NVSSCOPE_WIFI;
                            break;
        
      case MENU_PASSWORD:   if ( setPassword() ) anythingChanged = true;
                            scope = scope | FTSWARM_NVSSCOPE_WIFI;
                            break;

      case MENU_CHANNEL:    anythingChanged = true;
                            nvs.wifi.channel = enterNumber( TRANSLATE( "Enter channel [1..13] - use 1,6 or 11 if possible: ", "Bitte Kanal eingeben [1..13] - wenn möglich 1, 6 oder 11 verwenden: " ), nvs.wifi.channel, 1, 13 );
                            scope = scope | FTSWARM_NVSSCOPE_WIFI;
                            break;

      case MENU_WEBUI:      anythingChanged = true;
                            nvs.wifi.webUI = !nvs.wifi.webUI;
                            scope = scope | FTSWARM_NVSSCOPE_WEBUI;
                            break;
        
      case MENU_PIXELS:     anythingChanged = true;
                            nvs.pixels = enterNumber( TRANSLATE( "Enter number of ftPixel in WebUI [2..18]: ", "Bitte Anzahl der ftPixel in der WebUI eingeben [2..18]: " ), nvs.pixels, 2, MAXLEDS );
                            scope = scope | FTSWARM_NVSSCOPE_PIXEL;
                            break;

      case MENU_GYRO:       anythingChanged = true;
                            nvs.extensionPort.gyro = !nvs.extensionPort.gyro;
                            if ( ( nvs.extensionPort.gyro ) && ( nvs.CPU != FTSWARMRS_2V1 ) ) nvs.extensionPort.mode = FTSWARM_EXT_I2C_MASTER;
                            scope = scope | FTSWARM_NVSSCOPE_EXTPORT;
                            break;

      case MENU_I2CADDR:    anythingChanged = true;
                            nvs.extensionPort.I2CAddr = (uint8_t) enterNumber( "[16..127]: ", nvs.extensionPort.I2CAddr, 16, 127 );
                            scope = scope | FTSWARM_NVSSCOPE_EXTPORT;
                            break;

      case MENU_I2CHIGH:    anythingChanged = true;
                            nvs.extensionPort.interruptOnOff[1] = (int16_t) enterNumber( TRANSLATE( "High Value [-255..255]", "Wert für 1 [-255..255]" ), nvs.extensionPort.interruptOnOff[1], -255, 255 );
                            scope = scope | FTSWARM_NVSSCOPE_EXTPORT;
                            break;

      case MENU_I2CLOW:     anythingChanged = true;
                            nvs.extensionPort.interruptOnOff[0] = (int16_t) enterNumber( TRANSLATE( "Low Value [-255..255]: ", "Wert für 0 [-255..255]: " ), nvs.extensionPort.interruptOnOff[0], -255, 255 );
                            scope = scope | FTSWARM_NVSSCOPE_EXTPORT;
                            break;

      case MENU_I2CINT:     anythingChanged = true;
                            nvs.extensionPort.interruptLine = (uint8_t) enterNumber( TRANSLATE( "motor (1 for M1, 2 for M2, ...) or 0 to skip: ", "Motor (1 für M1, 2 für M2, ...) oder 0 zum Überspringen: " ), nvs.extensionPort.interruptLine, 0, FTSWARM_HAL_MOTORS );
                            scope = scope | FTSWARM_NVSSCOPE_EXTPORT;
                            break;

      case MENU_I2CREGS:    anythingChanged = true;
                            nvs.extensionPort.I2CRegisters = (uint8_t) enterNumber( TRANSLATE( "I2C Registers [1..8]:", "I2C-Register [1..8]: " ), nvs.extensionPort.I2CRegisters, 1, MAXI2CREGISTERS);
                            scope = scope | FTSWARM_NVSSCOPE_EXTPORT;
                            break;

      case MENU_EXT:        anythingChanged = true;
                            
                            cpu = myOSSwarm.Ctrl[0]->getCPU();

                            if ( ( cpu == FTSWARMCONTROL_1V3 ) || ( cpu == FTSWARMCONTROL_1V3UC ) ) {

                              if ( nvs.extensionPort.mode == FTSWARM_EXT_OFF ) 
                                nvs.extensionPort.mode = FTSWARM_EXT_I2C_MASTER;
                              else 
                                nvs.extensionPort.mode = FTSWARM_EXT_OFF;

                            } else if ( ( cpu == FTSWARMJST_1V15 ) || ( cpu == FTSWARMRS_2V1 ) || ( cpu == FTSWARMXL_1V00 ) ) {
                              nvs.extensionPort.mode = (FtSwarmExtMode_t) enterNumber( TRANSLATE( "Extension Port Mode: OFF (0), I2C-MASTER (1), I2C-SLAVE (2), OUTPUT (3), SERVO (4), LIDA (5): ", "Modus Extension Port: aus (0), I2C_MASTER (1), I2C_SLAVE (2), OUTPUT (3), SERVO (4), LIDA (5): "), nvs.extensionPort.mode, 0, FTSWARM_EXT_LIDAR );
                            
                            }
                            
                            scope = scope | FTSWARM_NVSSCOPE_EXTPORT;
                            
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
    static const int8_t MENU_DELALL   = -6;
    static const int8_t MENU_CFG      = -7;
    static const int8_t MENU_PREVIOUS = -8;
    static const int8_t MENU_NEXT     = -9;
    static const int8_t MENU_OFFSET   = -10;
    static const int8_t MENU_CALIBRATE = -11;

    SwOSIO* io;
    bool    selfSave = false;
    uint8_t event[MENUITEMSPERPAGE + 1];
    int8_t  maxEvent = -1;

    int     pageOffset = 0;
    bool    morePages  = false;

    void changeType( void );
    void changeAlias( void );
    void changeLabel( void );

    void fillEventList( void );
    bool enterIO( const char* prompt, SwOSIOUID *uio, bool input );

    uint8_t enterBlinkColor( const char *text, uint8_t color );
    void enterBlinkEffect( SwOSIO* actor, FtSwarmTriggerParameter *parameter );
    
    void enterConstant( SwOSIO* actor, FtSwarmTriggerParameter *parameter );
    bool enterEvent( SwOSNVSEvent *event );

    bool changeEvent( SwOSNVSEvent *event );

    void addEvent( void );
    void deleteEvent( void );
    void deleteAllEvents( void );

    void printConstant( SwOSIO *actor, FtSwarmTriggerParameter parameter );
    void printEventParameter( FtSwarmOperand_t op, SwOSIO *sensor, SwOSIO *actor, char *doing, FtSwarmTriggerParameter parameter );
    void printEvent( SwOSNVSEvent event, uint8_t details = 0 );

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
  else    begin( basePrompt, TRANSLATE( "Remote Configuration", "Remote Konfiguration" ), TRANSLATE( "Remote Configuration", "Remote Konfiguration" ), 10, ':' );

}

void MenuIOConfig::fillEventList( void ) {

  uint8_t item = 0;
  maxEvent = -1;
  morePages = false;

  for (uint8_t i=0; i<MAXNVSEVENTS; i++) {

    // end of list?
    if ( nvs.events.events[nvs.events.activeConfig][i].sensor.serialNumber == 0) break;

    // io not specified OR event is about my io
    if ( (!io) || 
         ( io == myOSSwarm.getIO( nvs.events.events[nvs.events.activeConfig][i].sensor ) ) ||
         ( io == myOSSwarm.getIO( nvs.events.events[nvs.events.activeConfig][i].actor  ) ) 
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

void MenuIOConfig::changeLabel( void ) {

  char prompt[250];
  char text[4];

  SwOSLabel_t label = io->getLabel();
  
  // ask user for new label
  sprintf( prompt, TRANSLATE( "Please enter new label [%s]: ", "Neues Label [%s]: " ), nvs.events.oledLabel[nvs.events.activeConfig][label] );
  if ( ( label == SWOSLABEL_J1 ) || ( label == SWOSLABEL_J2 ) ) enterString( prompt, text, 3 );
  else enterString( prompt, text, 4 );

  // nothing changed
  if ( strcmp( text, nvs.events.oledLabel[nvs.events.activeConfig][label] ) == 0 ) return;

  strcpy( nvs.events.oledLabel[nvs.events.activeConfig][label], text );

  io->setLabelText( text );

  anythingChanged[ myOSSwarm.getIndex( io->getCtrl()->serialNumber ) ] = true;

}

void MenuIOConfig::changeAlias( void ) {
  
  char prompt[250];
  char alias[MAXIDENTIFIER];
  
  // ask user for new alias
  sprintf( prompt, TRANSLATE( "%s - please enter new alias: ", "%s - Aliasname: " ), io->getAliasOrName() );
  enterIdentifier( prompt, alias, MAXIDENTIFIER );

  // nothing changed
  if ( strcmp( alias, io->getAlias() ) == 0 ) return;
                
  // test on duplicates
  SwOSIO *testIO = myOSSwarm.getIO( alias );
  if ( (testIO) && ( testIO != io ) ) {
    printf( TRANSLATE( "ERROR: This alias is already used in the swarm.\n", "FEHLER: Dieser Alias wird bereits im Schwarm verwendet.\n" ) );
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
    printf( TRANSLATE( "ERROR: IO type %s could not be changed to another IO type.\n\n", "FEHLER: IO-Typ %s kann nicht in einen anderen IO-Typ geändert werden.\n\n" ), SWOSIOTYPE[ioType] );
    return;
  }
  
  // list compatible types and ask user
  int8_t       maxType = -1;
  SwOSIOType_t defaultValue, type[SWOSIO_MAXIOTYPE];

  for (uint8_t i=0; i<SWOSIO_MAXIOTYPE; i++) {

    // compatible type?
    if ( ( SWOSIOCLASS[ioType] == SWOSIOCLASS[i] ) &&
         ( ! ( ( FTSWARM_HAL_RCSERVOS == 0 ) && ( (SwOSIOType_t)i == SWOSIO_RCSERVO ) ) )
       ) {

      maxType++;

      // default?
      if ( ioType == (SwOSIOType_t) i ) defaultValue = (SwOSIOType_t)i;

      // menu entry
      type[maxType] = (SwOSIOType_t) i;
      printf( "(%2d) %s\n", maxType, SWOSIOTYPE[i] );

    }

  }

  sprintf( prompt, TRANSLATE( "Choose new IO Type [%s]:", "Neuer IO-Typ [%s]: " ), SWOSIOTYPE[defaultValue] );
  SwOSIOType_t newIOType = type[enterNumber( prompt, defaultValue, 0, maxType )];

  // change type?
  if ( ioType != newIOType ) { 

    // need my controller
    SwOSCtrl *ctrl = io->getCtrl();

    // my index within the io list
    uint8_t index = ctrl->getIndex(io);

    // change it
    if ( ctrl->changeIOType( ctrl->getIndex(io), newIOType, io->getFlags() ) ) {

      // since I changed my type, io was deleted. Need to refresh io.
      io = ctrl->io[index];

      // yes I did it
      anythingChanged[ myOSSwarm.getIndex( io->getCtrl()->serialNumber ) ] = true;

    }

  }

}

bool MenuIOConfig::enterIO( const char* prompt, SwOSIOUID *uio, bool input ) {

  char   alias[MAXIDENTIFIER] = "";
  SwOSIO *io;

  // default
  io = myOSSwarm.getIO( *uio );
  if (io) strcpy( alias, io->getAliasOrName() );
  
  while (1) {

    enterString( prompt, alias, alias, sizeof(alias) );

    // get IO via alias or controller.ioname
    io = myOSSwarm.getIO( alias );

    // error handling;
    if      ( alias[0] == '\0' )                 return false; // no entry
    else if (!io)                                printf( TRANSLATE( "Error: %s doesn't exists.\n", "Fehler: %s existiert nicht.\n" ), alias ); 
    else if ( ( input  ) && ( !io->isInput() ) ) printf( TRANSLATE( "Error: %s is not a sensor\n", "Fehler: %s ist kein Sensor\n" ), alias );
    else if ( ( !input ) && ( !io->isActor() ) ) printf( TRANSLATE( "Error: %s is not a actor\n", "Fehler: %s ist kein Aktor\n" ), alias );
    else break; // all good
    
  }

  uio->serialNumber = io->getCtrl()->serialNumber;
  uio->ioType       = io->getIOType();
  uio->port         = io->getPort();

  return true;

}

static char FTSWARM_EFFECT_COLOR[FTSWARM_EFFECT_COLOR_MAX][8] = { "Black", "Red", "Green", "Blue", "Yellow", "Orange", "Cyan", "Pink", "Magenta", "White" };

uint8_t MenuIOConfig::enterBlinkColor( const char *text, uint8_t c ) {

  char prompt[250];

  strcpy( prompt, text );
  
  for (uint8_t i=0; i<FTSWARM_EFFECT_COLOR_MAX; i++ ) sprintf( prompt, "%s (%d) %s ", prompt, i, FTSWARM_EFFECT_COLOR[i] );

  sprintf( prompt, "%s [%s]: ", prompt, FTSWARM_EFFECT_COLOR[c] );
  return enterNumber( prompt, c, 0, FTSWARM_EFFECT_COLOR_MAX-1 );

}

void MenuIOConfig::enterBlinkEffect( SwOSIO* actor, FtSwarmTriggerParameter *parameter ) {

  char prompt[250];

  // let's blink
  parameter->base.effectType = FTSWARM_EFFECT_BLINK;
                            
  sprintf( prompt, TRANSLATE( "Enter the duration of one beat (0.25s .. 7.00s in 0.25 steps) [%d]: ", "Takt in 1/10s (0..31) [%0.2f]: "), parameter->getPeriod() / 1000 );
  parameter->setPeriod( enterNumberF( prompt, parameter->blink.period, 0, 31 ) * 1000 );

  sprintf( prompt, TRANSLATE( "Enter number of signal beats (0..15) [%d]: ", "Anzahl Signaltakte (0..15) [%d]: "), parameter->blink.signal );
  parameter->blink.signal = enterNumber( prompt, parameter->blink.signal, 0, 15 );

  sprintf( prompt, TRANSLATE( "Enter signal duty (0) 25/75 (1) 50/50 (2) 75/25 [%d]: ", "Pulsbreite (0) 25/75 (1) 50/50 (2) 75/25 [%d]: "), parameter->blink.duty );
  parameter->blink.duty = enterNumber( prompt, parameter->blink.duty, 0, 2 );

  sprintf( prompt, TRANSLATE( "Enter number of pause beats (0..15) [%d]: ", "Anzahl der Pausentakte (0..15) [%d]: "), parameter->blink.pause );
  parameter->blink.pause = enterNumber( prompt, parameter->blink.pause, 0, 15 );

  if ( actor->isPixel() ) {

    parameter->blink.p1 = enterBlinkColor( TRANSLATE( "Enter signal color","Signalfarbe"),  parameter->blink.p1 );
    parameter->blink.p2 = enterBlinkColor( TRANSLATE( "Enter signal pause color","Signal Pausenfarbe"), parameter->blink.p2 );
    parameter->blink.p3 = enterBlinkColor( TRANSLATE( "Enter pause color","Pausenfarbe"),             parameter->blink.p3 );

  } else if ( actor->isLamp() ) {

    parameter->blink.p1 = enterBlinkColor( TRANSLATE( "Enter signal brightness","Signal Brightness"),  parameter->blink.p1 );
    parameter->blink.p2 = enterBlinkColor( TRANSLATE( "Enter signal pause brightness","Signalpause Brightness"), parameter->blink.p2 );
    parameter->blink.p3 = enterBlinkColor( TRANSLATE( "Enter pause brightness","Pause Brightness"),             parameter->blink.p3 );

  } else SWARM_LOG_FATAL( "Please report bug: not a pixel nor a lamp." );

}

void MenuIOConfig::enterConstant( SwOSIO *actor, FtSwarmTriggerParameter *parameter ) {

  char prompt[250];
  
  if ( actor->isPixel() ) {

    // just enter a fixed color?
    sprintf( prompt, TRANSLATE( "Enter RGB value (0..#FFFFFF) [#%06X]: ", "RGB-Wert (0..#FFFFFF) [#%06X]: "), parameter->getValue() );
    parameter->setValue( enterNumber( prompt, parameter->getValue(), 0, 0xFFFFFF ) );
    return;

  }

  if ( actor->isServo() ) {
    sprintf( prompt, TRANSLATE( "Enter position (-45..45) [%d]: ", "Servo Position (-45..45) [%d]: "), parameter->getValue() );
    parameter->setValue( enterNumber( prompt, parameter->getValue(), -45, 45 ) );
    return;
  }

  if ( actor->isStepper() ) {
    sprintf( prompt, TRANSLATE( "Enter speed (-4096..4096) [%d]: ", "Geschwindigkeit (-4096..4096) [%d]: "), parameter->getValue() );
    parameter->setValue( enterNumber( prompt, parameter->getValue(), -4096, 4096 ) );
    return;
  }

  if ( actor->isMotor() ) {
    sprintf( prompt, TRANSLATE( "Enter speed (-100..100) [%d]: ", "Geschwindigkeit (-100..100) [%d]: "), parameter->getValue() );
    parameter->setValue( enterNumber( prompt, parameter->getValue(), -100, 100 ) );
    return;
  }

  if ( actor->isLamp() ) {

    // just enter a fixed brightness?
    sprintf( prompt, TRANSLATE( "Enter brightness (0..100) [%d]: ", "Helligkeit (0..100) [%d]: "), parameter->getValue() );
    parameter->setValue( enterNumber( prompt, parameter->getValue(), 0, 100 ) );
    return;
    
  }

  // catchup all other
  sprintf( prompt, TRANSLATE( "Enter constant value [%d]: ", "Geben Sie die Konstante ein [%d]: "), parameter->getValue() );
  parameter->setValue( enterNumber( prompt, parameter->getValue(), -1 * 0xFFFFF, 0xFFFFF ) );
  
}

bool MenuIOConfig::enterEvent( SwOSNVSEvent *event ) {

  char prompt[250];
  SwOSIO *eventIO;

  // *** sensor ***

  if ( (io) && ( io->isInput() ) ) {

    printf("sensor's name: %s\n", io->getAliasOrName() );
    io->getUID( &event->sensor );

  } else {

    eventIO = myOSSwarm.getIO( event->sensor );
    
    if (eventIO) sprintf( prompt, TRANSLATE( "Enter sensor's name [%s]: ", "Namen des Sensors [%s]: " ), eventIO->getAliasOrName() );
    else         sprintf( prompt, TRANSLATE( "Enter sensor's name: ", "Namen des Sensors: " ) );
    
    if (!enterIO( prompt, &event->sensor, true ) ) return false;

  }

  // *** trigger ***

  if ( !myOSSwarm.getIO( event->sensor )->isDigitalInput() ) {
    printf( TRANSLATE( "Enter trigger event: change value.\n", "Trigger Event: change value.\n" ) );
    event->triggerMath.bits.trigger = FTSWARM_TRIGGERVALUE;
  
  } else {
    FtSwarmTrigger_t trigger = event->triggerMath.bits.trigger;
    sprintf( prompt, TRANSLATE( "Enter trigger event - (0) trigger down  (1) trigger up  (2) change value [%d]: ", "Trigger Event - (0) Trigger down  (1) Trigger up  (2) Change Value [%d]: " ), trigger );
    event->triggerMath.bits.trigger = (FtSwarmTrigger_t) enterNumber( prompt, trigger, 0, 2 );
    printEvent( *event, 1 );
  }

  // *** actor ***

  if ( (io) && ( io->isActor() ) ) {

    printf( TRANSLATE( "actors's name: %s\n", "Name des Aktors: %s\n" ), io->getAliasOrName() );
    io->getUID( &event->actor );

  } else {

    eventIO = myOSSwarm.getIO( event->actor );
    
    if (eventIO) sprintf( prompt, TRANSLATE( "Enter actor's name [%s]: ", "Namen des Aktors [%s]: " ), eventIO->getAliasOrName() );
    else         sprintf( prompt, TRANSLATE( "Enter actor's name: ", "Name des Aktors: " ) );
  
    if (!enterIO( prompt,  &event->actor,  false ) ) return false;

  }

  printEvent( *event, 2 );

  // **** operand v1 ****

  SwOSIO *actor = myOSSwarm.getIO(event->actor);
  
  uint8_t v1 = event->triggerMath.bits.v1;
  uint8_t maxVal = 3;
  
  strcpy( prompt, TRANSLATE( "Use - (0) fixed value  (1) sensor's value (2) sensor's delta (3) actor's value", "(0) Konstante (1) Sensor Messwert (2) Sensor Delta (3) Aktor Stellwert" ) );

  // add blink effect?
  if ( ( actor->isPixel() ) || ( actor->isLamp() ) ) { 

    // add option to prompt
    sprintf( prompt, "%s %s", prompt, TRANSLATE( "(4) blink effect", "(4) Blinkeffekt") );
    maxVal = 4;

    // blink effect was already chosen
    if ( event->parameter.getEffectType() == FTSWARM_EFFECT_BLINK ) v1 = 4;
  }
  
  sprintf( prompt, "%s [%d]: ", prompt, v1 );
  event->triggerMath.bits.v1 = (FtSwarmOperand_t) enterNumber( prompt, v1, 0, maxVal );

  // constant?
  switch ( event->triggerMath.bits.v1 ) {

    case FTSWARM_CONSTANT:  enterConstant( actor, &event->parameter ); 
                            break;

    case 4:                 // additional option blink effect
                            event->triggerMath.bits.v1 = FTSWARM_CONSTANT;
                            enterBlinkEffect( actor, &event->parameter ); 
                            printEvent( *event );
                            return true;

  }

  printEvent( *event, 3 );

  // operator
  sprintf( prompt, TRANSLATE( "(1) add (2) subtract (3) multiply another value - (0) done [%d]: ", "(1) Addieren oder  (2) Subtrahieren oder (3) Multiplizieren mit einem anderen Wert - (0) Fertig [%d]: " ), event->triggerMath.bits.op );
  event->triggerMath.bits.op = (FtSwarmOperator_t) enterNumber( prompt, event->triggerMath.bits.op, 0, 3 );
  
  printEvent( *event, 4 );

  // V2
  if ( event->triggerMath.bits.op != FTSWARM_ASSIGN ) {

    if ( event->triggerMath.bits.v1==FTSWARM_CONSTANT ) {
      
      if ( event->triggerMath.bits.v2 == FTSWARM_CONSTANT ) event->triggerMath.bits.v2 = FTSWARM_SENSORVALUE;
      
      sprintf( prompt, TRANSLATE( "Use - (1) sensor's value (2) sensor's delta (3) actor's value [%d]: ", "Geben Sie den Wert ein - (1) Sensor (2) Sensor Delta (3) Aktor [%d]: " ), event->triggerMath.bits.v2 );
      event->triggerMath.bits.v2 = (FtSwarmOperand_t) enterNumber( prompt, event->triggerMath.bits.v2, 1, 3 );

    } else {
      
      sprintf( prompt, TRANSLATE( "Use - (0) fixed value  (1) sensor's value (2) sensor's delta (3) actor's value [%d]: ", "(0) Konstante (1) Sensor (2) Sensor Delta (3) Aktor [%d]: " ), event->triggerMath.bits.v2 );
      event->triggerMath.bits.v2 = (FtSwarmOperand_t) enterNumber( prompt, event->triggerMath.bits.v2, 0, 3 );

    }

    // constant?
    if ( event->triggerMath.bits.v2 == FTSWARM_CONSTANT ) enterConstant( actor, &event->parameter );

    printEvent( *event );

  }

  return true;

}

bool MenuIOConfig::changeEvent( SwOSNVSEvent *event ) {
 
  if ( event->sensor.serialNumber != 0 ) {

    SwOSIO *sensor = dynamic_cast<SwOSIO*>( myOSSwarm.getIO( event->sensor ) );
    SwOSIO *actor  = dynamic_cast<SwOSIO*>( myOSSwarm.getIO( event->actor ) );

    if (!sensor) { printf( TRANSLATE( "Error: ftSwarm%d is offline.\n", "Fehler: ftSwarm%d ist offline.\n" ), event->sensor.serialNumber); return false; }
    if (!actor)  { printf( TRANSLATE( "Error: ftSwarm%d is offline.\n", "Fehler: ftSwarm%d ist offline.\n" ), event->actor.serialNumber);  return false; }

  }

  // create a copy of the event
  SwOSNVSEvent newEvent = *event;

  // ask user
  if ( !enterEvent( &newEvent ) ) return false;

  // nothing changed?
  if ( newEvent.cmp( event ) ) return false;

  // duplicates?
  if ( nvs.exists( &newEvent ) ) { printf( TRANSLATE( "ERROR: This event already exists.", "FEHLER: Dieses Event gibt es bereits." ) ); return false; }

  // change event
  myOSSwarm.deleteEvent( event );
  myOSSwarm.addEvent( &newEvent );
  memcpy( event, &newEvent, sizeof(SwOSNVSEvent) );
  
  // events are stored locally only
  anythingChanged[0] = true;

  return true;

}

void MenuIOConfig::addEvent( void ) {

  for ( uint8_t i=0; i<MAXNVSEVENTS; i++ ) {

    // free space found?
    if ( nvs.events.events[nvs.events.activeConfig][i].sensor.serialNumber == 0) {

      changeEvent( &nvs.events.events[nvs.events.activeConfig][i] );

      return;

    }

  }

}

void MenuIOConfig::deleteEvent( void ) {

  char prompt[255];
  sprintf( prompt, TRANSLATE( "Which event should be deleted? [0 - abort, 1..%d]:", "Welches Event soll gelöscht werden? [0 - Abbruch, 1..%d]:" ), maxEvent+1 );
  uint8_t selected = enterNumber( prompt, 0, 0, maxEvent+1 );

  // abort
  if ( selected==0 ) return;

  // adjust selected to event list index
  selected = selected -1;

  // delete event
  myOSSwarm.deleteEvent( &nvs.events.events[nvs.events.activeConfig][event[selected]] );
  nvs.deleteEvent( nvs.events.activeConfig, selected );

  // events are stored locally only
  anythingChanged[0] = true;

}

void MenuIOConfig::deleteAllEvents( void ) {

  // security question
  if (!yesNo( TRANSLATE( "Do you want to clear all events [Y/N]?", "Möchten Sie alle Events löschen? [J/N]" ), false ) ) return;

  // delete all events
  myOSSwarm.deleteEvents();
  nvs.deleteAllEvents( nvs.events.activeConfig );

  // events are stored locally only
  anythingChanged[0] = true;

}

void MenuIOConfig::printConstant( SwOSIO *actor, FtSwarmTriggerParameter parameter ) {

  switch ( parameter.base.effectType ) {

    case FTSWARM_EFFECT_BLINK:  printf( TRANSLATE( "Blink( period: %d ms, signal beats: %d, duty: %d, pause beats: %d, ", 
                                                   "Blink( period: %d ms, Signaltakte: %d, Pulsbreite: %d, Pausentakte: %d, "), 
                                        parameter.getPeriod(), 
                                        parameter.blink.signal,
                                        parameter.blink.duty,
                                        parameter.blink.pause 
                                      );
    
                                if ( actor->isPixel() ) printf( TRANSLATE( "signal color %s, signal pause color: %s, pause color: %s )", 
                                                                          "Signalfarbe %s, Signal Pausenfarbe %s, Pausenfarbe %s )" ),
                                                                FTSWARM_EFFECT_COLOR[parameter.blink.p1],
                                                                FTSWARM_EFFECT_COLOR[parameter.blink.p2],
                                                                FTSWARM_EFFECT_COLOR[parameter.blink.p3]
                                                              );
                                if ( actor->isLamp() )  printf( TRANSLATE( "signal brightness %d, signal pause brightness %d, pause brightness %d )", 
                                                                           "Signal Brightness %d, Signal Pause Brightness %d, Pause Brightness %d )" ),
                                                                parameter.blink.p1,
                                                                parameter.blink.p2,
                                                                parameter.blink.p3
                                                              );
                                break;

    case FTSWARM_EFFECT_NONE:   if ( actor->isPixel() ) printf( "#%06X", parameter.getValue() ); 
                                else                    printf( "%d", parameter.getValue() ); 

  }

}

void MenuIOConfig::printEventParameter( FtSwarmOperand_t op, SwOSIO *sensor, SwOSIO *actor, char *doing, FtSwarmTriggerParameter parameter ) {

  char uniqueName[2*MAXIDENTIFIER+1];
  
  switch ( op ) {
    case FTSWARM_CONSTANT:    printConstant( actor, parameter );
                              break;

    case FTSWARM_SENSORVALUE: sensor->getUniqueName( uniqueName );
                              printf( "%s.getValue()", uniqueName ); 
                              break;

    case FTSWARM_SENSORDELTA: sensor->getUniqueName( uniqueName );
                              printf( "%s.getDelta()", uniqueName ); 
                              break;

    case FTSWARM_ACTORVALUE:  actor->getUniqueName( uniqueName );
                              printf( "%s.get%s()", uniqueName, doing );
                              break;
  }

}

void MenuIOConfig::printEvent( SwOSNVSEvent event, uint8_t details ) {

  SwOSIO *sensor = dynamic_cast<SwOSIO*>( myOSSwarm.getIO( event.sensor ) );
  SwOSIO *actor  = dynamic_cast<SwOSIO*>( myOSSwarm.getIO( event.actor ) );

  if (details) printf("\n");

  char uniqueName[2*MAXIDENTIFIER+1];

  // print sensor
  if (!sensor) { 
    printf("ftSwarm%d.???", event.sensor.serialNumber ); 

  } else {
    sensor->getUniqueName( uniqueName );
    printf("%s.%s",  uniqueName, FTSWARMTRIGGER[ event.triggerMath.bits.trigger ] );
    if (details == 1 ) { printf("\n"); return; }
  }

  // print actor
  if (!actor) { 
    printf("-> ftSwarm%d.???\n", event.actor.serialNumber ); 

  } else {
    char doing[15];
    if      ( actor->isMotor() ) strcpy( doing, "Speed");
    else if ( actor->isServo() ) strcpy( doing, "Position");
    else if ( actor->isPixel() ) strcpy( doing, "Color" );
    else                         strcpy( doing, "???" );

    actor->getUniqueName( uniqueName );
    printf(" -> %s.set%s( ", uniqueName, doing );
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

}

void MenuIOConfig::changeConfig( void ) {

  char line[80];
  uint8_t newConfig;

  sprintf( line, TRANSLATE( "Switch to configuration [1..%d]", "Neue Konfiguration [1..%d]" ), MAXEVENTCONFIGS );
  newConfig = enterNumber( line, nvs.events.activeConfig+1, 1, MAXEVENTCONFIGS ) -1;
  
  if ( newConfig != nvs.events.activeConfig ) {
      anythingChanged[0] = true;
      nvs.events.activeConfig = newConfig;
      myOSSwarm.deleteEvents();
      myOSSwarm.addEvents( newConfig );
  }

}

void MenuIOConfig::run( void ) {

  while (1) {

    start();
    fillEventList();

    if (io) {
    
      add( TRANSLATE("name", "Name"), io->getName(), MENU_DEACTIVATED, MENU_NOKEY );
      add( TRANSLATE("IO type", "IO-Typ"), SWOSIOTYPE[ io->getIOType() ], MENU_TYPE, 't' );
      add( TRANSLATE("alias", "Alias"),   io->getAlias(), MENU_ALIAS, 'a' );

      if ( io->isServo() ) {
        add( TRANSLATE("offset", "Offset"), ((SwOSServo *)io)->getOffset(), MENU_OFFSET, 'o' );
      }

      if ( io->getIOType() == SWOSIO_RCSERVO ) {
        add( TRANSLATE("calibrate", "Kalibrieren"), "", MENU_CALIBRATE, 'c' );
      }
      // test on label
      SwOSLabel_t label = io->getLabel();
      if ( (label != SWOSLABEL_UNDEF ) && ( label < SWOSLABEL_MAX ) ) add( TRANSLATE("label", "Label"), nvs.events.oledLabel[nvs.events.activeConfig][label], MENU_LABEL, 'l' );

      if ( maxEvent >= 0 ) printf("\n     Events:\n");

    }

    for (uint8_t i=0; i<=maxEvent; i++) {

      printf("(%2d) ", i+1 );
      printEvent( nvs.events.events[nvs.events.activeConfig][event[i]] );

      add( i );

    }

    printf("\n");

    if (morePages)      add( TRANSLATE("next page", "Nächste Seite"), "", MENU_NEXT, '>' );
    if (pageOffset > 0) add( TRANSLATE("previous page", "Vorherige Seite"), "", MENU_PREVIOUS, '<' );
    if ( ( morePages ) || (pageOffset > 0) ) printf("\n");

    if ( maxEvent < MAXNVSEVENTS ) add( TRANSLATE("add event", "neues Event"), "", MENU_ADD, '+' );
    if ( maxEvent >= 0           ) {
      add( TRANSLATE("delete one event", "Event löschen"), "", MENU_DEL,   '-' );
      add( TRANSLATE("delete all events", "alle Events löschen"), "", MENU_DELALL, '*' );
    }

    add( TRANSLATE( "switch configuration", "Konfiguration wechseln"), "", MENU_CFG, 's' );
    
    addExit();

    int8_t choice = userChoice();

    switch ( choice ) {

      case MENU_EXIT:     if ( selfSave ) save();
                          return;

      case MENU_ALIAS:    changeAlias( );
                          break;

      case MENU_TYPE:     changeType( );
                          break;

      case MENU_LABEL:    changeLabel( );
                          break;

      case MENU_OFFSET:   ((SwOSServo*)io)->setOffset( enterNumber( TRANSLATE( "new offset [0..90]", "neuer Offset [0..90]" ), ((SwOSServo*)io)->getOffset(), 0, 90 ) );
                          anythingChanged[0] = true;
                          break;

      case MENU_CALIBRATE: ((SwOSRCServo*)io)->calibrate( 100 );
                          break;

      case MENU_ADD:      printf("\n" ); 
                          addEvent( );
                          break;

      case MENU_DEL:      printf("\n");
                          deleteEvent( );
                          break;

      case MENU_DELALL:   printf("\n");
                          deleteAllEvents( );
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
                          changeEvent( &nvs.events.events[nvs.events.activeConfig][event[choice]] );
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

  const char ioconfig[] = TRANSLATE("IO configuration", "IO-Konfiguration");

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
  pageOffset = 0;

  // all controllers
  for ( int8_t c=0; c<=myOSSwarm.maxCtrl; c++ ) {
    
    // check, if the controller needs to be listed
    if ( ( myOSSwarm.Ctrl[c] ) && ( ( !controller ) || ( myOSSwarm.Ctrl[c] == controller ) ) ) {

      // all IOs
      for (uint8_t i=0; i<myOSSwarm.Ctrl[c]->IOs; i++ ) {

        if ( ( myOSSwarm.Ctrl[c]->io[i] ) && 
             ( !myOSSwarm.Ctrl[c]->io[i]->testFlag( FTSWARM_HAL_FLAG_HIDDEN ) ) &&
             ( ( listInputs && myOSSwarm.Ctrl[c]->io[i]->isInput() || ( myOSSwarm.Ctrl[c]->io[i]->getIOType() == SWOSIO_JOYSTICK ) ) ||
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
  
    printf(TRANSLATE("     Name               Type            Events Alias\n", "     Name               Typ             Events Alias\n"));

    // list IOs
    char data[80];
    char name[MAXIDENTIFIER+10];

    // list IOs
    for (int8_t i=0; i<=maxItem; i++ ) {
         
      // different ways to print the IO name
      if ( io[i]->getCtrl()->isLocal() ) sprintf( name, "%s",    io[i]->getName() );
      else                               sprintf( name, "%s.%s", io[i]->getCtrl()->getName(), io[i]->getName() );

      // count events
      uint8_t events=0;
      for (uint8_t e=0; i<MAXNVSEVENTS; e++) {

        // end of list?
        if ( nvs.events.events[nvs.events.activeConfig][e].sensor.serialNumber == 0) break;

        // my event?
        if ( ( io[i] == myOSSwarm.getIO( nvs.events.events[nvs.events.activeConfig][e].sensor ) ) ||
             ( io[i] == myOSSwarm.getIO( nvs.events.events[nvs.events.activeConfig][e].actor  ) ) 
           ) 
          events++;

      }

      sprintf( data, "%-18s %-15s %-3d    %s", name, SWOSIOTYPE[ io[i]->getIOType() ], events, io[i]->getAlias() );

      add( data, "", i+1, '\0', true );

    }

    printf("\n");

    if (morePages)      add( TRANSLATE("next page", "Nächste Seite"), "", MENU_NEXT, '>' );
    if (pageOffset > 0) add( TRANSLATE("previous page", "Vorherige Seite"), "", MENU_PREVIOUS, '<' );
    if ( ( morePages ) || (pageOffset > 0) ) printf("\n");

    if (!listInputs) add( TRANSLATE("show inputs", "Eingänge"), "", MENU_INPUT, 'i' );
    if (!listActors) add( TRANSLATE("show actors", "Aktoren"), "", MENU_ACTOR, 'a' );
    if (!listPixels) add( TRANSLATE("show pixels", "Pixel/LEDs"), "", MENU_PIXEL, 'p' );

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
    static const int8_t MENU_NEW       = -1;
    static const int8_t MENU_ADD       = -2;
    static const int8_t MENU_DELETE    = -3;
    static const int8_t MENU_COM_WIFI  = -4;
    static const int8_t MENU_COM_RS485 = -5;
    static const int8_t MENU_COM_BT    = -6;
    static const int8_t MENU_SPEED     = -7;
    static const int8_t MENU_PIN       = -8;
    static const int8_t MENU_ALIAS     = -9;
    
    SwOSCtrl *ctrl[MAXCTRL];
    int8_t   maxCtrl = -1;
    bool     communicationChanges = false;

    void fillCtrlList( void );
    void changeAlias( void );
    void newSwarm( void );
    void addController( void );
    void deleteController( void );

  public:
    MenuSwarmConfig( char *basePrompt = NULL );
    void run( void );
};

MenuSwarmConfig::MenuSwarmConfig( char *basePrompt ):Menu( basePrompt, TRANSLATE("Swarm Configuration", "Swarm-Konfiguration"), TRANSLATE("Swarm Configuration", "Swarm-Konfiguration"), MAXCTRL + 10 ) {
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
  sprintf( prompt, TRANSLATE("Please select the controller to be changed [1..%d]:", "Controller [1..%d]:"), maxCtrl+1 );
  int8_t selected = enterNumber( prompt, 0, 1, maxCtrl+1 ) -1;

  // nothing selected
  if (selected < 0 ) return;

  // controlle isn't online
  if (ctrl[selected]->getComState() != COMSTATE_ONLINE ) {
    printf( TRANSLATE("ERROR: %s is not online.\n", "FEHLER: %s ist nicht online.\n"), ctrl[selected]->getAliasOrName() );
    return;
  }
  
  // ask user for new alias
  sprintf( prompt, TRANSLATE("%s - please enter new alias: ", "%s - bitte geben Sie den neuen Alias Namen ein: "), ctrl[selected]->getAliasOrName() );
  enterIdentifier( prompt, alias, MAXIDENTIFIER );

  // no changes
  if ( strcmp( alias, ctrl[selected]->getAliasOrName() ) == 0 ) return;
                
  // test on duplicates
  SwOSCtrl *testCtrl = myOSSwarm.getController( alias );
  if ( (testCtrl) && ( testCtrl != ctrl[selected] ) ) {
    printf( TRANSLATE("ERROR: This alias is already used in the swarm.\n", "FEHLER: Dieser Alias wird bereits im Schwarm verwendet.\n") );
    return;
  }

  // change name
  ctrl[selected]->setAlias( alias );
  ctrl[selected]->save( FTSWARM_NVSSCOPE_ALIAS, SWOS_NOPORT );

}

void MenuSwarmConfig::newSwarm( void ) {

  char name[MAXIDENTIFIER];
  name[0] = '\0';
  while (strlen(name) < 5) enterString( TRANSLATE("New Swarm Name [min. 5 chars]: ", "Neuer Swarm Name [min. 5 Zeichen]: "), name, MAXIDENTIFIER );
  uint16_t pin = enterNumber( TRANSLATE("New Swarm Pin [1..9999]: ", "Neuer Swarm Pin [1..9999]: "), -1, 1, 9999 );

  if ( myOSSwarm.Ctrl[0]->IAmKelda ) {
    if (!yesNo( TRANSLATE("Destroy the existing swarm and create a new one? [Y/N] ", "Bestehenden Schwarm löschen und einen neuen erstellen? [J/N] ") ) ) return;
  } else {
    if (!yesNo( TRANSLATE("Leave the existing swarm and create a new one? [Y/N] ", "Bestehenden Schwarm verlassen und einen neuen erstellen? [J/N] ") ) ) return;
  }

  strcpy( nvs.swarm.name, name );
  nvs.swarm.pin = pin;

  myOSSwarm.newSwarm(  );

  nvs.save( FTSWARM_NVSSCOPE_SWARM );

}

void MenuSwarmConfig::addController( void ) {

  FtSwarmSerialNumber_t serialNumber = (FtSwarmSerialNumber_t) enterNumber( TRANSLATE( "Enter new swarm members serial number [1..9999]: ", "Seriennummer des neuen Controllers [1..9999]: " ), -1, 1, 9999 );

  if ( myOSSwarm.isMember( serialNumber ) ) { printf( TRANSLATE("ERROR: This controller is already part of this swarm.\n", "FEHLER: Dieser Controller ist bereits im Swarm.\n") ); return; }
  
  if ( !myOSSwarm.addController( serialNumber ) ) {
    // no space left
    printf( TRANSLATE("ERROR: No space left in swarm. Controller #%d was declined.\n", "FEHLER: Der Controller #%d kann nicht hinzugenommen werden, da die maximale Controlleranzahl erreicht ist.\n"), serialNumber );
    return;
  }

  printf( TRANSLATE("Controller SN %d was added to the swarm.\n", "Controller SN %d wurde dem Swarm hinzugefügt.\n"), serialNumber );

  // wait max 1.5 seconds to get the controller connected
  uint8_t i=0;
  while ( !myOSSwarm.getController( serialNumber )->isOnline( ) ) {
    i++;
    vTaskDelay( 100 / portTICK_PERIOD_MS );
    if (i > 15 ) break;
  }

  nvs.save( FTSWARM_NVSSCOPE_SWARM );
  
  if ( !myOSSwarm.getController( serialNumber )->isOnline( ) ) {

    if ( nvs.wifi.mode == wifiOFF ) {
      printf( TRANSLATE("WARNING: Controller #%d is offline. Turn it on.\n", "WARNUNG: Der Controller #%d ist offline. Schalten Sie ihn ein.\n"), serialNumber );
    
    } else {

      printf( TRANSLATE("WARNING: Controller #%d is offline.\na) Turn it on.\nb) Correct wifi it's settings\n", "WARNUNG: Der Controller #%d ist offline.\na)Schalten Sie ihn ein.\nb) Korrigieren Sie seine WLAN Einstellungen\n"), serialNumber );

      if ( yesNo( TRANSLATE( "Shall I try to send wifi settings and reboot? [Y/N]", "Soll ich versuchen die WLAN-Einstellungen zu korrigieren und neu starten [J/N]?" ) ) ) {

        char SSID[64];
        sprintf( SSID, "ftSwarm%d", serialNumber );
        wifiHandler->change_wifi_network( SSID, SSID );

        printf("Connecting to %s", SSID ); flushStdIO();

        // wait max 5 seconds to get the controller connected
        uint8_t i=0;
        while ( !wifiHandler->STAConnected ) {
          i++;
          vTaskDelay( 100 / portTICK_PERIOD_MS );
          printf("."); flushStdIO();
          if (i > 20 ) {
            printf( TRANSLATE( "\nCan't connect to controller. Rebooting.\n", "\nKonnte den Controller nicht erreichen. Starte neu.\n") );
            vTaskDelay( 250 / portTICK_PERIOD_MS );
            esp_restart();
          }
        }

        // best choice
        vTaskDelay( 2000 / portTICK_PERIOD_MS );

        myOSSwarm.getController( serialNumber )->setWifi( wifiClient, nvs.wifi.SSID, nvs.wifi.Password, true );
        printf( TRANSLATE( "\nWifi settings have been corrected.\nBoth controllers will restart.\n", "\nDie WLAN Einstellungen wurden korrkigiert.\nBeide Controller starten neu.\n") );
        vTaskDelay( 2000 / portTICK_PERIOD_MS );
        esp_restart();

      }

    }

  }

}

void MenuSwarmConfig::deleteController( void ) {

  FtSwarmSerialNumber_t serialNumber = (FtSwarmSerialNumber_t) enterNumber( TRANSLATE( "Enter serial number to be revoked [1..9999]: ", "Seriennummer des zu löschenden Controllers [1..9999]: " ), -1, 1, 9999 );

  if ( !myOSSwarm.isMember( serialNumber ) ) { printf( TRANSLATE("ERROR: This controller isn't part of this swarm.\n", "FEHLER: Dieser Controller ist nicht im Swarm.\n") ); return; }
  
  if ( !myOSSwarm.deleteController( serialNumber ) ) {
    // not found
    printf( TRANSLATE("ERROR: This controller isn't part of this swarm.\n", "FEHLER: Dieser Controller ist nicht im Swarm.\n") );
    return;
  }

  printf( TRANSLATE("Controller SN %d was revoked from the swarm.\n", "Controller SN %d wurde aus dem Swarm gelöscht.\n"), serialNumber );

  nvs.save( FTSWARM_NVSSCOPE_SWARM );

}

void MenuSwarmConfig::run( void ) {

  FtSwarmCommunication_t swarmCommunication = nvs.swarm.communication;
  uint8_t swarmSpeed = nvs.swarm.speed;

  while (1) {
    
    fillCtrlList();
    start();

    add("Swarm Name", nvs.swarm.name, MENU_DEACTIVATED, MENU_NOKEY );
    if ( myOSSwarm.Kelda ) add("Kelda", myOSSwarm.Kelda->getAliasOrName(), MENU_DEACTIVATED, MENU_NOKEY );

    // wifi
    if ( nvs.wifi.mode != wifiOFF ) add( TRANSLATE("Communication WIFI", "WLAN-Kommunikation"), ONOFF[swarmCommunication.wifi], MENU_COM_WIFI, 'w' );

    // rs485
    if ( FTSWARM_HAL_RS485 ) {    
      add( TRANSLATE("Communication RS485", "RS485-Kommunikation"), ONOFF[swarmCommunication.rs485], MENU_COM_RS485, 'r' );
      if ( swarmCommunication.rs485 ) {
        add( TRANSLATE("Swarm speed", "Swarm-Geschwindigkeit"), swarmSpeed, MENU_SPEED, 's' );
      }
    }

    // no communication channel selected?
    if ( !( ( ( nvs.wifi.mode != wifiOFF ) && ( swarmCommunication.wifi ) ) || ( swarmCommunication.rs485 ) ) ) {

      addExit();
      printf(TRANSLATE("\n*** You need to invoke a communication method. Maybe you need to setup wifi first. ***\n", "\n*** Sie müssen eine Kommunikationsmethode aktivieren. Kontrollieren Sie die WLAN-Einstellungen. ***\n") );

    // need to apply some communication changes before applying swarm operations?
    } else if ( communicationChanges ) {

      addExit();
      printf(TRANSLATE("\n*** to apply your changes you need to save & restart the device first. ***\n", "\n*** Um Ihre Änderungen anzuwenden, müssen Sie das Gerät neu starten. ***\n") );

    } else {

      add("Pin", nvs.swarm.pin, MENU_DEACTIVATED, MENU_NOKEY );
    
      printf("\n");

      printf("     Name         Status   NW-Age    Alias\n" );

      for ( int8_t i=0; i<=maxCtrl; i++ ) {
      
        if ( ( ctrl[i]->isOnline() ) && ( myOSSwarm.Ctrl[0]->IAmKelda ) ) { 
          printf("(%2d) ", i+1); 
          add( i ); 
      
        } else { 
          printf("     "); 
        }

        printf( "%-11.11s  %-7s  [%.6lu]  %s\n", ctrl[i]->getName(), SWOSCOMSTATE[ctrl[i]->getComState()], ctrl[i]->networkAge(), ctrl[i]->getAlias() );
      }

      printf("\n\n");
      add( TRANSLATE("create new swarm", "neuen Swarm erstellen"), "", MENU_NEW,    'n' );

      if (myOSSwarm.Ctrl[0]->IAmKelda) {
        add( TRANSLATE("add a controller to my swarm", "Controller hinzufügen"), "", MENU_ADD,    '+' );
        if (myOSSwarm.maxCtrl > 0) add( TRANSLATE("revoke a controller from my swarm", "Controller löschen"), "", MENU_DELETE, '-' );
      }

      add( TRANSLATE("set alias name", "Alias-Name"), "", MENU_ALIAS,  'a' );

      addExit();
    }
  
    int8_t choice = userChoice();

    switch( choice ) {
      case MENU_EXIT:       if ( communicationChanges ) {
                              if ( yesNo( TRANSLATE( "To apply your changes, the device needs to be restarted.\nSave settings and restart now (Y/N)?", "Um Ihre Änderungen anzuwenden, muss das Gerät neu gestartet werden.\nEinstellungen speichern und jetzt neu starten (J/N)?") ) ) {
                                nvs.swarm.speed = swarmSpeed;
                                nvs.swarm.communication = swarmCommunication;
                                nvs.saveAndRestart( FTSWARM_NVSSCOPE_SWARM );
                              }                           
                            }
                            return;

      case MENU_COM_RS485:  
      case MENU_COM_WIFI:   if ( choice == MENU_COM_RS485 ) swarmCommunication.rs485 = !swarmCommunication.rs485; 
                            if ( choice == MENU_COM_WIFI)   swarmCommunication.wifi  = !swarmCommunication.wifi;
                            communicationChanges = true;
                            break;

      case MENU_SPEED:      swarmSpeed = enterNumber( TRANSLATE("(0) low ... (4) highspeed (max. 50m)", "(0) niedrig ... (4) Maximalgeschwindigkeit (max. 50m)"), nvs.swarm.speed, 0, 4 );
                            communicationChanges = true;
                            break;

      case MENU_NEW:        newSwarm();
                            break;
        
      case MENU_ADD:        addController();
                            break;

      case MENU_DELETE:     deleteController();
                            break;

      case MENU_ALIAS:      changeAlias();
                            break;

      default:              if (ctrl[choice]->getComState() != COMSTATE_ONLINE ) {
                              printf( TRANSLATE( "ERROR: %s is not online.\n\n", "FEHLER: %s ist nicht online.\n\n" ), ctrl[choice]->getAliasOrName() );
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
    MainMenu( ):Menu( NULL, TRANSLATE("setup", "Setup"), TRANSLATE("Main Menu", "Hauptmenü"), 14) {};
    void factorySettings( void );
    void run( void );

};

void MainMenu::factorySettings( void ) {
  // reset controller to factory settings

  if (yesNo( TRANSLATE( "Do you want to reset this device to it's factory settings and reboot (Y/N)?", "Möchten Sie dieses Gerät auf die Werkseinstellungen zurücksetzen und neu starten (J/N)?" ) ) ) {

    delay(2000);

    myOSSwarm.factoryReset();

  }

}

void MainMenu::run( void ) {

  while (1) {

    start( );
    add( TRANSLATE( "Wifi & Local Settings", "WLAN & Lokale Einstellungen" ), "", MENU_WEB, 'w' );
    if ( ( myOSSwarm.wifiConnected ) || ( nvs.wifi.mode == wifiAP ) || ( FTSWARM_HAL_RS485 ) ) {
      add( TRANSLATE( "Swarm Configuration", "Swarm-Konfiguration" ), "", MENU_SWARM, 's' );
    } else {
      add( TRANSLATE( "Swarm Configuration - activate WiFi", "Swarm-Konfiguration - WLAN aktivieren" ), "", MENU_DEACTIVATED );
    }

    add( TRANSLATE( "IO Configuration", "IO-Konfiguration" ), "", MENU_IOCONFIG, 'i' );
    add( TRANSLATE( "Remote/Event Configuration", "Remote/Event-Konfiguration" ), "", MENU_REMOTE, 'r' );

    add( TRANSLATE( "Factory Reset", "Werkseinstellungen" ), "", MENU_FACTORY, 'f' );
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

  if ( nvs.swarm.IAmKelda ) {
    
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