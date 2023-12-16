/*
 * SwOSSwarm.cpp
 *
 * internal represenation of my swarm. Use FtSwarm-Classes in FtSwarm.h to access your swarm!
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOS.h"

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <ESPmDNS.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_task_wdt.h>

#include "SwOSNVS.h"
#include "SwOSHW.h"
#include "SwOSCom.h"
#include "SwOSSwarm.h"
#include "SwOSWeb.h"
#include "easyKey.h"

// There can only be once!
SwOSSwarm myOSSwarm;

// #define DEBUG_COMMUNICATION
// #define DEBUG_READTASK

/***************************************************
 *
 *   Backgrund tasks
 *
 ***************************************************/

void recvTask( void *parameter ) {
  // After a valid datagram is sent to me, it's stored in the recvNotification queue.
  // This tasks waits for such events and process them vai myOSSwarm.OnDataRecv.

  SwOSCom event( MacAddr( noMac ), NULL, 0 );
  
  // forever
  while (1) {

    // new data available?
    if ( xQueueReceive( myOSNetwork.recvNotification, &event, ESPNOW_MAXDELAY ) == pdTRUE ) {

      #ifdef DEBUG_COMMUNICATION
        if ( event.data.cmd != CMD_STATE ) {
          printf("\n\n-----------------------------\nmy friend sends some data...\n" );
          event.macAddr.print();
          printf("secret %04X cmd %d valid %d\n", event.data.secret, event.data.cmd, event.isValid() );
          event.print();
        }
      #endif
      
      myOSSwarm.lock();
      myOSSwarm.OnDataRecv( &event );
      myOSSwarm.unlock();

    }
    
  }
  
};

void readTask( void *parameter ) {
  // This tasks reads the value of the local inputs and sends the readings to all other controllers.

  TickType_t xDelay;

  while (true) {

    myOSSwarm.lock();
    
    // read
    myOSSwarm.Ctrl[0]->read();

    // Do I know a Kelda and I am not the Kelda, so I need to send my state
    if ( ( myOSSwarm.Kelda ) && ( myOSSwarm.Kelda != myOSSwarm.Ctrl[0] ) ) {
      
      // copy my state to a datagram
      SwOSCom *com = myOSSwarm.Ctrl[0]->state2Com( myOSSwarm.Kelda->macAddr );
      // com->print();
      com->send();
      delete com;

    }
 
    // calc delay time
    #ifdef DEBUG_READTASK
      xDelay = 2000 / portTICK_PERIOD_MS;
    #else
      xDelay = myOSSwarm.getReadDelay() / portTICK_PERIOD_MS;
    #endif
    
    myOSSwarm.unlock();
    
    vTaskDelay( xDelay );
  
  }

}

/***************************************************
 *
 *   SwOSSwarm - all controllers in the swarm.
 *
 ***************************************************/

uint16_t SwOSSwarm::nextToken( bool rotateToken ) {

  uint32_t pin      = nvs.swarmPIN;
  uint16_t newToken = ( 2 + ( lastToken ^ pin ) ) & 0xFFFF;

  if ( rotateToken ) lastToken = newToken;

  return newToken;
  
}

SwOSIO *SwOSSwarm::waitFor( char *alias, FtSwarmIOType_t ioType ) {

  SwOSIO *me = NULL;
  bool   firstTry = true;

  while (!me) {

    me = getIO( alias, ioType );

    // no success, wait 25 ms
    if ( (!me) && ( firstTry ) ) {
      printf( "Waiting on device %s. Press anykey to enter setup and change remote control settings.\n", alias );
      setState( WAITING );
      firstTry = false;
    }
    
    if (!me) vTaskDelay( 25 / portTICK_PERIOD_MS );
    
    // any key pressed?
    if ( anyKey() ) return NULL;

  }

  return me;
  
}

