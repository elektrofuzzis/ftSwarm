/*
 * SwOSSwarm.cpp
 *
 * internal represenation of my swarm. Use FtSwarm-Classes in FtSwarm.h to access your swarm!
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOS.h"

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#include "esp_mac.h"
#endif

#include <esp_now.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "mdns.h"

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
#include "SwOSLog.h"
#include "SwOSHW/SwOSHWLocal.h"

#include <string.h>

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
        if ( ( event.data.cmd != CMD_STATE ) && ( event.data.cmd != CMD_HARTBEAT ) ) {
          printf("\n\n-----------------------------\nmy friend sends some data...\n" ); 
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
    myOSSwarm.Ctrl[0]->operate();
    #if FTSWARM_HAL_OLEDS > 0
    screenManager.operate();
    #endif

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

      SwOSComState_t comState = Ctrl[i]->getComState();

      // if controller was not seen for a longer time or is new: try to reconnect
      if ( ( comState == COMSTATE_UNDEFINED ) ||
           ( ( Ctrl[i]->networkAge() > 1000L ) && ( comState != COMSTATE_ERROR ) ) ) {
      
        Ctrl[i]->setComState( COMSTATE_CONNECT_PHASE1 );
        joinMySwarm( MacAddr( broadcast ), Ctrl[i]->serialNumber );

      // if it's online send him an hart beat
      } else if ( comState == COMSTATE_ONLINE ) {
        
        SwOSCom hartBeat( Ctrl[i]->macAddr, Ctrl[i]->serialNumber, CMD_HARTBEAT );
        hartBeat.send();

      }

    }

  }

}

SwOSIO *SwOSSwarm::waitFor( char *alias ) {

  SwOSIO *me = NULL;
  bool   firstTry = true;

  while (!me) {

    me = getIO( alias );

    // no success, wait 25 ms
    if ( (!me) && ( firstTry ) ) {
      SWARM_LOG_WAIT( TRANSLATE( "Waiting for device %s. Press anykey to enter setup and change remote control settings.\n", "Warte auf IO %s. Drücken Sie eine beliebige Taste, um das Setup zu starten.\n" ), alias );
      firstTry = false;
    }
    
    if (!me) vTaskDelay( 25 / portTICK_PERIOD_MS );
    
    // any key pressed?
    if ( anyKey() ) return NULL;

  }

  return me;
  
}

void SwOSSwarm::startWifi( void ) {

  // 1. Initialize TCP/IP stack
  ESP_ERROR_CHECK( esp_netif_init() );

  // 2. Create default event loop if not already running
  if ( esp_event_loop_create_default() != ESP_OK ) {
  // Handle error or assume it's already created
  }

  // 3. Create Netif instances for Station & AP
  esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
  esp_netif_t *ap_netif  = esp_netif_create_default_wifi_ap();

  esp_netif_set_hostname(sta_netif, Ctrl[0]->getHostname() );
  esp_netif_set_hostname(ap_netif,  Ctrl[0]->getHostname() );

  // 4. Init WiFi with default config
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init( &cfg ) );

  ESP_ERROR_CHECK( esp_wifi_set_mode( (nvs.wifi.mode == wifiAP) ? WIFI_MODE_AP : WIFI_MODE_STA ) );

  // 5. Set Storage to RAM (to avoid flash wear during frequent reboots)
  ESP_ERROR_CHECK( esp_wifi_set_storage( WIFI_STORAGE_RAM ) );

  // 6. start wifi
  if ( nvs.wifi.mode == wifiAP ) {
    // Provide network via SoftAP

    // setup soft ap config
    wifi_config_t ap_config = {};
    strlcpy( (char *) ap_config.ap.ssid,     nvs.wifi.SSID, sizeof( ap_config.ap.ssid ) );
    strlcpy( (char *) ap_config.ap.password, nvs.wifi.Password,  sizeof( ap_config.ap.password ) );
    ap_config.ap.channel = nvs.wifi.channel;
    ap_config.ap.max_connection = MAX_AP_CONNECTIONS;
    ap_config.ap.authmode = ( strlen( nvs.wifi.Password ) == 0) ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK;

    // set config
    ESP_ERROR_CHECK( esp_wifi_set_config( WIFI_IF_AP, &ap_config ) );

    if (verbose) printf("Create own SSID: %s\n", nvs.wifi.SSID );
    
    wifiHandler = new WifiHandler();

    ESP_ERROR_CHECK( esp_wifi_start() );
    esp_wifi_set_ps(WIFI_PS_NONE);
    wifiConnected = true;
    
  } else {
    // use infrastructure as client

    // setup config
    wifi_config_t sta_config = {};
    strlcpy( (char *) sta_config.sta.ssid,     nvs.wifi.SSID, sizeof( sta_config.sta.ssid ) );
    strlcpy( (char *) sta_config.sta.password, nvs.wifi.Password,  sizeof( sta_config.sta.password ) );
        
    // Disable PMF (Protected Management Frames) for better compatibility
    sta_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    
    // set config
    ESP_ERROR_CHECK( esp_wifi_set_config( WIFI_IF_STA, &sta_config ) );

    if (verbose) printf( TRANSLATE( "Connecting to SSID: %s ", "Verbinde mit SSID: %s " ), nvs.wifi.SSID );

    wifiHandler = new WifiHandler();
    
    // Start WiFi
    ESP_ERROR_CHECK( esp_wifi_start() );
    esp_wifi_set_ps(WIFI_PS_NONE);

    esp_netif_set_hostname(sta_netif, Ctrl[0]->getHostname() );
  
    // connect
    esp_wifi_connect();
        
    // Manual polling loop (simulating your 10s wait)
    for (int i = 0; i < 20; i++) {
      
      esp_netif_ip_info_t ip_info;
      if (esp_netif_get_ip_info( sta_netif, &ip_info ) == ESP_OK && ip_info.ip.addr != 0) {
        wifiConnected = true;
        if ( verbose ) printf(TRANSLATE( " Connected!\n", " Verbunden!\n" ) );
        break;
      }

      // user interrupt?
      if ( anyKey() ) { 
        printf( TRANSLATE( "\nStarting setup..\n", "\nStarte Setup..\n" ) );
        mainMenu();
        ESP.restart();
      }
      
      if (verbose) { printf("."); flushStdIO(); }
      
      vTaskDelay(pdMS_TO_TICKS(500));

    }

    // connection failed?
    if ( !wifiConnected ) {
      
      #if FTSWARM_HAL_OLEDS > 0
        // start local operate/read task & show wifi dialog
        xTaskCreatePinnedToCore( readTask,    "ReadTask",    20000, NULL, 1, NULL, ARDUINO_EVENT_RUNNING_CORE );
        screenManager.wifiMenu( true );
      #endif

      SWARM_LOG_ERROR( TRANSLATE( "Can't connect to SSID %s", "Kann SSID %s nicht verbinden" ), nvs.wifi.SSID );

      printf( TRANSLATE( "\nStarting setup..\n", "\nStarte Setup..\n" ) );
      mainMenu();
      ESP.restart();
    }

  }

  // 7. MDNS
  esp_err_t err = mdns_init();

  if (err != ESP_OK) {
    SWARM_LOG_ERROR( TRANSLATE( "MDNS init failed: %s", "MDNS Initialisierung fehlgeschlagen: %s" ), err );
  } else {
    ESP_ERROR_CHECK( mdns_hostname_set( Ctrl[0]->getHostname() ) );
    mdns_service_add( nullptr, "_http", "_tcp", 80, nullptr, 0 );
  }

  // 8. MAC
  uint8_t mac[ESP_NOW_ETH_ALEN];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  Ctrl[0]->macAddr.set( mac );

  // 9. some pretty print
  if ( verbose ) {

    esp_netif_ip_info_t ip_info;

    esp_netif_t* netif;
    if (nvs.wifi.mode == wifiAP) netif = ap_netif;
    else                         netif = sta_netif;

    if ( esp_netif_get_ip_info( netif, &ip_info ) == ESP_OK ) 
      SWARM_LOG_INFO( TRANSLATE( "hostname: %s ip-address: %d.%d.%d.%d MAC: %02X:%02X:%02X:%02X:%02X:%02X", "Hostname: %s IP-Adresse: %d.%d.%d.%d MAC: %02X:%02X:%02X:%02X:%02X:%02X" ), 
                      Ctrl[0]->getHostname(), 
                      IP2STR( &ip_info.ip ),
                      mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );

  }

}

FtSwarmSerialNumber_t SwOSSwarm::begin( bool verbose ) {

  Serial.begin(115200);

  if (initialized) return Ctrl[0]->serialNumber;

  // redirect IO to feed the web console
  redirectStdIO();

  this->verbose = verbose;

  printf("\n\nftSwarmOS " ); 
  printf(SWOSVERSION);
  printf(TRANSLATE( "\n\n(C) Christian Bergschneider & Stefan Fuss\n\nPress any key to enter bios settings.\n", "\n\n(C) Christian Bergschneider & Stefan Fuss\n\nDrücken Sie eine Taste, um das Setup zu starten.\n" ));

  /*
  // set watchdog to 30s
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
  // New API for ESP-IDF v5.x / Arduino Core 3.x
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 30000,          // 30 seconds converted to milliseconds
    .idle_core_mask = 0,          // Automatically monitor idle tasks on all cores
    .trigger_panic = false        // Equivalent to your old 'false' parameter
  };
  esp_task_wdt_init(&wdt_config);
#else
  // Old API for ESP-IDF v4.x / Arduino Core 2.x
  esp_task_wdt_init(30, false);
#endif
*/
  esp_task_wdt_deinit();
  
  // initialize random
  srand( time( NULL ) );

  // initialize nvs
  nvs.begin();

  // Who I am?
  if (this->verbose) {
    printf( TRANSLATE( "Boot %s (SN:%d).\n", "Starte %s (SN:%d).\n" ), nvs.swarm.name, nvs.serialNumber );
    if ( nvs.swarm.IAmKelda )  { printf( TRANSLATE( "I am KELDA!\n", "Ich bin KELDA!\n" ) ); }

    // PSRAM
    uint32_t totalPsram = ESP.getPsramSize();
    printf( "PSRAM: %u Bytes (%.2f MB)\n", totalPsram, totalPsram / 1024.0 / 1024.0);

    // cores
    printf( TRANSLATE( "User space is running on core #%d.\nFirmware is running on core #%d.\n", "Programm läuft auf Kern #%d.\nFirmware läuft auf Kern #%d.\n" ), ARDUINO_RUNNING_CORE, ARDUINO_EVENT_RUNNING_CORE);
  }

  SwOSCtrlConfig_t localCtrlConfig = {
    .CPU           = nvs.CPU,
    .IAmKelda      = nvs.swarm.IAmKelda,
    .extensionPort = nvs.extensionPort.mode,
    .IOs           = 0,
    .pixels        = nvs.pixels,
    .gyro          = nvs.extensionPort.gyro
  };

  // initial setup?
  if (nvs.CPU >= FTSWARMMAXVERSION ) nvs.initialSetup();

  maxCtrl = 0;
  Ctrl[0] = new SwOSCtrl( nvs.serialNumber, noMac, true, localCtrlConfig );
  Ctrl[0]->setComState ( COMSTATE_ONLINE );

  SwOSCtrlConfig_t noCtrlConfig;
  bzero( &noCtrlConfig, sizeof(noCtrlConfig) );
  noCtrlConfig.CPU      = FTSWARM_NOVERSION;

  // initialize all swarm members from nvs list
  for (uint8_t i=0; i<MAXCTRL; i++) {
    
    if ( nvs.swarm.member[i] ) {
      maxCtrl++;
      Ctrl[maxCtrl] = new SwOSCtrl( nvs.swarm.member[i],  MacAddr( broadcast ), false, noCtrlConfig );
    }

  }

  // set Kelda link if i'm the Kelda
  if ( nvs.swarm.IAmKelda ) Kelda = Ctrl[0];

  // check on nvs version upgrades
  if ( nvs.upgrade() ) myOSSwarm.Ctrl[0]->saveToNVS( );

  // factory reset cycle?
  if ( nvs.factoryReset ) {

    if (verbose) printf( TRANSLATE( "finalizing factoryReset\n", "Setze auf Werkseinstellung zurück.\n" ) );

    // reset flag, don't load IO settings and save
    nvs.factoryReset = false;
    myOSSwarm.Ctrl[0]->saveToNVS( );
    nvs.save( FTSWARM_NVSSCOPE_FACTORYRESET );

  } else {

    // Open NVS again & load alias names
    myOSSwarm.Ctrl[0]->loadFromNVS( );

  }

  // now I can visualize my state
  setState( BOOTING );
  
  // wifi
  if ( nvs.wifi.mode != wifiOFF ) startWifi( );

  // Init Communication
  if (!myOSNetwork.begin( nvs.swarm.secret, nvs.swarm.pin, nvs.swarm.communication )) SWARM_LOG_FATAL("Error initializing swarm communication.");

  // start the tasks
  xTaskCreatePinnedToCore( recvTask,    "RecvTask",    10000, NULL, 1, NULL, ARDUINO_EVENT_RUNNING_CORE );
  xTaskCreatePinnedToCore( readTask,    "ReadTask",    20000, NULL, 1, NULL, ARDUINO_EVENT_RUNNING_CORE );
  xTaskCreatePinnedToCore( connectTask, "connectTask", 10000, NULL, 1, NULL, ARDUINO_EVENT_RUNNING_CORE );

  // start web server
  if ( ( nvs.wifi.webUI ) && ( nvs.wifi.mode != wifiOFF ) ) SwOSStartWebServer();

  // firmware events?
  addEvents( nvs.events.activeConfig, myOSSwarm.Ctrl[0]->serialNumber );

  if ( nvs.wifi.mode == wifiAP ) {
    if ( verbose )           SWARM_LOG_INFO( TRANSLATE( "Wifi ap mode is limited to %d network clients.", "WLAN im AP-Modus ist auf %d Netzwerk-Clients begrenzt." ), MAX_AP_CONNECTIONS );
    if ( nvs.swarm.IAmKelda) SWARM_LOG_WARN( TRANSLATE( "A swarm using wifi ap mode provided by the Kelda isn't stable. Best practice is to use your local wifi or to provide the AP via a swarm member.", "Der AP-Modus auf der Kelda ist nicht evtl. stabil. Verwenden Sie einen anderen Controller um den AP bereitzustellen." ));

  }

  initialized = true;

  testFactoryReset();
 
  setState( RUNNING );

  return Ctrl[0]->serialNumber;

}

