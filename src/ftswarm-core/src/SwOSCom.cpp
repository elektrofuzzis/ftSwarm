/*
 * SwOSCom.cpp
 *
 * Communication between your controlers
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <esp_now.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <esp32/rom/crc.h>
#include <soc/uart_struct.h>
#include <driver/uart.h>

#include "SwOSCom.h"
#include "SwOSNVS.h"

#include <esp_debug_helpers.h>

// #define DEBUG_COMMUNICATION
// #define DEBUG_TXCOMMUNICATION
// #define DEBUG_MONITOR

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define RS485_TXD       GPIO_NUM_18
#define RS485_RXD       GPIO_NUM_6
#define RS485_REB       GPIO_NUM_7
#define RS485_DE        GPIO_NUM_17
#define RS485_UART      UART_NUM_2
#define RS485_BAUD_RATE 1843200 
#define RS485_BUF_SIZE  (1024)
#define PATTERN_CHR_NUM (3) 

#define RS485_MAXDELAY  100

typedef struct {
    uint8_t macAddr;
    esp_now_send_status_t status;
} sendNotificationEvent_t;

#define RS485MAXPAYLOAD 256

const uint32_t RS485FRAMESTART = 0xFEFEFEFE;
const uint32_t RS485FRAMEEND   = 0xEFEFEFEF;

#define RS485_NOPAYLOAD -1
#define RS485_INVALIDFRAME -2
#define RS485_PAYLOAD 0

struct RS485Frame_t {
  uint32_t              frameStart;
  uint16_t              size;
  uint32_t              crc;
  uint8_t               payload[RS485MAXPAYLOAD];
} __attribute__((packed));


void dumpBuffer( uint8_t *buf, int len ) {
  
  uint8_t *ptr = buf;

  for (int i=0; i<len; i++) {
    switch ( i % 16 ) {
      case 0:  printf("%02X - %02X ", i, *ptr++); break;
      case 15: printf("%02X\n", *ptr++ );break;
      default: printf("%02X ", *ptr++ ); break;
    }
  }

  printf("\n");

}

/***************************************************
 * 
 * MacAddr
 * 
 ***************************************************/

MacAddr::MacAddr() {
  bzero( addr, ESP_NOW_ETH_ALEN );
}

MacAddr::MacAddr( uint8_t addr[ESP_NOW_ETH_ALEN] ) {

  memcpy( this->addr, addr, ESP_NOW_ETH_ALEN);

}

MacAddr::MacAddr( const uint8_t *addr ) {

  memcpy( this->addr, addr, ESP_NOW_ETH_ALEN);

}

bool MacAddr::isEqual( uint8_t addr[ESP_NOW_ETH_ALEN]) {

  for (uint8_t i=0; i<ESP_NOW_ETH_ALEN; i++ ) {
    if (this->addr[i] != addr[i] ) return false;
  }

  return true;

}

bool MacAddr::isNull( void ) {
  return isEqual( (uint8_t *) noMac );
}

bool MacAddr::isBroadcast( void ) {
  return isEqual( (uint8_t *) broadcast );
}

void MacAddr::set( MacAddr macAddr ) {
  memcpy( this->addr, macAddr.addr, ESP_NOW_ETH_ALEN );
}

void MacAddr::print( void ) {
  printf( "mac:");
  for (uint8_t i=0; i<ESP_NOW_ETH_ALEN;i++) printf(" %02X", addr[i] );
}

/***************************************************
 * 
 * SwOSCom
 * 
 ***************************************************/

SwOSCom::SwOSCom() {

  // just cleanup everything
  bzero( &data, sizeof(data) );
  bufferIndex = 0;
  _isValid = false;

}

