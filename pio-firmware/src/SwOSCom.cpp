/*
 * SwOSCom.cpp
 *
 * Communication between your controlers
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */

#include <arduino.h>
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

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define RS485_TXD       GPIO_NUM_18
#define RS485_RXD       GPIO_NUM_6
#define RS485_REB       GPIO_NUM_7
#define RS485_DE        GPIO_NUM_17
#define RS485_UART      UART_NUM_1
#define RS485_BAUD_RATE 1843200 
#define RS485_BUF_SIZE  (1024)
#define PATTERN_CHR_NUM (3) 

#define RS485_MAXDELAY  100

#define RS485_GOON            1
#define RS485_HEADERFOUND     0
#define RS485_WRONGEVENT     -1
#define RS485_BUFFEROVERFLOW -2
#define RS485_NODATA         -3

QueueHandle_t sendNotificationWifi = NULL;
QueueHandle_t sendNotificationRS485 = NULL;
QueueHandle_t recvNotification = NULL;

typedef struct {
    uint8_t               mac_addr[ESP_NOW_ETH_ALEN];
    esp_now_send_status_t status;
} sendNotificationEvent_t;

void dumpBuffer( uint8_t *buf, uint16_t len ) {
  
  uint8_t *ptr = buf;

  for (uint8_t i=0; i<len; i++) {
    switch ( i % 16 ) {
      case 0:  printf("%02X - %02X ", i, *ptr++); break;
      case 15: printf("%02X\n", *ptr++ );break;
      default: printf("%02X ", *ptr++ ); break;
    }
  }

  printf("\n");

}

SwOSCom::SwOSCom( const uint8_t *buffer, int length) {
  SwOSCom( NULL, buffer, length );
}

SwOSCom::SwOSCom( const uint8_t *mac_addr, const uint8_t *buffer, int length) {

  if (length>sizeof(data)) {
    ESP_LOGE( LOGFTSWARM, "SwOSCOM: max. packet size exceeded.");
    dumpBuffer( (uint8_t *) buffer, length );
  }

  // clear data
  memset(&data, 0, sizeof(data));

  // copy data
  setMAC( mac_addr, broadcastSN ); // don't mind about source & destination, they are set by the next line
  memcpy( &data, buffer, min( length, sizeof(data) ) );

  bool isJoin    = ( ( data.secret == DEFAULTSECRET ) && ( data.cmd == CMD_SWARMJOIN )    && ( data.joinCmd.pin == myOSNetwork.pin ) && ( myOSNetwork.pin != 0 ) ) ;
  bool isJoinAck = ( ( data.secret == DEFAULTSECRET ) && ( data.cmd == CMD_SWARMJOINACK ) && ( data.joinCmd.pin == myOSNetwork.pin ) && ( myOSNetwork.pin != 0 ) ) ;

  // check crc
  uint32_t crcReceived = data.crc;
  data.crc = 0;
  uint32_t crcData = (~crc32_le((uint32_t)~(0xffffffff), (const uint8_t*)&data, data.size))^0xffffffff;
  data.crc = crcReceived;
  bool crcOK = ( crcReceived == crcData );

  // sendBuffered
  bufferIndex = 0;

  #ifdef DEBUG_COMMUNICATION
    printf("isvalid: isJoin = %d, isJoinAck = %d, length = %d, size = %d, cmd = %d, crcOK = %d, version = %d\n",
            isJoin, isJoinAck, length, _size(), data.cmd, crcOK, data.version);
  #endif

  _isValid = ( ( ( data.secret == myOSNetwork.secret ) || isJoin || isJoinAck ) &&
               ( length == _size( ) ) &&
               ( data.cmd < CMD_MAX ) &&
               ( crcOK ) &&
               ( data.version == VERSIONDATA ) );

}

SwOSCom::SwOSCom( const uint8_t *mac_addr, FtSwarmSerialNumber_t affectedSN, SwOSCommand_t cmd, boolean broadcast  ) {

  // clear data
  memset(&data, 0, sizeof(data));

  // set MAC, source & affected SNs
  setMAC( mac_addr, affectedSN );
  
  // set header
  data.secret    = myOSNetwork.secret;
  data.version   = VERSIONDATA;
  data.cmd       = cmd;
  data.broadcast = broadcast;

  // sendBuffered
  bufferIndex = 0;
  
  // valid data
  _isValid = true;
}

