/*
 * SwOSSNVS.cpp
 *
 * internal represenation of nvs values.
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */

#include <nvs.h>
#include <nvs_flash.h>
#include <esp_err.h>
#include <esp_task_wdt.h>
#include <esp_system.h>

#include "SwOS.h"
#include "SwOSNVS.h"
#include "easyKey.h"
#include "SwOSLog.h"
#include "lsm6dsr_reg.h"

SwOSNVS nvs;

bool SwOSNVSEvent::cmp(  SwOSNVSEvent *otherEvent ) {

  if ( ( memcmp( &sensor, &otherEvent->sensor, sizeof(SwOSIOUID) ) == 0 ) && 
       ( memcmp( &actor,  &otherEvent->actor,  sizeof(SwOSIOUID) ) == 0 ) && 
       ( triggerMath.bits.trigger == otherEvent->triggerMath.bits.trigger ) &&
       ( triggerMath.bits.op      == otherEvent->triggerMath.bits.op  ) ) {

    if ( (triggerMath.raw == otherEvent->triggerMath.raw) && ( parameter.raw == otherEvent->parameter.raw ) ) return 2;
    else return 1;

  } else return 0;
  
}

/***************************************************
 *
 *   SwOSNVS
 *
 ***************************************************/

uint16_t generateSecret( FtSwarmSerialNumber_t serialNumber ) {
  return ( ( ( ( uint16_t) serialNumber ) & 0xFF ) << 8 ) | ( rand() & 0xFF ) ;
}

void SwOSNVS::initialSetup( void ) {

  version = NVSVERSION;

  CPU = CPUFirmware;

  pixels = FTSWARM_HAL_PIXELS;
  extensionPort.mode = ( FTSWARM_HAL_EXT_PORT ) ? FTSWARM_EXT_OFF : FTSWARM_EXT_I2C_MASTER; 

  serialNumber = enterNumber( TRANSLATE("Serial number [1..9999]>", "Seriennummer [1..9999]"), 0, 1, 9999 );

  sprintf( wifi.SSID, "ftSwarm%d", serialNumber );
  strcpy( wifi.Password, wifi.SSID );

  strcpy( swarm.name, wifi.SSID );

  swarm.secret = generateSecret( serialNumber ); 
  swarm.pin    = serialNumber;

  // check on i2c/spi LSMR
  if ( ( CPU == FTSWARMRS_2V1 ) ||
       ( CPU == FTSWARMRC_1V141 ) ||
       ( CPU == FTSWARMCONTROL_1V3UC ) ) {
    Wire.begin( 4, 5 );
    Wire.beginTransmission( 0xD7 );
    spiGyro = (Wire.endTransmission(true) != 0);
  }

  if ( yesNo( TRANSLATE( "Save configuration (Y/N)?>", "Konfiguration speichern (J/N)?" ) ) ) {
    save( FTSWARM_NVSSCOPE_INITAL );
    printf( "saving");
    for (uint8_t i=0; i<3; i++ ) {
      putchar('.');
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    printf("\n");
  }

  printf( "restarting...");
  vTaskDelay(500 / portTICK_PERIOD_MS);
  esp_restart();

}

SwOSNVS::SwOSNVS() {

  reset( false );

}

void SwOSNVS::begin() {

  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      // NVS partition was truncated and needs to be erased
      // Retry nvs_flash_init
      SWARM_LOG_ERROR( TRANSLATE( "Invalid NVS found. Erasing NVS.", "Ungültige NVS Daten gefunden. Lösche NVS-Partition." ) );
      nvs_flash_erase();
      err = nvs_flash_init();
  }

  if (!load() ) {
    initialSetup();
  }

  if (nvs.CPU != CPUFirmware ) {
    printf( TRANSLATE( "\n\nFATAL: Incompatible firmware hardware settings.\n", "\n\nFEHLER: Die Firmware passt nicht zur Hardware.\n" ) );
    printf("       Firmware %s\n", FTSWARMVERSION[CPUFirmware] );
    printf("       Board    %s\n", FTSWARMVERSION[CPU] );
    while (1) delay(1000);
  }

}