SwOSCom::SwOSCom( MacAddr macAddr, const uint8_t *buffer, int length):SwOSCom() {

  if (length>sizeof(data)) {
    ESP_LOGE( LOGFTSWARM, "SwOSCOM: max. packet size exceeded.");
    dumpBuffer( (uint8_t *) buffer, length );
  }

  // set mac
  this->macAddr.set( macAddr );
  
  // copy data
  bzero( &data, sizeof( SwOSDatagram_t ) );
  memcpy( &data, buffer, min( length, sizeof(data) ) );

  bool isJoin    = ( ( data.secret == DEFAULTSECRET ) && ( data.cmd == CMD_SWARMJOIN )    && ( data.joinCmd.pin == myOSNetwork.pin ) && ( myOSNetwork.pin != 0 ) ) ;
  bool isJoinAck = ( ( data.secret == DEFAULTSECRET ) && ( data.cmd == CMD_SWARMJOINACK ) && ( data.joinCmd.pin == myOSNetwork.pin ) && ( myOSNetwork.pin != 0 ) ) ;

  #ifdef DEBUG_COMMUNICATION
    printf("isvalid: isJoin = %d, isJoinAck = %d, length = %d, size = %d, cmd = %d, version = %d\n",
            isJoin, isJoinAck, length, size(), data.cmd, data.version);
  #endif

  _isValid = ( ( ( data.secret == myOSNetwork.secret ) || isJoin || isJoinAck ) &&
               ( length == size( ) ) &&
               ( data.cmd < CMD_MAX ) &&
               ( data.version == VERSIONDATA ) );

}

SwOSCom::SwOSCom( MacAddr macAddr, FtSwarmSerialNumber_t affectedSN, SwOSCommand_t cmd ):SwOSCom() {

  // set mac
  this->macAddr.set( macAddr );

  // set SNs
  data.sourceSN = nvs.serialNumber;
  data.affectedSN = affectedSN;
  
  // set header
  data.secret    = myOSNetwork.secret;
  data.version   = VERSIONDATA;
  data.cmd       = cmd;

  // sendBuffered
  bufferIndex = 0;
  
  // valid data
  _isValid = true;
}

size_t SwOSCom::size( void ) {

  // size is everything with non-zero values
  uint8_t *ptr = (uint8_t *) (&data) + sizeof(data)-1;
  size_t len = sizeof(data);

  while (len > 0) {
    if (*ptr != 0 ) break;
    *ptr--;
    len--;
  }

  return len;

}

void SwOSCom::sendBuffered(char *name, char *alias ) {

  // anyting ToDo?
  if ( (!alias) || (alias[0]=='\0') ) return;

  // copy data to buffer
  strcpy( data.aliasCmd.alias[bufferIndex].name, name );
  strcpy( data.aliasCmd.alias[bufferIndex].alias, alias );
  (bufferIndex)++;

  // buffer full?
  if (bufferIndex>=MAXALIAS) flushBuffer( );
  
}

void SwOSCom::flushBuffer( ) {

  // send data
  send();

  // cleanup
  bufferIndex = 0;
  memset( &data.aliasCmd, 0, sizeof( data.aliasCmd ) );
  
}

void SwOSCom::print() {

  macAddr.print();
  printf("size: %d\n", size() );
  printf("secret: %04X\n", data.secret);
  printf("source: %d\n", data.sourceSN);
  printf("affected: %d\n", data.affectedSN);
  printf("command: %d\n", data.cmd);
  printf("isValid: %d _isvalid:%d\n", isValid(), _isValid);

  uint8_t *buffer = (uint8_t *) &data;
  size_t len = size();
  for (uint8_t i=0; i<len; i=i+16) {

    // print hex
    for (uint8_t j=0; j<16; j++ ) {
      if (i+j < len) printf("%02X ", buffer[i+j] );
      else printf("   ");
    }

    printf (" - ");

    // print ascii
    for (uint8_t j=0; j<16; j++ ) {
      if (i+j < len) {
        if ( ( buffer[i+j] > 32 ) && ( buffer[i+j] < 127 ) )
          printf("%c", (char)buffer[i+j] );
        else
          printf(".");
      }
    }

    printf("\n");

  }

}