void SwOSSwarm::testFactoryReset( void ) {

 // is a factory reset button defined?
  SwOSDigitalInput *reset = (SwOSDigitalInput *) Ctrl[0]->getIO( FACTORYSETTINGS );
  if (!reset) return;

  // wait to operate my controller and get button values
  delay(50);

  // button pressed?
  if (!reset->getValueI32()) return;

  // Controller has RGB-LEDs
  #if FTSWARM_HAL_PIXELS > 0 
  // reset toggle state
  reset->getToggle();
  
  // visualize potential factory reset
  Ctrl[0]->setState( FACTORY1 );

  // now wait max. 2s to release button
  bool toggled = false;
  for (uint8_t i=0; i<20; i++ ) {
    if (reset->getToggle() == FTSWARM_TOGGLEDOWN ) { toggled = true; break; }
    delay(100);
  }

  if (toggled) {

    // visualize going to reset
    for ( uint8_t i=0; i<4; i++ ) {
      setState( FACTORY2 );
      delay(250 );
      setState( FACTORY1 );
      delay(250 );
    }          

    factoryReset();

  }
  #endif
  
}

void SwOSSwarm::factoryReset( void ) {
    
  // halt all motors
  halt();

  // NVS - local controller & swarm settings
  nvs.reset( true );

  // restart
  nvs.saveAndRestart( FTSWARM_NVSSCOPE_FACTORYRESET );

}