bool SwOSNVS::load() {

  // Open
  nvs_handle_t my_handle;
  
  esp_err_t nvserror = nvs_open( NVSNAMESPACE, NVS_READONLY, &my_handle);
  // initial setup?
  if ( nvserror == ESP_ERR_NVS_NOT_FOUND ) return false; 

  // start reading my version to check my data is valid
  nvs_get_i32( my_handle, "NVSVersion", &version);
  if ( version <= 1 ) {
    // no version data, or old version 1
    return false;
  }

  // factoryReset
  nvs_get_u8( my_handle, "factoryReset", (uint8_t *) &factoryReset );

  // start with HW configuration
  uint32_t ui32;
  nvs_get_u16( my_handle, "serialNumber",  (uint16_t *) &serialNumber );
  nvs_get_u32( my_handle, "CPU",           (uint32_t *) &CPU );

  // check on valid hw data
  if ( ( serialNumber == 0 ) ||
       ( CPU == FTSWARM_NOVERSION ) ) {
    nvs_close( my_handle );
    return false;
  }

  size_t dummy;
       
  // ftSwarmControl / joystick calibration
  dummy = sizeof( joystick );
  nvs_get_blob( my_handle, "joystick", &joystick, &dummy );
  // invalid values?
  for ( uint8_t i=0; i<4; i++ ) {
    if ( ( joystick[i].minValue <    0) || ( joystick[i].minValue > 1000 ) ) joystick[i].minValue =  200;
    if ( ( joystick[i].midValue < 1000) || ( joystick[i].midValue > 2500 ) ) joystick[i].midValue = 1000;
    if ( ( joystick[i].maxValue < 2500) || ( joystick[i].maxValue > 4095 ) ) joystick[i].maxValue = 3700;
  }

  // servoOffsets
  dummy = sizeof( servo );
  // nvs_get_blob( my_handle, "servo", &servo, &dummy );

  // RGBLeds
  nvs_get_u8( my_handle, "RGBLeds", &pixels );
  
  // wifi
  nvs_get_u32( my_handle, "wifiMode", (uint32_t *) &wifi.mode );
  nvs_get_u8 ( my_handle, "Channel",  &wifi.channel );
  dummy = sizeof( wifi.SSID ); nvs_get_str( my_handle, "wifiSSID", wifi.SSID, &dummy );
  dummy = sizeof( wifi.Password ); nvs_get_str( my_handle, "wifiPwd",  wifi.Password,  &dummy );

  // swarm
  nvs_get_u16( my_handle, "swarmSecret", &swarm.secret );
  nvs_get_u16( my_handle, "swarmPIN",  &swarm.pin );
  dummy = sizeof( swarm.name ); nvs_get_str( my_handle, "swarmName", swarm.name, &dummy );
  nvs_get_u8(  my_handle, "swarmSpeed", &swarm.speed );

  // Kelda & swarm.members
  nvs_get_u8( my_handle, "IAmKelda", (uint8_t *) &swarm.IAmKelda );
  nvs_get_u8( my_handle, "swarmCom", &swarm.communication.raw);
  dummy = sizeof( swarm.member ); nvs_get_blob( my_handle, "swarmMember", &swarm.member, &dummy );

  // to avoid hickups
  if ( wifi.mode == wifiOFF ) swarm.communication.wifi = 0;

  // webUI
  nvs_get_u8 ( my_handle, "webUI", (uint8_t *) &wifi.webUI );

  // ExtentionPort
  nvs_get_u32( my_handle, "extensionPort", (uint32_t *) &extensionPort);
  if ( ( CPU == FTSWARMCONTROL_1V3 ) && ( extensionPort.mode == FTSWARM_EXT_OFF ) ) { extensionPort.mode = FTSWARM_EXT_I2C_MASTER; } // backward compatibility
  nvs_get_u8 ( my_handle, "I2CAddr",       &extensionPort.I2CAddr );
  nvs_get_u8 ( my_handle, "interruptLine", &extensionPort.interruptLine );
  nvs_get_i16( my_handle, "interruptLow",  &extensionPort.interruptOnOff[0] );
  nvs_get_i16( my_handle, "interruptHigh", &extensionPort.interruptOnOff[1] );
  nvs_get_u8 ( my_handle, "I2CRegisters",  &extensionPort.I2CRegisters );
  nvs_get_u8 ( my_handle, "Gyro",          (uint8_t *) &extensionPort.gyro);
  nvs_get_u8 ( my_handle, "spiGyro",       (uint8_t *) &spiGyro);

  nvs_close( my_handle );

  loadEvents();

  return true;

}

