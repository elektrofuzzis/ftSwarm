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

#include "SwOSCom.h"

uint16_t secret = DEFAULTSECRET;
uint16_t pin    = 0;

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

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
  setMAC( mac_addr );
  memcpy( &data, buffer, min( length, sizeof(data) ) );

  bool isJoin    = ( ( data.secret == DEFAULTSECRET ) && ( data.cmd == CMD_SWARMJOIN )    && ( data.joinCmd.pin == pin ) && ( pin != 0 ) ) ;
  bool isJoinAck = ( ( data.secret == DEFAULTSECRET ) && ( data.cmd == CMD_SWARMJOINACK ) && ( data.joinCmd.pin == pin ) && ( pin != 0 ) ) ;

  _isValid = ( ( ( data.secret == secret ) || isJoin || isJoinAck ) &&
               ( length >= 5 ) &&
               ( data.cmd < CMD_MAX ) &&
               ( data.version == VERSIONDATA ) );

}

SwOSCom::SwOSCom( const uint8_t *mac_addr, FtSwarmSerialNumber_t serialNumber, FtSwarmControler_t ctrlType, SwOSCommand_t cmd ) {

  // clear data
  memset(&data, 0, sizeof(data));

  // store mac
  setMAC( mac_addr );
  // set header
  data.secret       = secret;
  data.version      = VERSIONDATA;
  data.serialNumber = serialNumber;
  data.ctrlType     = ctrlType;
  data.cmd          = cmd;
  
  // valid data
  _isValid = true;
}

void SwOSCom::setMAC( const uint8_t *mac_addr ) {

  if (mac_addr) {
    // valid mac
    memcpy( mac, mac_addr, ESP_NOW_ETH_ALEN );
  } else {
    // clear mac
    memset( mac, 0, ESP_NOW_ETH_ALEN );
  }

}

void SwOSCom::print() {

  printf("mac: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], mac[6] );
  printf("secret: %04X\n", data.secret);
  printf("serialNumber: %d\n", data.serialNumber);
  printf("ctrlType: %d\n", data.ctrlType);
  printf("command: %d\n", data.cmd);

  uint8_t *ptr = (uint8_t *) &data;

  for (uint8_t i=0; i<sizeof(data); i++) {
    printf("%02X ", *ptr++ );
    if ( ( i % 16 ) == 15) printf("\n");
  }
  printf("\n");

}

esp_err_t SwOSCom::send( void ) {

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
  return esp_now_send( mac, (uint8_t *) &data, sizeof(data) );
  
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

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    return false;
  }

  // register callback funtions
  esp_now_register_send_cb(_OnDataSent);
  esp_now_register_recv_cb(_OnDataRecv);

  return true;
}
