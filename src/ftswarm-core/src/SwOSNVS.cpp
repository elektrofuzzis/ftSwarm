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

    if ( (triggerMath.raw == otherEvent->triggerMath.raw) && ( parameter == otherEvent->parameter ) ) return 2;
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
  extensionPort = ( FTSWARM_HAL_EXT_PORT ) ? FTSWARM_EXT_OFF : FTSWARM_EXT_I2C_MASTER; 

  serialNumber = enterNumber("Serial number [1..65535]>", 0, 1, 65535 );

  wifiMode = wifiAP;
  sprintf( wifiSSID, "ftSwarm%d", serialNumber );
  wifiPwd[0]  = '\0';

  strcpy( swarmName, wifiSSID );
  swarmSecret = generateSecret( serialNumber ); 
  swarmPIN    = serialNumber;

  // check on i2c/spi LSMR
  if ( ( CPU == FTSWARMRS_2V1 ) ||
       ( CPU == FTSWARMRC_1V141 ) ||
       ( CPU == FTSWARMCONTROL_1V3UC ) ) {
    Wire.begin( 4, 5 );
    Wire.beginTransmission( 0xD7 );
    spiGyro = (Wire.endTransmission(true) != 0);
  }

  if ( yesNo("Save configuration (Y/N)?>") ) {
    save( true );
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

	// initialize to undefined
	version            = NVSVERSION;
	serialNumber       = 0;
	CPU                = FTSWARM_NOVERSION;
	wifiSSID[0]        = '\0';
	wifiPwd[0]         = '\0';
  wifiMode           = wifiAP;
  swarmSecret        = 0xFFFF;
  swarmPIN           = 9999;
  swarmName[0]       = '\0';
  webUI              = true;
  swarmCommunication.wifi = 1;
  IAmKelda           = true;
  memset( &swarmMember, 0, sizeof( swarmMember ) );
  extensionPort      = FTSWARM_EXT_OFF;
  I2CAddr            = 0x66;
  interruptLine      = 0;
  interruptOnOff[0]  = 0;
  interruptOnOff[1]  = 255;
  I2CRegisters       = MAXI2CREGISTERS;
  gyro               = false;
  spiGyro            = false;

  // initialize zero positions
  for (uint8_t j=0;j<4;j++) {
    calibration[j].minValue = 200;
    calibration[j].midValue = 1900;
    calibration[j].maxValue = 3700;
  }

  // initialize events
  activeEventConfig = 0;
  bzero( events, sizeof( events ) );

  // initialize lables
  bzero( oledLabel, sizeof( oledLabel ) );

}

void SwOSNVS::begin() {

  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      // NVS partition was truncated and needs to be erased
      // Retry nvs_flash_init
      SWARM_LOG_ERROR( "Invalid NVS found. Erasing NVS.");
      nvs_flash_erase();
      err = nvs_flash_init();
  }

  if (!load() ) {
    initialSetup();
  }

  if (nvs.CPU != CPUFirmware ) {
    printf("\n\nFATAL: Incompatible firmware hardware settings.\n");
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
  dummy = sizeof( calibration );
  nvs_get_blob( my_handle, "calibration", &calibration, &dummy );

  // RGBLeds
  nvs_get_u8( my_handle, "RGBLeds", &pixels );
  
  // wifi
  nvs_get_u32( my_handle, "wifiMode", (uint32_t *) &wifiMode );
  nvs_get_u8 ( my_handle, "Channel",  &channel );
  dummy = sizeof( wifiSSID ); nvs_get_str( my_handle, "wifiSSID", wifiSSID, &dummy );
  dummy = sizeof( wifiPwd );  nvs_get_str( my_handle, "wifiPwd",  wifiPwd,  &dummy );

  // swarm
  nvs_get_u16( my_handle, "swarmSecret",               &swarmSecret );
  nvs_get_u16( my_handle, "swarmPIN",                  &swarmPIN );
  dummy = sizeof( swarmName );  nvs_get_str( my_handle, "swarmName", swarmName, &dummy );
  nvs_get_u8(  my_handle, "swarmSpeed",                &swarmSpeed );

  // Kelda & swarmMembers
  nvs_get_u8( my_handle, "IAmKelda", (uint8_t *) &IAmKelda );
  nvs_get_u8( my_handle, "swarmCom", &swarmCommunication.raw);
  dummy = sizeof( swarmMember );  nvs_get_blob( my_handle, "swarmMember", &swarmMember, &dummy );

  // to avoid hickups
  if ( wifiMode == wifiOFF ) swarmCommunication.wifi = 0;

  // webUI
  nvs_get_u8 ( my_handle, "webUI", (uint8_t *) &webUI );

  // ExtentionPort
  nvs_get_u32( my_handle, "extensionPort", (uint32_t *) &extensionPort);
  if ( ( CPU == FTSWARMCONTROL_1V3 ) && ( extensionPort == FTSWARM_EXT_OFF ) ) { extensionPort = FTSWARM_EXT_I2C_MASTER; } // backward compatibility
  nvs_get_u8 ( my_handle, "I2CAddr",        &I2CAddr );
  nvs_get_u8 ( my_handle, "interruptLine", &interruptLine );
  nvs_get_i16( my_handle, "interruptLow",  &interruptOnOff[0] );
  nvs_get_i16( my_handle, "interruptHigh", &interruptOnOff[1] );
  nvs_get_u8 ( my_handle, "I2CRegisters",  &I2CRegisters );
  nvs_get_u8 ( my_handle, "Gyro",          (uint8_t *) &gyro);
  nvs_get_u8 ( my_handle, "spiGyro",       (uint8_t *) &spiGyro);

  nvs_close( my_handle );

  loadEvents();

  return true;

}