bool SwOSSwarm::startEvents( void ) {

  SwOSIO *sensor;
  SwOSIO *actor;
  NVSEvent *event;

  for (uint8_t i=0; i<MAXNVSEVENT; i++ ) {

    event = &nvs.eventList.event[i];
    
    if ( ( event->sensor[0] != '\0' ) && ( event->actor[0] != '\0' ) ) {

      // get IOs and stop on error
      sensor = waitFor( event->sensor, FTSWARM_INPUT ); if (!sensor) return false;
      actor  = waitFor( event->actor, FTSWARM_ACTOR );  if (!actor)  return false;

      switch ( sensor->getIOType() ) {
    
        case FTSWARM_INPUT: 
          static_cast<SwOSInput *>(sensor)->registerEvent( event->triggerEvent, actor, event->usePortValue, event->parameter ); 
          break;
    
        case FTSWARM_BUTTON: 
          static_cast<SwOSButton *>(sensor)->registerEvent( event->triggerEvent, actor, event->usePortValue, event->parameter ); 
          break;
    
        case FTSWARM_JOYSTICK: 
          if ( event->LR == 1 ) static_cast<SwOSJoystick *>(sensor)->triggerLR.registerEvent( event->triggerEvent, actor, event->usePortValue, event->parameter );
          else                  static_cast<SwOSJoystick *>(sensor)->triggerFB.registerEvent( event->triggerEvent, actor, event->usePortValue, event->parameter );
          break;
        
      }
    
    }
    
  }

  return true;

}