void SwOSSwarm::halt( void ) {

  for (uint8_t i=0; i<=maxCtrl; i++)
    if (Ctrl[i]) Ctrl[i]->halt();
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
  if (!ctrl->changeIOType( index, ioType, io->getFlags() ) ) return NULL;

  // return corrected io
  return ctrl->io[index];

}

void SwOSSwarm::getAlias( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType, char *alias ) {

  SwOSIO* io=getIO( serialNumber, port, ioType );
  if (io) 
    strcpy( alias, io->getAlias() );
  else
    // offline?
    strcpy( alias, "???" );

}

void SwOSSwarm::getAliasOrName( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType, char *alias ) {

  SwOSIO* io=getIO( serialNumber, port, ioType );
  if (io) 
    strcpy( alias, io->getAliasOrName() );
  else
    // offline?
    strcpy( alias, "???" );

}

SwOSIO* SwOSSwarm::getIO( const char *name, SwOSIOType_t ioType ) {

  SwOSIO   *io   = NULL;
  SwOSCtrl *ctrl = NULL;

  char ctrlName[MAXIDENTIFIER];
  strcpy( ctrlName, name );
  char *ioName = strchr( ctrlName, '.' );

  if ( ioName ) {

    // via controller.ioname
    ioName[0] = '\0';
    ioName++;

    SwOSCtrl* ctrl = myOSSwarm.getController( ctrlName );
    if ( ctrl ) io = ctrl->getIO( ioName );

  } else {

    // via alias
 
    // list all controllers and check for the name
    for ( uint8_t i=0; i<=maxCtrl; i++ ) {

      // check next controller
      ctrl = Ctrl[i];
      if ( ctrl ) io = ctrl->getIO( name );

      // io found
      if (io) break;

    }

  }

  // nothing found?
  if (!io) return NULL;

  // if no special type is required, take it as it is
  if ( ioType == SWOSIO_UNDEF ) return io;

  // 100% match?
  if ( io->getIOType() == ioType ) return io;

  // compatible ioType?
  uint8_t index = ctrl->getIndex( io );
  if (!ctrl->changeIOType( index, ioType, io->getFlags() ) ) return NULL;

  // return corrected io
  return ctrl->io[index];
    
  // nothing found
  return NULL;
  
}