void SwOSNVS::deleteAllControllers(  void ) {

  bzero( swarmMember, MAXCTRL );

}

void SwOSNVS::save( bool writeAll ) {

  // Open
  nvs_handle_t my_handle;
  if ( nvs_open(NVSNAMESPACE, NVS_READWRITE, &my_handle) != ESP_OK ) { SWARM_LOG_ERROR("NVS save: Can't open NVS."); return; };

  // Write
  if (writeAll) {
    nvs_set_i32( my_handle, "NVSVersion", NVSVERSION ) ;
    nvs_set_u16( my_handle, "serialNumber", (FtSwarmSerialNumber_t) serialNumber ) ;
    nvs_set_u32( my_handle, "CPU", (uint32_t) CPU ) ;
  }

  // factoryReset
  nvs_set_u8( my_handle, "factoryReset", factoryReset );
  
  // ftSwarmControl: set joystick calibration
  nvs_set_blob( my_handle, "calibration",  (void *)&calibration, sizeof( calibration ) );

  // RGBLeds
  nvs_set_u8( my_handle, "RGBLeds", pixels );

  // wifi
  nvs_set_u32( my_handle, "wifiMode", wifiMode );
  nvs_set_u8 ( my_handle, "Channel",  (uint8_t) channel );
  nvs_set_str( my_handle, "wifiSSID",           wifiSSID);
  nvs_set_str( my_handle, "wifiPwd",            wifiPwd);

  // swarm
  nvs_set_u16( my_handle, "swarmSecret", swarmSecret );
  nvs_set_u16( my_handle, "swarmPIN",    swarmPIN );
  nvs_set_str( my_handle, "swarmName",   swarmName );
  
  // Kelda & swarmMembers
  nvs_set_u8  ( my_handle, "IAmKelda",     (uint8_t) IAmKelda );
  nvs_set_u8  ( my_handle, "swarmCom",     swarmCommunication.raw );
  nvs_set_blob( my_handle, "swarmMember",  (void *)swarmMember, sizeof( swarmMember ) );
  nvs_set_u8  ( my_handle, "swarmSpeed",   swarmSpeed );

  // webUI
  nvs_set_u8 ( my_handle,  "webUI",   (uint8_t) webUI );

  // extensionPort
  nvs_set_u8 ( my_handle, "I2CAddr",       I2CAddr );
  nvs_set_u8 ( my_handle, "interruptLine", interruptLine );
  nvs_set_i16( my_handle, "interruptLow",  interruptOnOff[0] );
  nvs_set_i16( my_handle, "interruptHigh", interruptOnOff[1] );
  nvs_set_u8 ( my_handle, "I2CRegisters",  I2CRegisters );
  nvs_set_u32( my_handle, "extensionPort", (uint32_t) extensionPort);
  nvs_set_u8 ( my_handle, "Gyro",          (uint8_t)  gyro);
  nvs_set_u8 ( my_handle, "spiGyro",       (uint8_t)  spiGyro);

  // commit
  nvs_commit( my_handle );

  nvs_close( my_handle );

  saveEvents();

}

void SwOSNVS::saveAndRestart( void ) {

  // save settings
  save();

  // Arduino + S3-Bug
  esp_task_wdt_delete(NULL);

  // reboot
  ESP.restart();

}

