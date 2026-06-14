/*
 * SwOSCom.cpp
 *
 * Communication between your controllers
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
#include <soc/uart_struct.h>
#include <driver/uart.h>
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#include <rom/crc.h>
#else
#include <esp32/rom/crc.h>
#endif

#include "SwOSCom.h"
#include "SwOSNVS.h"

#include <esp_debug_helpers.h>
#include <SwOSLog.h>


// Debug options:
// DEBUG_COMMUNICATION    show RX-Communication
// DEBUG_TXCOMMUNICATION  show TX-Connumication
// DEBUG_MONITOR          just monitor the bus and show communication

// #define DEBUG_COMMUNICATION
// #define DEBUG_COMMUNICATION_DETAIL
// #define DEBUG_TXCOMMUNICATION
// #define DEBUG_MONITOR

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define RS485_UART      UART_NUM_2
#define RS485_BUF_SIZE  4096
#define PATTERN_CHR_NUM (3) 

#define RS485_MAXDELAY  100

const int BAUDRATE[5] = { 115200, 230400, 460800, 921600, 1843200};

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

}

SwOSCom::SwOSCom( MacAddr macAddr, const uint8_t *buffer, int length):SwOSCom() {

  if (length>sizeof(data)) {
    SWARM_LOG_ERROR( TRANSLATE( "SwOSCOM: max. packet size exceeded.", "SwOSCOM: maximale Paketgröße überschritten." ) );
    dumpBuffer( (uint8_t *) buffer, length );
  }

  // set mac
  this->macAddr.set( macAddr );
  
  // copy data
  bzero( &data, sizeof( SwOSDatagram_t ) );
  memcpy( &data, buffer, min( length, sizeof(data) ) );

  #ifdef DEBUG_COMMUNICATION_DETAIL
    SWARM_LOG_INFO( "isvalid: length = %d, size = %d, cmd = %d, version = %d\n", length, size(), data.cmd, data.version);
  #endif

  _isValid = ( ( length == size( ) ) &&
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

void SwOSCom::pushIO( uint8_t index, SwOSIOType_t ioType, uint8_t port, const char *name, const char *alias, uint8_t flags, uint8_t *parameter, uint8_t size ) {

  uint8_t len_name  = strlen( name );
  uint8_t len_alias = strlen( alias );
  uint8_t psize = parameter ? size:0;
  
  // not enough space to add to buffer?
  if ( ( bufferIndex + len_name + len_alias + psize + 8 ) >= MAXCONFIGPAYLOAD ) flushBuffer();

  // index
  data.ioConfigCmd.payload[bufferIndex++] = index;

  // ioType
  data.ioConfigCmd.payload[bufferIndex++] = (uint8_t) ioType;

  // port
  data.ioConfigCmd.payload[bufferIndex++] = port;

  // name
  strcpy( (char*) &(data.ioConfigCmd.payload[bufferIndex]), name );
  bufferIndex += len_name + 1;

  // alias
  strcpy( (char*) &(data.ioConfigCmd.payload[bufferIndex]), alias );
  bufferIndex += len_alias + 1;

  // flags
  data.ioConfigCmd.payload[bufferIndex++] = (uint8_t) flags;

  // parameter
  data.ioConfigCmd.payload[bufferIndex++] = psize;
  if (psize) {
    memcpy( &data.ioConfigCmd.payload[bufferIndex], parameter, psize );
    bufferIndex += psize;
  }
  
}

void SwOSCom::flushBuffer( ) {

  // end of data
  data.ioConfigCmd.payload[bufferIndex] = 255;

  // send data
  send();

  // cleanup
  bufferIndex = 0;
  memset( &data.ioConfigCmd, 0, sizeof( data.ioConfigCmd ) );
  
}

bool SwOSCom::popIO( uint8_t *index, SwOSIOType_t *ioType, uint8_t *port, char **name, char **alias, uint8_t *flags, uint8_t *parameter, uint8_t *size ) {

  // end of data?
  if ( data.ioConfigCmd.payload[bufferIndex] == 255 ) return false;

  uint8_t len_name  = strlen( (char *) &(data.ioConfigCmd.payload[bufferIndex + 3]) );
  uint8_t len_alias = strlen( (char *) &(data.ioConfigCmd.payload[bufferIndex + 4 + len_name] ) );
  
  // corrupt packet?
  if ( ( bufferIndex + len_name + len_alias + 5 ) >= MAXCONFIGPAYLOAD ) {
    SWARM_LOG_FATAL( TRANSLATE( "SwOSCOM::popIO corrupt packet found.", "SwOSCOM::popIO beschädigtes Paket gefunden." ) );
  }
 
  *index        = data.ioConfigCmd.payload[bufferIndex++]; 
  *ioType       = ( SwOSIOType_t ) data.ioConfigCmd.payload[bufferIndex++];
  *port         = data.ioConfigCmd.payload[bufferIndex++];
  *name         = ( char * ) &(data.ioConfigCmd.payload[bufferIndex]); bufferIndex += len_name+1;
  *alias        = ( char * ) &(data.ioConfigCmd.payload[bufferIndex]); bufferIndex += len_alias+1;
  *flags        = ( uint8_t ) data.ioConfigCmd.payload[bufferIndex++];
  uint8_t pSize = ( uint8_t ) data.ioConfigCmd.payload[bufferIndex++];

  // enough space to copy parameters?
  if ( pSize > *size ) SWARM_LOG_FATAL( TRANSLATE( "SwOSCOM::popIO parameter exceeds size.", "SwOSCOM::popIO Parameter überschreitet die Maximalgröße." ) );

  // copy parameters
  *size = pSize;
  if ( pSize ) {
    memcpy( parameter, (uint8_t *) &data.ioConfigCmd.payload[bufferIndex], pSize );
    bufferIndex += pSize;
  }
  
  return true;

}

void SwOSCom::print() {

  macAddr.print();
  printf("size: %d\n", size() );
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
    return;
  #endif

  if (!myOSNetwork.active ) return;

  // header
  data.size = size();
  
  #ifdef DEBUG_TXCOMMUNICATION
  if ( data.cmd != CMD_STATE ) {
    SWARM_LOG_INFO("SwOSCom.send\n");
    print();
  }
  #endif

  if ( xQueueSend( myOSNetwork.tx_queue, this, ESPNOW_MAXDELAY ) != pdTRUE ) {
    SWARM_LOG_ERROR( TRANSLATE( "xQueueSend tx_queue queue failed.", "xQueueSend tx_queue queue ist fehlgeschlagen." ) );
    print();
  }

}

/******************************************************************************
 * 
 * Communication stack
 * 
 *****************************************************************************/

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 1, 0)
static void _OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  const uint8_t* mac_addr = tx_info->des_addr;
#else
static void _OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
#endif

  // works on transmitted packets

  // stop on uninitialized
  if (!myOSNetwork.sendNotificationWifi) return;
  if (!myOSNetwork.active ) return;

  // store event
  sendNotificationEvent_t event;
  event.status = status;
  memcpy( &event.macAddr, mac_addr, ESP_NOW_ETH_ALEN );

  // and send it back
  if ( xQueueSend( myOSNetwork.sendNotificationWifi, &event, ESPNOW_MAXDELAY ) != pdTRUE ) {
    SWARM_LOG_ERROR( TRANSLATE( "SendNotification queue fail", "SendNotification queue ist fehlgeschlagen." ) );
  } 

};