void SwOSNVS::deleteAllControllers(  void ) {

  bzero( swarm.member, MAXCTRL );

}

void SwOSNVS::save( FtSwarmNVSScope_t scope ) {

  // Open
  nvs_handle_t my_handle;
  if ( nvs_open(NVSNAMESPACE, NVS_READWRITE, &my_handle) != ESP_OK ) { SWARM_LOG_ERROR("NVS save: Can't open NVS."); return; };

  // Write
  if ( scope | FTSWARM_NVSSCOPE_CORE ) {
    nvs_set_i32( my_handle, "NVSVersion", NVSVERSION ) ;
    nvs_set_u16( my_handle, "serialNumber", (FtSwarmSerialNumber_t) serialNumber ) ;
    nvs_set_u32( my_handle, "CPU", (uint32_t) CPU ) ;
  }

  // factoryReset
  if ( scope | FTSWARM_NVSSCOPE_FACTORYRESET ) nvs_set_u8( my_handle, "factoryReset", factoryReset );
  
  // ftSwarmControl: set joystick calibration
  if ( scope | FTSWARM_NVSSCOPE_JOYSTICK ) nvs_set_blob( my_handle, "joystick",  (void *)&joystick, sizeof( joystick ) );

  // servo offset
  if ( scope | FTSWARM_NVSSCOPE_SERVO ) nvs_set_blob( my_handle, "servo",  (void *)&servo, sizeof( servo ) );

  // RGBLeds
  if ( scope | FTSWARM_NVSSCOPE_PIXEL ) nvs_set_u8( my_handle, "RGBLeds", pixels );

  // wifi
  if ( scope | FTSWARM_NVSSCOPE_WIFI ) {
    nvs_set_u32( my_handle, "wifiMode", wifi.mode );
    nvs_set_u8 ( my_handle, "Channel",  (uint8_t) wifi.channel );
    nvs_set_str( my_handle, "wifiSSID",           wifi.SSID);
    nvs_set_str( my_handle, "wifiPwd",            wifi.Password);
  }

  // swarm
  if ( scope | FTSWARM_NVSSCOPE_SWARM ) {
    nvs_set_u16( my_handle, "swarmSecret", swarm.secret );
    nvs_set_u16( my_handle, "swarmPIN",    swarm.pin );
    nvs_set_str( my_handle, "swarmName",   swarm.name );
  
    // Kelda & swarm.members
    nvs_set_u8  ( my_handle, "IAmKelda",     (uint8_t) swarm.IAmKelda );
    nvs_set_u8  ( my_handle, "swarmCom",     swarm.communication.raw );
    nvs_set_blob( my_handle, "swarmMember",  (void *)swarm.member, sizeof( swarm.member ) );
    nvs_set_u8  ( my_handle, "swarmSpeed",   swarm.speed );

  }

  // webUI
  if ( scope | FTSWARM_NVSSCOPE_WEBUI ) nvs_set_u8 ( my_handle,  "webUI",   (uint8_t) wifi.webUI );

  // extensionPort
  if ( scope | FTSWARM_NVSSCOPE_EXTPORT ) {
    nvs_set_u8 ( my_handle, "I2CAddr",       extensionPort.I2CAddr );
    nvs_set_u8 ( my_handle, "interruptLine", extensionPort.interruptLine );
    nvs_set_i16( my_handle, "interruptLow",  extensionPort.interruptOnOff[0] );
    nvs_set_i16( my_handle, "interruptHigh", extensionPort.interruptOnOff[1] );
    nvs_set_u8 ( my_handle, "I2CRegisters",  extensionPort.I2CRegisters );
    nvs_set_u32( my_handle, "extensionPort", (uint32_t) extensionPort.mode);
    nvs_set_u8 ( my_handle, "Gyro",          (uint8_t)  extensionPort.gyro);
    nvs_set_u8 ( my_handle, "spiGyro",       (uint8_t)  spiGyro);
  }

  // commit
  nvs_commit( my_handle );

  nvs_close( my_handle );

  if ( scope | FTSWARM_NVSSCOPE_EVENTS ) saveEvents();

}