SwOSCtrl* SwOSSwarm::getController(char *name) {

	// search controller
	for (uint8_t i=0;i<=maxCtrl;i++) {
		if ( ( Ctrl[i] ) && ( Ctrl[i]->equals(name) ) ) {
			return Ctrl[i];
		}
	}

	// no hit
	return nullptr;

}

SwOSCtrl* SwOSSwarm::getController( FtSwarmSerialNumber_t SN ) {

	// search controller
	for (uint8_t i=0;i<=maxCtrl;i++) {
		if ( ( Ctrl[i] ) && ( Ctrl[i]->serialNumber == SN ) ) {
			return Ctrl[i];
		}
	}

	// no hit
	return nullptr;

}

size_t SwOSSwarm::approxSerialize( SerialFormat_t format ) {

  size_t size = 0;

  for (uint8_t i=0; i<=maxCtrl;i++) {

    if ( Ctrl[i] ) size += Ctrl[i]->IOs;
  
  }

  if ( format == SERIALIZE_JSON) size = size * ( MAXIDENTIFIER + 100 );
  else                           size = size * ( MAXIDENTIFIER + 30 );

  return size;

}

void SwOSSwarm::serializeEvents( Serialize *serialize ) {

  serialize->startObject( );
  serialize->item( SERIALIZE_LITERAL_ACTIVECONFIG, nvs.events.activeConfig );
  serialize->startArray( SERIALIZE_LITERAL_EVENTS );
  for (uint8_t i=0; i<=maxCtrl; i++ ) {
    if ( Ctrl ) Ctrl[i]->serializeEvents( serialize );
  }
  serialize->endArray();
  serialize->endObject();                                  

}