bool _OnDataRecv( SwOSCom *payload ) {

  if (!myOSNetwork.active ) return false;

  if (!payload) {
    // shouldn't happen at all
    SWARM_LOG_ERROR( TRANSLATE( "_OnDataRecv payload is null", "_OnDataRecv Payload ist null" ) );
    return false;
  }

  // Did a ftSwarm sent this data?
  if ( ( !payload->isValid() ) || ( !myOSNetwork.recvNotification ) ) {
    #ifdef DEBUG_COMMUNICATION
      SWARM_LOG_INFO( TRANSLATE( "_OnDataRecv: invalid buffer\n", "_OnDataRecv: ungültiger Buffer\n" ) );
      payload->print();
    #endif
    return false;
  }

  #ifdef DEBUG_COMMUNICATION_DETAIL
    SWARM_LOG_INFO( TRANSLATE( "_OnDataRecv buffer me: %d:", "_OnDataRecv Buffer SN: %d:" ), nvs.serialNumber );
    payload->print();
  #endif

  if  ( payload->data.sourceSN == nvs.serialNumber ) {
    // multicast from myself
    #ifdef DEBUG_COMMUNICATION
      SWARM_LOG_INFO( TRANSLATE( "_onDataRecv: my own multicast.", "_onDataRecv: meine eigene Multicast-Nachricht." ) );
    #endif

  } else if ( ( payload->data.affectedSN != nvs.serialNumber ) && ( payload->data.affectedSN != broadcastSN )  && ( !nvs.swarm.IAmKelda ) ) {
    // direct communication to somebody else
    #ifdef DEBUG_COMMUNICATION_DETAIL
      SWARM_LOG_INFO( TRANSLATE( "_onDataRecv: to someone else.", "_onDataRecv: an jemand anderen." ) );
    #endif

  } else {
    // relevant data
    if ( xQueueSend( myOSNetwork.recvNotification, payload, ESPNOW_MAXDELAY ) != pdTRUE ) { SWARM_LOG_ERROR( TRANSLATE( "RecvNotification queue fail", "RecvNotification queue ist fehlgeschlagen." ) ); } 
  }

  return true;

}

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 1, 0)