void SwOSNVS::saveAndRestart( FtSwarmNVSScope_t scope ) {

  // save settings
  save( scope );

  // reboot
  esp_restart();

}

void SwOSNVS::saveEvents( void ) {

  char config[16];

  // Open
  nvs_handle_t my_handle;
  if ( nvs_open( NVSNAMESPACE, NVS_READWRITE, &my_handle) != ESP_OK ) { SWARM_LOG_ERROR("Save events failed: Can't open NVS."); return; };

  // active config
  nvs_set_u8( my_handle, "activeConfig", events.activeConfig );

  // quick config
  nvs_set_blob( my_handle, "quickConfig", (void *)events.quickConfig, sizeof( events.quickConfig ) );

  // save events
  nvs_set_blob( my_handle, "event0", (void *)events.events[0], sizeof( events.events[0] ) );
  nvs_set_blob( my_handle, "event1", (void *)events.events[1], sizeof( events.events[1] ) );
  nvs_set_blob( my_handle, "event2", (void *)events.events[2], sizeof( events.events[2] ) );
  nvs_set_blob( my_handle, "event3", (void *)events.events[3], sizeof( events.events[3] ) );

  // save labels
  nvs_set_blob( my_handle, "label0", (void *)events.oledLabel[0], sizeof( events.oledLabel[0] ) );
  nvs_set_blob( my_handle, "label1", (void *)events.oledLabel[1], sizeof( events.oledLabel[1] ) );
  nvs_set_blob( my_handle, "label2", (void *)events.oledLabel[2], sizeof( events.oledLabel[2] ) );
  nvs_set_blob( my_handle, "label3", (void *)events.oledLabel[3], sizeof( events.oledLabel[3] ) );

  // commit
  nvs_commit( my_handle );

  nvs_close( my_handle );

}

void SwOSNVS::loadEvents( void ) {
  
  char   config[16];
  size_t dummy;

  // Open
  nvs_handle_t my_handle;
  if ( nvs_open( NVSNAMESPACE, NVS_READWRITE, &my_handle) != ESP_OK ) SWARM_LOG_FATAL("Can't open NVS.");

  // active config
  nvs_get_u8( my_handle, "activeConfig", &events.activeConfig );

  // quick config
  dummy = sizeof( events.quickConfig ); nvs_get_blob( my_handle, "quickConfig", events.quickConfig, &dummy );

  // load events
  dummy = sizeof( events.events[0] ); nvs_get_blob( my_handle, "event0", events.events[0], &dummy );
  dummy = sizeof( events.events[1] ); nvs_get_blob( my_handle, "event1", events.events[1], &dummy );
  dummy = sizeof( events.events[2] ); nvs_get_blob( my_handle, "event2", events.events[2], &dummy );
  dummy = sizeof( events.events[3] ); nvs_get_blob( my_handle, "event3", events.events[3], &dummy );

  // load labels
  dummy = sizeof( events.oledLabel[0] ); nvs_get_blob( my_handle, "label0", events.oledLabel[0], &dummy );
  dummy = sizeof( events.oledLabel[1] ); nvs_get_blob( my_handle, "label1", events.oledLabel[1], &dummy );
  dummy = sizeof( events.oledLabel[2] ); nvs_get_blob( my_handle, "label2", events.oledLabel[2], &dummy );
  dummy = sizeof( events.oledLabel[3] ); nvs_get_blob( my_handle, "label3", events.oledLabel[3], &dummy );

  nvs_close( my_handle );

}