void SwOSCom::send( void ) {

  #ifdef DEBUG_MONITOR
    // Monitor mode, don't send data
    return ESP_OK;
  #endif

  // header
  data.size = size();
  
  #ifdef DEBUG_TXCOMMUNICATION
  if ( data.cmd != 8 ) {
    printf("\nSwOSCom.send\n");
    print();
  }
  #endif

  if ( xQueueSend( myOSNetwork.tx_queue, this, ESPNOW_MAXDELAY ) != pdTRUE ) {
    ESP_LOGE( LOGFTSWARM, "xQueueSend tx_queue queue failed." );
    print();
  }

}

/******************************************************************************
 * 
 * Communication stack
 * 
 *****************************************************************************/

static void _OnDataSent(const uint8_t *macAddr, esp_now_send_status_t status) { 
  // works on transmitted packets
  
  // stop on uninitialized
  if (!myOSNetwork.sendNotificationWifi) return;

  // store event
  sendNotificationEvent_t event;
  event.status = status;
  memcpy( &event.macAddr, macAddr, ESP_NOW_ETH_ALEN );

  // and send it back
  if ( xQueueSend( myOSNetwork.sendNotificationWifi, &event, ESPNOW_MAXDELAY ) != pdTRUE ) {
    ESP_LOGW( LOGFTSWARM, "SendNotification queue fail" );
  } 

};

bool _OnDataRecv( SwOSCom *payload ) {

  if (!payload) {
    // shouldn't happen at all
    ESP_LOGE( LOGFTSWARM, "_OnDataRecv payload is null");
    return false;
  }

  // Did a ftSwarm sent this data?
  if ( ( !payload->isValid() ) || ( !myOSNetwork.recvNotification ) ) {
    #ifdef DEBUG_COMMUNICATION
      printf( "_OnDataRecv: invalid buffer\n");
      payload->print();
    #endif
    return false;
  }

  #ifdef DEBUG_COMMUNICATION
    printf("\n_OnDataRecv buffer me: %d:\n", nvs.serialNumber);
    payload->print();
  #endif

  if  ( payload->data.sourceSN == nvs.serialNumber ) {
    // multicast from myself
    #ifdef DEBUG_COMMUNICATION
      printf( "_onDataRecv: my own multicast.\n");
    #endif

  } else if ( ( payload->data.affectedSN != nvs.serialNumber ) && ( payload->data.affectedSN != broadcastSN )  && ( !nvs.IAmKelda ) ) {
    // direct communication to somebody else
    #ifdef DEBUG_COMMUNICATION
      printf( "_onDataRecv: to someone else.\n");
    #endif

  } else {
    // relevant data
    if ( xQueueSend( myOSNetwork.recvNotification, payload, ESPNOW_MAXDELAY ) != pdTRUE ) { ESP_LOGW( LOGFTSWARM, "RecvNotification queue fail" ); } 
  }

  return true;

}

void _OnDataRecvWifi(const uint8_t *macAddr, const uint8_t *incomingData, int len ) {

  SwOSCom payload( MacAddr( macAddr ), incomingData, len );
  _OnDataRecv( &payload );

} 


static void logBuffer( uint8_t *buffer, int bufPtr ) {

  #ifdef DEBUG_COMMUNICATION

    for (uint8_t i=0;i<bufPtr;i++) {
      printf("%02X ", buffer[i]);
      if ((i % 16) == 15) printf("\n");
    }
  
    printf("\n");

  #endif

}

static void tx_RS485( SwOSCom *com ) {

  RS485Frame_t frame;
  bool         collision;
  
  // cleanup frame
  bzero(&frame, sizeof(frame));

  // set frame header
  frame.frameStart = RS485FRAMESTART;
  frame.size       = (uint8_t *)&frame.payload - (uint8_t *)&frame + com->data.size + sizeof( RS485FRAMEEND );
  frame.crc        = 0;

  // copy payload
  memcpy( &frame.payload, &com->data, com->data.size );
      
  // set frame end delimiter
  memcpy( &frame.payload[ com->data.size ], &RS485FRAMEEND, sizeof( RS485FRAMEEND ) );
      
  // crc 
  frame.crc = (~crc32_le((uint32_t)~(0xffffffff), (const uint8_t*)&frame, frame.size))^0xffffffff;
  
  // try 3 times to send data
  for ( uint8_t retry=0; retry<3; retry++ ) {

    // send data
    digitalWrite( RS485_DE, 1 );
    uart_write_bytes(RS485_UART, (void *)&frame, frame.size );
    uart_wait_tx_done(RS485_UART, RS485_MAXDELAY);
    digitalWrite( RS485_DE, 0 );

    uart_get_collision_flag(RS485_UART, &collision );

    // stop on correct transmission
    if ( !collision ) break;

    // wait a random time to start retransmission
    ets_delay_us( myOSNetwork.delayTime ); 

  }
    
}

