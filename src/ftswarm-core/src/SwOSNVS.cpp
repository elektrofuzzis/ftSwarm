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

#include "SwOS.h"
#include "SwOSNVS.h"
#include "easyKey.h"

SwOSNVS nvs;

NVSEvent::NVSEvent() {
  sensor[0] = '\0';
  actor[0]  = '\0';
}

uint16_t generateSecret( FtSwarmSerialNumber_t serialNumber ) {
  return ( ( ( ( uint16_t) serialNumber ) & 0xFF ) << 8 ) | ( rand() & 0xFF ) ;
}

void SwOSNVS::initialSetup( void ) {

  version = 2;

  switch ( enterNumber(("Controler Type\n (1) ftSwarm\n (2) ftSwarmRS\n (3) ftSwarmControl\n (4) ftSwarmCAM\n (5) ftSwarmPwrDrive\n (6) ftSwarmDuino\n (8) special config\n>"), 0, 1, 7 ) ) {
    case 1:  controlerType = FTSWARM;         CPU = FTSWARMJST_1V15;       break;
    case 2:  controlerType = FTSWARM;         CPU = FTSWARMRS_2V1;         break;
    case 3:  controlerType = FTSWARMCONTROL;  CPU = FTSWARMCONTROL_1V3;    break;
    case 4:  controlerType = FTSWARMCAM;      CPU = FTSWARMCAM_2V11;       break;
    case 5:  controlerType = FTSWARMPWRDRIVE; CPU = FTSWARMPWRDRIVE_1V141; break;
    case 6:  controlerType = FTSWARMDUINO;    CPU = FTSWARMDUINO_1V141;    break;
    default: // manual configuration
             controlerType = (FtSwarmControler_t) (enterNumber(("Controler Type\n (1) ftSwarm\n (2) ftSwarmControl\n (3) ftSwarmCAM\n (4) ftSwarmPwrDrive\n (5) ftSwarmDuino\n>"), 0, 1, 5 ) - 1 );
             CPU = ( FtSwarmVersion_t ) ( enterNumber(("CPU Version\n (1) FTSWARMJST_1V0\n (2) FTSWARMCONTROL_1V3\n (3) FTSWARMJST_1V15\n (4) FTSWARMRS_2V0\n (5) FTSWARMRS_2V1\n (6) FTSWARMCAM_1V0\n (7) FTSWARMDUINO_1V141\n (8) FTSWARMPWRDRIVE_1V141\n"), 0, 1, 8 ) -1 );
  }

  serialNumber = enterNumber("Serial number [1..65535]>", 0, 1, 65535 );

  wifiMode = wifiAP;
  sprintf( wifiSSID, "ftSwarm%d", serialNumber );
  wifiPwd[0]  = '\0';

  strcpy( swarmName, wifiSSID );
  swarmSecret = generateSecret( serialNumber ); 
  swarmPIN    = serialNumber;

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
	version            = -1;
	controlerType      = FTSWARM_NOCTRL;
	serialNumber       = 0;
	CPU                = FTSWARM_NOVERSION;
	wifiSSID[0]        = '\0';
	wifiPwd[0]         = '\0';
  wifiMode           = wifiAP;
  swarmSecret        = 0xFFFF;
  swarmPIN           = 9999;
  swarmName[0]       = '\0';
  webUI              = true;
  swarmCommunication = swarmComWifi;
  IAmKelda           = false;
  memset( &swarmMembers, 0, sizeof( swarmMembers ) );
  extentionPort      = FTSWARM_EXT_OFF;
  I2CAddr            = 0x66;
  gyro               = false;

  // initialize zero positions
  for (uint8_t i=0;i<2;i++)
    for (uint8_t j=0;j<2;j++)
      joyZero[i][j]=2000;

}

void SwOSNVS::begin() {

  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      // NVS partition was truncated and needs to be erased
      // Retry nvs_flash_init
      ESP_LOGW( LOGFTSWARM, "Invalid NVS found. Erasing NVS.");
      ESP_ERROR_CHECK(nvs_flash_erase());
      err = nvs_flash_init();
  }
  ESP_ERROR_CHECK( err );

   if (!load() ) {
    initialSetup();
   }

}