void SwOSSwarm::startWifi( void ) {

  // no wifi config?
  if (nvs.wifiSSID[0]=='\0') {
    if (verbose) printf("Invalid wifi configuration found. Starting AP mode.\n");
    strcpy( nvs.wifiSSID, Ctrl[0]->getHostname() );
    nvs.wifiMode = wifiAP;
  }

  // Start wifi
  setState( STARTWIFI  );

  // best practise to throw away anything during a soft reboot
  WiFi.disconnect();

  // some common stuff  
  WiFi.useStaticBuffers(true); 
  WiFi.mode(WIFI_AP_STA);

  if ( ( nvs.wifiMode == wifiAP ) || Ctrl[0]->maintenanceMode() ) {
    // work as AP in standard or maintennace cable was set
    if (verbose) printf("Create own SSID: %s\n", Ctrl[0]->getHostname());

    esp_wifi_set_ps(WIFI_PS_NONE);
    WiFi.softAPsetHostname(Ctrl[0]->getHostname());
    WiFi.softAP( nvs.wifiSSID, "", nvs.channel); // passphrase not allowed on ESP32WROOM
    
  } else {
    // normal operation
    if (verbose) printf("Attempting to connect to SSID: %s", nvs.wifiSSID);

    WiFi.setHostname(Ctrl[0]->getHostname() );
    WiFi.begin(nvs.wifiSSID, nvs.wifiPwd);

    bool keyBreak = false;
    
    // try 10 seconds to join my wifi
    for (uint8_t i=0; i<20; i++ ) {

      // connected?
      if (WiFi.status() == WL_CONNECTED) break;

      // any key ?
      keyBreak = anyKey();
      if ( keyBreak ) break;

      // user entertainment
      if (verbose) { 
        printf("."); 
        fflush(stdout);
      }

      // wait
      delay(500);
      
    }

    // any key?
    if ( keyBreak ) {
      printf( "\nStarting setup..\n" );
      mainMenu();
      ESP.restart();
    }

    // connection failed?
    if (WiFi.status() != WL_CONNECTED) {
      printf( "ERROR: Can't connect to SSID %s\n\nstarting setup...\n", nvs.wifiSSID );
      setState( ERROR );
      mainMenu();
      ESP.restart();
    }

    // register hostname
    MDNS.begin(Ctrl[0]->getHostname());

    esp_wifi_set_ps(WIFI_PS_NONE);
    
    if (verbose) printf("connected!\n");
  }

  // set mac addr of local controler
  uint8_t mac[ESP_NOW_ETH_ALEN];
  WiFi.macAddress( mac );
  Ctrl[0]->macAddr.set( mac );

  if (verbose) {
    if ( nvs.wifiMode == wifiAP )
      printf("hostname: %s\nip-address: %d.%d.%d.%d\n", Ctrl[0]->getHostname(), WiFi.softAPIP()[0], WiFi.softAPIP()[1], WiFi.softAPIP()[2], WiFi.softAPIP()[3]);
    else
      printf("hostname: %s\nip-address: %d.%d.%d.%d\n", Ctrl[0]->getHostname(), WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  }

}

FtSwarmSerialNumber_t SwOSSwarm::begin( bool verbose ) {

  if (initialized) return Ctrl[0]->serialNumber;

  this->verbose = verbose;

  printf("\n\nftSwarmOS " ); 
  printf(SWOSVERSION);
  printf("\n\n(C) Christian Bergschneider & Stefan Fuss\n\nPress any key to enter bios settings.\n");

  // set watchdog to 30s
  esp_task_wdt_init(30, false);
  
  // initialize random
  srand( time( NULL ) );

  // initialize nvs
  nvs.begin();
  if ( ( nvs.IAmKelda ) && ( this->verbose ) ) { printf( "I am KELDA!\n"); }

	// create local controler
	maxCtrl++;
  switch (nvs.controlerType) {
  case FTSWARM:         Ctrl[maxCtrl] = new SwOSSwarmJST( nvs.serialNumber, noMac, true, nvs.CPU, nvs.IAmKelda, nvs.RGBLeds, nvs.extentionPort );
                        break;
	case FTSWARMCONTROL:  Ctrl[maxCtrl] = new SwOSSwarmControl( nvs.serialNumber, noMac, true, nvs.CPU, nvs.IAmKelda, nvs.joyZero, nvs.displayType );
                        break;
	case FTSWARMCAM:      Ctrl[maxCtrl] =new SwOSSwarmCAM( nvs.serialNumber, noMac, true, nvs.CPU, nvs.IAmKelda );
                        break;
	case FTSWARMDUINO:    Ctrl[maxCtrl] = new SwOSSwarmDuino( nvs.serialNumber, noMac, true, nvs.CPU, nvs.IAmKelda );
                        break;
	case FTSWARMPWRDRIVE: Ctrl[maxCtrl] = new SwOSSwarmPwrDrive( nvs.serialNumber, noMac, true, nvs.CPU, nvs.IAmKelda );
                        break;
	case FTSWARMXL:       Ctrl[maxCtrl] = new SwOSSwarmXL( nvs.serialNumber, noMac, true, nvs.CPU, nvs.IAmKelda, nvs.RGBLeds, nvs.extentionPort );
                        break;
  default:              // wrong setup
                        nvs.initialSetup();
                        break;
  } 

  if ( nvs.IAmKelda ) Kelda = Ctrl[0];

  // Who I am?
  printf("Boot %s (SN:%d).\n", Ctrl[maxCtrl]->getHostname(), Ctrl[maxCtrl]->serialNumber );

  // Open NVS again & load alias names
  nvs_handle_t my_handle;
  ESP_ERROR_CHECK( nvs_open("ftSwarm", NVS_READWRITE, &my_handle) );
  myOSSwarm.Ctrl[0]->loadAliasFromNVS( my_handle );

  // now I can visualize my state
  setState( BOOTING );

  // wifi
  if ( nvs.wifiMode != wifiOFF ) startWifi( );

  // Init Communication
  if (!myOSNetwork.begin( nvs.swarmSecret, nvs.swarmPIN, nvs.swarmCommunication )) {
    if (verbose) printf("Error initializing swarm communication.\n");
    setState( ERROR );
    return 0;
  }

  // start the tasks
  xTaskCreatePinnedToCore( recvTask, "RecvTask", 10000, NULL, 1, NULL, 0 );
  xTaskCreatePinnedToCore( readTask, "ReadTask", 20000, NULL, 1, NULL, 0 );
  
  // now I need to find some friends
  registerMe( );

  // start web server
  if ( ( nvs.webUI ) && ( nvs.wifiMode != wifiOFF ) ) SwOSStartWebServer();

  // firmware events?
  if ( !startEvents( ) ) {
      printf( "\nStarting setup..\n" );
      mainMenu();
      ESP.restart();
    }

  delay(1000);
  myOSSwarm.lock();
  setState( RUNNING );
  myOSSwarm.unlock();

  if (verbose) printf("Start normal operation.\n");

  initialized = true;
  return Ctrl[0]->serialNumber;

}

void SwOSSwarm::halt( void ) {

  for (uint8_t i=0; i<maxCtrl; i++)
    Ctrl[i]->halt();
}

void SwOSSwarm::unsubscribe( void ) {

  for (uint8_t i=0; i<maxCtrl; i++)
    Ctrl[i]->unsubscribe( true );

}

uint8_t SwOSSwarm::getIndex( FtSwarmSerialNumber_t serialNumber ) {

  uint8_t i = 0;
  uint8_t f = maxCtrl+1;  // free index

  // check on existing controler
  while ( i<=maxCtrl) {
    
    // controler found?
    if ( ( Ctrl[i] ) && ( Ctrl[i]->serialNumber == serialNumber ) ) return i;

    // free index foud?
    if ( ( !Ctrl[i] ) && ( i < f) ) f = i;

    // go on
    i++;
  }

  // new player in town, check if I need to increase my high water mark
  if ( f > maxCtrl ) maxCtrl = f;

  return f;
  
 }

SwOSIO* SwOSSwarm::getIO( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmIOType_t ioType ) {

  SwOSIO *IO = NULL;
  
  // check on valid controler
  uint8_t i = getIndex( serialNumber );

  if ( Ctrl[i] ) IO = Ctrl[i]->getIO( ioType, port );

  if ( IO ) {

    // everything is fine...
    if ( ( ioType == FTSWARM_UNDEF ) || ( IO->getIOType() == ioType ) ) return IO;

    // test, if the controller could change the IOType
    if ( IO->isInUse() ) {
      printf("ERROR: Can't change IO Type. %s.%s is in use.\n", Ctrl[i]->getName(), IO->getName() );
      setState( ERROR );
    }

    if ( Ctrl[i]->changeIOType( port, IO->getIOType(), ioType ) ) return Ctrl[i]->getIO( ioType, port );

  }

  return IO;
  
}

SwOSIO* SwOSSwarm::getIO( const char *name, FtSwarmIOType_t ioType ) {

  SwOSIO *IO;

  for ( uint8_t i=0; i<=maxCtrl; i++ ) {

    if ( Ctrl[i] ) {

      IO = Ctrl[i]->getIO( name );
      
      if ( IO ) {

        // everything is fine...
        if ( ( ioType == FTSWARM_UNDEF ) || ( IO->getIOType() == ioType ) ) return IO;

        // test, if the controller could change the IOType
        if ( IO->isInUse() ) {
          printf("ERROR: Can't change IO Type. %s.%s is in use.\n", Ctrl[i]->getName(), IO->getName() );
          setState( ERROR );
          while (1) delay(50);
        }

        if ( Ctrl[i]->changeIOType( IO->getPort(), IO->getIOType(), ioType ) ) return Ctrl[i]->getIO( name );
        
        // not possible at all
        return NULL;
        
      }

    }
    
  }

  // nothing found
  return NULL;
  
}

void *SwOSSwarm::getControler(char *name) {

	// search controler
	for (uint8_t i=0;i<=maxCtrl;i++) {
		if ( ( Ctrl[i] ) && ( Ctrl[i]->equals(name) ) ) {
			return (void *) Ctrl[i];
		}
	}

	// no hit
	return NULL;

}


void SwOSSwarm::jsonize( JSONize *json) {

	json->startArray( NULL );

	for (uint8_t i=0; i<=maxCtrl;i++) {

    // send data
    if ( Ctrl[i] ) Ctrl[i]->jsonize( json, i );

    // visualize others only if I'm a Kelda
    if ( !Ctrl[0]->IAmAKelda ) break;
    
	}

	json->endArray();

}

void SwOSSwarm::getToken( JSONize *json) {

  lastToken = rand();

  json->startObject();
  json->variableUI16( "token", lastToken );
  json->endObject();
  
}

bool SwOSSwarm::splitID( char *id, uint8_t *index, char *io, size_t sizeIO) {

	// split id "12-LED1" into i=12 and io="LED1"

	uint16_t i = 0;
	char    *x = id;

	// search for delimiter
	while ( *x != '-' ) {

		if ( *x == '\0')               return false; // no delimiter found
		if ( (*x < '0') || (*x >'9') ) return false; // not a digit
		if ( ( x - id ) > 3 )          return false; // more than 3 digits is senseless

		// I know it's a digit, so I can calc i
		i = i*10 + (*x - '0');

		// next chat in string
		x++;
	}

	// some additional checks
	if ( x == id )     return false;  // no i found e.g. "-LED1"
	if ( i > maxCtrl ) return false;  // i needs to reference an existing Ctrl

	// skip -
	x++;
	if ( *x == '\0' )         return false; // no IO;
	if ( strlen(x) > sizeIO ) return false; // IO to long

	strcpy( io, x );

	// return index
	*index = i;

	return true;
} 

uint16_t SwOSSwarm::apiIsAuthorized( uint16_t token, bool rotateToken ) {

  uint16_t n = nextToken(rotateToken);

  if ( token != n ) return 401;

  return 200;
}

bool SwOSSwarm::apiPeekIsAuthorized( uint16_t token ) {
  return token == lastToken;
}

uint16_t SwOSSwarm::apiActorCmd( uint16_t token, char *id, int cmd, bool rotateToken ) {
// send an actor's command (from api)

	uint8_t i;
	char    io[20];

  // Token error
  if ( token != nextToken(rotateToken) ) return 401;

	// split ID to device and nr
	if (!splitID(id, &i, io, sizeof(io))) return 400;

	// execute cmd
  if ( ( Ctrl[i] ) && ( Ctrl[i]->apiActorCmd( io, cmd ) ) ) return 200;

  return 400;

}

uint16_t SwOSSwarm::apiActorSpeed( uint16_t token, char *id, int speed, bool rotateToken ) {
// send an actor's speed(from api)

  uint8_t i;
  char    io[20];

  // Token error
  if ( token != nextToken(rotateToken) ) return 401;

  // split ID to device and nr
  if (!splitID(id, &i, io, sizeof(io))) return 400;

  // execute cmd
  if ( ( Ctrl[i] ) && ( Ctrl[i]->apiActorSpeed( io, speed ) ) ) return 200;

  return 400;
  
}

uint16_t SwOSSwarm::apiLEDBrightness( uint16_t token, char *id, int brightness, bool rotateToken ) {
  // send a LED command (from api)

	uint8_t i;
	char    io[20];

  // Token error
  if ( token != nextToken(rotateToken) ) return 401;

	// split ID to device and nr
	if (!splitID(id, &i, io, sizeof(io))) return 400;

	// execute cmd
	if ( ( Ctrl[i] ) && ( Ctrl[i]->apiLEDBrightness( io, brightness) ) ) return 200;
  
  return 400;

}

uint16_t SwOSSwarm::apiLEDColor( uint16_t token, char *id, int color, bool rotateToken ) {
  // send a LED command (from api)

  uint8_t i;
  char    io[20];

  // Token error
  if ( token != nextToken(rotateToken) ) return 401;

  // split ID to device and nr
  if (!splitID(id, &i, io, sizeof(io))) return 400;

  // execute cmd
  if ( ( Ctrl[i] ) && ( Ctrl[i]->apiLEDColor( io, color ) ) ) return 200;
  
  return 400;

}

uint16_t SwOSSwarm::apiServoOffset( uint16_t token, char *id, int offset, bool rotateToken ) {
  // send a Servo command (from api)
  
	uint8_t i;
	char    io[20];

  // Token error
  if ( token != nextToken(rotateToken) ) return 401;

	// split ID to device and nr
	if (!splitID(id, &i, io, sizeof(io))) return 400;

	// execute cmd
	if ( ( Ctrl[i] ) && ( Ctrl[i]->apiServoOffset( io, offset  ) ) ) return 200;

  return 400;
  
}

uint16_t SwOSSwarm::apiServoPosition( uint16_t token, char *id, int position, bool rotateToken) {
  // send a Servo command (from api)
  
  uint8_t i;
  char    io[20];

  // Token error
  if ( token != nextToken(rotateToken) ) return 401;

  // split ID to device and nr
  if (!splitID(id, &i, io, sizeof(io))) return 400;

  // execute cmd
  if ( ( Ctrl[i] ) && ( Ctrl[i]->apiServoPosition( io, position ) ) ) return 200;

  return 400;
  
}

uint16_t SwOSSwarm::apiCAMStreaming( uint16_t token, char *id, int onOff, bool rotateToken) {
  
  uint8_t i;
  char    io[20];

  // Token error
  if ( token != nextToken(rotateToken) ) return 401;

  // split ID to device and nr
  if (!splitID(id, &i, io, sizeof(io))) return 400;

  // execute cmd
  if ( ( Ctrl[i] ) && ( Ctrl[i]->apiCAMStreaming( io, onOff > 0 ) ) ) return 200;

  return 400;
  
}

uint16_t SwOSSwarm::apiCAMFramesize( uint16_t token, char *id, int framesize, bool rotateToken) {
  
  uint8_t i;
  char    io[20];

  // Token error
  if ( token != nextToken(rotateToken) ) return 401;

  // split ID to device and nr
  if (!splitID(id, &i, io, sizeof(io))) return 400;

  // execute cmd
  if ( ( Ctrl[i] ) && ( Ctrl[i]->apiCAMFramesize( io, framesize ) ) ) return 200;

  return 400;
  
}

uint16_t SwOSSwarm::apiCAMQuality( uint16_t token, char *id, int quality, bool rotateToken ) {
  
  uint8_t i;
  char    io[20];

  // Token error
  if ( token != nextToken(rotateToken) ) return 401;

  // split ID to device and nr
  if (!splitID(id, &i, io, sizeof(io))) return 400;

  // execute cmd
  if ( ( Ctrl[i] ) && ( Ctrl[i]->apiCAMQuality( io, quality ) ) ) return 200;

  return 400;
  
}

uint16_t SwOSSwarm::apiCAMBrightness( uint16_t token, char *id, int brightness, bool rotateToken ) {
  
  uint8_t i;
  char    io[20];

  // Token error
  if ( token != nextToken(rotateToken) ) return 401;

  // split ID to device and nr
  if (!splitID(id, &i, io, sizeof(io))) return 400;

  // execute cmd
  if ( ( Ctrl[i] ) && ( Ctrl[i]->apiCAMBrightness( io, brightness ) ) ) return 200;

  return 400;
  
}

uint16_t SwOSSwarm::apiCAMContrast( uint16_t token, char *id, int contrast, bool rotateToken ) {
  
  uint8_t i;
  char    io[20];

  // Token error
  if ( token != nextToken(rotateToken) ) return 401;

  // split ID to device and nr
  if (!splitID(id, &i, io, sizeof(io))) return 400;

  // execute cmd
  if ( ( Ctrl[i] ) && ( Ctrl[i]->apiCAMContrast( io, contrast ) ) ) return 200;

  return 400;
  
}

uint16_t SwOSSwarm::apiCAMSaturation( uint16_t token, char *id, int saturation, bool rotateToken ) {
  
  uint8_t i;
  char    io[20];

  // Token error
  if ( token != nextToken(rotateToken) ) return 401;

  // split ID to device and nr
  if (!splitID(id, &i, io, sizeof(io))) return 400;

  // execute cmd
  if ( ( Ctrl[i] ) && ( Ctrl[i]->apiCAMSaturation( io, saturation ) ) ) return 200;

  return 400;
  
}

uint16_t SwOSSwarm::apiCAMSpecialEffect( uint16_t token, char *id, int specialEffect, bool rotateToken ) {
  
  uint8_t i;
  char    io[20];

  // Token error
  if ( token != nextToken(rotateToken) ) return 401;

  // split ID to device and nr
  if (!splitID(id, &i, io, sizeof(io))) return 400;

  // execute cmd
  if ( ( Ctrl[i] ) && ( Ctrl[i]->apiCAMSpecialEffect( io, specialEffect ) ) ) return 200;

  return 400;
  
}

uint16_t SwOSSwarm::apiCAMWbMode( uint16_t token, char *id, int wbMode, bool rotateToken ) {
  
  uint8_t i;
  char    io[20];

  // Token error
  if ( token != nextToken(rotateToken) ) return 401;

  // split ID to device and nr
  if (!splitID(id, &i, io, sizeof(io))) return 400;

  // execute cmd
  if ( ( Ctrl[i] ) && ( Ctrl[i]->apiCAMWbMode( io, wbMode ) ) ) return 200;

  return 400;
  
}

uint16_t SwOSSwarm::apiCAMHMirror( uint16_t token, char *id, int hMirror, bool rotateToken ) {
  
  uint8_t i;
  char    io[20];

  // Token error
  if ( token != nextToken(rotateToken) ) return 401;

  // split ID to device and nr
  if (!splitID(id, &i, io, sizeof(io))) return 400;

  // execute cmd
  if ( ( Ctrl[i] ) && ( Ctrl[i]->apiCAMHMirror( io, hMirror>0 ) ) ) return 200;

  return 400;
  
}

uint16_t SwOSSwarm::apiCAMVFlip( uint16_t token, char *id, int vFlip, bool rotateToken ) {
  
  uint8_t i;
  char    io[20];

  // Token error
  if ( token != nextToken(rotateToken) ) return 401;

  // split ID to device and nr
  if (!splitID(id, &i, io, sizeof(io))) return 400;

  // execute cmd
  if ( ( Ctrl[i] ) && ( Ctrl[i]->apiCAMVFlip( io, vFlip>0 ) ) ) return 200;

  return 400;
  
}



void SwOSSwarm::setState( SwOSState_t state ) {

  if (Ctrl[0]) Ctrl[0]->setState( state, members(), nvs.wifiSSID );

}


/***************************************************
 *
 *   Locking...
 *
 ***************************************************/

void SwOSSwarm::lock() {
  xSemaphoreTake( _xAccessLock, portMAX_DELAY );
};

void SwOSSwarm::unlock() { 
  xSemaphoreGive( _xAccessLock );
};


/***************************************************
 *
 *   Communication...
 *
 ***************************************************/

void SwOSSwarm::registerMe( void ) {

  // register myself
  SwOSCom com( MacAddr( broadcast ), broadcastSN, CMD_ANYBODYOUTTHERE );
  Ctrl[0]->registerMe( &com );
  com.send();

}

int haltan =20;

void SwOSSwarm::OnDataRecv(SwOSCom *com) {
  // callback function receiving data from other controllers

  // ignore invalid data
  if (!com) return;

  uint8_t source   = getIndex( com->data.sourceSN );
  uint8_t affected = getIndex( com->data.affectedSN );

  // check on join message
  if ( com->data.cmd == CMD_SWARMJOIN ) {

    #ifdef DEBUG_COMMUNICATION
      printf( "CMD_SWARMJOIN PIN %d swarmName %s\n", com->data.joinCmd.pin, com->data.joinCmd.swarmName );
    #endif

    // pin and swarm name ok?
    if ( ( nvs.swarmPIN == com->data.joinCmd.pin ) && ( strcmp( com->data.joinCmd.swarmName, nvs.swarmName ) == 0 ) ) {
      // send acknowledge
      #ifdef DEBUG_COMMUNICATION
        printf( "CMD_SWARMJOIN accepted.\n" ); 
      #endif
      SwOSCom ack( com->macAddr, com->data.sourceSN, CMD_SWARMJOINACK );
      ack.data.joinCmd.pin = nvs.swarmPIN;
      ack.data.joinCmd.swarmSecret = nvs.swarmSecret;
      strcpy( ack.data.joinCmd.swarmName, nvs.swarmName );
      ack.data.secret = DEFAULTSECRET;
      ack.send();
      return;
    }

  } // end CMD_SWARMJOIN

  // acknowledge join?
  if ( com->data.cmd == CMD_SWARMJOINACK ) {
    
    #ifdef DEBUG_COMMUNICATION
      printf( "CMD_SWARMJOINACK new secret: %04X old pin %d new pin %d allowPairing %d\n", com->data.joinCmd.swarmSecret, nvs.swarmPIN, com->data.joinCmd.pin, allowPairing );
    #endif
    
    // ack's PIN needs to be the same as my pin and I need to be in pairing mode
    // if ( ( com->data.registerCmd.pin == nvs.swarmPIN ) && allowPairing ) { 
    if ( allowPairing ) {
      nvs.swarmSecret = com->data.joinCmd.swarmSecret;
      myOSNetwork.setSecret( nvs.swarmSecret, nvs.swarmPIN );
    }
    
    return;

  } // end CMD_SWARMJOINACK

  if ( com->data.cmd == CMD_SWARMLEAVE ) {

    // someone leaves the swarm
    if ( Ctrl[source] ) { 
      // I know this controler and kill it
      delete Ctrl[source];
      Ctrl[source] = NULL;
    }

    setState( RUNNING );
    return;

  } // end CMD_SWARMLEAVE

  // check, on welcome messages if I'm a Kelda or a Kelda is asking
  if ( ( ( com->data.cmd == CMD_ANYBODYOUTTHERE ) || ( com->data.cmd == CMD_GOTYOU ) ) &&
       ( ( com->data.registerCmd.IAmAKelda ) || ( Ctrl[0]->IAmAKelda ) ) ) {

    #ifdef DEBUG_COMMUNICATION
      printf( "register msg %d source: %d affected: %d maxCtrl %d\n", com->data.cmd, source, affected, maxCtrl );
    #endif

    // check on unkown controler
    if ( !Ctrl[source] ) {

      #ifdef DEBUG_COMMUNICATION
      printf( "add a new controler type %d at %d\n", com->data.registerCmd.ctrlType, source);
      #endif

      // test, if the new controler is a Kelda and there is already a Kelda in my swarm
      if ( ( com->data.registerCmd.IAmAKelda ) && ( Ctrl[0]->IAmAKelda ) ) {
        setState( ERROR );
        printf("ERROR: Multiple Keldas found! %d %d\n", Kelda->serialNumber, com->data.sourceSN );
        return;
      }

      myOSNetwork.AddPeer( com->macAddr );

      // add to swarm
      switch (com->data.registerCmd.ctrlType) {
        case FTSWARM:         Ctrl[source] = new SwOSSwarmJST     ( com ); break;
        case FTSWARMCONTROL:  Ctrl[source] = new SwOSSwarmControl ( com ); break;
        case FTSWARMCAM:      Ctrl[source] = new SwOSSwarmCAM     ( com ); break;
        case FTSWARMPWRDRIVE: Ctrl[source] = new SwOSSwarmPwrDrive( com ); break;
        // case FTSWARMDUINO:    Ctrl[source] = new SwOSSwarmDuino   ( com ); break;
        case FTSWARMXL:       Ctrl[source] = new SwOSSwarmXL      ( com ); break;
        default: ESP_LOGW( LOGFTSWARM, "Unknown controler type while adding a new controller to my swarm." ); return;
      } 

      if ( Ctrl[source]->IAmAKelda ) {
        // register Kelda
        if (verbose) { printf("Kelda %s with MAC ", Ctrl[source]->getHostname() ); Ctrl[source]->macAddr.print(); printf(" joined the swarm \n"); }
        Kelda = Ctrl[source];
      } 

      // update oled display
      setState( RUNNING );
      
    }

    // reply GOTYOU, if needed
    if ( com->data.cmd == CMD_ANYBODYOUTTHERE ) {
      
      // frist introduce myself
      SwOSCom reply( com->macAddr, com->data.sourceSN, CMD_GOTYOU );
      Ctrl[0]->registerMe( &reply );
      reply.print();
      reply.send();

      // send my alias names, if the new controler is a Kelda
      if ( com->data.registerCmd.IAmAKelda ) Ctrl[0]->sendAlias( com->macAddr );

    }

  } // end welcome messages
 
  // any other type of msg will be processed on controler level
  if ( Ctrl[affected] ) {
    Ctrl[affected]->OnDataRecv( com );
  } 

}

void SwOSSwarm::leaveSwarm( void ) {

  // Prepare a leave message
  SwOSCom leaveMsg( MacAddr( broadcast ), Ctrl[0]->serialNumber, CMD_SWARMLEAVE );
  leaveMsg.send();

  // change network pin to avoid swarm self healings
  myOSNetwork.setSecret( 0, 0 );

  // now I could kill all swarm members
  for (uint8_t i=1; i<maxCtrl; i++) {
    if (Ctrl[i]) delete Ctrl[i];
    Ctrl[i] = NULL;
  }
  maxCtrl = 0;

}

void SwOSSwarm::joinSwarm( bool createNewSwarm, char * newName, uint16_t newPIN ) {

  // setup new swarm
  nvs.createSwarm( newName, newPIN );
  myOSNetwork.setSecret( DEFAULTSECRET, nvs.swarmPIN );

  // send CMD_SWARMJOIN
  SwOSCom joinMsg( MacAddr( broadcast ), broadcastSN, CMD_SWARMJOIN );
  joinMsg.data.joinCmd.pin = newPIN;
  strcpy( joinMsg.data.joinCmd.swarmName, newName );
  joinMsg.send();

}

void SwOSSwarm::reload( void ) {

  leaveSwarm();
  nvs.load();
  myOSNetwork.setSecret( nvs.swarmSecret, nvs.swarmPIN );
  registerMe( );

}

uint8_t SwOSSwarm::members( void ) {

  uint8_t members = 0;

  for (uint8_t i=0; i<=maxCtrl; i++ ) {
    if ( Ctrl[i] ) members++;
  }

  return members;
  
}