void SwOSNetwork::AddPeer( MacAddr macAddr ) {

  // need to register a new peer?  
  if ( esp_now_is_peer_exist( macAddr.addr ) ) return;

  // allocate some memory
  esp_now_peer_info_t *peerInfo = (esp_now_peer_info_t *) malloc( sizeof( esp_now_peer_info_t ) );

  // initialize
  memset( peerInfo, 0, sizeof(esp_now_peer_info_t) );
  memcpy( peerInfo->peer_addr, macAddr.addr, ESP_NOW_ETH_ALEN );
  // peerInfo->channel = 0;  
  // peerInfo->encrypt = true;

  // and add it to the internal peer list
  esp_err_t err = esp_now_add_peer( peerInfo );
  if ( err != ESP_OK ){
    ESP_LOGE(LOGFTSWARM, "Failed to add peer\n");
    return;
  }

}

static void tx_Wifi( SwOSCom *com ) {

  int waitAck = 0;
  sendNotificationEvent_t event;

  if ( com->macAddr.isNull() ) {
    printf( "ERROR: try rto send data via wifi to MAC 00:00:00:00\n" );
    com->print();
    while (1) delay(50);
  }
    
  myOSNetwork.AddPeer( com->macAddr );
  waitAck = 1;
  esp_now_send( com->macAddr.addr, (uint8_t *) &com->data, com->size() );

  // wait until all packages are sent
  while ( waitAck > 0) {
    if ( xQueueReceive( myOSNetwork.sendNotificationWifi, &event, ESPNOW_MAXDELAY ) == pdTRUE ) {
      waitAck--;
    }
  }

  #ifdef DEBUG_TXCOMMUNICATION
    printf("\n****\nsend wifi end\n");
  #endif

}


static void tx_task( void *pvParameters) {

  SwOSCom com;
  bool wifi  = ( myOSNetwork.communication & swarmComWifi );
  bool rs485 = ( myOSNetwork.communication & swarmComRS485 );

  while(1) {

    // wait to send data
    if (pdTRUE == xQueueReceive( myOSNetwork.tx_queue, &com, portMAX_DELAY )) {

      if (rs485) tx_RS485( &com );
      if (wifi)  tx_Wifi( &com );
      

    }

  }

}

uint8_t *RS485ScanPattern( uint8_t *buffer, uint32_t pattern, int len ) {

  if ( len < sizeof(RS485FRAMEEND) ) return NULL;

  uint8_t *bufPtr = buffer;

  // search in buffer
  for (int i=0; i <= len-sizeof(pattern); i++ ) {

    // check on Pattern
    if ( *((uint32_t *)bufPtr) == pattern ) return bufPtr;

    // incr bufPtr
    bufPtr++;

  }

  // no pattern found
  return NULL;
  
}

