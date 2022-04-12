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
#include "WiFi.h"

#include "ftSwarm.h"
#include "SwOSNVS.h"
#include "easykey.h"

uint16_t generateSecret( FtSwarmSerialNumber_t serialNumber ) {
  return ( ( ( ( uint16_t) serialNumber ) & 0xFF ) << 8 ) | ( rand() & 0xFF ) ;
}

void SwOSNVS::_initialSetup( void ) {

  controlerType = (FtSwarmControler_t) (enterNumber(("Controler Type\n (1) ftSwarm\n (2) ftSwarmControl\n>"), 0, 1, 2 ) - 1 );

  switch( enterNumber("CPU Version\n (1) 1V0\n (2) 1V3\n (3) 1V15\n>", 0, 1, 3) ) {
    case '1':  CPU = FTSWARM_1V0;  break;
    case '2':  CPU = FTSWARM_1V3;  break;
    case '3':  CPU = FTSWARM_1V15; break;
    default:   CPU = FTSWARM_1V0;  break;
  }

  HAT = FTSWARM_1V0;

  serialNumber = enterNumber("Serial number [1..65535]>", 0, 1, 65535 );

  APMode = true;
  if ( controlerType == FTSWARM ) {
    sprintf( wifiSSID, "ftSwarm%d", serialNumber );
  } else {
    sprintf( wifiSSID, "ftSwarmControl%d", serialNumber );
  };
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
	version       = -1;
	controlerType = FTSWARM_NOCTRL;
	serialNumber  = 0;
	CPU           = FTSWARM_NOVERSION;
	HAT           = FTSWARM_NOVERSION;
	wifiSSID[0]   = '\0';
	wifiPwd[0]    = '\0';
  APMode        = false;
  swarmSecret   = 0xFFFF;
  swarmPIN      = 9999;
  swarmName[0]  = '\0';

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

  if (!load()) {
    _initialSetup();
  }

}

bool SwOSNVS::load() {

  // Open
  nvs_handle_t my_handle;
  ESP_ERROR_CHECK( nvs_open("ftSwarm", NVS_READWRITE, &my_handle) );

  // start reading my version to check my data is valid
  nvs_get_i32( my_handle, "NVSVersion", &version);
  if ( version == -1 ) {
     return false;
  }
     
  // start with HW configuration
  nvs_get_u32( my_handle, "controlerType", (uint32_t *) &controlerType );

  nvs_get_u16( my_handle, "serialNumber",  (uint16_t *) &serialNumber );
  nvs_get_u32( my_handle, "CPU",           (uint32_t *) &CPU );
  nvs_get_u32( my_handle, "HAT",           (uint32_t *) &HAT );

  // check on valid hw data
  if ( ( controlerType == FTSWARM_NOCTRL ) ||
       ( serialNumber == 0 ) ||
       ( CPU == FTSWARM_NOVERSION ) ||
       ( HAT == FTSWARM_NOVERSION ) ) {
    return false;
  }

  // ftSwarmControl: get joystick calibration
  if ( controlerType == FTSWARMCONTROL ) {
    nvs_get_i16( my_handle, "joyZero00", &joyZero[0][0]);
    nvs_get_i16( my_handle, "joyZero01", &joyZero[0][1]);
    nvs_get_i16( my_handle, "joyZero10", &joyZero[1][0]);
    nvs_get_i16( my_handle, "joyZero11", &joyZero[1][1]);
  }
  
  size_t dummy;
       
  // wifi
  nvs_get_u8 ( my_handle, "APMode",        (uint8_t *) &APMode );
  nvs_get_u8 ( my_handle, "Channel",                   &channel );
  dummy = sizeof( wifiSSID ); nvs_get_str( my_handle, "wifiSSID", wifiSSID, &dummy );
  dummy = sizeof( wifiPwd );  nvs_get_str( my_handle, "wifiPwd",  wifiPwd,  &dummy );

  // swarm
  nvs_get_u16( my_handle, "swarmSecret",               &swarmSecret );
  nvs_get_u16( my_handle, "swarmPIN",                  &swarmPIN );
  dummy = sizeof( swarmName );  nvs_get_str( my_handle, "swarmName", swarmName, &dummy );

  return true;

}

void SwOSNVS::save( bool writeAll ) {

  // Open
  nvs_handle_t my_handle;
  ESP_ERROR_CHECK( nvs_open("ftSwarm", NVS_READWRITE, &my_handle) );

  // Write
  if (writeAll) {
    ESP_ERROR_CHECK( nvs_set_i32( my_handle, "NVSVersion", 1) );
    ESP_ERROR_CHECK( nvs_set_u32( my_handle, "controlerType", (uint32_t) controlerType ) );
    ESP_ERROR_CHECK( nvs_set_u16( my_handle, "serialNumber", (FtSwarmSerialNumber_t) serialNumber ) );
    ESP_ERROR_CHECK( nvs_set_u32( my_handle, "CPU", (uint32_t) CPU ) );
    ESP_ERROR_CHECK( nvs_set_u32( my_handle, "HAT", (uint32_t) HAT )  );
  }

  // ftSwarmControl: set joystick calibration
  if ( controlerType == FTSWARMCONTROL ) {
    ESP_ERROR_CHECK( nvs_set_i16( my_handle, "joyZero00", joyZero[0][0]) );
    ESP_ERROR_CHECK( nvs_set_i16( my_handle, "joyZero01", joyZero[0][1]) );
    ESP_ERROR_CHECK( nvs_set_i16( my_handle, "joyZero10", joyZero[1][0]) );
    ESP_ERROR_CHECK( nvs_set_i16( my_handle, "joyZero11", joyZero[1][1]) );
  }

  // wifi
  ESP_ERROR_CHECK( nvs_set_u8 ( my_handle, "APMode",   (uint8_t) APMode ) );
  ESP_ERROR_CHECK( nvs_set_u8 ( my_handle, "Channel",  (uint8_t) channel ) );
  ESP_ERROR_CHECK( nvs_set_str( my_handle, "wifiSSID",           wifiSSID)  );
  ESP_ERROR_CHECK( nvs_set_str( my_handle, "wifiPwd",            wifiPwd)  );

  // swarm
  ESP_ERROR_CHECK( nvs_set_u16( my_handle, "swarmSecret", swarmSecret ) );
  ESP_ERROR_CHECK( nvs_set_u16( my_handle, "swarmPIN",    swarmPIN ) );
  ESP_ERROR_CHECK( nvs_set_str( my_handle, "swarmName",   swarmName ) );
   
  // commit
  ESP_ERROR_CHECK( nvs_commit( my_handle ) );

}

void SwOSNVS::saveAndRestart( void ) {
  save();
  ESP.restart();
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
  printf( "HAT: %d\n", HAT );
  printf( "controlerType: %d\n", controlerType );
  printf( "serialNumber: %d\n", serialNumber );
  printf( "APMode: %d\n", APMode );
  printf( "wifiSSID: >%s<\n", wifiSSID );
  printf( "wifiPwd: >%s<\n", wifiPwd );
  printf( "swarmSecret: 0x%4X\n", swarmSecret );
  printf( "swarmPIN: %d\n", swarmPIN );
  printf( "swarmName: >%s<\n", swarmName );
 
 
}
