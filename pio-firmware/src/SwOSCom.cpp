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

uint16_t secret = DEFAULTSECRET;
uint16_t pin    = 0;

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

QueueHandle_t sendNotificationWifi = NULL;
QueueHandle_t sendNotificationRS485 = NULL;
QueueHandle_t recvNotification = NULL;

typedef struct {
    uint8_t               mac_addr[ESP_NOW_ETH_ALEN];
    esp_now_send_status_t status;
} sendNotificationEvent_t;

SwOSCom::SwOSCom( const uint8_t *mac_addr, const uint8_t *buffer, int length) {

  // clear data
  memset(&data, 0, sizeof(data));

  // copy data
  setMAC( mac_addr, broadcastSN ); // don't mind about source & destination, they are set by the next line
  memcpy( &data, buffer, min( length, sizeof(data) ) );

  bool isJoin    = ( ( data.secret == DEFAULTSECRET ) && ( data.cmd == CMD_SWARMJOIN )    && ( data.joinCmd.pin == pin ) && ( pin != 0 ) ) ;
  bool isJoinAck = ( ( data.secret == DEFAULTSECRET ) && ( data.cmd == CMD_SWARMJOINACK ) && ( data.joinCmd.pin == pin ) && ( pin != 0 ) ) ;

  // check crc
  uint32_t crcReceived = data.crc;
  data.crc = 0;
  uint32_t crcData = (~crc32_le((uint32_t)~(0xffffffff), (const uint8_t*)&data, data.size))^0xffffffff;
  data.crc = crcReceived;
  bool crcOK = ( crcReceived == crcData );

  // sendBuffered
  bufferIndex = 0;

  _isValid = ( ( ( data.secret == secret ) || isJoin || isJoinAck ) &&
               ( length == _size( ) ) &&
               ( data.cmd < CMD_MAX ) &&
               ( crcOK ) &&
               ( data.version == VERSIONDATA ) );

}

