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
#include <WiFi.h>
#include <esp_task_wdt.h>

#include "SwOS.h"
#include "SwOSNVS.h"
#include "easyKey.h"
#include "SwOSHW/SwOSHWHAL.h"
#include "SwOSLog.h"
#include "lsm6dsr_reg.h"


SwOSNVS nvs;

bool cmpEvent( SwOSNVSEvent_t *a, SwOSNVSEvent_t *b ) {

  // 2 identical
  // 1 only parameters different
  // 0 else

  if ( ( memcmp( a, b, sizeof(SwOSIOUID_t) *2 ) == 0 ) && 
       ( a->triggerMath.bits.trigger == b->triggerMath.bits.trigger ) &&
       ( a->triggerMath.bits.op      == b->triggerMath.bits.op  ) ) {

    if ( (a->triggerMath.raw == b->triggerMath.raw) && ( a->parameter == b->parameter ) ) return 2;
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

  switch ( enterNumber(("Controller Type\n (1) ftSwarm\n (2) ftSwarmRS\n (3) ftSwarmControl\n (4) ftSwarmCAM\n (5) ftSwarmPwrDrive\n (6) ftSwarmDuino\n (7) ftSwarmXL\n (8) ftSwarmRC\n (9) special config\n>"), 0, 1, 8 ) ) {
    case 1:  controllerType = FTSWARM;         CPU = FTSWARMJST_1V15;       break;
    case 2:  controllerType = FTSWARM;         CPU = FTSWARMRS_2V1;         break;
    case 3:  controllerType = FTSWARMCONTROL;  
             #if CONFIG_IDF_TARGET_ESP32S3 
              CPU = FTSWARMCONTROL_1V3UC; 
             #else
              CPU = FTSWARMCONTROL_1V3; 
             #endif
             break;
    case 4:  controllerType = FTSWARMCAM;      CPU = FTSWARMCAM_3V12;       break;
    case 5:  controllerType = FTSWARMPWRDRIVE; CPU = FTSWARMPWRDRIVE_1V141; break;
    case 6:  controllerType = FTSWARMDUINO;    CPU = FTSWARMDUINO_1V141;    break;
    case 7:  controllerType = FTSWARM;         CPU = FTSWARMXL_1V00;        break;
    case 8:  controllerType = FTSWARM;         CPU = FTSWARMRC_1V140;       break;
    default: // manual configuration
             controllerType = (FtSwarmController_t) (enterNumber(("controller Type\n (1) ftSwarm\n (2) ftSwarmControl\n (3) ftSwarmCAM\n (4) ftSwarmPwrDrive\n (5) ftSwarmDuino\n\n>"), 0, 1, 5 ) - 1 );
             CPU = ( FtSwarmVersion_t ) ( enterNumber(("CPU Version\n (1) FTSWARMJST_1V0\n (2) FTSWARMCONTROL_1V3\n (3) FTSWARMJST_1V15\n (4) FTSWARMRS_2V0\n (5) FTSWARMRS_2V1\n (6) FTSWARMCAM_3V12\n (7) FTSWARMDUINO_1V141\n (8) FTSWARMPWRDRIVE_1V141\n (9) FTSWARMXL_1V00\n (10) FTSWARMRC_1V140\n (11) FTSWARMCONTROL_1V3UC\n"), 0, 1, 11 ) -1 );
  }

  pixels = MAXIOS[CPU].pixels;
  extensionPort = ( controllerType == FTSWARMCONTROL ) ? FTSWARM_EXT_I2C_MASTER : FTSWARM_EXT_OFF; 

  serialNumber = enterNumber("Serial number [1..65535]>", 0, 1, 65535 );

  wifiMode = wifiAP;
  sprintf( wifiSSID, "ftSwarm%d", serialNumber );
  wifiPwd[0]  = '\0';

  strcpy( swarmName, wifiSSID );
  swarmSecret = generateSecret( serialNumber ); 
  swarmPIN    = serialNumber;

  // check on i2c/spi LSMR
  if ( ( CPU == FTSWARMRS_2V0 ) ||
       ( CPU == FTSWARMRS_2V1 ) ||
       ( CPU == FTSWARMRC_1V140 ) ||
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
	controllerType     = FTSWARM_NOCTRL;
	serialNumber       = 0;
	CPU                = FTSWARM_NOVERSION;
	wifiSSID[0]        = '\0';
	wifiPwd[0]         = '\0';
  wifiMode           = wifiAP;
  swarmSecret        = 0xFFFF;
  swarmPIN           = 9999;
  swarmName[0]       = '\0';
  webUI              = true;
  swarmCommunication = SWARMCOM_WIFI;
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

  // start with HW configuration
  nvs_get_u32( my_handle, "controlerType", (uint32_t *) &controllerType );

  uint32_t ui32;
  nvs_get_u16( my_handle, "serialNumber",  (uint16_t *) &serialNumber );
  nvs_get_u32( my_handle, "CPU",           (uint32_t *) &CPU );

  // check on valid hw data
  if ( ( controllerType == FTSWARM_NOCTRL ) ||
       ( serialNumber == 0 ) ||
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
  nvs_get_u8 ( my_handle, "IAmKelda", (uint8_t *) &IAmKelda );
  nvs_get_u32( my_handle, "swarmCom", (uint32_t *) &swarmCommunication);
  dummy = sizeof( swarmMember );  nvs_get_blob( my_handle, "swarmMember", &swarmMember, &dummy );

  // webUI
  nvs_get_u8 ( my_handle, "webUI", (uint8_t *) &webUI );

  // ExtentionPort
  nvs_get_u32( my_handle, "extensionPort", (uint32_t *) &extensionPort);
  if ( ( controllerType == FTSWARMCONTROL ) && ( extensionPort == FTSWARM_EXT_OFF ) ) { extensionPort = FTSWARM_EXT_I2C_MASTER; }
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
    nvs_set_u32( my_handle, "controlerType", (uint32_t) controllerType ) ;
    nvs_set_u16( my_handle, "serialNumber", (FtSwarmSerialNumber_t) serialNumber ) ;
    nvs_set_u32( my_handle, "CPU", (uint32_t) CPU ) ;
  }

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
  nvs_set_u32 ( my_handle, "swarmCom",     swarmCommunication );
  nvs_set_blob( my_handle, "swarmMember",  (void *)&swarmMember, sizeof( swarmMember ) );
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

  for ( uint8_t i=0; i<MAXEVENTCONFIGS; i++ ) {
    sprintf( config, "config#%d", i );
    nvs_set_blob( my_handle, config, (void *)events[i], sizeof( events[i] ) );
  }

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

  // get configs
  for ( uint8_t i=0; i<MAXEVENTCONFIGS; i++ ) {
    sprintf( config, "config#%d", i );
    dummy = sizeof( events );
    nvs_get_blob( my_handle, config, events[i], &dummy );
  }

  nvs_close( my_handle );

}

bool SwOSNVS::RS485Available( void ) {
  return ( CPU == FTSWARMRS_2V0 ) || ( CPU == FTSWARMRS_2V1 ) || 
         ( CPU == FTSWARMPWRDRIVE_1V141 ) || 
         ( CPU == FTSWARMDUINO_1V141 ) || 
         ( CPU == FTSWARMXL_1V00 );
}

void SwOSNVS::factorySettings( void ) {

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
  swarmCommunication = SWARMCOM_WIFI;
  swarmSpeed         = 4;

  for (uint8_t j=0;j<4;j++) {
    calibration[j].minValue = 200;
    calibration[j].midValue = 1900;
    calibration[j].maxValue = 3700;
  }

  pixels             = MAXIOS[CPU].pixels;

  extensionPort      = FTSWARM_EXT_OFF;
  I2CAddr            = 0x66;
  gyro               = false;
  // no factory settings for spiGyro

  bzero(swarmMember, sizeof(swarmMember));
  activeEventConfig  = 0;
  bzero(events,      sizeof(events));

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
  printf( "controllerType: %d\n", controllerType );
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