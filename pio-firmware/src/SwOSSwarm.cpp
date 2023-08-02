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

  SwOSCom event( broadcast, NULL, 0 );
  
  // forever
  while (1) {

    // new data available?
    if ( xQueueReceive( recvNotification, &event, ESPNOW_MAXDELAY ) == pdTRUE ) {

      #ifdef DEBUG_COMMUNICATION
        if ( event.data.cmd != CMD_STATE ) {
          ESP_LOGD( LOGFTSWARM, "my friend sends some data...\nMAC: %02X:%02X:%02X:%02X:%02X:%02X secret %04X cmd %d valid %d", event.mac[0], event.mac[1], event.mac[2], event.mac[3], event.mac[4], event.mac[5], event.data.secret, event.data.cmd, event.isValid() );
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
    
    // copy my state to a datagram
    SwOSCom *com = myOSSwarm.Ctrl[0]->state2Com( );
    
    // send data to all Keldas
    myOSSwarm.send( com );
    
    // cleanup
    delete com;

    // calc delay time
    #ifdef DEBUG_READTASK
      xDelay = 25000 / portTICK_PERIOD_MS;
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

uint16_t SwOSSwarm::_nextToken( bool rotateToken ) {

  uint32_t pin      = nvs.swarmPIN;
  uint16_t newToken = ( 2 + ( _lastToken ^ pin ) ) & 0xFFFF;

  if ( rotateToken ) _lastToken = newToken;

  return newToken;
  
}

SwOSIO *SwOSSwarm::_waitFor( char *alias ) {

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

bool SwOSSwarm::_startEvents( void ) {

  SwOSIO *sensor;
  SwOSIO *actor;
  NVSEvent *event;

  for (uint8_t i=0; i<MAXNVSEVENT; i++ ) {

    event = &nvs.eventList.event[i];
    
    if ( ( event->sensor[0] != '\0' ) && ( event->actor[0] != '\0' ) ) {

      // get IOs and stop on error
      sensor = _waitFor( event->sensor ); if (!sensor) return false;
      actor  = _waitFor( event->actor );  if (!actor)  return false;

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

void SwOSSwarm::_startWifi( bool verbose ) {

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
  
  if ( ( nvs.wifiMode == wifiAP ) || Ctrl[0]->maintenanceMode() ) {
    // work as AP in standard or maintennace cable was set
    if (verbose) printf("Create own SSID: %s\n", Ctrl[0]->getHostname());
    WiFi.mode(WIFI_AP_STA);
    esp_wifi_set_ps(WIFI_PS_NONE);
    WiFi.softAPsetHostname(Ctrl[0]->getHostname());
    WiFi.softAP( nvs.wifiSSID, "", nvs.channel); // passphrase not allowed on ESP32WROOM
    Ctrl[0]->IP = WiFi.softAPIP();
    
  } else {
    // normal operation
    if (verbose) printf("Attempting to connect to SSID: %s", nvs.wifiSSID);

    WiFi.mode(WIFI_AP_STA);  // AP_STA needed to get espnow running
    
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
    Ctrl[0]->IP = WiFi.localIP();
  }

  if (verbose) printf("hostname: %s\nip-address: %d.%d.%d.%d\n", Ctrl[0]->getHostname(), Ctrl[0]->IP[0],  Ctrl[0]->IP[1], Ctrl[0]->IP[2], Ctrl[0]->IP[3]);

}

FtSwarmSerialNumber_t SwOSSwarm::begin( bool IAmAKelda, bool verbose ) {

  if (verbose) {
    printf("\n\nftSwarmOS " ); 
    printf(SWOSVERSION);
    printf("\n\n(C) Christian Bergschneider & Stefan Fuss\n\nPress any key to enter bios settings.\n");
  }

  // set watchdog to 30s
  esp_task_wdt_init(30, false);
  
  // initialize random
  srand( time( NULL ) );

  // initialize nvs
  nvs.begin();

  // initialize I2C
  switch ( nvs.CPU ) {
    case FTSWARM_2V1: 
    case FTSWARM_2V0: Wire.begin( 8, 9 );   break;
    case FTSWARM_1V0: Wire.begin( 13, 12 ); break;
    default:          Wire.begin( 21, 22 ); break;
  }

	// create local controler
	maxCtrl++;
	if (nvs.controlerType == FTSWARM ) {
    Ctrl[maxCtrl] = new SwOSSwarmJST( nvs.serialNumber, NULL, true, nvs.CPU, nvs.HAT, IAmAKelda, nvs.RGBLeds );
	} else if (nvs.controlerType == FTSWARMCONTROL ) {
		Ctrl[maxCtrl] = new SwOSSwarmControl( nvs.serialNumber, NULL, true, nvs.CPU, nvs.HAT, IAmAKelda, nvs.joyZero, nvs.displayType );
	} else {
    Ctrl[maxCtrl] = new SwOSSwarmCAM( nvs.serialNumber, NULL, true, nvs.CPU, nvs.HAT, IAmAKelda );
  } 
  
  // Who I am?
  printf("Boot %s (SN:%d).\n", Ctrl[maxCtrl]->getHostname(), Ctrl[maxCtrl]->serialNumber );

  // Open NVS again & load alias names
  nvs_handle_t my_handle;
  ESP_ERROR_CHECK( nvs_open("ftSwarm", NVS_READWRITE, &my_handle) );
  myOSSwarm.Ctrl[0]->loadAliasFromNVS( my_handle );

  // now I can visualize my state
  setState( BOOTING );

  // wifi
  if ( nvs.wifiMode != wifiOFF ) _startWifi( verbose );

  // Init Communication
  if (!SwOSStartCommunication( nvs.swarmSecret, nvs.swarmPIN )) {
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
  if ( !_startEvents( ) ) {
      printf( "\nStarting setup..\n" );
      mainMenu();
      ESP.restart();
    }

  delay(1000);
  myOSSwarm.lock();
  setState( RUNNING );
  myOSSwarm.unlock();

  if (verbose) printf("Start normal operation.\n");

  return Ctrl[0]->serialNumber;

}

void SwOSSwarm::halt( void ) {

  for (uint8_t i=0; i<maxCtrl; i++)
    Ctrl[i]->halt();
}

void SwOSSwarm::unsubscribe( void ) {

  for (uint8_t i=0; i<maxCtrl; i++)
    Ctrl[i]->unsubscribe();

}

uint8_t SwOSSwarm::_getIndex( FtSwarmSerialNumber_t serialNumber ) {

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

SwOSIO* SwOSSwarm::getIO( FtSwarmSerialNumber_t serialNumber, FtSwarmIOType_t ioType,FtSwarmPort_t port) {

  // check on valid controler
  uint8_t i = _getIndex( serialNumber );

  if ( Ctrl[i] ) return Ctrl[i]->getIO( ioType, port );

  // nothing found
  return NULL;
  
}

SwOSIO* SwOSSwarm::getIO( const char *name ) {

  SwOSIO *IO = NULL;

  for ( uint8_t i=0; i<=maxCtrl; i++ ) {

    if ( Ctrl[i] ) {
      IO = Ctrl[i]->getIO( name );
      if (IO) break;
    }
    
  }

  return IO;
  
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

    // visualize others, if I'm a Kelda only
    if (!Ctrl[0]->AmIAKelda()) break;
    
	}

	json->endArray();

}

void SwOSSwarm::getToken( JSONize *json) {

  _lastToken = rand();

  json->startObject();
  json->variableUI16( "token", _lastToken );
  json->endObject();
  
}

bool SwOSSwarm::_splitId( char *id, uint8_t *index, char *io, size_t sizeIO) {

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

  uint16_t n = _nextToken(rotateToken);

  if ( token != n ) return 401;

  return 200;
}

bool SwOSSwarm::apiPeekIsAuthorized( uint16_t token ) {
  return token == _lastToken;
}

uint16_t SwOSSwarm::apiActorCmd( uint16_t token, char *id, int cmd, bool rotateToken ) {
// send an actor's command (from api)

	uint8_t i;
	char    io[20];

  // Token error
  if ( token != _nextToken(rotateToken) ) return 401;

	// split ID to device and nr
	if (!_splitId(id, &i, io, sizeof(io))) return 400;

	// execute cmd
  if ( ( Ctrl[i] ) && ( Ctrl[i]->apiActorCmd( io, cmd ) ) ) return 200;

  return 400;

}

uint16_t SwOSSwarm::apiActorPower( uint16_t token, char *id, int power, bool rotateToken ) {
// send an actor's power(from api)

  uint8_t i;
  char    io[20];

  // Token error
  if ( token != _nextToken(rotateToken) ) return 401;

  // split ID to device and nr
  if (!_splitId(id, &i, io, sizeof(io))) return 400;

  // execute cmd
  if ( ( Ctrl[i] ) && ( Ctrl[i]->apiActorPower( io, power ) ) ) return 200;

  return 400;
  
}

uint16_t SwOSSwarm::apiLEDBrightness( uint16_t token, char *id, int brightness, bool rotateToken ) {
  // send a LED command (from api)

	uint8_t i;
	char    io[20];

  // Token error
  if ( token != _nextToken(rotateToken) ) return 401;

	// split ID to device and nr
	if (!_splitId(id, &i, io, sizeof(io))) return 400;

	// execute cmd
	if ( ( Ctrl[i] ) && ( Ctrl[i]->apiLEDBrightness( io, brightness) ) ) return 200;
  
  return 400;

}

uint16_t SwOSSwarm::apiLEDColor( uint16_t token, char *id, int color, bool rotateToken ) {
  // send a LED command (from api)

  uint8_t i;
  char    io[20];

  // Token error
  if ( token != _nextToken(rotateToken) ) return 401;

  // split ID to device and nr
  if (!_splitId(id, &i, io, sizeof(io))) return 400;

  // execute cmd
  if ( ( Ctrl[i] ) && ( Ctrl[i]->apiLEDColor( io, color ) ) ) return 200;
  
  return 400;

}

uint16_t SwOSSwarm::apiServoOffset( uint16_t token, char *id, int offset, bool rotateToken ) {
  // send a Servo command (from api)
  
	uint8_t i;
	char    io[20];

  // Token error
  if ( token != _nextToken(rotateToken) ) return 401;

	// split ID to device and nr
	if (!_splitId(id, &i, io, sizeof(io))) return 400;

	// execute cmd
	if ( ( Ctrl[i] ) && ( Ctrl[i]->apiServoOffset( io, offset  ) ) ) return 200;

  return 400;
  
}

uint16_t SwOSSwarm::apiServoPosition( uint16_t token, char *id, int position, bool rotateToken) {
  // send a Servo command (from api)
  
  uint8_t i;
  char    io[20];

  // Token error
  if ( token != _nextToken(rotateToken) ) return 401;

  // split ID to device and nr
  if (!_splitId(id, &i, io, sizeof(io))) return 400;

  // execute cmd
  if ( ( Ctrl[i] ) && ( Ctrl[i]->apiServoPosition( io, position ) ) ) return 200;

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

  SwOSCom com( broadcast, broadcastSN, CMD_ANYBODYOUTTHERE);
  Ctrl[0]->registerMe( &com );
  com.send();

}

void SwOSSwarm::OnDataRecv(SwOSCom *com) {
  // callback function receiving data from other controllers

  // ignore invalid data
  if (!com) return;

  uint8_t i = _getIndex( com->data.sourceSN );

  // check on join message
  if ( com->data.cmd == CMD_SWARMJOIN ) {

    #ifdef DEBUG_COMMUNICATION
      ESP_LOGD( LOGFTSWARM, "CMD_SWARMJOIN PIN %d swarmName %s", com->data.joinCmd.pin, com->data.joinCmd.swarmName );
    #endif

    // pin and swarm name ok?
    if ( ( nvs.swarmPIN == com->data.joinCmd.pin ) && ( strcmp( com->data.joinCmd.swarmName, nvs.swarmName ) == 0 ) ) {
      // send acknowledge
      #ifdef DEBUG_COMMUNICATION
        ESP_LOGD( LOGFTSWARM, "CMD_SWARMJOIN accepted" ); 
      #endif
      SwOSCom ack( com->mac, com->data.sourceSN, CMD_SWARMJOINACK );
      ack.data.joinCmd.pin = nvs.swarmPIN;
      ack.data.joinCmd.swarmSecret = nvs.swarmSecret;
      strcpy( ack.data.joinCmd.swarmName, nvs.swarmName );
      ack.data.secret = DEFAULTSECRET;
      ack.send();
      return;
    }
  }

  // acknowledge join?
  if ( com->data.cmd == CMD_SWARMJOINACK ) {
    
    #ifdef DEBUG_COMMUNICATION
      ESP_LOGD( LOGFTSWARM, "CMD_SWARMJOINACK new secret: %04X old pin %d new pin %d allowPairing %d\n", com->data.joinCmd.swarmSecret, nvs.swarmPIN, com->data.joinCmd.pin, allowPairing );
    #endif
    
    // ack's PIN needs to be the same as my pin and I need to be in pairing mode
    // if ( ( com->data.registerCmd.pin == nvs.swarmPIN ) && allowPairing ) { 
    if ( allowPairing ) {
      nvs.swarmSecret = com->data.joinCmd.swarmSecret;
      SwOSSetSecret( nvs.swarmSecret, nvs.swarmPIN );
    }
    
    return;
  }

  if ( com->data.cmd == CMD_SWARMLEAVE ) {
    // someone leaves the swarm
    if ( Ctrl[i] ) { 
      // I know this controler and kill it
      delete Ctrl[i];
      Ctrl[i] = NULL;
    }
    setState( RUNNING );
    return;
  }

  // check, on welcome messages
  if ( ( com->data.cmd == CMD_ANYBODYOUTTHERE ) ||
       ( com->data.cmd == CMD_GOTYOU ) ) {

    #ifdef DEBUG_COMMUNICATION
      ESP_LOGD( LOGFTSWARM, "register msg %d i:%d maxCtrl %d\n", com->data.cmd, i, maxCtrl );
    #endif
    
    // check on unkown controler
    if ( !Ctrl[i] ) {
      
      #ifdef DEBUG_COMMUNICATION
        ESP_LOGD( LOGFTSWARM, "add a new controler type %d", com->data.registerCmd.ctrlType);
      #endif

      // add to swarm
      switch (com->data.registerCmd.ctrlType) {
        case FTSWARM:        Ctrl[i] = new SwOSSwarmJST    ( com ); break;
        case FTSWARMCONTROL: Ctrl[i] = new SwOSSwarmControl( com ); break;
        case FTSWARMCAM:     Ctrl[i] = new SwOSSwarmCAM    ( com ); break;
        default: ESP_LOGW( LOGFTSWARM, "Unknown controler type while adding a new controller to my swarm." ); return;
      }

      // update oled display
      setState( RUNNING );
      
    } else {
      // controler rebooted unexpected, send his last state
      // if ( Ctrl[0]->AmIAKelda() ) Ctrl[i]->sendConfig(com->mac);
    }

    // reply GOTYOU, if needed
    if ( com->data.cmd == CMD_ANYBODYOUTTHERE ) {
      SwOSCom reply( com->mac, com->data.sourceSN, CMD_GOTYOU);
      Ctrl[0]->registerMe( &reply );
      reply.send( );

      // TODO send ALIAS names

    }

  // send my alias names to requestor
  Ctrl[0]->sendAlias( com->mac, com->data.sourceSN );

  return;
    
  } 

  // any other type of msg will be processed on controler level
  if ( Ctrl[i] ) {
    Ctrl[i]->OnDataRecv( com );
  } 

}

void SwOSSwarm::send(SwOSCom *com) {

  if (!com) return;

  // send to all Keldas in swarm
  for (uint8_t i=1;i<=maxCtrl;i++) {
    
    if ( ( Ctrl[i] ) && ( Ctrl[i]->AmIAKelda() ) ) { 
      com->setMAC( Ctrl[i]->mac, Ctrl[i]->serialNumber );
      com->data.sourceSN = nvs.serialNumber;
      com->send( ); 
    }
    
  }
  
}

void SwOSSwarm::leaveSwarm( void ) {

  // Prepare a leave message
  SwOSCom leaveMsg( NULL, broadcastSN, CMD_SWARMLEAVE );

  // send to all others
  for (uint8_t i=1;i<=maxCtrl;i++) {
 
    if ( Ctrl[i] ) {
      // set remote controllers mac & send leave msg
      leaveMsg.setMAC( Ctrl[i]->mac, Ctrl[i]->serialNumber );
      leaveMsg.send( );

      // cleanup
      delete Ctrl[i];
      Ctrl[i] = NULL;
    }
  }
    
  // now everbody nows, i'm gone
  maxCtrl = 0;

}

void SwOSSwarm::joinSwarm( bool createNewSwarm, char * newName, uint16_t newPIN ) {

  // setup new swarm
  nvs.createSwarm( newName, newPIN );
  SwOSSetSecret( DEFAULTSECRET, nvs.swarmPIN );

  // send CMD_SWARMJOIN
  SwOSCom joinMsg( broadcast, broadcastSN, CMD_SWARMJOIN);
  joinMsg.data.joinCmd.pin = newPIN;
  strcpy( joinMsg.data.joinCmd.swarmName, newName );
  joinMsg.send();

}

void SwOSSwarm::reload( void ) {

  leaveSwarm();
  nvs.load();
  SwOSSetSecret( nvs.swarmSecret, nvs.swarmPIN );
  registerMe( );

}

uint8_t SwOSSwarm::members( void ) {

  uint8_t members = 0;

  for (uint8_t i=0; i<=maxCtrl; i++ ) {
    if ( Ctrl[i] ) members++;
  }

  return members;
  
}