SwOSCom::SwOSCom( const uint8_t *mac_addr, FtSwarmSerialNumber_t destinationSN, SwOSCommand_t cmd ) {

  // clear data
  memset(&data, 0, sizeof(data));

  // store mac
  setMAC( mac_addr, destinationSN );
  data.sourceSN      = nvs.serialNumber;
  
  // set header
  data.secret        = secret;
  data.version       = VERSIONDATA;
  data.cmd           = cmd;

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


void SwOSCom::setMAC( const uint8_t *mac_addr, FtSwarmSerialNumber_t destinationSN ) {

  if (mac_addr) {
    // valid mac
    memcpy( mac, mac_addr, ESP_NOW_ETH_ALEN );
  } else {
    // clear mac
    memset( mac, 0, ESP_NOW_ETH_ALEN );
  }

  data.sourceSN = nvs.serialNumber; // allways myself
  data.destinationSN = destinationSN;

}

void SwOSCom::print() {

  printf("mac: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], mac[6] );
  printf("size: %d\n", _size() );
  printf("secret: %04X\n", data.secret);
  printf("source: %d\n", data.sourceSN);
  printf("destination: %d\n", data.destinationSN);
  printf("command: %d\n", data.cmd);

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
      printf("Failed to add broadcast peer\n");
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

  while ( (retry < 3) && (!transmitOk) ) {

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

    // in case of any error, wait between 0 and 63 us and retry
    // max packet takes 52.5us at 4MBaud
    if (!transmitOk) {
      ets_delay_us( esp_random() & 63 );   
      retry++;
    }

  }

  if (!transmitOk) return ESP_FAIL;

  return ESP_OK;

}

esp_err_t SwOSCom::send( void ) {

  data.header  = 0xFEFEFEFE;
  data.size    = _size();
  data.crc     = 0;
  uint32_t crc = (~crc32_le((uint32_t)~(0xffffffff), (const uint8_t*)&data, data.size))^0xffffffff;
  data.crc     = crc;

  if ( nvs.wifiMode & 0x1 ) { // use wifi
    return _sendWiFi();
  } else { // RS485
    return _sendRS485();
  }
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

void _OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len ) {

  // classic callback funtion to hand over data to the swarm 
  SwOSCom buffer( mac, incomingData, len );  

  // Did a ftSwarm sent this data?
  if ( ( !buffer.isValid() ) || ( !recvNotification ) ) {
    ESP_LOGI( LOGFTSWARM, "_OnDataRecv: invalid buffer");
    buffer.print();
    return;
  }

  // Did I send this packet?
  if ( buffer.data.sourceSN == nvs.serialNumber ) {
    // Check on transmittion failures
    if ( xQueueSend( sendNotificationRS485, &buffer.data, ESPNOW_MAXDELAY ) != pdTRUE ) {
      ESP_LOGW( LOGFTSWARM, "sendNotification queue fail" );
    }  
    return;
  } 

  // Is it's not for me?
  if ( ( buffer.data.destinationSN != nvs.serialNumber ) && ( buffer.data.destinationSN != 0xFFFF ) ) {
    ESP_LOGI( LOGFTSWARM, "_onDataRecv: packet is for someone else.");
    return;
  } 

  if ( xQueueSend( recvNotification, &buffer, ESPNOW_MAXDELAY ) != pdTRUE ) {
    ESP_LOGW( LOGFTSWARM, "RecvNotification queue fail" );
  }  

}

void SwOSSetSecret( uint16_t swarmSecret, uint16_t swarmPIN ) {

  // set swarmSecret
  secret = swarmSecret;
  pin    = swarmPIN;

}

static QueueHandle_t RS485_queue;

static void _RS485_event_task(void *pvParameters) {
  uart_event_t event;
  size_t buffered_size;
  uint8_t* buffer = (uint8_t*) malloc(RS485_BUF_SIZE);
  int bufPtr = 0;
  int header = false; // flag, if there is a header starting at buffer[0]
  uint16_t size = 0;

  bzero(buffer, RS485_BUF_SIZE);
    
  for(;;delay(1)) {

    //Waiting for UART event.
    if(xQueueReceive(RS485_queue, (void * )&event, (TickType_t)portMAX_DELAY)) {

      if (event.type == UART_DATA) {

        // read data
        uart_read_bytes(RS485_UART, &buffer[bufPtr], event.size, portMAX_DELAY);
        bufPtr = bufPtr + event.size;

        // search for an valid header
        if ( !header ) {
          // no valid data in buffer
          for (uint8_t i=0; i<bufPtr; i++) {
              
            if ( ( buffer[i] == 0xFE ) && ( buffer[i+1] == 0xFE ) && ( buffer[i+2] == 0xFE ) && ( buffer[i+3] == 0xFE ) ) {
              // Header found, move data to start of buffer
              memmove( buffer, &buffer[i], bufPtr-i);
              // clear trailing data
              bzero( &buffer[bufPtr], RS485_BUF_SIZE - bufPtr );
              // relocate Ptr
              bufPtr = bufPtr-i;
              // there is a at buffer [0]
              header = true;
            }
          }
        }
          
        // test on data complete
        if ((header) && ( bufPtr >= 6 ) )   {
            
          memcpy( &size, &buffer[4], 2);
            
          if ( size <= bufPtr ) {

            // data packet found
            _OnDataRecv(broadcast, buffer, size );
              
            // cleanup: move training data to buffer[0]
            memmove( &buffer, &buffer[size], bufPtr-size);
            bzero( &buffer[size], RS485_BUF_SIZE - size );
            bufPtr = bufPtr - size;
            header = false;

          }

        }        

      } 
    
  	}
  
  }
  
  free(buffer);
  buffer = NULL;
  vTaskDelete(NULL);

}

bool _StartRS485( void ) {
  
    // Initialize RS485 communication stack

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
    UART1.rs485_conf.rs485rxby_tx_en = 1;  
    UART1.rs485_conf.rs485tx_rx_en = 1; // to see my own data

    //uart_enable_pattern_det_baud_intr(RS485_UART, '@', 3, 9, 0, 0);
    //uart_pattern_queue_reset(RS485_UART, 20);

    xTaskCreate( _RS485_event_task, "RS485_event_task", 10240, NULL, 12, NULL);

    return true;
  
}

bool _StartWifi( void ) {

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
    printf("ERROR: couldn't initialize esp_now\n");
    return false;
  }

  // register callback funtions
  esp_now_register_send_cb(_OnDataSent);
  esp_now_register_recv_cb(_OnDataRecv);

  return true;

}

bool SwOSStartCommunication( uint16_t swarmSecret, uint16_t swarmPIN ) {
  // initialize all stuff

  SwOSSetSecret( swarmSecret, swarmPIN );

  // create Receive queue
  recvNotification = xQueueCreate(100, sizeof( SwOSCom ) );

  if ( nvs.wifiMode & 0x1 ) { // use wifi

    return _StartWifi( );

  } else { // RS485

    return _StartRS485( );
  }

  return true;
}