void SwOSNVS::saveEvents( void ) {

  char config[16];

  // Open
  nvs_handle_t my_handle;
  if ( nvs_open( NVSNAMESPACE, NVS_READWRITE, &my_handle) != ESP_OK ) { SWARM_LOG_ERROR("Save events failed: Can't open NVS."); return; };

  // active config
  nvs_set_u8( my_handle, "activeConfig", activeEventConfig );
  
  // save events
  nvs_set_blob( my_handle, "event0", (void *)events[0], sizeof( events[0] ) );
  nvs_set_blob( my_handle, "event1", (void *)events[1], sizeof( events[1] ) );
  nvs_set_blob( my_handle, "event2", (void *)events[2], sizeof( events[2] ) );
  nvs_set_blob( my_handle, "event3", (void *)events[3], sizeof( events[3] ) );

  // save labels
  nvs_set_blob( my_handle, "label0", (void *)oledLabel[0], sizeof( oledLabel[0] ) );
  nvs_set_blob( my_handle, "label1", (void *)oledLabel[1], sizeof( oledLabel[1] ) );
  nvs_set_blob( my_handle, "label2", (void *)oledLabel[2], sizeof( oledLabel[2] ) );
  nvs_set_blob( my_handle, "label3", (void *)oledLabel[3], sizeof( oledLabel[3] ) );

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
  nvs_get_u8( my_handle, "activeConfig", &activeEventConfig );

  // load events
  dummy = sizeof( events[0] ); nvs_get_blob( my_handle, "event0", events[0], &dummy );
  dummy = sizeof( events[1] ); nvs_get_blob( my_handle, "event1", events[1], &dummy );
  dummy = sizeof( events[2] ); nvs_get_blob( my_handle, "event2", events[2], &dummy );
  dummy = sizeof( events[3] ); nvs_get_blob( my_handle, "event3", events[3], &dummy );

  // load labels
  dummy = sizeof( oledLabel[0] ); nvs_get_blob( my_handle, "label0", oledLabel[0], &dummy );
  dummy = sizeof( oledLabel[1] ); nvs_get_blob( my_handle, "label1", oledLabel[1], &dummy );
  dummy = sizeof( oledLabel[2] ); nvs_get_blob( my_handle, "label2", oledLabel[2], &dummy );
  dummy = sizeof( oledLabel[3] ); nvs_get_blob( my_handle, "label3", oledLabel[3], &dummy );

  nvs_close( my_handle );

}

void SwOSNVS::deleteAllEvents( uint8_t configuration ) {

  // set all events to "NULL"
  bzero( events[configuration], MAXNVSEVENTS * sizeof( SwOSNVSEvent ) );

}

bool SwOSNVS::addEvent( uint8_t configuration, SwOSNVSEvent *event ) {

  uint8_t i = 0;

  while ( i<MAXNVSEVENTS ) {
    
    if ( nvs.events[configuration][i].isNull() ) {
      nvs.events[configuration][i] = *event;
      return true;
    }

    i++;

  }

  return false;

}

bool SwOSNVS::exists( uint8_t configuration, SwOSNVSEvent *event ) {

  for (uint8_t i=0; i<MAXNVSEVENTS; i++ ) {
    if ( events[configuration][i].cmp( event )) return true;
  }

  return false;

}

void SwOSNVS::factorySettings( void ) {

  factoryReset = true;

  channel = 1;
  
  memset( wifiSSID, '\0', 64 );
  memset( wifiPwd,  '\0', 128 );
  
  wifiMode = wifiAP;
  sprintf( wifiSSID, "ftSwarm%d", serialNumber );
  webUI = true;

  strcpy( swarmName, wifiSSID );
  swarmSecret        = generateSecret( serialNumber ); 
  swarmPIN           = serialNumber;
  IAmKelda           = true;
  swarmCommunication.wifi = 1;
  swarmSpeed         = 4;

  for (uint8_t j=0;j<4;j++) {
    calibration[j].minValue = 200;
    calibration[j].midValue = 1900;
    calibration[j].maxValue = 3700;
  }

  pixels             = FTSWARM_HAL_PIXELS;

  extensionPort      = FTSWARM_EXT_OFF;
  I2CAddr            = 0x66;
  gyro               = false;
  // no factory settings for spiGyro

  bzero(swarmMember, sizeof(swarmMember));
  activeEventConfig  = 0;
  bzero(events,      sizeof(events));

  // initialize lables
  bzero( oledLabel, sizeof( oledLabel ) );

}

void SwOSNVS::createSwarm( char *name, uint16_t pin ) {

  // failsave copy of swarmName
  strncpy( swarmName, name, MAXIDENTIFIER ); swarmName[MAXIDENTIFIER] = '\0';
  swarmPIN = pin;

  // kill all old swarm members
  bzero( swarmMember, sizeof( swarmMember ) );

  // genenerate a really new secret
  uint16_t newSecret;
  while (1) {
    newSecret = generateSecret( serialNumber );
    if ( newSecret != swarmSecret ) break;
  }
  swarmSecret = newSecret; 
  
}