void SwOSNVS::deleteAllEvents( uint8_t configuration ) {

  // set all events to "NULL"
  bzero( events.events[configuration], MAXNVSEVENTS * sizeof( SwOSNVSEvent ) );
  bzero( events.oledLabel[configuration], sizeof( events.oledLabel[configuration] ) );
  events.quickConfig[configuration] = FTSWARM_CFG_INDIVIDUAL;

}

bool SwOSNVS::addEvent( uint8_t configuration, SwOSNVSEvent *event ) {

  events.quickConfig[configuration] = FTSWARM_CFG_INDIVIDUAL;

  uint8_t i = 0;

  while ( i<MAXNVSEVENTS ) {
    
    if ( nvs.events.events[configuration][i].isNull() ) {
      nvs.events.events[configuration][i] = *event;
      return true;
    }

    i++;

  }

  return false;

}

void SwOSNVS::deleteEvent( uint8_t configuration, uint8_t eventIndex ) {

  // move all successors
  if ( eventIndex+1 < MAXNVSEVENTS ) memcpy( &nvs.events.events[nvs.events.activeConfig][eventIndex], &nvs.events.events[nvs.events.activeConfig][eventIndex+1], ( MAXNVSEVENTS - eventIndex -1 ) * sizeof( SwOSNVSEvent ) );

  // cleanup last event
  bzero( &nvs.events.events[nvs.events.activeConfig][MAXNVSEVENTS-1], sizeof( SwOSNVSEvent) );

}

bool SwOSNVS::exists( uint8_t configuration, SwOSNVSEvent *event, SwOSNVSEvent *exclude ) {

  for (uint8_t i=0; i<MAXNVSEVENTS; i++ ) {
    if ( ( events.events[configuration][i].cmp( event ) ) && ( &events.events[configuration][i] != exclude ) ) return true;
  }

  return false;

}

void SwOSNVS::reset( bool factoryReset ) {

  // initial settings
  if ( !factoryReset ) {
    version = NVSVERSION;
    CPU = FTSWARM_NOVERSION;
    serialNumber = 0;
    spiGyro = false;
  }

  this->factoryReset = factoryReset;

  // wifi
  wifi.channel = 1;
  wifi.mode = wifiAP;
  sprintf( wifi.SSID, "ftSwarm%d", serialNumber );
  strcpy( wifi.Password, wifi.SSID );

  wifi.webUI = true;

  // swarm
  strcpy( swarm.name, wifi.SSID );
  swarm.secret             = generateSecret( serialNumber ); 
  swarm.pin                = serialNumber;
  swarm.IAmKelda           = true;
  swarm.communication.wifi = 1;
  swarm.speed              = 4;
  bzero( swarm.member, sizeof( swarm.member ) );

  // joystick calibration
  for ( uint8_t j=0; j<4; j++ ) {
    joystick[j].minValue = 200;
    joystick[j].midValue = 1900;
    joystick[j].maxValue = 3700;
  }

  // servo offset
  for ( uint8_t j=0; j<4; j++ ) {
    servo[j].offset = 45;
    servo[j].minValue = 510;
    servo[j].maxValue = 890;
  }

  pixels = FTSWARM_HAL_PIXELS;

  extensionPort.mode               = FTSWARM_EXT_OFF;
  extensionPort.I2CAddr            = 0x66;
  extensionPort.interruptLine      = 0;
  extensionPort.interruptOnOff[0]  = 0;
  extensionPort.interruptOnOff[1]  = 255;
  extensionPort.I2CRegisters       = MAXI2CREGISTERS;
  extensionPort.gyro               = false;

  bzero( swarm.member, sizeof( swarm.member ) );
  events.activeConfig  = 0;
  bzero( events.events, sizeof( events.events ) );

  // initialize lables
  bzero( events.oledLabel, sizeof( events.oledLabel ) );

  // Quick Configs
  for ( uint8_t j=0; j<MAXEVENTCONFIGS; j++ ) events.quickConfig[j] = FTSWARM_CFG_INDIVIDUAL;

}