int RS485GetPayload( uint8_t *buffer, int buflen, uint8_t **payload, int *sizePayload, int *cleanup ) {

  bool validFrame = false;
  RS485Frame_t *frame;
  
  *cleanup = 0;

  // scan for end characters
  uint8_t *endFrame = RS485ScanPattern( buffer, RS485FRAMEEND, buflen );
  if (!endFrame) return RS485_NOPAYLOAD;

  // now check, if there are fitting start frame characters
  uint8_t *startFrame = RS485ScanPattern( buffer, RS485FRAMESTART, endFrame - buffer );

  while ( startFrame ) {

    // check frame
    int framesize = ( endFrame - startFrame ) + sizeof( RS485FRAMEEND );
    frame = (RS485Frame_t *) startFrame;

    // size?
    // validFrame = framesize >( (uint8_t*)&frame->payload - (uint8_t*)frame ); 
    validFrame = framesize > ( sizeof( RS485Frame_t ) - RS485MAXPAYLOAD );  // frame must contain at least one byte data
    validFrame = validFrame && ( frame->size == framesize );
    
    // crc?
    if (validFrame) {
      uint32_t crcReceived = frame->crc;
      frame->crc = 0;
      uint32_t crcFrame = (~crc32_le((uint32_t)~(0xffffffff), (const uint8_t*)frame, frame->size))^0xffffffff;
      frame->crc = crcReceived;
      validFrame = validFrame && ( crcReceived == crcFrame );
    }

    if (validFrame) {
      // valid frame found, extract payload
      *sizePayload = framesize - ( (uint8_t*)&(frame->payload) - (uint8_t *)frame )- sizeof( RS485FRAMEEND );
      *payload = frame->payload;
      break;
  
    } else {
      // no valid frame found, search for next startFrame
      startFrame += sizeof(RS485FRAMESTART);
      startFrame  = RS485ScanPattern( startFrame, RS485FRAMESTART, endFrame - startFrame );

    }
  
  }

  *cleanup = endFrame - buffer + sizeof( RS485FRAMEEND );

  // result
  if (validFrame) {
    return RS485_PAYLOAD;

  } else {
    return RS485_INVALIDFRAME;
  
  }

}

static void RS485_rx_task(void *pvParameters) {
  
  uart_event_t   event;
  uint8_t        *buffer = (uint8_t *) calloc(1, RS485_BUF_SIZE);
  int            buflen  = 0;
  uint8_t *payload;
  int     sizePayload;
  int     cleanup;

  while(1) {

    // wait for data
    if( pdTRUE == xQueueReceive(myOSNetwork.RS485_rx_queue, (void * )&event, portMAX_DELAY ) ) { 

      switch (event.type) {
        
        case UART_FIFO_OVF:
        case UART_BUFFER_FULL:  ESP_LOGE( LOGFTSWARM, "RS485 Buffer Overflow");
                                uart_flush(RS485_UART);
                                bzero( buffer, RS485_BUF_SIZE);
                                buflen = 0;
                                break;

        case UART_BREAK:       
        case UART_DATA:         if ( event.size > (size_t) RS485_BUF_SIZE ) {

                                  // UART_BREAK has sometimes unrealistic sizes
                                  // just ignore

                                } else if ( buflen + event.size > RS485_BUF_SIZE ) {

                                  // new data exceeds my buffer size
                                  uart_flush(RS485_UART);
                                  bzero( buffer, RS485_BUF_SIZE);
                                  buflen = 0;
                                
                                } else {

                                  // process real data
                                  
                                  uart_read_bytes(RS485_UART, &buffer[buflen], event.size, portMAX_DELAY);
                                  #ifdef DEBUG_MONITOR
                                    dumpBuffer( &buffer[buflen], event.size );
                                  #endif
                                  buflen += event.size;

                                  // interpret data
                                  int result = RS485_PAYLOAD; // just to start with something else than RS485_NOPAYLOAD 

                                  while ( result != RS485_NOPAYLOAD ) {

                                    result = RS485GetPayload( buffer, buflen, &payload, &sizePayload, &cleanup );

                                    if ( result ==  RS485_PAYLOAD ) {
                                      // copy payload to a SwOSCom packet
                                      SwOSCom packet( MacAddr( noMac), payload, sizePayload );
                                      // process packet
                                      _OnDataRecv( &packet );
                                    } 

                                    // cleanup
                                    if ( ( cleanup > RS485_BUF_SIZE) || ( cleanup < 0 ) || ( cleanup > buflen ) ) {
                                      ESP_LOGE( LOGFTSWARM, "cleanup %d\n", cleanup );
                                      dumpBuffer( buffer, buflen );
                                      while (1) delay(500);
                                    }

                                    if ( cleanup > 0) {
                                      int newbuflen = buflen - cleanup;
                                      memcpy( buffer, &buffer[cleanup], newbuflen );
                                      bzero( &buffer[newbuflen], buflen - newbuflen );
                                      buflen = newbuflen;                                   
                                    }

                                  }

                                  #ifdef DEBUG_MONITOR
                                    printf("done\n");                                      
                                  #endif

                                }
                                break;
                               
        default:               ESP_LOGE( LOGFTSWARM, "RS485 unhandled event %d", event.type);
                               break;
      }

    }

  }

}