size_t SwOSCom::_size( void ) {

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


void SwOSCom::setMAC( const uint8_t *mac_addr, FtSwarmSerialNumber_t affectedSN ) {

  if (mac_addr) {
    // valid mac
    memcpy( mac, mac_addr, ESP_NOW_ETH_ALEN );
  } else {
    // clear mac
    memset( mac, 0, ESP_NOW_ETH_ALEN );
  }

  data.sourceSN = nvs.serialNumber; // allways myself
  data.affectedSN = affectedSN;

}

void SwOSCom::print() {

  printf("mac: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], mac[6] );
  printf("size: %d\n", _size() );
  printf("secret: %04X\n", data.secret);
  printf("source: %d\n", data.sourceSN);
  printf("affected: %d\n", data.affectedSN);
  printf("broadcast: %d\n", data.broadcast );
  printf("command: %d\n", data.cmd);
  printf("isValid: %d _isvalid:%d\n", isValid(), _isValid);

  uint8_t *ptr = (uint8_t *) &data;
  size_t len = _size();
  for (uint8_t i=0; i<len; i++) {
      printf("%02X ", *ptr++ );
    if ( ( i % 16 ) == 15) printf("\n");
  }

  printf("\n");

}

esp_err_t SwOSCom::_sendWiFi( void ) {

  // need to register a new peer?  
  if ( ( mac ) && ( !esp_now_is_peer_exist( mac ) ) ) {

    // allocate some memory
    esp_now_peer_info_t *peerInfo = (esp_now_peer_info_t *) malloc( sizeof( esp_now_peer_info_t ) );

    // initialize
    memset( peerInfo, 0, sizeof(esp_now_peer_info_t) );
    memcpy( peerInfo->peer_addr, mac, ESP_NOW_ETH_ALEN );
    peerInfo->channel = 0;  
    peerInfo->encrypt = false;

    // and add it to the internal peer list
    esp_err_t err = esp_now_add_peer( peerInfo );
    if ( err != ESP_OK ){
      ESP_LOGE(LOGFTSWARM, "Failed to add broadcast peer\n");

      while(1) delay(100);
      return err;
    }

  }

  // wait until last transfer is done
  sendNotificationEvent_t event;
  xQueueReceive( sendNotificationWifi, &event, ESPNOW_MAXDELAY );

  // now we could send the data
  return esp_now_send( mac, (uint8_t *) &data, _size() );
  
}

esp_err_t SwOSCom::_sendRS485( void ) {

  bool           transmitOk = false;
  uint8_t        retry = 0;
  SwOSDatagram_t recv;

  while ( true ) {

    // send data
    digitalWrite( RS485_DE, 1 );  // sender
    uart_write_bytes(RS485_UART, (void *)&data, _size() );
    transmitOk = ( ESP_OK == uart_wait_tx_done(RS485_UART, RS485_MAXDELAY) );
    digitalWrite( RS485_DE, 0 );  // receiver

    // wait for own data from _onDataRecv
    // transmitOk = transmitOk && (pdTRUE == xQueueReceive( sendNotificationRS485, &recv, RS485_MAXDELAY ) );
    transmitOk = transmitOk && (pdTRUE == xQueueReceive( sendNotificationRS485, &recv, RS485_MAXDELAY ) );

    // identical to origin? data!
    transmitOk = transmitOk && ( 0 == memcmp( &data, &recv, _size() ) );

    // everything ok?
    if (transmitOk) return ESP_OK;

    // in case of any error, wait between 0 and 63 us and retry
    // max packet takes 52.5us at 4MBaud
    ets_delay_us( esp_random() & 63 );   
    retry++;

    if (retry>3) return ESP_FAIL;

  }

  // dead code, but avoids compiler warning
  return ESP_OK;

}

esp_err_t SwOSCom::send( void ) {

  esp_err_t errWifi  = ESP_OK;
  esp_err_t errRS485 = ESP_OK;

  // header
  data.header  = 0xFEFEFEFE;
  data.size    = _size();
  
  // broadcast?
  if (data.broadcast) memcpy( mac, broadcast, ESP_NOW_ETH_ALEN );

  // crc
  data.crc     = 0;
  uint32_t crc = (~crc32_le((uint32_t)~(0xffffffff), (const uint8_t*)&data, data.size))^0xffffffff;
  data.crc     = crc;

  #ifdef DEBUG_COMMUNICATION
    printf("SwOSCom.send\n");
    print();
  #endif

  if ( nvs.swarmCommunication & swarmComWifi ) { // use wifi
    errWifi = _sendWiFi();
  }
  
  if (nvs.swarmCommunication & swarmComRS485 ) { // RS485
    errRS485 = _sendRS485();
  }

  // return error code
  if (errWifi != ESP_OK) return errWifi;
  return errRS485;

}

/******************************************************************************
 * 
 * Communication stack
 * 
 *****************************************************************************/

void _OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) { 
  // classic callback funtion to hand over data to the swarm
  // This function is called in an ISR Routine, so it's needed to send the result back via sendNotification

  // stop on uninitialized
  if (!sendNotificationWifi) return;

  // store event
  sendNotificationEvent_t event;
  event.status = status;
  memcpy( &event.mac_addr, mac_addr, ESP_NOW_ETH_ALEN );

  // and send it back
  if ( xQueueSend( sendNotificationWifi, &event, ESPNOW_MAXDELAY ) != pdTRUE ) {
    ESP_LOGW( LOGFTSWARM, "SendNotification queue fail" );
  }
};

bool _OnDataRecv( const uint8_t *mac, const uint8_t *incomingData, int len ) {

  SwOSCom buffer( mac, incomingData, len );

  #ifdef DEBUG_COMMUNICATION
    printf("\n_OnDataRecv buffer me:%d:\n", nvs.serialNumber);
    buffer.print();
  #endif

  // Did a ftSwarm sent this data?
  if ( ( !buffer.isValid() ) || ( !recvNotification ) ) {
    #ifdef DEBUG_COMMUNICATION
      printf( "_OnDataRecv: invalid buffer\n");
      buffer.print();
    #endif
    return false;
  }

  // Did I send this packet?
  if ( buffer.data.sourceSN == nvs.serialNumber ) {
    // Check on transmittion failures
    if ( xQueueSend( sendNotificationRS485, &buffer.data, ESPNOW_MAXDELAY ) != pdTRUE ) {
      ESP_LOGW( LOGFTSWARM, "sendNotificationRS485 queue fail" );
    }  
    return true;
  } 

  // Is it's not for me?
  if ( ! ( ( buffer.data.affectedSN == nvs.serialNumber ) || ( buffer.data.broadcast ) ) ) {
    #ifdef DEBUG_COMMUNICATION
      printf( "_onDataRecv: packet is for someone else.\n");
      buffer.print();
    #endif
    return true;
  } 

  if ( xQueueSend( recvNotification, &buffer, ESPNOW_MAXDELAY ) != pdTRUE ) {
    ESP_LOGW( LOGFTSWARM, "RecvNotification queue fail" );
  }  

  return true;

}

void _OnDataRecvWifi(const uint8_t *mac, const uint8_t *incomingData, int len ) {

  _OnDataRecv( mac, incomingData, len );

} 

bool _OnDataRecvRS485(const uint8_t *incomingData, int len ) { 
  // classic callback funtion to hand over data to the swarm 

  return _OnDataRecv( NULL, incomingData, len );

}

static QueueHandle_t RS485_queue;

static void logBuffer( uint8_t *buffer, int bufPtr ) {

  #ifdef DEBUG_COMMUNICATION

    for (uint8_t i=0;i<bufPtr;i++) {
      printf("%02X ", buffer[i]);
      if ((i % 16) == 15) printf("\n");
    }
  
    printf("\n");

  #endif

}

static int _RS485_FindHeader( uint8_t *buffer, int startAt, int bufLen ) {
  // scans for a header FEFEFEFE
  // return position if found, -1 if not 

  for (int i=startAt; i<=bufLen-4; i++) {

    if ( ( buffer[i] == 0xFE ) && ( buffer[i+1] == 0xFE ) && ( buffer[i+2] == 0xFE ) && ( buffer[i+3] == 0xFE ) && ( buffer[i+4] != 0xFE ) ) {
      return i;
    }
  
  }

  return -1;

}

static int _RS485_ReadData( uint8_t *buffer, int *used) {

  int bufPtr = *used;
  uart_event_t event;

  // wait for data
  if(xQueueReceive(RS485_queue, (void * )&event, 10000 )) { // (TickType_t)portMAX_DELAY)) {

    switch (event.type) {
      case UART_BUFFER_FULL:
      case UART_FIFO_OVF:   // data loss
                            ESP_LOGW(LOGFTSWARM, "RS485 Overflow");
                            bzero( buffer, RS485_BUF_SIZE);
                            *used=0;
                            uart_flush(RS485_UART);
                            return RS485_GOON;

      case UART_BREAK:      // end of transmission, work with data
      case UART_DATA:       // data, go on
                            break;

      default:              // anything else?
                            ESP_LOGE(LOGFTSWARM, "event.type = %d", event.type );
                            return RS485_WRONGEVENT;
    }

    // to much data?
    if (bufPtr+event.size>RS485_BUF_SIZE) return RS485_BUFFEROVERFLOW;

    // no data in buffer
    if ( event.size == 0 ) return RS485_GOON;

    // read data
    if ( uart_read_bytes(RS485_UART, &buffer[bufPtr], event.size, portMAX_DELAY) <= 0 ) return RS485_NODATA;
    
    // set used bytes in buffer
    bufPtr = bufPtr + event.size;

    // test on header
    int firstHeader  = _RS485_FindHeader( buffer, 0, bufPtr );                // first 0xFEFEFEFE in buffer
    int secondHeader = _RS485_FindHeader( buffer, firstHeader + 4, bufPtr );  // is there a 2nd 0xFEFEFEFE in buffer?
    int validHeader  = firstHeader;                                           // most time there is only one header in my buffer

    /* now check, if the first header is obviously corrupted:
        - a second header is found AND
        - the size of the first packet exceeds the position of the second header AND
        - the second packet is completly in my buffer */
    if ( secondHeader > 0 ) {

      if ( ( buffer[firstHeader+4] > ( secondHeader - firstHeader ) ) &&     
           ( secondHeader + buffer[secondHeader+5] <= bufPtr ) ) {
        validHeader = secondHeader;
      }

    }

    // move validHeader to start of buffer
    if (validHeader>=0) {
      // header found, shift left, all done
      bufPtr = bufPtr - validHeader;
      if (validHeader>0) memcpy( buffer, &buffer[validHeader], bufPtr);
      bzero( &buffer[bufPtr], RS485_BUF_SIZE - bufPtr);
      *used = bufPtr;
      return RS485_HEADERFOUND;
    }

    // no header found, part of a header could be at the end. keep the last 3 bytes
    int start, len;

    if (bufPtr > 3) { 
      start = bufPtr-3; 
      len = 3;
    } else {
      start = 0;
      len = bufPtr;
    }

    memcpy( buffer, &buffer[start], len);

    logBuffer( buffer, len );

    *used = len;
    return RS485_GOON;

  }

  return RS485_GOON;

}

static void _RS485_event_task(void *pvParameters) {
  
  uint8_t* buffer = (uint8_t*) malloc(RS485_BUF_SIZE);
  int bufPtr = 0;
  int status;
  int size;

  bzero(buffer, RS485_BUF_SIZE);
    
  for(;;delay(1)) {

    // get some new data
    status = _RS485_ReadData( buffer, &bufPtr);

    if ( status < 0) {
      // got some invalid data, flush buffer
      bzero( buffer, RS485_BUF_SIZE);
      bufPtr = 0;
      ESP_LOGE(LOGFTSWARM, "error %d receiving RS485 data", status);
    
    } else if ( ( status == RS485_HEADERFOUND ) && ( bufPtr > 5 ) ) {

      // Test if it's a valid packet
      if ( buffer[0] != 0xFE ) {
        ESP_LOGE(LOGFTSWARM, "RS485_event_task: packet recognition error.");
        logBuffer(buffer,bufPtr);
        while (1) vTaskDelay( 2000 );
      }

      // get size 
      size = buffer[4];

      if (size>200) ESP_LOGW(LOGFTSWARM, "size %d", size);

      // all data in?
      if ( size <= bufPtr ) {
        
        #ifdef DEBUG_COMMUNICATION
          printf("packet identified\n");
          logBuffer( buffer, size);
        #endif
        
        // valid packet found
        if ( _OnDataRecvRS485( buffer, size ) ) {
          // valid packet     
          // cleanup: move training data to buffer[0]
          memmove( buffer, &buffer[size], bufPtr-size);
          bzero( &buffer[size], RS485_BUF_SIZE - size );
          bufPtr = bufPtr - size;
        } else {
          // corrupted packet
          // cleanup: all up to next FEFEFE is stuff
          int i = _RS485_FindHeader( buffer, 0, size );
          if ( i < 0 ) {
            // no header, clean all
            bzero( buffer, RS485_BUF_SIZE);
            bufPtr = 0;
          } else {
            memmove( buffer, &buffer[i], bufPtr-i);
            bzero( &buffer[i], RS485_BUF_SIZE - i );
            bufPtr = bufPtr - i;
          }
        }
        
      }

    }
  
  }
  
  free(buffer);
  buffer = NULL;
  vTaskDelete(NULL);

}

bool SwOSNetwork::_StartRS485( void ) {
  
  // Initialize RS485 communication stack

  # if !defined(CONFIG_IDF_TARGET_ESP32S3)
    return false; // ftSwarmRS only

  #else

    sendNotificationRS485 = xQueueCreate( 10, sizeof( SwOSDatagram_t ) );

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
    ESP_ERROR_CHECK(uart_driver_install(RS485_UART, RS485_BUF_SIZE * 2,  0, 20, &RS485_queue, 0));

    uart_set_mode(RS485_UART, UART_MODE_RS485_APP_CTRL);
    // UART1.rs485_conf.rs485rxby_tx_en = 1; // send data even if receiver is busy
    // UART1.rs485_conf.rs485rxby_tx_en = 0; // don't send data if receiver is busy - reduce collitions
    UART1.rs485_conf.rs485tx_rx_en = 1; // to see my own data

    //uart_enable_pattern_det_baud_intr(RS485_UART, '@', 3, 9, 0, 0);
    //uart_pattern_queue_reset(RS485_UART, 20);

    xTaskCreate( _RS485_event_task, "RS485_event_task", 10240, NULL, 12, NULL);

    return true;
  
  #endif

}

bool SwOSNetwork::_StartWifi( void ) {

  sendNotificationWifi = xQueueCreate(100, sizeof( sendNotificationWifi ) );

  // create dummy event
  sendNotificationEvent_t event;
  event.status = ESP_NOW_SEND_SUCCESS ;
  memcpy( &event.mac_addr, broadcast, ESP_NOW_ETH_ALEN );

  // send a dummy event, so the first com->send assumes a perfect communication before
  if ( xQueueSend( sendNotificationWifi, &event, ESPNOW_MAXDELAY ) != pdTRUE ) {
    ESP_LOGW( LOGFTSWARM, "SendNotification queue fail" );
  }  

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

bool SwOSNetwork::begin( uint16_t swarmSecret, uint16_t swarmPIN ) {
  // initialize all stuff

  myOSNetwork.setSecret( swarmSecret, swarmPIN );

  // create Receive queue
  recvNotification = xQueueCreate(100, sizeof( SwOSCom ) );

  bool ok = true;

  if ( nvs.wifiMode != wifiOFF ) { 
    // if wifi is on, initialize wifi
    ok = ok && _StartWifi( );
  } 
  
  // always initialize RS485
  ok = ok && _StartRS485( );
  
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