bool SwOSNVS::load() {

  // Open
  nvs_handle_t my_handle;
  ESP_ERROR_CHECK( nvs_open("ftSwarm", NVS_READWRITE, &my_handle) );

  // start reading my version to check my data is valid
  nvs_get_i32( my_handle, "NVSVersion", &version);
  if ( version <= 1 ) {
    // no version data, or old version 1
    return false;
  }
     
  // start with HW configuration
  nvs_get_u32( my_handle, "controlerType", (uint32_t *) &controlerType );

  uint32_t ui32;
  nvs_get_u16( my_handle, "serialNumber",  (uint16_t *) &serialNumber );
  nvs_get_u32( my_handle, "CPU",           (uint32_t *) &CPU );

  // check on valid hw data
  if ( ( controlerType == FTSWARM_NOCTRL ) ||
       ( serialNumber == 0 ) ||
       ( CPU == FTSWARM_NOVERSION ) ) {
    return false;
  }

  // joystick zero position & # RGB Leds

  // ftSwarmControl
  if ( controlerType == FTSWARMCONTROL ) {
    nvs_get_i16( my_handle, "joyZero00", &joyZero[0][0]);
    nvs_get_i16( my_handle, "joyZero01", &joyZero[0][1]);
    nvs_get_i16( my_handle, "joyZero10", &joyZero[1][0]);
    nvs_get_i16( my_handle, "joyZero11", &joyZero[1][1]);
    nvs_get_u8(  my_handle, "displayType", &displayType);
    RGBLeds = 0;

  // ftSwarm
  } else {
    nvs_get_u8( my_handle, "RGBLeds", &RGBLeds );
    if ( ( RGBLeds < 2 ) || ( RGBLeds > MAXLEDS ) ) RGBLeds = 2;

  }
  
  size_t dummy;
       
  // wifi
  nvs_get_u32( my_handle, "wifiMode", (uint32_t *) &wifiMode );
  nvs_get_u8 ( my_handle, "Channel",  &channel );
  dummy = sizeof( wifiSSID ); nvs_get_str( my_handle, "wifiSSID", wifiSSID, &dummy );
  dummy = sizeof( wifiPwd );  nvs_get_str( my_handle, "wifiPwd",  wifiPwd,  &dummy );

  // swarm
  nvs_get_u16( my_handle, "swarmSecret",               &swarmSecret );
  nvs_get_u16( my_handle, "swarmPIN",                  &swarmPIN );
  dummy = sizeof( swarmName );  nvs_get_str( my_handle, "swarmName", swarmName, &dummy );

  // events
  dummy = sizeof( eventList );  nvs_get_blob( my_handle, "eventList", &eventList, &dummy );

  // Kelda & swarmMembers
  nvs_get_u8 ( my_handle, "IAmKelda", (uint8_t *) &IAmKelda );
  nvs_get_u32( my_handle, "swarmCom", (uint32_t *) &swarmCommunication);
  dummy = sizeof( swarmMembers );  nvs_get_blob( my_handle, "swarmMembers", &swarmMembers, &dummy );

  // webUI
  nvs_get_u8 ( my_handle, "webUI", (uint8_t *) &webUI );

  // extentionPort
  nvs_get_u8 ( my_handle, "I2CAddr", &I2CAddr );
  nvs_get_u32( my_handle, "extentionPort", (uint32_t *) &extentionPort);
  nvs_get_u8 ( my_handle, "Gyro",    (uint8_t *) &gyro);

  return true;

}