void SwOSSwarm::serialize( Serialize *serialize) {

  serialize->startObject( );
  serialize->item( SERIALIZE_LITERAL_NAME, nvs.swarm.name );
  serialize->item( SERIALIZE_LITERAL_KELDA, Ctrl[0]->IAmKelda );
  serialize->item( SERIALIZE_LITERAL_SYNC , sync++ );

	serialize->startArray( SERIALIZE_LITERAL_CTRLS );

	for (uint8_t i=0; i<=maxCtrl;i++) {

    // send data
    if ( Ctrl[i] ) { Ctrl[i]->lock(); Ctrl[i]->serialize( serialize ); Ctrl[i]->unlock(); }

    // visualize others only if I'm a Kelda
    if ( !Ctrl[0]->IAmKelda ) break;
    
	}

	serialize->endArray();
  serialize->endObject();

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

void SwOSSwarm::setState( SwOSState_t state, const char *errorText ) {

  if (Ctrl[0]) {
    Ctrl[0]->lock();
    Ctrl[0]->setState( state, errorText );
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

  // printf("joinMySwarm\n");
  // com.print();

}

void SwOSSwarm::replaceCtrl( SwOSCom *com, uint8_t source, uint8_t affected, const SwOSCtrlConfig_t *ctrlConfig ) {
  // replace controller in swarm list
      
  SwOSCtrl *newCtrl = NULL;
  SwOSCtrl *oldCtrl = Ctrl[source];
  
  const SwOSCtrlConfig_t *config = ctrlConfig ? ctrlConfig : &com->data.registerCmd.ctrlConfig;

  if ( config->CPU >= FTSWARMMAXVERSION ) {
    SWARM_LOG_ERROR( TRANSLATE( "Unknown controller type while adding a new controller to my swarm.", "Unbekannter Controller-Typ möchte dem Swarm beitreten." ) ); return;

  } else {
    
    newCtrl = new SwOSCtrl( com->data.sourceSN , com->macAddr, false, *config );
    
    if (verbose) { 
      SWARM_LOG_INFO( TRANSLATE( "ftSwarm%d joined the swarm.", "ftSwarm%d ist dem Swarm beigetreten." ), com->data.sourceSN ); 
    }

  }

  // replace the new controller in my list
  newCtrl->setComState( COMSTATE_ONLINE );
  Ctrl[source] = newCtrl;
  if (oldCtrl) delete oldCtrl; 
  
}

void SwOSSwarm::clearPendingIOConfig( uint8_t index ) {

  PendingIOConfigPacket *packet = pendingIOConfig[index].first;
  while ( packet ) {
    PendingIOConfigPacket *next = packet->next;
    delete packet->com;
    delete packet;
    packet = next;
  }

  pendingIOConfig[index].valid = false;
  pendingIOConfig[index].first = NULL;
  pendingIOConfig[index].last = NULL;

}

bool SwOSSwarm::queuePendingIOConfig( uint8_t index, SwOSCom *com ) {

  if ( !pendingIOConfig[index].valid ) {
    pendingIOConfig[index].ctrlConfig = com->data.ioConfigCmd.ctrlConfig;
    pendingIOConfig[index].valid = true;
  } else if ( memcmp( &pendingIOConfig[index].ctrlConfig,
                      &com->data.ioConfigCmd.ctrlConfig,
                      sizeof( SwOSCtrlConfig_t ) ) != 0 ) {
    SWARM_LOG_ERROR( TRANSLATE( "SwOSSwarm: inconsistent IOConfig for controller %d.", "SwOSSwarm: Uneinheitliche IOConfig für Controller %d." ), Ctrl[index]->serialNumber );
    clearPendingIOConfig( index );
    return false;
  }

  PendingIOConfigPacket *packet = new PendingIOConfigPacket;
  packet->com = new SwOSCom( *com );
  packet->com->bufferIndex = 0;
  packet->next = NULL;

  if ( pendingIOConfig[index].last ) pendingIOConfig[index].last->next = packet;
  else                                  pendingIOConfig[index].first = packet;
  pendingIOConfig[index].last = packet;

  return true;

}

void SwOSSwarm::replayPendingIOConfig( uint8_t index ) {

  PendingIOConfigPacket *packet = pendingIOConfig[index].first;
  while ( packet ) {
    Ctrl[index]->ioConfig( packet->com );
    packet = packet->next;
  }

  clearPendingIOConfig( index );

}

void SwOSSwarm::cmdJoinMySwarm( SwOSCom *com, uint8_t source, uint8_t affected ) {

  #ifdef DEBUG_COMMUNICATION_SWARM
    printf( "CMD_JOINMYSWARM %d source: %d affected: %d maxCtrl %d\n", com->data.cmd, source, affected, maxCtrl );
  #endif

  // not my SN and not a wildcard: ignore
  if ( ( com->data.affectedSN != Ctrl[0]->serialNumber ) && ( com->data.affectedSN != 0 ) ) return;

  // I'm a Kelda with at leat a member: decline
  if ( ( Ctrl[0]->IAmKelda ) && ( members() > 1 ) ) {

    SWARM_LOG_ERROR( TRANSLATE( "Declining to join swarm %s. I'm a Kelda with %d swarm members.", "Kann dem Swarm %s nicht beitreten. Ich bin eine Kelda mit %d Controllern im Swarm." ), com->data.joinCmd.swarmName, members() );
    SwOSCom reply( com->macAddr, com->data.sourceSN, CMD_JOINNACK );
    Ctrl[0]->registerMe( &reply );
    reply.send();

    return;

  }

  // I'm fine to join the swarm: ack

  // take swarm settings
  nvs.swarm.pin = com->data.registerCmd.swarmPIN;
  strcpy( nvs.swarm.name, com->data.registerCmd.swarmName );

  // replace old controller
  replaceCtrl( com, source, affected );

  // getting member, knowing my Kelda
  deleteEvents();
  Ctrl[0]->IAmKelda = false;
  nvs.swarm.IAmKelda = false;
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
    if ( Ctrl[source]->getCPU() == FTSWARM_NOVERSION ) {
      if ( pendingIOConfig[source].valid ) replaceCtrl( com, source, affected, &pendingIOConfig[source].ctrlConfig );
      else                              replaceCtrl( com, source, affected );
      if ( Ctrl[source]->getCPU() != FTSWARM_NOVERSION ) {
        replayPendingIOConfig( source );
        if ( Ctrl[0]->IAmKelda ) addEvents( nvs.events.activeConfig, Ctrl[source]->serialNumber );
      }
    }
    // ToDo else - send the controller his state

    // wait for alias settings
    Ctrl[source]->setComState( COMSTATE_CONNECT_PHASE2 ); 

  }

}

void SwOSSwarm::cmdRevokeFromSwarm( SwOSCom *com, uint8_t source, uint8_t affected ) {
  // Kelda to member: get out of my swarm

  // for me?
  if ( ( com->data.affectedSN != Ctrl[0]->serialNumber ) || (com->data.joinCmd.pin == nvs.swarm.pin) || strcmp( com->data.registerCmd.swarmName, nvs.swarm.name ) ) return;

  // User info
  SWARM_LOG_INFO( TRANSLATE( "Leaving swarm %s and rebooting.", "Verlasse Swarm %s und starte neu." ), com->data.registerCmd.swarmName );

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

    case CMD_IOCONFIG:        // needs to be initiated at swarm level to be able to start events
                              if ( !Ctrl[affected] ) break;
                              if ( Ctrl[affected]->IOs == 0 ) {
                                queuePendingIOConfig( affected, com );
                                break;
                              }
                              if ( Ctrl[affected]->IOs != com->data.ioConfigCmd.ctrlConfig.IOs ) {
                                SWARM_LOG_ERROR( TRANSLATE( "SwOSSwarm: SN %d IO count %d differs from received %d.", "SwOSSwarm: SN %d IO-Anzahl %d unterscheidet sich von empfangenen %d." ), Ctrl[affected]->serialNumber, Ctrl[affected]->IOs, com->data.ioConfigCmd.ctrlConfig.IOs );
                                break;
                              }
                              if ( ( Ctrl[affected]->ioConfig( com ) ) && ( Ctrl[0]->IAmKelda ) ) {
                                addEvents( nvs.events.activeConfig, Ctrl[affected]->serialNumber );
                              }
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
      clearPendingIOConfig( i );
      oldCtrl->lock();
      delete oldCtrl;
    }
    
  }

  // delete all nvs events
  nvs.deleteAllEvents();

  // set new swarm
  nvs.swarm.IAmKelda = true;
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