void _OnDataRecvWifi(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {

  SwOSCom payload( MacAddr( recv_info->src_addr ), incomingData, len );
  _OnDataRecv( &payload );

} 

#else

void _OnDataRecvWifi(const uint8_t *macAddr, const uint8_t *incomingData, int len ) {

  SwOSCom payload( MacAddr( macAddr ), incomingData, len );
  _OnDataRecv( &payload );

} 

#endif


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

  #if FTSWARM_HAL_RS458 > 0

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
  
  bool datasent = false;

  // try 3 times to send data
  for ( uint8_t retry=0; retry<5; retry++ ) {

    // send data
    digitalWrite( RS485_DE, 1 );
    uart_write_bytes(RS485_UART, (void *)&frame, frame.size );
    uart_wait_tx_done(RS485_UART, RS485_MAXDELAY);
    digitalWrite( RS485_DE, 0 );

    uart_get_collision_flag(RS485_UART, &collision );

    // stop on correct transmission
    if ( !collision ) { datasent = true; break; }

    // wait a random time to start retransmission
    ets_delay_us( (retry+1)*myOSNetwork.delayTime ); 

  }

  #ifdef DEBUG_COMMUNICATION
  if (!datasent) {
    SWARM_LOG_INFO( TRANSLATE"[not sent]\n"); com->print();
  }
  #endif

  #endif
    
}

void SwOSNetwork::AddPeer( MacAddr macAddr ) {

  // need to register a new peer?  
  if ( esp_now_is_peer_exist( macAddr.addr ) ) return;

  // allocate some memory
  esp_now_peer_info_t *peerInfo = (esp_now_peer_info_t *) malloc( sizeof( esp_now_peer_info_t ) );

  // initialize
  memset( peerInfo, 0, sizeof(esp_now_peer_info_t) );
  memcpy( peerInfo->peer_addr, macAddr.addr, ESP_NOW_ETH_ALEN );

  if ( nvs.wifi.mode == wifiAP ) {
    peerInfo->ifidx   = WIFI_IF_AP;
    peerInfo->channel = nvs.wifi.channel;

  } else {
    peerInfo->ifidx   = WIFI_IF_STA;
    peerInfo->channel = 0;             // use router setting
  }

  // and add it to the internal peer list
  esp_err_t err = esp_now_add_peer( peerInfo );
  if ( err != ESP_OK ){
    SWARM_LOG_ERROR( TRANSLATE( "Failed to add peer\n", "Fehler beim Hinzufügen des Peers\n" ) );
    return;
  }

}