bool SwOSNVS::addController( FtSwarmSerialNumber_t serialNumber ) {

  for (int8_t i=0; i<MAXCTRL; i++ ) {

    // already in list? done.
    if ( swarmMember[i] == serialNumber ) {
      return false;
    }

  }

  for (int8_t i=0; i<MAXCTRL; i++ ) {

    // free space in list? save index
    if ( swarmMember[i] == 0 ) {
      swarmMember[i] = serialNumber;
      return true;
    };

  }

  // no free space? error
  printf("no space\n");
  return false;

}

bool SwOSNVS::deleteController( FtSwarmSerialNumber_t serialNumber ) {

  for (uint8_t i=0; i<MAXCTRL; i++) {

    // serialNumber found? kill it
    if (swarmMember[i] == serialNumber) {
      swarmMember[i] = 0;
      return true;
    }

  }

  // didn't find serialNumber? error.
  return false;

}

uint8_t SwOSNVS::swarmMembers( void ) {

  uint8_t result = 0;
  for (uint8_t i=0; i<MAXCTRL; i++ ) {
    if ( swarmMember[i] ) result++;
  }

  return result;

}

void SwOSNVS::printNVS() {

  printf( "NVSVersion: %d\n", version );
  printf( "CPU: %d\n", CPU );
  printf( "serialNumber: %d\n", serialNumber );
  printf( "wifiMode: %d\n", wifiMode );
  printf( "wifiSSID: >%s<\n", wifiSSID );
  printf( "pixels: %d\n", pixels );
  printf( "gyro: %d\n", gyro );
  printf( "spiGyro: %d\n", spiGyro );
  printf( "extensionPort: %d\n", extensionPort );
  printf( "I2CAddr: %d\n", I2CAddr );
  printf( "I2CRegisters: %d\n", I2CRegisters );
  printf( "InterruptLine: %d\n", interruptLine );
  printf( "interruptOnOff: %d %d\n", interruptOnOff[0], interruptOnOff[1] );
  printf( "swarmSecret: 0x%4X\n", swarmSecret );
  printf( "swarmPIN: %d\n", swarmPIN );
  printf( "swarmName: >%s<\n", swarmName );
  printf( "IAmKelda: %d\n", IAmKelda );
  printf( "calibration: %d %d %d %d\n", calibration[0], calibration[1], calibration[2], calibration[3] );
  printf( "swarmCommunication %d\n", swarmCommunication );
  printf( "swarmSpeed %d\n", swarmSpeed );

  printf( "swarm members:");
  for (uint8_t i=0; i<MAXCTRL; i++) { if (swarmMember[i]) printf(" %d", swarmMember[i]); }
  printf( "\n");

  printf( "events:\n" );
  printf( "activeEventConfig %d\n", activeEventConfig);

  // configs
  for ( uint8_t c=0; c<MAXEVENTCONFIGS; c++ ) {
  
    printf("Event configuration %d", c);
    if ( c== activeEventConfig ) printf(" - active -");
    printf("\n");

    // events
      for ( uint8_t i=0; i<MAXNVSEVENTS; i++ ) {

        if ( events[c][i].sensor.serialNumber != 0 ) {
          printf( "#%d input %d.%d.%d actor %d.%d.%d trigger %d parameter %d\n", 
                  i, 
                  events[c][i].sensor.serialNumber, events[c][i].sensor.ioType, events[c][i].sensor.port,
                  events[c][i].actor.serialNumber,  events[c][i].actor.ioType,  events[c][i].actor.port,
                  events[c][i].parameter,
                  events[c][i].triggerMath.raw
                );
        }
      }
  }
 
}

bool SwOSNVS::upgrade( void ) {

  if ( version != NVSVERSION ) {

    printf("[INFO] upgrading NVS setting from version %d to %d\n", version, NVSVERSION );
   
    // erase all
    nvs_handle_t my_handle;
    if ( nvs_open(NVSNAMESPACE, NVS_READWRITE, &my_handle) != ESP_OK ) SWARM_LOG_FATAL("NVS-Upgrade failed: Can't open NVS.");
    if ( nvs_erase_all( my_handle ) != ESP_OK ) SWARM_LOG_FATAL("NVS-Upgrade failed: Can't earase NVS.");
    if ( nvs_commit( my_handle ) != ESP_OK ) SWARM_LOG_FATAL("NVS-Upgrade failed: Can't commit NVS.");
    nvs_close( my_handle );
    
    // save again
    save( true );
    return true;   
  }

  return false;

}