bool SwOSSwarm::isOnline( void ) {
  // Test, if SN is online

  for (uint8_t i=0; i<=maxCtrl; i++) {
    if ( ( Ctrl[i] ) && ( !Ctrl[i]->isOnline( ) ) ) return false;
  }

  return true;

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
  clearPendingIOConfig( i );
  oldCtrl->lock();
  delete oldCtrl;
  nvs.deleteController( serialNumber );

  return true;

}

// delete an event
bool SwOSSwarm::deleteEvent( SwOSNVSEvent *event ) {

  // no event
  if (!event) return false;

  // get IOs
  SwOSInput *sensor = (SwOSInput *) getIO( event->sensor );
  SwOSIO    *actor  = getIO( event->actor );

  // sensor or actor doesn't exist
  if ( (!sensor) || (!actor) ) return false;

  return sensor->deleteEvent( event->triggerMath.bits.trigger, event->triggerMath.bits.op, actor );

}

// add an event
bool SwOSSwarm::addEvent( SwOSNVSEvent *event ) {

  // no event
  if (!event) return false;

  // get IOs
  SwOSInput *sensor = (SwOSInput *) getIO( event->sensor );
  SwOSIO    *actor  = getIO( event->actor );

  // sensor or actor doesn't exist
  if ( (!sensor) || (!actor) ) return false;

  return sensor->addEvent( event->triggerMath.bits.trigger, event->triggerMath.bits.op, event->triggerMath.bits.v1, event->triggerMath.bits.v2, actor, event->parameter );

}