static void tx_Wifi( SwOSCom *com ) {

  int waitAck = 0;
  sendNotificationEvent_t event;

  if (!myOSNetwork.active ) return;

  if ( com->macAddr.isNull() ) {
    SWARM_LOG_ERROR( TRANSLATE( "Try to send data via wifi to MAC 00:00:00:00\n", "Versuch, Daten über WiFi an MAC 00:00:00:00 zu senden\n" ) );
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

}


static void tx_task( void *pvParameters) {

  SwOSCom com;

  while(1) {

    // wait to send data
    if ( ( myOSNetwork.active ) && (pdTRUE == xQueueReceive( myOSNetwork.tx_queue, &com, portMAX_DELAY ) ) ) {

      if (myOSNetwork.communication.rs485) tx_RS485( &com );
      if (myOSNetwork.communication.wifi)  tx_Wifi( &com );      

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

  #ifdef DEBUG_COMMUNICATION
    SWARM_LOG_INFO(( TRANSLATE( "RS485_rx_task started", "RS485_rx_task gestartet" ) ));
  #endif
  
  uart_event_t   event;
  uint8_t        *buffer = (uint8_t *) calloc(1, RS485_BUF_SIZE);
  int            buflen  = 0;
  uint8_t *payload;
  int     sizePayload;
  int     cleanup;

  while(1) {

    // wait for data
    if ( ( myOSNetwork.active ) && ( pdTRUE == xQueueReceive(myOSNetwork.RS485_rx_queue, (void * )&event, portMAX_DELAY ) ) ) { 

      #ifdef DEBUG_COMMUNICATION
        SWARM_LOG_INFO( TRANSLATE( "RS485_rx_task event %d size %d", "RS485_rx_task Ereignis %d Größe %d" ), event.type, event.size);
      #endif

      switch (event.type) {
        
        case UART_FIFO_OVF:
        case UART_BUFFER_FULL:  SWARM_LOG_ERROR( TRANSLATE( "RS485 Buffer Overflow", "RS485 Bufferüberlauf" ) );
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
                                    SWARM_LOG_INFO( TRANSLATE( "DEBUG MONITOR: incoming data:", "DEBUG MONITOR: eingehende Daten:" ) );
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

                                      // if needed show the packet
                                      #ifdef DEBUG_MONITOR
                                        SWARM_LOG_INFO( TRANSLATE( "DEBUG MONITOR: Packet identified:", "DEBUG MONITOR: Paket identifiziert:" ) );
                                        packet.print();
                                      #endif

                                      // process packet
                                      _OnDataRecv( &packet );
                                    } 

                                    // cleanup
                                    if ( ( cleanup > RS485_BUF_SIZE) || ( cleanup < 0 ) || ( cleanup > buflen ) ) {
                                      SWARM_LOG_ERROR( "cleanup %d\n", cleanup );
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
                                    SWARM_LOG_INFO( TRANSLATE( "DEBUG MONITOR: done", "DEBUG MONITOR: fertig" ) );
                                  #endif

                                }
                                break;
                               
        default:               SWARM_LOG_ERROR(TRANSLATE( "RS485 unhandled event %d", "RS485 unbehandeltes Ereignis %d" ), event.type);
                               break;
      }

    }

  }

}

bool SwOSNetwork::_StartRS485( void ) {
  
  // Initialize RS485 communication stack

  # if FTSWARM_HAL_RS485 > 0
    // ftSwarmRS, XL, PwrDrive, Duino
    RS485_rx_queue = xQueueCreate(10, sizeof( SwOSDatagram_t ) );

    // REB & DE
    pinMode( RS485_REB, OUTPUT );
    digitalWrite( RS485_REB, 0 ); // enable
    pinMode( RS485_DE, OUTPUT );
    digitalWrite( RS485_DE, 0 );  // set to receiver

    // UART configuration
    uart_config_t uart_config = {
        .baud_rate = BAUDRATE[nvs.swarm.speed],
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };
    ESP_ERROR_CHECK(uart_param_config(RS485_UART, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(RS485_UART, RS485_D, RS485_R, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(RS485_UART, RS485_BUF_SIZE,  0, 20, &RS485_rx_queue, 0));
    ESP_ERROR_CHECK(uart_set_mode(RS485_UART, UART_MODE_RS485_COLLISION_DETECT));
    UART1.rs485_conf.rs485tx_rx_en = 1;   // loopback
    UART1.rs485_conf.rs485rxby_tx_en = 0; // don't send data if receiver is busy - reduce collitions

    if ( myOSNetwork.communication.rs485 ) {
      xTaskCreatePinnedToCore( RS485_rx_task, "RS485_rx_task", 10240, NULL, 12, NULL, ARDUINO_EVENT_RUNNING_CORE );
    }

  #endif

  return true;
  
}

bool SwOSNetwork::_StartWifi( void ) {

  myOSNetwork.sendNotificationWifi = xQueueCreate(MAXCTRL+10, sizeof( sendNotificationEvent_t ) );

  // create dummy event
  sendNotificationEvent_t event;
  event.status = ESP_NOW_SEND_SUCCESS ;
  memcpy( &event.macAddr, broadcast, ESP_NOW_ETH_ALEN );

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    SWARM_LOG_ERROR( TRANSLATE( "ERROR: couldn't initialize esp_now.", "Fehler: esp_now konnte nicht initialisiert werden." ) );
    return false;
  }

  // register callback funtions
  esp_now_register_send_cb( _OnDataSent );
  esp_now_register_recv_cb( _OnDataRecvWifi );

  return true;

}

bool SwOSNetwork::begin( uint16_t swarmSecret, uint16_t swarmPIN, FtSwarmCommunication_t swarmCommunication ) {

  // initialize all stuff
  active = true;
  communication = swarmCommunication;
  myOSNetwork.setSecret( swarmSecret, swarmPIN );
  delayTime = 5 + nvs.serialNumber & 0x1F; // between 5 and 36 us

  // create queues - recv & tx needs the size of max. controllers  in the swarm
  myOSNetwork.recvNotification = xQueueCreate( MAXCTRL, sizeof( SwOSCom ) ); 
  myOSNetwork.tx_queue         = xQueueCreate( MAXCTRL, sizeof( SwOSCom ) );
  myOSNetwork.userEvent        = xQueueCreate(       5, sizeof( SwOSCom ) );

  bool ok = true;

  if ( swarmCommunication.wifi ) { 
    // if wifi is on, initialize wifi
    ok = ok && _StartWifi( );
  } 

  if ( swarmCommunication.rs485 ) {
    // initialize RS485
    ok = ok && _StartRS485( );
  }

  xTaskCreatePinnedToCore( tx_task, "tx_task", 10240, NULL, 12, NULL, ARDUINO_EVENT_RUNNING_CORE );
  
  return ok;
}

void SwOSNetwork::stop( void ) {
  active = false;
}

void SwOSNetwork::setSecret( uint16_t swarmSecret, uint16_t swarmPIN ) {

  // set swarmSecret
  secret = swarmSecret;
  pin    = swarmPIN;

}

bool SwOSNetwork::hasJoinedASwarm( void ) {
  return secret != DEFAULTSECRET;
}

SwOSNetwork myOSNetwork;