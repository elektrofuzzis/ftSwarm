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

QueueHandle_t sendNotification = NULL;
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

  // sendBuffered
  bufferIndex = 0;

  _isValid = ( ( ( data.secret == secret ) || isJoin || isJoinAck ) &&
               ( length == _size( ) ) &&
               ( data.cmd < CMD_MAX ) &&
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
  printf("secret: %04X\n", data.secret);
  printf("source: %d\n", data.sourceSN);
  printf("destination: %d\n", data.destinationSN);
  printf("command: %d\n", data.cmd);

  uint8_t *ptr = (uint8_t *) &data;
  size_t len = _size();
  //for (uint8_t i=0; i<sizeof(data); i++) {
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
  xQueueReceive( sendNotification, &event, ESPNOW_MAXDELAY );

  // now we could send the data
  return esp_now_send( mac, (uint8_t *) &data, _size() );
  
}

esp_err_t SwOSCom::_sendRS485( void ) {
  
  digitalWrite( RS485_DE, 1 );  // sender
  uart_write_bytes(RS485_UART, (void *)&data, _size() );
  esp_err_t err = uart_wait_tx_done(RS485_UART, 100);
  digitalWrite( RS485_DE, 0 );  // receiver

  return err;
  
}

esp_err_t SwOSCom::send( void ) {

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
  if (!sendNotification) return;

  // store event
  sendNotificationEvent_t event;
  event.status = status;
  memcpy( &event.mac_addr, mac_addr, ESP_NOW_ETH_ALEN );

  // and send it back
  if ( xQueueSend( sendNotification, &event, ESPNOW_MAXDELAY ) != pdTRUE ) {
    ESP_LOGW( LOGFTSWARM, "SendNotification queue fail" );
  }
};

void _OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len ) {
  // classic callback funtion to hand over data to the swarm 
  SwOSCom buffer( mac, incomingData, len );  

  // Did a ftSwarm sent this data?
  if ( ( !buffer.isValid() ) || ( !recvNotification ) ) {
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

static void _RS485_event_task(void *pvParameters)
{
    uart_event_t event;
    size_t buffered_size;
    uint8_t* buffer = (uint8_t*) malloc(RS485_BUF_SIZE);
    int pos;
    
    for(;;) {
        //Waiting for UART event.
        if(xQueueReceive(RS485_queue, (void * )&event, (TickType_t)portMAX_DELAY)) {
         
            bzero(buffer, RS485_BUF_SIZE);
            ESP_LOGI(LOGFTSWARM, "uart[%d] event:", RS485_UART);
            switch(event.type) {
                case UART_DATA:
                    uart_read_bytes(RS485_UART, buffer, event.size, portMAX_DELAY);
                    _OnDataRecv(broadcast, buffer, event.size );
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGI(LOGFTSWARM, "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(RS485_UART);
                    xQueueReset(RS485_queue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(LOGFTSWARM, "ring buffer full");
                    // If buffer full happened, you should consider increasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(RS485_UART);
                    xQueueReset(RS485_queue);
                    break;
                //Event of UART RX break detected
                case UART_BREAK:
                    ESP_LOGI(LOGFTSWARM, "uart rx break");
                    break;
                //Event of UART parity check error
                case UART_PARITY_ERR:
                    ESP_LOGI(LOGFTSWARM, "uart parity error");
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                    ESP_LOGI(LOGFTSWARM, "uart frame error");
                    break;
                //UART_PATTERN_DET
                case UART_PATTERN_DET:
                    uart_get_buffered_data_len(RS485_UART, &buffered_size);
                    pos = uart_pattern_pop_pos(RS485_UART);
                    ESP_LOGI(LOGFTSWARM, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                    if (pos == -1) {
                        // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                        // record the position. We should set a larger queue size.
                        // As an example, we directly flush the rx buffer here.
                        uart_flush_input(RS485_UART);
                    } else {
                        uart_read_bytes(RS485_UART, buffer, pos, 100 / portTICK_PERIOD_MS);
                        uint8_t pat[PATTERN_CHR_NUM + 1];
                        memset(pat, 0, sizeof(pat));
                        uart_read_bytes(RS485_UART, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                        ESP_LOGI(LOGFTSWARM, "read data: %s", buffer);
                        ESP_LOGI(LOGFTSWARM, "read pat : %s", pat);
                    }
                    break;
                //Others
                default:
                    ESP_LOGI(LOGFTSWARM, "uart event type: %d", event.type);
                    break;
            }
        } 
    }
  
    free(buffer);
    buffer = NULL;
    vTaskDelete(NULL);
}

bool _StartRS485( void ) {

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

    xTaskCreate( _RS485_event_task, "RS485_event_task", 2048, NULL, 12, NULL);

    return true;
  
}

bool SwOSStartCommunication( uint16_t swarmSecret, uint16_t swarmPIN ) {
  // initialize all stuff

  SwOSSetSecret( swarmSecret, swarmPIN );

  // create queues
  sendNotification = xQueueCreate(100, sizeof( sendNotification ) );
  recvNotification = xQueueCreate(100, sizeof( SwOSCom ) );

  // create dummy event
  sendNotificationEvent_t event;
  event.status = ESP_NOW_SEND_SUCCESS ;
  memcpy( &event.mac_addr, broadcast, ESP_NOW_ETH_ALEN );

  // send a dummy event, so the first com->send assumes a perfect communication before
  if ( xQueueSend( sendNotification, &event, ESPNOW_MAXDELAY ) != pdTRUE ) {
    ESP_LOGW( LOGFTSWARM, "SendNotification queue fail" );
  }  

  if ( nvs.wifiMode & 0x1 ) { // use wifi

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
      printf("ERROR: couldn't initialize esp_now\n");
      return false;
    }

    // register callback funtions
    esp_now_register_send_cb(_OnDataSent);
    esp_now_register_recv_cb(_OnDataRecv);

  } else { // RS485

    return _StartRS485( );
  }

  return true;
}