void SwOSSwarm::deleteEvents( void ) {

  for (uint8_t i=0; i<=maxCtrl; i++) {

    if (Ctrl[i]) Ctrl[i]->deleteEvents();

  }

}

void SwOSSwarm::addEvents( uint8_t config, FtSwarmSerialNumber_t sn ) {

  // Kelda only
  if (!Ctrl[0]->IAmKelda) return;

  // stop old config
  deleteEvents();

  // start new config
  for ( uint i=0; i<MAXNVSEVENTS; i++ ) {

    // end of list?
    if ( nvs.events.events[config][i].sensor.serialNumber == 0 ) return;

    // add event, if sn is fitting
    if ( ( sn == 0 ) || ( nvs.events.events[config][i].sensor.serialNumber == sn ) || ( nvs.events.events[config][i].actor.serialNumber == sn ) ) addEvent( &nvs.events.events[config][i] );

  }

}

void SwOSSwarm::save( FtSwarmNVSScope_t scope ) {
 
  for (uint8_t i=0; i<=maxCtrl; i++) {
    if ( Ctrl[i] ) Ctrl[i]->save( scope, SWOS_NOPORT );
  }
  
}

bool SwOSSwarm::IOAvaliable( const char *name ) {

  for ( uint8_t i=0; i<MAXCTRL; i++ ) {
    SwOSCtrl *ctrl = Ctrl[i];
    if ( ctrl && ctrl->IOAvaliable( name ) ) return true;
  }

  return false;
  
}