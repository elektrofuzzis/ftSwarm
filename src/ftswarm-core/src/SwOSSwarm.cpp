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

// #define DEBUG_COMMUNICATION_SWARM
// #define DEBUG_READTASK

// time to wait in Connect-Thread
#define CONNECTDELAY 500

/***************************************************
 *
 *   Backgrund tasks
 *
 ***************************************************/

static void recvTask( void *parameter ) {
  // After a valid datagram is sent to me, it's stored in the recvNotification queue.
  // This tasks waits for such events and process them vai myOSSwarm.OnDataRecv.

  SwOSCom event( MacAddr( noMac ), NULL, 0 );
  
  // forever
  while (1) {

    // new data available?
    if ( xQueueReceive( myOSNetwork.recvNotification, &event, ESPNOW_MAXDELAY ) == pdTRUE ) {

      #ifdef DEBUG_COMMUNICATION_SWARM
        if ( event.data.cmd != CMD_STATE ) {
          printf("\n\n-----------------------------\nmy friend sends some data...\n" );
          event.macAddr.print();
          printf("cmd %d valid %d\n", event.data.cmd, event.isValid() );
          event.print();
        }
      #endif
      
      myOSSwarm.OnDataRecv( &event );

    }
    
  }
  
};

static void readTask( void *parameter ) {
  // This tasks reads the value of the local inputs and sends the readings to all other controllers.

  TickType_t xDelay;
  SwOSCom    *com = NULL;

  while (true) {

    // lock
    myOSSwarm.Ctrl[0]->lock();
    
    // read sensors
    myOSSwarm.Ctrl[0]->read();

    // Do I know a Kelda and I am not the Kelda, so I need to send my state
    if ( ( myOSSwarm.Kelda ) && ( myOSSwarm.Kelda != myOSSwarm.Ctrl[0] ) ) {
      
      // copy my state to a datagram
      com = myOSSwarm.Ctrl[0]->state2Com( myOSSwarm.Kelda->macAddr );

    }

    // first unlock myself
    myOSSwarm.Ctrl[0]->unlock();
    
    // send data and cleanup
    if (com) { 
      com->send();
      delete com; 
      com = NULL; }
    
    // calc delay time
    #ifdef DEBUG_READTASK
      xDelay = 2000 / portTICK_PERIOD_MS;
    #else
      xDelay = myOSSwarm.getReadDelay() / portTICK_PERIOD_MS;
    #endif
 
    vTaskDelay( xDelay );
  
  }

}

static void connectTask( void *Parameter ) {

  while (1) {

    myOSSwarm.connect( );
    vTaskDelay( CONNECTDELAY / portTICK_PERIOD_MS );

  }

}

/***************************************************
 *
 *   SwOSSwarm - all controllers in the swarm.
 *
 ***************************************************/

// Start status: OFFLINE
// if OFFLINE or longer not seen -> CONNECT_PHASE1, send a connect-request
// if Member denies by sending NAK -> ERROR
// if Member sends GOTYOU -> CONNECT_PHASE2
// if Member sends ALIASe -> ONLINE

void SwOSSwarm::connect( void ) {

  // if I'm not the Kelda, just check if Kelda is online
  if (!Ctrl[0]->IAmKelda) {
    if ( ( Kelda ) && ( Kelda->networkAge() > 1000L ) ) Kelda->setComState( COMSTATE_UNDEFINED );
    return;
  }

  for (uint8_t i=1; i<=maxCtrl; i++) {

    if ( Ctrl[i] ) { 

      //if controller was not seen for a longer time or is new: try to reconnect
      if ( ( Ctrl[i]->getComState() == COMSTATE_UNDEFINED ) ||
           ( ( Ctrl[i]->networkAge() > 1000L ) && ( Ctrl[i]->getComState() != COMSTATE_ERROR ) ) ) {
      
        Ctrl[i]->setComState( COMSTATE_CONNECT_PHASE1 );
        joinMySwarm( MacAddr( broadcast ), Ctrl[i]->serialNumber );

      // if it's online send him an hart beat
      } else {
        
        SwOSCom hartBeat( Ctrl[i]->macAddr, Ctrl[i]->serialNumber, CMD_HARTBEAT );
        hartBeat.send();

      }

    }

  }

}


uint16_t SwOSSwarm::nextToken( bool rotateToken ) {

  uint16_t newToken = ( 2 + ( lastToken ^ nvs.swarmPIN ) ) & 0xFFFF;

  if ( rotateToken ) lastToken = newToken;

  return newToken;
  
}