bool SwOSNetwork::_StartRS485( void ) {
  
  // Initialize RS485 communication stack

  # if !defined(CONFIG_IDF_TARGET_ESP32S3)
    // ftSwarmJST, ftSwarmControl: no RS485
    return true; 

  #else
    // ftSwarmRS, XL, PwrDrive, Duino
    RS485_rx_queue = xQueueCreate(10, sizeof( SwOSDatagram_t ) );

    // REB & DE
    pinMode( RS485_REB, OUTPUT );
    digitalWrite( 7, 0 ); // enable
    pinMode( RS485_DE, OUTPUT );
    digitalWrite( RS485_DE, 0 );  // set to receiver

    // UART configuration
    uart_config_t uart_config = {
        .baud_rate = RS485_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };
    ESP_ERROR_CHECK(uart_param_config(RS485_UART, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(RS485_UART, RS485_TXD, RS485_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(RS485_UART, RS485_BUF_SIZE,  0, 20, &RS485_rx_queue, 0));
    ESP_ERROR_CHECK(uart_set_mode(RS485_UART, UART_MODE_RS485_COLLISION_DETECT));
    UART1.rs485_conf.rs485tx_rx_en = 1;   // loopback
    UART1.rs485_conf.rs485rxby_tx_en = 0; // don't send data if receiver is busy - reduce collitions

    if ( myOSNetwork.communication & swarmComRS485 ) {
      xTaskCreate( RS485_rx_task, "RS485_rx_task", 10240, NULL, 12, NULL);
    }

    return true;
  
  #endif

}

bool SwOSNetwork::_StartWifi( void ) {

  myOSNetwork.sendNotificationWifi = xQueueCreate(MAXCTRL+10, sizeof( sendNotificationEvent_t ) );

  // create dummy event
  sendNotificationEvent_t event;
  event.status = ESP_NOW_SEND_SUCCESS ;
  memcpy( &event.macAddr, broadcast, ESP_NOW_ETH_ALEN );

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    ESP_LOGE(LOGFTSWARM, "ERROR: couldn't initialize esp_now.");
    return false;
  }

  // register callback funtions
  esp_now_register_send_cb(_OnDataSent);
  esp_now_register_recv_cb(_OnDataRecvWifi);

  return true;

}

bool SwOSNetwork::begin( uint16_t swarmSecret, uint16_t swarmPIN, FtSwarmCommunication_t swarmCommunication ) {

  // initialize all stuff
  communication = swarmCommunication;
  myOSNetwork.setSecret( swarmSecret, swarmPIN );
  delayTime = 5 + nvs.serialNumber & 0x1F; // between 5 and 36 us

  // create queues
  myOSNetwork.recvNotification = xQueueCreate( 20, sizeof( SwOSCom ) );
  myOSNetwork.tx_queue         = xQueueCreate( 20, sizeof( SwOSCom ) );
  myOSNetwork.userEvent        = xQueueCreate(  5, sizeof( SwOSCom ) );

  bool ok = true;

  if ( nvs.wifiMode != wifiOFF ) { 
    // if wifi is on, initialize wifi
    ok = ok && _StartWifi( );
  } 
  
  // always initialize RS485
  ok = ok && _StartRS485( );

  xTaskCreate( tx_task, "tx_task", 10240, NULL, 12, NULL);
  
  return ok;
}

void SwOSNetwork::setSecret( uint16_t swarmSecret, uint16_t swarmPIN ) {

  // set swarmSecret
  secret = swarmSecret;
  pin    = swarmPIN;

}

bool SwOSNetwork::hasJoinedASwarm( void ) {
  return secret == DEFAULTSECRET;
}

SwOSNetwork myOSNetwork;