#include <Arduino.h>


// #define FIRMWARE

#ifdef FIRMWARE

#include "ftSwarmRS.h"

#include "SwOSFirmware.h"

void setup() {

Serial.begin(115200);

/*
#if CONFIG_IDF_TARGET_ESP32S3
   pinMode(12, INPUT);
   delay(100);
   if (analogRead(12)>2000) {
    Serial.println("stopped");
    while ((analogRead(12)>2000)) delay( 500 );
   }
   Serial.println("continued");
#endif
*/

   // start the swarm
   ftSwarm.verbose(true);
   FtSwarmSerialNumber_t local = ftSwarm.begin( false );
/*
   while (1) {
    if ( Serial.available()>0 ) {
      printf("%d\n", Serial.read() );
    }
    delay(1);
   }
  */

  firmware();
  ESP.restart();

}

void loop() {

  delay(250);

}

#else // no Firmware


#include <soc/uart_struct.h>
#include <driver/uart.h>
#include <queue.h>
#include <SwOSNVS.h>

#define RS485_TXD       GPIO_NUM_18
#define RS485_RXD       GPIO_NUM_6
#define RS485_REB       GPIO_NUM_7
#define RS485_DE        GPIO_NUM_17
#define RS485_UART      UART_NUM_1
#define RS485_BAUD_RATE UART_BITRATE_MAX 
#define RS485_BUF_SIZE  (1024)
#define PATTERN_CHR_NUM (3) 

#define RS485_MAXDELAY  100

uint8_t data[256];
static QueueHandle_t RS485_rx_queue;
static QueueHandle_t RS485_tx_queue;

struct data_t {
  uint32_t header;
  FtSwarmSerialNumber_t source;
  uint32_t packetno;
  unsigned long time;
  uint16_t  retry; 
  uint32_t footer;
} __attribute__((packed));


void dump( uint8_t *buf, uint16_t len ) {
  
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
/*
header
source
destination
retry
size
crc

*/

static void RS485_event_task(void *pvParameters) {
  
  uart_event_t event;
  uint8_t buffer[1024];
  uint16_t bufPtr = 0;

  while(1) {

    // wait for data
    if( pdTRUE == xQueueReceive(RS485_rx_queue, (void * )&event, portMAX_DELAY ) ) { 

      switch (event.type) {
        case UART_BUFFER_FULL: printf( "%lu UART_BUFFER_FULL\n", millis()); 
                               break;

        case UART_FIFO_OVF:    printf( "%lu UART_FIFO_OVF\n", millis()); 
                               break;

        case UART_BREAK:       
        case UART_DATA:        uart_read_bytes(RS485_UART, &buffer[bufPtr], event.size, portMAX_DELAY);
                              
                               

        default:               printf( "%lu event.type = %d", millis(), event.type ); 
                               break;
      }

    } else {

      printf( "IDLE\n");

    }

  }

}

static void RS485_send_task(void *pvParameters) {
  
  printf("%lu starting send task\n", millis() );
  data_t buffer;
  bool   transmitOk;
  bool   collision;

  uint32_t packet = 0;

  while(1) {

    // wait to send data
    if (pdTRUE == xQueueReceive( RS485_tx_queue, &buffer, portMAX_DELAY )) {

      printf("%lu start sending packet\n", millis());

      buffer.packetno = packet++;
      buffer.time = millis();

      // try 3 times to send data
      for ( uint8_t retry=0; retry<3; retry++ ) {

        buffer.retry = retry;

        uart_write_bytes(RS485_UART, (void *)&buffer, sizeof(buffer) );
        transmitOk = ( ESP_OK == uart_wait_tx_done(RS485_UART, RS485_MAXDELAY) );
        transmitOk = transmitOk && ( ESP_OK == uart_get_collision_flag(RS485_UART, &collision) );

        // stop on correct transmission
        if ( transmitOk && !collision ) break;

        // wait a random time to start retransmission
        ets_delay_us( esp_random() & 0x3F ); 

      }
    
    }
  
  }

}

void setup( ) {

  Serial.begin(115200);

  RS485_rx_queue = xQueueCreate(10, sizeof( uart_event_t ) );
  RS485_tx_queue = xQueueCreate(10, sizeof( data_t ) );

  // REB & DE
  pinMode( RS485_REB, OUTPUT );
  digitalWrite( 7, 0 ); // enable
  pinMode( RS485_DE, OUTPUT );
  digitalWrite( RS485_DE, 0 );  // set to receiver

  // UART configuration
  uart_config_t uart_config = {
    .baud_rate = UART_BITRATE_MAX,
    .data_bits = UART_DATA_8_BITS,
    .parity    = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
  //  .rx_flow_ctrl_thresh = 122,
    .source_clk = UART_SCLK_APB,
  };
  
  ESP_ERROR_CHECK(uart_param_config(RS485_UART, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(RS485_UART, RS485_TXD, RS485_RXD, RS485_DE, UART_PIN_NO_CHANGE ));
  ESP_ERROR_CHECK(uart_driver_install(RS485_UART, RS485_BUF_SIZE * 2,  0, 20, &RS485_rx_queue, 0));
  ESP_ERROR_CHECK(uart_set_mode(RS485_UART,  UART_MODE_RS485_HALF_DUPLEX));
  UART1.rs485_conf.rs485tx_rx_en = 1;   // loopback
  UART1.rs485_conf.rs485rxby_tx_en = 0; // don't send data if receiver is busy - reduce collitions

  nvs.begin();

  printf("SN: %d\n", nvs.serialNumber);
  delay(1000);

  xTaskCreate( RS485_send_task, "RS485_send_task", 10240, NULL, 12, NULL);
  xTaskCreate( RS485_event_task, "RS485_event_task", 10240, NULL, 12, NULL);

}

void loop( ) {

  data_t buffer;

  if ( nvs.serialNumber != 1 ) {
    buffer.header = 0xFEFEFEFF;
    buffer.footer = 0xEFEFEFEF;
    buffer.source = nvs.serialNumber;

    if ( xQueueSend( RS485_tx_queue, &buffer, 512 ) != pdTRUE ) {
      printf( "%lu send data to queue failed.\n", millis() );
    }
  }

  delay(esp_random() & 0x4F );

}

#endif