void SwOSNVS::save( bool writeAll ) {

  // Open
  nvs_handle_t my_handle;
  ESP_ERROR_CHECK( nvs_open("ftSwarm", NVS_READWRITE, &my_handle) );

  // Write
  if (writeAll) {
    ESP_ERROR_CHECK( nvs_set_i32( my_handle, "NVSVersion", version ) );
    ESP_ERROR_CHECK( nvs_set_u32( my_handle, "controlerType", (uint32_t) controlerType ) );
    ESP_ERROR_CHECK( nvs_set_u16( my_handle, "serialNumber", (FtSwarmSerialNumber_t) serialNumber ) );
    ESP_ERROR_CHECK( nvs_set_u32( my_handle, "CPU", (uint32_t) CPU ) );
  }

  // ftSwarmControl: set joystick calibration
  if ( controlerType == FTSWARMCONTROL ) {
    ESP_ERROR_CHECK( nvs_set_i16( my_handle, "joyZero00", joyZero[0][0]) );
    ESP_ERROR_CHECK( nvs_set_i16( my_handle, "joyZero01", joyZero[0][1]) );
    ESP_ERROR_CHECK( nvs_set_i16( my_handle, "joyZero10", joyZero[1][0]) );
    ESP_ERROR_CHECK( nvs_set_i16( my_handle, "joyZero11", joyZero[1][1]) );
    ESP_ERROR_CHECK( nvs_set_u8(  my_handle, "displayType", displayType) );
    
  } else { // FtSwarm RGBLeds
    ESP_ERROR_CHECK( nvs_set_u8( my_handle, "RGBLeds", RGBLeds ) );
  }

  // wifi
  ESP_ERROR_CHECK( nvs_set_u32( my_handle, "wifiMode", wifiMode ) );
  ESP_ERROR_CHECK( nvs_set_u8 ( my_handle, "Channel",  (uint8_t) channel ) );
  ESP_ERROR_CHECK( nvs_set_str( my_handle, "wifiSSID",           wifiSSID)  );
  ESP_ERROR_CHECK( nvs_set_str( my_handle, "wifiPwd",            wifiPwd)  );

  // swarm
  ESP_ERROR_CHECK( nvs_set_u16( my_handle, "swarmSecret", swarmSecret ) );
  ESP_ERROR_CHECK( nvs_set_u16( my_handle, "swarmPIN",    swarmPIN ) );
  ESP_ERROR_CHECK( nvs_set_str( my_handle, "swarmName",   swarmName ) );

  // events
  ESP_ERROR_CHECK( nvs_set_blob( my_handle, "eventList",  (void *)&eventList, sizeof( eventList ) ) );
   
  // Kelda & swarmMembers
  ESP_ERROR_CHECK( nvs_set_u8 ( my_handle,  "IAmKelda",     (uint8_t) IAmKelda ) );
  ESP_ERROR_CHECK( nvs_set_u32( my_handle,  "swarmCom",     swarmCommunication ) );
  ESP_ERROR_CHECK( nvs_set_blob( my_handle, "swarmMembers", (void *)&swarmMembers, sizeof( swarmMembers ) ) );

  // webUI
  ESP_ERROR_CHECK( nvs_set_u8 ( my_handle,  "webUI",   (uint8_t) webUI ) );

  // extentionPort
  ESP_ERROR_CHECK( nvs_set_u8 ( my_handle, "I2CAddr", I2CAddr ) );
  ESP_ERROR_CHECK( nvs_set_u32( my_handle, "extentionPort", (uint32_t) extentionPort) );
  ESP_ERROR_CHECK( nvs_set_u8 ( my_handle, "Gyro",    (uint8_t)  gyro) );


  // commit
  ESP_ERROR_CHECK( nvs_commit( my_handle ) );

}

void SwOSNVS::saveAndRestart( void ) {
  save();
  ESP.restart();
}

bool SwOSNVS::RS485Available( void ) {
  return ( CPU == FTSWARMRS_2V0 ) || ( CPU == FTSWARMRS_2V1 ) || ( CPU == FTSWARMPWRDRIVE_1V141 ) || ( CPU == FTSWARMDUINO_1V141 );
}

void SwOSNVS::factorySettings( void ) {

  channel = 1;
  
  memset( wifiSSID, '\0', 64 );
  memset( wifiPwd,  '\0', 128 );
  
  wifiMode = wifiAP;
  sprintf( wifiSSID, "ftSwarm%d", serialNumber );

  strcpy( swarmName, wifiSSID );
  swarmSecret = generateSecret( serialNumber ); 
  swarmPIN    = serialNumber;

  displayType = 1;
  RGBLeds = 2;

}

void SwOSNVS::createSwarm( char *name, uint16_t pin ) {

  // failsave copy of swarmName
  strncpy( swarmName, name, MAXIDENTIFIER ); swarmName[MAXIDENTIFIER] = '\0';
  swarmPIN = pin;
  swarmSecret = generateSecret( serialNumber ); 
  
}

void SwOSNVS::printNVS() {

  printf( "NVSVersion: %d\n", version );
  printf( "CPU: %d\n", CPU );
  printf( "controlerType: %d\n", controlerType );
  printf( "serialNumber: %d\n", serialNumber );
  printf( "wifiMode: %d\n", wifiMode );
  printf( "wifiSSID: >%s<\n", wifiSSID );
  printf( "wifiPwd: >%s<\n", wifiPwd );
  printf( "swarmSecret: 0x%4X\n", swarmSecret );
  printf( "swarmPIN: %d\n", swarmPIN );
  printf( "swarmName: >%s<\n", swarmName );
  printf( "IAmKelda: %d\n", IAmKelda );
  printf( "swarmCommunication %d\n", swarmCommunication );
  printf( "RGBLeds: %d\n", RGBLeds );
  printf( "displayType: %d\n", displayType );
 
 
}
