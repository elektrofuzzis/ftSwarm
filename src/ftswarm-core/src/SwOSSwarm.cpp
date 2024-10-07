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

#include <debug.h>

#define CONNECTDELAY 2500

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

      #ifdef DEBUG_COMMUNICATION
        if ( (  event.data.cmd != CMD_STATE ) && ( event.data.cmd != CMD_ALIAS ) ) {
          printf("\n\n-----------------------------\nmy friend sends some data...\n" );
          event.macAddr.print();
          printf("secret %04X cmd %d valid %d\n", event.data.secret, event.data.cmd, event.isValid() );
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

void SwOSSwarm::connect( void ) {

  if (!nvs.IAmKelda) return;

  for (uint8_t i=1; i<=maxCtrl; i++) {

    if ( ( Ctrl[i] ) && ( Ctrl[i]->networkAge() > 1000L ) ) Ctrl[i]->comState = ASKFORDETAILS;

    if ( ( Ctrl[i] ) && ( Ctrl[i]->comState == ASKFORDETAILS ) ) {
      
      registerMe( MacAddr( broadcast ), Ctrl[i]->serialNumber );
      vTaskDelay( 75 / portTICK_PERIOD_MS );

    }

  }

}

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

  // test if I'm not a Kelda, I won't start the events
  if ( !Ctrl[0]->IAmKelda ) return true;

  for (uint8_t i=0; i<MAXNVSEVENT; i++ ) {

    event = &nvs.eventList.event[i];
    
    if ( ( event->sensor[0] != '\0' ) && ( event->actor[0] != '\0' ) ) {

      // get IOs and stop on error
      sensor = waitFor( event->sensor, FTSWARM_UNDEF );  
      if (!sensor) return false;
      if (!sensor->isSensor()) return false;
      
      actor  = waitFor( event->actor,  FTSWARM_UNDEF );  
      if (!actor)  return false;
      if (!actor->isActor()) return false;

      switch ( sensor->getIOType() ) {
    
        case FTSWARM_INPUT: 
          static_cast<SwOSInput *>(sensor)->registerEvent( event->triggerEvent, actor, event->usePortValue, event->parameter ); 
          break;
    
        case FTSWARM_DIGITALINPUT: 
          static_cast<SwOSDigitalInput *>(sensor)->registerEvent( event->triggerEvent, actor, event->usePortValue, event->parameter ); 
          break;
    
        case FTSWARM_ANALOGINPUT: 
          static_cast<SwOSAnalogInput *>(sensor)->registerEvent( event->triggerEvent, actor, event->usePortValue, event->parameter ); 
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
  if ( ( nvs.IAmKelda ) && ( this->verbose ) ) { printf( "I am KELDA!\n"); }

	// create local controller
	maxCtrl++;
  switch (nvs.controllerType) {
  case FTSWARM:         Ctrl[maxCtrl] = new SwOSSwarmJST( nvs.serialNumber, noMac, true, nvs.CPU, nvs.IAmKelda, nvs.extensionPort );
                        break;
	case FTSWARMCONTROL:  Ctrl[maxCtrl] = new SwOSSwarmControl( nvs.serialNumber, noMac, true, nvs.CPU, nvs.IAmKelda, nvs.joyZero, nvs.displayType );
                        break;
	case FTSWARMCAM:      Ctrl[maxCtrl] = new SwOSSwarmCAM( nvs.serialNumber, noMac, true, nvs.CPU, nvs.IAmKelda );
                        break;
	case FTSWARMDUINO:    Ctrl[maxCtrl] = new SwOSSwarmDuino( nvs.serialNumber, noMac, true, nvs.CPU, nvs.IAmKelda );
                        break;
	case FTSWARMPWRDRIVE: Ctrl[maxCtrl] = new SwOSSwarmPwrDrive( nvs.serialNumber, noMac, true, nvs.CPU, nvs.IAmKelda );
                        break;
  default:              // wrong setup
                        nvs.initialSetup();
                        break;
  }
  Ctrl[0]->comState = UP;

  // initialize all swarm members from nvs list
  for (uint8_t i=0; i<MAXCTRL; i++) {
    
    if ( nvs.swarmMember[i] ) {
      maxCtrl++;
      Ctrl[maxCtrl] = new SwOSCtrl( nvs.swarmMember[i],  MacAddr( broadcast ), false, FTSWARM_NOVERSION, false, FTSWARM_EXT_OFF );
      Ctrl[maxCtrl]->comState = ASKFORDETAILS;
    }

  }

  // set Kelda link if i'm the Kelda
  if ( nvs.IAmKelda ) Kelda = Ctrl[0];

  // Who I am?
  printf("Boot %s (SN:%d).\n", Ctrl[0]->getHostname(), Ctrl[0]->serialNumber );

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
    printf("\n\n*** WARNING ***:\nA swarm using wifi ap mode provided by the Kelda isn't stable.\nBest practice is to use your local wifi or to provide the AP via a swarm member.\n\n");

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

SwOSIO* SwOSSwarm::getIO( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmIOType_t ioType ) {

  SwOSIO *IO = NULL;
  
  // check on valid controller
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

void SwOSSwarm::registerMe( MacAddr destinationMac, FtSwarmSerialNumber_t destinationSN ) {

  // register myself
  SwOSCom com( destinationMac, destinationSN, CMD_ANYBODYOUTTHERE );
  Ctrl[0]->registerMe( &com );
  com.send();

}

void SwOSSwarm::cmdJoin( SwOSCom *com, uint8_t source, uint8_t affected ) {

  #ifdef DEBUG_COMMUNICATION
    printf( "CMD_SWARMJOIN PIN %d swarmName %s\n", com->data.joinCmd.pin, com->data.joinCmd.swarmName );
  #endif

  // case 1: member asks Kelda: Join, if pin & swarm name are ok
  // case 2: Kelda asks Kelda:  Join, if I don't have swarm members.
  // case 3: Kelda asks member: Join, if I'm not connected to a swarm.

  SwOSError_t result = SWOS_OK;

  // case 1: member asks Kelda: Join, if pin & swarm name are ok
  if  ( (Ctrl[0]->IAmKelda) && (!com->data.joinCmd.IAmKelda ) ) {

    // pin and swarm name ok?
    result = ( ( nvs.swarmPIN == com->data.joinCmd.pin ) && ( strcmp( com->data.joinCmd.swarmName, nvs.swarmName ) == 0 ) )? SWOS_OK : SWOS_DENY;

    // join?
    if ( ( result == SWOS_OK )  && ( nvs.addController( com->data.sourceSN ) ) ) {  
      Ctrl[source] = new SwOSCtrl( com->data.sourceSN, com->macAddr, false, FTSWARM_NOVERSION, false, FTSWARM_EXT_OFF );
      nvs.save();
      Ctrl[source]->comState = ASKFORDETAILS;
    }

  // case 2: Kelda asks Kelda:  Join, if I'm not connected to a swarm.
  } else if ( (Ctrl[0]->IAmKelda) && (com->data.joinCmd.IAmKelda ) ) {

    result = ( nvs.swarmMembers() > 1 ) ? SWOS_DENY : SWOS_OK;

    // join ?
    if ( result == SWOS_OK ) {

      // I'm not a Kelda any more
      nvs.IAmKelda      = false;
      Ctrl[0]->IAmKelda = false;

      // set new swarm values
      nvs.swarmPIN       = com->data.joinCmd.pin;
      nvs.swarmSecret    = com->data.joinCmd.swarmSecret;
      strcpy( nvs.swarmName, com->data.joinCmd.swarmName );
      myOSNetwork.setSecret( com->data.joinCmd.swarmSecret, com->data.joinCmd.pin );

      // save
      nvs.save();

    }

  // case 3: Kelda asks member: Join, if I'm not connected to a swarm.
  } else {

    result = ( Kelda == NULL ) ? SWOS_OK : SWOS_DENY;

    if ( result == SWOS_OK ) {

      // set new swarm values
      nvs.swarmPIN       = com->data.joinCmd.pin;
      nvs.swarmSecret    = com->data.joinCmd.swarmSecret;
      strcpy( nvs.swarmName, com->data.joinCmd.swarmName );
      myOSNetwork.setSecret( com->data.joinCmd.swarmSecret, com->data.joinCmd.pin );

      // save
      nvs.save();

    }

  }
    
  // send acknowledge
  #ifdef DEBUG_COMMUNICATION
    printf( "CMD_SWARMJOIN accepted.\n" ); 
  #endif

  sendAck( com->data.sourceSN, CMD_SWARMJOIN, result, myOSNetwork.secret );

}

void SwOSSwarm::cmdAck( SwOSCom *com, uint8_t source, uint8_t affected ) {

  SwOSCtrl *ctrl;

  if (!Ctrl[source]) {

    // member asked kelda without knowing keldas SN and used boradcastSN instead?
    ctrl = (SwOSCtrl *) getController( broadcastSN );

    // didn't find a broadcast controller
    if (!ctrl) return;

    // add serial number
    ctrl->serialNumber = com->data.sourceSN;

  } else {

    // just work with controller found
    ctrl = Ctrl[source];

  }

  ctrl->lastAck.cmd    = com->data.ackCmd.cmd;
  ctrl->lastAck.error  = com->data.ackCmd.error;
  ctrl->lastAck.secret = com->data.ackCmd.secret;

}

void SwOSSwarm::sendAck( FtSwarmSerialNumber_t destinationSN, SwOSCommand_t cmd, SwOSError_t error, uint16_t secret ) {

  SwOSCom ack( MacAddr( broadcast), destinationSN, CMD_ACK );

  // if a member asks a kelda, the secret isn't known yet
  if ( cmd == CMD_SWARMJOIN ) ack.data.secret = DEFAULTSECRET;

  // other stuff 
  ack.data.ackCmd.cmd    = cmd;
  ack.data.ackCmd.error  = error;
  ack.data.ackCmd.secret = secret;

  // send it
  ack.send();

}

void SwOSSwarm::cmdLeave( SwOSCom *com, uint8_t source, uint8_t affected ) {

  // I'm asked to leave the swarm.
  if ( com->data.affectedSN == Ctrl[0]->serialNumber ) {

    // I'm Kelda, so don't send this command to me 
    if ( Ctrl[0]->IAmKelda ) {
      ESP_LOGE( LOGFTSWARM, "%s asked Kelda to leave the swarm.", com->data.sourceSN );
      sendAck( com->data.sourceSN, CMD_SWARMLEAVE, SWOS_DENY, myOSNetwork.secret );
      return;
    }

    // I just accept this command from Kelda
    if ( ( !Kelda ) && ( Kelda->serialNumber != com->data.sourceSN ) ) {
      ESP_LOGE( LOGFTSWARM, "%s asked me to leave the swarm, but it's not my Kelda.", com->data.sourceSN );
      sendAck( com->data.sourceSN, CMD_SWARMLEAVE, SWOS_DENY, myOSNetwork.secret );
      return;
    }

    // ack, reset to Default swarm and reboot  
    sendAck( com->data.sourceSN, CMD_SWARMLEAVE, SWOS_OK, myOSNetwork.secret );
    shortDelay();
    nvs.createSwarm( Ctrl[0]->getName(), Ctrl[0]->serialNumber );
    nvs.save();
    ESP.restart();

  // someone leaves the swarm
  } else {

    if ( Ctrl[source] ) { 
      // I know this controller and kill it
      nvs.deleteController( Ctrl[source]->serialNumber );
      nvs.save( );

      SwOSCtrl *old;
      Ctrl[source]->lock();
      old = Ctrl[source];
      Ctrl[source] = NULL;
      old->unlock();
      delete old;

    }

  }

  setState( RUNNING );

}

void SwOSSwarm::OnDataRecv(SwOSCom *com) {
  // callback function receiving data from other controllers

  // ignore invalid data
  if (!com) return;

  // get affected controllers
  uint8_t source   = getIndex( com->data.sourceSN );
  uint8_t affected = getIndex( com->data.affectedSN );

  // test messages
  switch ( com->data.cmd ) {
    case CMD_SWARMJOIN:   cmdJoin( com, source, affected );    return;
    case CMD_ACK:         cmdAck( com, source, affected );     return;
    case CMD_SWARMLEAVE:  cmdLeave( com, source, affected );   return;
  }


  // check, on welcome messages if I'm a Kelda or a Kelda is asking
  if ( ( ( com->data.cmd == CMD_ANYBODYOUTTHERE ) || ( com->data.cmd == CMD_GOTYOU ) ) &&
       ( ( com->data.registerCmd.IAmKelda ) || ( Ctrl[0]->IAmKelda ) ) ) {

    #ifdef DEBUG_COMMUNICATION
      printf( "register msg %d source: %d affected: %d maxCtrl %d\n", com->data.cmd, source, affected, maxCtrl );
    #endif

    // check on unkown controller
    if ( ( !Ctrl[source] ) || 
         ( ( Ctrl[source]->comState == ASKFORDETAILS ) && nvs.IAmKelda ) ) {

      #ifdef DEBUG_COMMUNICATION
      printf( "add a new controller type %d at %d\n", com->data.registerCmd.ctrlType, source);
      #endif

      // test, if the new controller is a Kelda and there is already a Kelda in my swarm
      if ( ( com->data.registerCmd.IAmKelda ) && ( Ctrl[0]->IAmKelda ) ) {
        setState( ERROR );
        printf("ERROR: Multiple Keldas found! %d %d\n", Kelda->serialNumber, com->data.sourceSN );
        while (1) delay(1000);
        return;
      }

      // add or replace controller in swarm list
      SwOSCtrl *newCtrl = NULL;
      SwOSCtrl *oldCtrl = Ctrl[source];

      switch (com->data.registerCmd.ctrlType) {
        case FTSWARM:         newCtrl = new SwOSSwarmJST     ( com ); break;
        case FTSWARMCONTROL:  newCtrl = new SwOSSwarmControl ( com ); break;
        case FTSWARMCAM:      newCtrl = new SwOSSwarmCAM     ( com ); break;
        case FTSWARMPWRDRIVE: newCtrl = new SwOSSwarmPwrDrive( com ); break;
        case FTSWARMDUINO:    newCtrl = new SwOSSwarmDuino   ( com ); break;
        default: ESP_LOGW( LOGFTSWARM, "Unknown controller type while adding a new controller to my swarm." ); return;
      }

      newCtrl->comState = UP;
      Ctrl[source] = newCtrl;
      if (oldCtrl) delete oldCtrl; 

      if ( Ctrl[source]->IAmKelda ) {
        // register Kelda
        // if (verbose) { printf("Kelda %s with MAC ", Ctrl[source]->getHostname() ); Ctrl[source]->macAddr.print(); printf(" joined the swarm \n"); }
        Kelda = Ctrl[source];
      } 
      
      if (verbose) { printf("[%s with MAC ", Ctrl[source]->getHostname() ); Ctrl[source]->macAddr.print(); printf(" joined the swarm]\n"); }

      // update oled display
      setState( RUNNING );
      
    }

    // reply GOTYOU, if needed
    if ( com->data.cmd == CMD_ANYBODYOUTTHERE ) {
      
      // frist introduce myself
      SwOSCom reply( com->macAddr, com->data.sourceSN, CMD_GOTYOU );
      Ctrl[0]->registerMe( &reply );
      reply.send();
    
    }

    // send my alias names, if the new controller is a Kelda
    if ( com->data.registerCmd.IAmKelda ) Ctrl[0]->sendAlias( com->macAddr ); 

    return;

  } // end welcome messages
 
  if ( Ctrl[affected] ) {
    // any other type of msg will be processed on controller level
    Ctrl[affected]->lock();
    Ctrl[affected]->OnDataRecv( com );
    Ctrl[affected]->unlock();

  } 

}

SwOSError_t SwOSSwarm::leaveSwarm( void ) {

  // Send a leave message
  SwOSCom leaveMsg( MacAddr( broadcast ), Ctrl[0]->serialNumber, CMD_SWARMLEAVE );
  leaveMsg.send();

  // wait for replys
  longDelay();

  bool ok = true;

  // now I could kill all swarm members
  for (uint8_t i=1; i<=maxCtrl; i++) {

    if ( Ctrl[i] ) { 

      // check if the controller send an ack message
      ok = ok && ( Ctrl[i]->lastAck.cmd == CMD_SWARMLEAVE ) && ( Ctrl[i]->lastAck.error == SWOS_OK );

      // delete it 
      SwOSCtrl *old = Ctrl[i]; 
      Ctrl[i] = NULL;
      delete old;
    
    }

  }

  // reset mail swarm parameters
  maxCtrl = 0;
  Kelda = NULL;
  Ctrl[0]->IAmKelda = false;

  return (ok) ? SWOS_OK : SWOS_TIMEOUT;

}

SwOSError_t SwOSSwarm::rejectController( FtSwarmSerialNumber_t serialNumber, bool force ) {
  uint8_t affected = getIndex( serialNumber );

  // do I know the controller?
  if ( !Ctrl[affected] ) return SWOS_TIMEOUT;

  // In use?
  if ( Ctrl[affected]->isInUse() ) return SWOS_DENY; 

  // Send a leave message
  SwOSCom leaveMsg( MacAddr( broadcast ), serialNumber, CMD_SWARMLEAVE );
  leaveMsg.send();

  // wait for replys
  longDelay();

  // get controllers response
  SwOSError_t result = SWOS_TIMEOUT;
  if ( Ctrl[affected]->lastAck.cmd == CMD_SWARMLEAVE ) result = Ctrl[affected]->lastAck.error;

  // delete him, if possible
  if ( ( result == SWOS_OK ) || force ) { 
    Ctrl[affected]->lock();
    SwOSCtrl *old = Ctrl[affected];
    Ctrl[affected] = NULL;
    old->unlock();
    delete old;
    result = SWOS_OK;
  }

  return result;

}

SwOSError_t SwOSSwarm::createSwarm( void ) {

  return createSwarm( Ctrl[0]->getHostname(), Ctrl[0]->serialNumber );

}

SwOSError_t SwOSSwarm::createSwarm( char * newName, uint16_t newPIN ) {

  // need to call leaveSwarm first to cleanup
  // test on existing clients
  if (members() > 1 ) return SWOS_DENY;

  // create new swarm
  nvs.createSwarm( newName, newPIN );
  myOSNetwork.setSecret( nvs.swarmSecret, nvs.swarmPIN );

  // change myself to Kelda
  nvs.IAmKelda = true;
  Ctrl[0]->IAmKelda = true;
  Kelda = Ctrl[0];

  // done
  return SWOS_OK;

}

SwOSError_t SwOSSwarm::inviteToSwarm( FtSwarmSerialNumber_t serialNumber ) {

  // already joined?
  if ( Ctrl[getIndex(serialNumber)] ) return SWOS_OK;

  // I'm not a Kelda?
  if ( !Ctrl[0]->IAmKelda ) return SWOS_DENY;

  uint8_t newMember = getIndex( serialNumber );
  Ctrl[newMember] = new SwOSCtrl( serialNumber,  MacAddr( broadcast ), false, FTSWARM_NOVERSION, false, FTSWARM_EXT_OFF );

 // invite controller
  SwOSCom joinMsg( MacAddr( broadcast ), serialNumber, CMD_SWARMJOIN );
  joinMsg.data.secret = DEFAULTSECRET;
  joinMsg.data.joinCmd.IAmKelda = true;
  joinMsg.data.joinCmd.pin = nvs.swarmPIN;
  strcpy( joinMsg.data.joinCmd.swarmName, nvs.swarmName );
  joinMsg.data.joinCmd.swarmSecret = nvs.swarmSecret;
  joinMsg.send();

  // wait for replys
  longDelay();

  // check result
  SwOSError_t result = SWOS_TIMEOUT;
  if (Ctrl[newMember]->lastAck.cmd == CMD_SWARMJOIN) result = Ctrl[newMember]->lastAck.error;

  // delete new Controller in case of any error
  if ( result != SWOS_OK ) { delete Ctrl[newMember]; Ctrl[newMember] = NULL; return result; }

  // Controller joined the swarm
  nvs.addController( serialNumber );
  Ctrl[newMember]->comState = ASKFORDETAILS;

  return result;

}

SwOSError_t SwOSSwarm::joinSwarm( char *name, uint16_t pin ) {

  // bound in a swarm?
  if ( myOSSwarm.members() > 1) return SWOS_DENY;

  uint8_t newMember = getIndex( broadcastSN );
  Ctrl[newMember] = new SwOSCtrl( broadcastSN,  MacAddr( broadcast ), false, FTSWARM_NOVERSION, false, FTSWARM_EXT_OFF );

  // ask Kelda to join
  SwOSCom joinMsg( MacAddr( broadcast ), broadcastSN, CMD_SWARMJOIN );
  joinMsg.data.joinCmd.IAmKelda = false;
  joinMsg.data.joinCmd.pin = pin;
  strcpy( joinMsg.data.joinCmd.swarmName, name );
  joinMsg.data.joinCmd.swarmSecret = DEFAULTSECRET;
  joinMsg.data.secret = DEFAULTSECRET;
  joinMsg.send();

  // wait for replys
  longDelay();

  // check result
  SwOSError_t result = SWOS_TIMEOUT;
  if (Ctrl[newMember]->lastAck.cmd == CMD_SWARMJOIN) result = Ctrl[newMember]->lastAck.error;

  // delete new Controller in case of any error
  if ( result != SWOS_OK ) {
    Ctrl[newMember]->lock();
    SwOSCtrl *temp = Ctrl[newMember];
    Ctrl[newMember] = NULL;
    temp->unlock();
    delete temp; 
    return result;
  }

  // setup new swarm
  nvs.IAmKelda       = false;
  Ctrl[0]->IAmKelda  = false; 
  nvs.swarmPIN       = pin;
  nvs.swarmSecret    = Ctrl[newMember]->lastAck.secret;
  strcpy( nvs.swarmName, name );
  myOSNetwork.setSecret( Ctrl[newMember]->lastAck.secret, pin );
  Kelda              = Ctrl[newMember];

  // continue starting up
  Ctrl[newMember]->comState = ASKFORDETAILS;

  return result;

}


uint8_t SwOSSwarm::members( void ) {

  uint8_t members = 0;

  for (uint8_t i=0; i<=maxCtrl; i++ ) {
    if ( Ctrl[i] ) members++;
  }

  return members;
  
}