void SwOSNVS::createSwarm( char *name, uint16_t pin ) {

  // failsave copy of swarm.name
  strlcpy( swarm.name, name, MAXIDENTIFIER );
  swarm.pin = pin;

  // kill all old swarm members
  bzero( swarm.member, sizeof( swarm.member ) );

  // genenerate a really new secret
  uint16_t newSecret;
  while (1) {
    newSecret = generateSecret( serialNumber );
    if ( newSecret != swarm.secret ) break;
  }
  swarm.secret = newSecret; 
  
}

bool SwOSNVS::addController( FtSwarmSerialNumber_t serialNumber ) {

  for (int8_t i=0; i<MAXCTRL; i++ ) {

    // already in list? done.
    if ( swarm.member[i] == serialNumber ) {
      return false;
    }

  }

  for (int8_t i=0; i<MAXCTRL; i++ ) {

    // free space in list? save index
    if ( swarm.member[i] == 0 ) {
      swarm.member[i] = serialNumber;
      return true;
    };

  }

  // no free space? error
  printf( TRANSLATE( "no space\n", "kein Platz\n" ) );
  return false;

}

bool SwOSNVS::deleteController( FtSwarmSerialNumber_t serialNumber ) {

  for (uint8_t i=0; i<MAXCTRL; i++) {

    // serialNumber found? kill it
    if (swarm.member[i] == serialNumber) {
      swarm.member[i] = 0;

      // check all configs
      for ( uint8_t config=0; config<MAXEVENTCONFIGS; config++ ) {

        // check all events
        uint8_t event = 0;

        while ( ( event < MAXNVSEVENTS ) && ( events.events[config][event].sensor.serialNumber ) )

          // if sensor or actor fits to SN, delete
          if ( ( events.events[config][event].sensor.serialNumber == serialNumber ) ||
               ( events.events[config][event].actor.serialNumber == serialNumber ) ) {
            deleteEvent( config, event );
          } else event++;

      }

      return true;
    }

  }

  // didn't find serialNumber? error.
  return false;

}

uint8_t SwOSNVS::swarmMembers( void ) {

  uint8_t result = 0;
  for (uint8_t i=0; i<MAXCTRL; i++ ) {
    if ( swarm.member[i] ) result++;
  }

  return result;

}