SwOSIO *SwOSSwarm::waitFor( char *alias ) {

  SwOSIO *me = NULL;
  bool   firstTry = true;

  while (!me) {

    me = getIO( alias );

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

  // test if I'm not a Kelda, I won't start the events
  if ( !Ctrl[0]->IAmKelda ) return true;

  for (uint8_t i=0; i<MAXNVSEVENT; i++ ) {

    event = &nvs.eventList.event[i];
    
    if ( ( event->sensor[0] != '\0' ) && ( event->actor[0] != '\0' ) ) {

      // get IOs and stop on error
      sensor = waitFor( event->sensor );  
      if (!sensor) return false;
      if (!sensor->isInput()) return false;
      
      actor  = waitFor( event->actor );  
      if (!actor)            return false;
      if (!actor->isMotor()) return false;

      if ( sensor->getIOType() == SWOSIO_JOYSTICK ) {
     
        if ( event->LR == 1 ) static_cast<SwOSJoystick *>(sensor)->triggerLR.registerEvent( event->triggerEvent, actor, event->usePortValue, event->parameter );
        else                  static_cast<SwOSJoystick *>(sensor)->triggerFB.registerEvent( event->triggerEvent, actor, event->usePortValue, event->parameter );
     
      } else {

        static_cast<SwOSInput *>(sensor)->registerEvent( event->triggerEvent, actor, event->usePortValue, event->parameter ); 

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

  if ( nvs.wifiMode == wifiAP ) {
    // work as AP in standard 
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
      printf( "\e[0;31mERROR: Can't connect to SSID %s\e[0m\n\nstarting setup...\n", nvs.wifiSSID );
      setState( ERROR );
      mainMenu();
      ESP.restart();
    }

    // register hostname
    MDNS.begin(Ctrl[0]->getHostname());

    esp_wifi_set_ps(WIFI_PS_NONE);
    
    if (verbose) printf("connected!\n");
  }

  // set mac addr of local controller
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

  // Who I am?
  if (this->verbose) {
    printf("Boot %s (SN:%d).\n", nvs.swarmName, nvs.serialNumber );
    if ( nvs.IAmKelda )  { printf( "I am KELDA!\n"); }
  }

SwOSCtrlConfig_t localCtrlConfig = {
    .ctrlType      = nvs.controllerType,
    .CPU           = nvs.CPU,
    .IAmKelda      = nvs.IAmKelda,
    .extensionPort = nvs.extensionPort,
    .IOs           = 0,
    .pixels        = nvs.pixels,
    .gyro          = nvs.gyro
  };

  memcpy( &localCtrlConfig.zero, &nvs.joyZero, sizeof(nvs.joyZero) );

  // initial setup?
  if (nvs.controllerType >= FTSWARM_MAXCONTROLLERTYPE ) nvs.initialSetup();

  maxCtrl = 0;
  Ctrl[0] = new SwOSCtrl( nvs.serialNumber, noMac, true, localCtrlConfig );
  Ctrl[0]->setComState ( COMSTATE_ONLINE );

  SwOSCtrlConfig_t noCtrlConfig;
  bzero( &noCtrlConfig, sizeof(noCtrlConfig) );
  noCtrlConfig.ctrlType = FTSWARM_NOCTRL;
  noCtrlConfig.CPU      = FTSWARM_NOVERSION;

  // initialize all swarm members from nvs list
  for (uint8_t i=0; i<MAXCTRL; i++) {
    
    if ( nvs.swarmMember[i] ) {
      maxCtrl++;
      Ctrl[maxCtrl] = new SwOSCtrl( nvs.swarmMember[i],  MacAddr( broadcast ), false, noCtrlConfig );
    }

  }

  // set Kelda link if i'm the Kelda
  if ( nvs.IAmKelda ) Kelda = Ctrl[0];

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
    if (verbose) printf("\e[0;31mError initializing swarm communication.\e[0;31m\n");
    setState( ERROR );
    return 0;
  }

  // start the tasks
  xTaskCreatePinnedToCore( recvTask,    "RecvTask",    10000, NULL, 1, NULL, 0 );
  xTaskCreatePinnedToCore( readTask,    "ReadTask",    20000, NULL, 1, NULL, 0 );
  xTaskCreatePinnedToCore( connectTask, "connectTask", 10000, NULL, 1, NULL, 0 );

  // start web server
  if ( ( nvs.webUI ) && ( nvs.wifiMode != wifiOFF ) ) SwOSStartWebServer();

  // firmware events?
  if ( !startEvents( ) ) {
      printf( "\nStarting setup..\n" );
      mainMenu();
      ESP.restart();
    }

  delay(1000);
  setState( RUNNING );

  if (verbose) printf("Start normal operation.\n");



  if ( ( nvs.IAmKelda) && ( nvs.wifiMode == wifiAP ) ) 
    printf("\n\n\e[0;31m*** WARNING ***:\nA swarm using wifi ap mode provided by the Kelda isn't stable.\nBest practice is to use your local wifi or to provide the AP via a swarm member.\e[0m\n\n");

  initialized = true;
  return Ctrl[0]->serialNumber;

}

void SwOSSwarm::halt( void ) {

  for (uint8_t i=0; i<=maxCtrl; i++)
    Ctrl[i]->halt();
}

void SwOSSwarm::unsubscribe( void ) {
  for (uint8_t i=0; i<=maxCtrl; i++)
    if ( Ctrl[i] )
      Ctrl[i]->unsubscribe( true );
}

uint8_t SwOSSwarm::getIndex( FtSwarmSerialNumber_t serialNumber ) {

  uint8_t i = 0;
  uint8_t f = maxCtrl+1;  // free index

  // check on existing controller
  while ( i<=maxCtrl) {
    
    // controller found?
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

SwOSIO* SwOSSwarm::getIO( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType ) {

  // check on valid controller
  SwOSCtrl *ctrl = Ctrl[ getIndex( serialNumber ) ];
  if (!ctrl) return NULL;

  // get an io candidate
  SwOSIO *io = ctrl->getIO( ioType, port );
  if (!io) return NULL;

  // if no special type is required, take it as it is
  if ( ioType == SWOSIO_UNDEF ) return io;

  // 100% match?
  if ( ( io->getIOType() == ioType ) && ( io->getPort() == port ) ) return io;

  // compatible ioType?
  uint8_t index = ctrl->getIndex( io );
  if (!ctrl->changeIOType( index, ioType ) ) return NULL;

  // return corrected io
  return ctrl->io[index];

}

SwOSIO* SwOSSwarm::getIO( const char *name, SwOSIOType_t ioType ) {

  SwOSIO   *io   = NULL;
  SwOSCtrl *ctrl = NULL;

  // list all controllers and check for the name
  for ( uint8_t i=0; i<=maxCtrl; i++ ) {

    // check next controller
    ctrl = Ctrl[i];
    if ( ctrl ) io = ctrl->getIO( name );

    // io found
    if (io) break;

  }

  // nothing found?
  if (!io) return NULL;

  // if no special type is required, take it as it is
  if ( ioType == SWOSIO_UNDEF ) return io;

  // 100% match?
  if ( io->getIOType() == ioType ) return io;

  // compatible ioType?
  uint8_t index = ctrl->getIndex( io );
  if (!ctrl->changeIOType( index, ioType ) ) return NULL;

  // return corrected io
  return ctrl->io[index];
    
  // nothing found
  return NULL;
  
}

void *SwOSSwarm::getController(char *name) {

	// search controller
	for (uint8_t i=0;i<=maxCtrl;i++) {
		if ( ( Ctrl[i] ) && ( Ctrl[i]->equals(name) ) ) {
			return (void *) Ctrl[i];
		}
	}

	// no hit
	return NULL;

}

void *SwOSSwarm::getController( FtSwarmSerialNumber_t SN ) {

	// search controller
	for (uint8_t i=0;i<=maxCtrl;i++) {
		if ( ( Ctrl[i] ) && ( Ctrl[i]->serialNumber == SN ) ) {
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
    if ( Ctrl[i] ) { Ctrl[i]->lock(); Ctrl[i]->jsonize( json, i ); Ctrl[i]->unlock(); }

    // visualize others only if I'm a Kelda
    if ( !Ctrl[0]->IAmKelda ) break;
    
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
  if ( Ctrl[i] ) {
    
    Ctrl[i]->lock();
    bool ok = Ctrl[i]->apiActorCmd( io, cmd );
    Ctrl[i]->unlock();
     
    if ( ok ) return 200;

  }

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
  if ( Ctrl[i] ) {
    
    Ctrl[i]->lock();
    bool ok = Ctrl[i]->apiActorSpeed( io, speed );
    Ctrl[i]->unlock();
     
    if ( ok ) return 200;

  }

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
  if ( Ctrl[i] ) {
    
    Ctrl[i]->lock();
    bool ok = Ctrl[i]->apiLEDBrightness( io, brightness) ;
    Ctrl[i]->unlock();
     
    if ( ok ) return 200;

  }

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
  if ( Ctrl[i] ) {
    
    Ctrl[i]->lock();
    bool ok = Ctrl[i]->apiLEDColor( io, color );
    Ctrl[i]->unlock();
     
    if ( ok ) return 200;

  }
  
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
  if ( Ctrl[i] ) {
    
    Ctrl[i]->lock();
    bool ok = Ctrl[i]->apiServoOffset( io, offset  );
    Ctrl[i]->unlock();
     
    if ( ok ) return 200;

  }

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
  if ( Ctrl[i] ) {
    
    Ctrl[i]->lock();
    bool ok = Ctrl[i]->apiServoPosition( io, position );
    Ctrl[i]->unlock();
     
    if ( ok ) return 200;

  }

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
  if ( Ctrl[i] ) {

    Ctrl[i]->lock();
    bool ok = Ctrl[i]->apiCAMStreaming( io, onOff > 0 );
    Ctrl[i]->unlock();

    if (ok) return 200;

  }

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
  if ( Ctrl[i] ) {

    Ctrl[i]->lock();
    bool ok = Ctrl[i]->apiCAMFramesize( io, framesize );
    Ctrl[i]->unlock();

    if ( ok ) return 200;

  }

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
  if ( Ctrl[i] ) {
    
    Ctrl[i]->lock();
    bool ok = Ctrl[i]->apiCAMQuality( io, quality );
    Ctrl[i]->unlock();
     
    if ( ok ) return 200;

  }

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
  if ( Ctrl[i] ) {
    
    Ctrl[i]->lock();
    bool ok = Ctrl[i]->apiCAMBrightness( io, brightness );
    Ctrl[i]->unlock();
     
    if ( ok ) return 200;

  }

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
  if ( Ctrl[i] ) {
    
    Ctrl[i]->lock();
    bool ok = Ctrl[i]->apiCAMContrast( io, contrast );
    Ctrl[i]->unlock();
     
    if ( ok ) return 200;

  }

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
  if ( Ctrl[i] ) {
    
    Ctrl[i]->lock();
    bool ok = Ctrl[i]->apiCAMSaturation( io, saturation );
    Ctrl[i]->unlock();
     
    if ( ok ) return 200;

  }

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
  if ( Ctrl[i] ) {
    
    Ctrl[i]->lock();
    bool ok = Ctrl[i]->apiCAMSpecialEffect( io, specialEffect );
    Ctrl[i]->unlock();
     
    if ( ok ) return 200;

  }

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
  if ( Ctrl[i] ) {
    
    Ctrl[i]->lock();
    bool ok = Ctrl[i]->apiCAMWbMode( io, wbMode ) ;
    Ctrl[i]->unlock();
     
    if ( ok ) return 200;

  }

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
  if ( Ctrl[i] ) {
    
    Ctrl[i]->lock();
    bool ok = Ctrl[i]->apiCAMHMirror( io, hMirror>0 ) ;
    Ctrl[i]->unlock();
     
    if ( ok ) return 200;

  }

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
  if ( Ctrl[i] ) {
    
    Ctrl[i]->lock();
    bool ok = Ctrl[i]->apiCAMVFlip( io, vFlip>0 ) ;
    Ctrl[i]->unlock();
     
    if ( ok ) return 200;

  }

  return 400;
  
}


void SwOSSwarm::setState( SwOSState_t state ) {

  if (Ctrl[0]) {
    Ctrl[0]->lock();
    Ctrl[0]->setState( state, members(), nvs.wifiSSID );
    Ctrl[0]->unlock();
  }

}

/***************************************************
 *
 *   Communication...
 *
 ***************************************************/

void SwOSSwarm::joinMySwarm( MacAddr destinationMac, FtSwarmSerialNumber_t destinationSN ) {
  // As a Kelda send CMD_JOINMYSWARM to a potential member

  // register myself
  SwOSCom com( destinationMac, destinationSN, CMD_JOINMYSWARM );
  Ctrl[0]->registerMe( &com );
  com.send();

}

void SwOSSwarm::replaceCtrl( SwOSCom *com, uint8_t source, uint8_t affected ) {
  // replace controller in swarm list
      
  SwOSCtrl *newCtrl = NULL;
  SwOSCtrl *oldCtrl = Ctrl[source];
  
  if ( com->data.registerCmd.ctrlConfig.ctrlType >= FTSWARM_MAXCONTROLLERTYPE ) {
    ESP_LOGW( LOGFTSWARM, "Unknown controller type while adding a new controller to my swarm." ); return;

  } else {
    
    newCtrl = new SwOSCtrl( com->data.sourceSN , com->macAddr, false, com->data.registerCmd.ctrlConfig );
    
    if (verbose) { 
      if ( com->data.registerCmd.ctrlConfig.IAmKelda ) printf("Kelda ");
      printf("\n[Info] ftSwarm%d joined the swarm.\n", com->data.sourceSN ); 
    }

  }

  // replace the new controller in my list
  newCtrl->setComState( COMSTATE_ONLINE );
  Ctrl[source] = newCtrl;
  if (oldCtrl) delete oldCtrl; 
  
}

void SwOSSwarm::cmdJoinMySwarm( SwOSCom *com, uint8_t source, uint8_t affected ) {

  #ifdef DEBUG_COMMUNICATION_SWARM
    printf( "CMD_JOINMYSWARM %d source: %d affected: %d maxCtrl %d\n", com->data.cmd, source, affected, maxCtrl );
  #endif

  // not my SN and not a wildcard: ignore
  if ( ( com->data.affectedSN != Ctrl[0]->serialNumber ) && ( com->data.affectedSN != 0 ) ) return;

  // I'm a Kelda with at leat a member: decline
  if ( ( Ctrl[0]->IAmKelda ) && ( members() > 1 ) ) {

    printf("[ERROR] Declining to join swarm %s. I'm a Kelda with %d swarm members.\n", com->data.joinCmd.swarmName, members() );
    SwOSCom reply( com->macAddr, com->data.sourceSN, CMD_JOINNACK );
    Ctrl[0]->registerMe( &reply );
    reply.send();

    return;

  }

  // I'm fine to join the swarm: ack

  // take swarm settings
  nvs.swarmPIN = com->data.registerCmd.swarmPIN;
  strcpy( nvs.swarmName, com->data.registerCmd.swarmName );

  // replace old controller
  replaceCtrl( com, source, affected );

  // getting member, knowing my Kelda
  Ctrl[0]->IAmKelda = false;
  nvs.IAmKelda = false;
  Kelda = Ctrl[source];

  // Send my data      
  SwOSCom reply( com->macAddr, com->data.sourceSN, CMD_JOINACK );
  Ctrl[0]->registerMe( &reply );
  reply.send();

  // send my alias names as well
  if ( com->data.registerCmd.ctrlConfig.IAmKelda ) Ctrl[0]->sendIOConfig( com->macAddr ); 

  // update status
  setState( RUNNING );

}

void SwOSSwarm::cmdJoinNAck( SwOSCom *com, uint8_t source, uint8_t affected ) {
  // Member to Kelda: I don't want to join your Swarm
  
  if ( Ctrl[source] ) Ctrl[source]->setComState( COMSTATE_ERROR ); 

}

void SwOSSwarm::cmdJoinAck( SwOSCom *com, uint8_t source, uint8_t affected ) {
  // Member to Kelda: I want to join your Swarm
  
  if ( Ctrl[source] )  {

    // if it's an unkown controller, update controller data
    if ( Ctrl[source]->getType() == FTSWARM_NOCTRL ) replaceCtrl( com, source, affected );
    // ToDo else - send the controller his state

    // wait for alias settings
    Ctrl[source]->setComState( COMSTATE_CONNECT_PHASE2 ); 

  }

}

void SwOSSwarm::cmdRevokeFromSwarm( SwOSCom *com, uint8_t source, uint8_t affected ) {
  // Kelda to member: get out of my swarm

  // for me?
  if ( ( com->data.affectedSN != Ctrl[0]->serialNumber ) || (com->data.joinCmd.pin == nvs.swarmPIN) || strcmp( com->data.registerCmd.swarmName, nvs.swarmName ) ) return;

  // User info
  printf("\n\n[INFO] leaving swarm %s and reboot.\n\n", com->data.registerCmd.swarmName );

  // just reboot
  ESP.restart();

}
  


void SwOSSwarm::OnDataRecv(SwOSCom *com) {
  // callback function receiving data from other controllers

  // ignore invalid data
  if (!com) return;

  // get affected controllers
  uint8_t source   = getIndex( com->data.sourceSN );
  uint8_t affected = getIndex( com->data.affectedSN );

  switch ( com->data.cmd ) {
    case CMD_JOINMYSWARM:     cmdJoinMySwarm( com, source, affected ); 
                              break;

    case CMD_REVOKEFROMSWARM: cmdRevokeFromSwarm( com, source, affected );
                              break;

    case CMD_JOINACK:         cmdJoinAck( com, source, affected ); 
                              break;

    case CMD_JOINNACK:        cmdJoinNAck( com, source, affected ); 
                              break;

    case CMD_HARTBEAT:        if ( Ctrl[source] ) Ctrl[source]->tick();
                              break;

    default:                  if ( Ctrl[affected] ) {
                                // any other type of msg will be processed on controller level
                                Ctrl[affected]->lock();
                                Ctrl[affected]->OnDataRecv( com );
                                Ctrl[affected]->unlock();
                              }
                              break;
  
  }

}

uint8_t SwOSSwarm::members( void ) {

  uint8_t members = 0;

  for (uint8_t i=0; i<=maxCtrl; i++ ) {
    if ( Ctrl[i] ) members++;
  }

  return members;
  
}

void SwOSSwarm::newSwarm( void ) {

  // block conneting new controllers
  Ctrl[0]->lock();
  Ctrl[0]->IAmKelda = false;
  Ctrl[0]->unlock();

  // delete old swarm members
  int8_t oldMaxCtrl = maxCtrl;
  maxCtrl = 0;
  for (int8_t i=1; i<=oldMaxCtrl; i++) {
    
    if ( Ctrl[i] != NULL ) {  
      nvs.deleteController( Ctrl[i]->serialNumber );
      SwOSCtrl *oldCtrl = Ctrl[i]; 
      Ctrl[i] = NULL;
      oldCtrl->lock();
      delete oldCtrl;
    }
    
  }

  // set new swarm
  nvs.IAmKelda = true;
  Ctrl[0]->lock();
  Ctrl[0]->IAmKelda = true;
  Ctrl[0]->unlock(); 

}

bool SwOSSwarm::isMember( FtSwarmSerialNumber_t serialNumber ) { 
  // Test, if SN is part my my Swarm

  for (uint8_t i=0; i<=maxCtrl; i++) {
    if ( (Ctrl[i] ) && ( Ctrl[i]->serialNumber == serialNumber ) ) return true;
  }

  return false;

}

bool SwOSSwarm::isOnline( FtSwarmSerialNumber_t serialNumber ) {
  // Test, if SN is online

  for (uint8_t i=0; i<=maxCtrl; i++) {
    if ( ( Ctrl[i] ) && ( Ctrl[i]->serialNumber == serialNumber ) && ( Ctrl[i]->isOnline( ) ) ) return true;
  }

  return false;

}

bool SwOSSwarm::addController( FtSwarmSerialNumber_t serialNumber ) {
  // add Controller SN to the swarm

  // get a slot in the controller list
  uint8_t i = getIndex( serialNumber );

  // no slot available?
  if ( i >= MAXCTRL ) return false;
  
  // is SN already added?
  if ( Ctrl[i] != NULL ) return true;

  // add new Controller to the list

  SwOSCtrlConfig_t noCtrlConfig;
  bzero( &noCtrlConfig, sizeof(noCtrlConfig) );
  noCtrlConfig.ctrlType = FTSWARM_NOCTRL;
  noCtrlConfig.CPU      = FTSWARM_NOVERSION;

  Ctrl[i] = new SwOSCtrl( serialNumber,  MacAddr( broadcast ), false, noCtrlConfig );
  nvs.addController( serialNumber );

  delay( CONNECTDELAY );

  return true;

}

bool SwOSSwarm::deleteController( FtSwarmSerialNumber_t serialNumber ) {
  // delete Controller SN

  // get index in the controller list
  uint8_t i = getIndex( serialNumber );

  // not found?
  if ( ( i >= MAXCTRL ) || ( Ctrl[i] == NULL ) ) return false;

  if ( Ctrl[i]->isOnline() ) {
    // Hoecker, you're out
    SwOSCom com( Ctrl[i]->macAddr, Ctrl[i]->serialNumber, CMD_REVOKEFROMSWARM );
    Ctrl[0]->registerMe( &com );
    com.send();
  }

  // delete
  SwOSCtrl *oldCtrl = Ctrl[i]; 
  Ctrl[i] = NULL;
  oldCtrl->lock();
  delete oldCtrl;
  nvs.deleteController( serialNumber );
 
  return true;

}