void SwOSNVS::printNVS() {

  printf( "NVSVersion: %d\n", version );
  printf( "CPU: %d\n", CPU );
  printf( "serialNumber: %d\n", serialNumber );

  printf( "wifi.Mode: %d\n", wifi.mode );
  printf( "wifi.Channel: %d\n", wifi.channel );
  printf( "wifi.SSID: >%s<\n", wifi.SSID );
  
  printf( "pixels: %d\n", pixels );
  printf( "spiGyro: %d\n", spiGyro );
  printf( "extensionPort.gyro: %d\n", extensionPort.gyro );
  printf( "extensionPort.extensionPort: %d\n", extensionPort.mode );
  printf( "extensionPort.I2CAddr: %d\n", extensionPort.I2CAddr );
  printf( "extensionPort.I2CRegisters: %d\n", extensionPort.I2CRegisters );
  printf( "extensionPort.InterruptLine: %d\n", extensionPort.interruptLine );
  printf( "extensionPort.interruptOnOff: %d %d\n", extensionPort.interruptOnOff[0], extensionPort.interruptOnOff[1] );
  printf( "swarm.secret: 0x%4X\n", swarm.secret );
  printf( "swarm.pin: %d\n", swarm.pin );
  printf( "swarm.name: %s\n", swarm.name );
  printf( "swarm.IAmKelda: %d\n", swarm.IAmKelda );
  printf( "swarm.communication %d\n", swarm.communication );
  printf( "swarm.speed %d\n", swarm.speed );

  printf( "swarm members:");
  for (uint8_t i=0; i<MAXCTRL; i++) { if (swarm.member[i]) printf(" %d", swarm.member[i]); }
  printf( "\n");

  for (uint8_t i=0; i<4; i++ ) printf( "joystick calibration[%d] min: %d mid: %d max: %d\n", i, joystick[i].minValue, joystick[i].midValue, joystick[i].maxValue );
  for (uint8_t i=0; i<4; i++ ) printf( "servo[%d] offset: %d min: %d max: %d\n", i, servo[i].offset, servo[i].minValue, servo[i].maxValue );

  printf( "events:\n" );
  printf( "events.activeConfig %d\n", events.activeConfig);

  // configs
  for ( uint8_t c=0; c<MAXEVENTCONFIGS; c++ ) {
  
    printf("Event configuration %d", c);
    if ( c== events.activeConfig ) printf(" - active -");
    printf("\n");

    printf("Config Type %s\n", FTSWARMQUICKCONFIG[events.quickConfig[c]] );

    // events
      for ( uint8_t i=0; i<MAXNVSEVENTS; i++ ) {

        if ( events.events[c][i].sensor.serialNumber != 0 ) {
          printf( "#%d input %d.%d.%d actor %d.%d.%d trigger %d op %d v1 %d v2 %d parameter %08X\n", 
                  i, 
                  events.events[c][i].sensor.serialNumber, events.events[c][i].sensor.ioType, events.events[c][i].sensor.port,
                  events.events[c][i].actor.serialNumber,  events.events[c][i].actor.ioType,  events.events[c][i].actor.port,
                  events.events[c][i].triggerMath.bits.trigger, events.events[c][i].triggerMath.bits.op, events.events[c][i].triggerMath.bits.v1, events.events[c][i].triggerMath.bits.v2,
                  events.events[c][i].parameter
                );
        }
      }
  }
 
}

bool SwOSNVS::upgrade( void ) {

  if ( version != NVSVERSION ) {

    SWARM_LOG_INFO( TRANSLATE( "NVS-Upgrade: Old NVS version %d found. Upgrading to version %d.", "NVS-Upgrade: Alte NVS-Version %d gefunden. Aktualisiere auf Version %d." ), version, NVSVERSION );

    // erase all
    nvs_handle_t my_handle;
    if ( nvs_open(NVSNAMESPACE, NVS_READWRITE, &my_handle) != ESP_OK ) SWARM_LOG_FATAL( TRANSLATE( "NVS-Upgrade failed: Can't open NVS.", "NVS-Upgrade fehlgeschlagen: Kann NVS nicht öffnen." ) );
    if ( nvs_erase_all( my_handle ) != ESP_OK ) SWARM_LOG_FATAL( TRANSLATE( "NVS-Upgrade failed: Can't erase NVS.", "NVS-Upgrade fehlgeschlagen: Kann NVS nicht löschen." ) );
    if ( nvs_commit( my_handle ) != ESP_OK ) SWARM_LOG_FATAL( TRANSLATE( "NVS-Upgrade failed: Can't commit NVS.", "NVS-Upgrade fehlgeschlagen: Kann NVS nicht speichern." ) );
    nvs_close( my_handle );
    
    // save again
    save( FTSWARM_NVSSCOPE_INITAL );
    return true;   
  }

  return false;

}