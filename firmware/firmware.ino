/*
 * firmware.ino
 *
 * ftSwarm Firmware
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#include "ftSwarm.h"
#include <driver/uart.h>

void setup() {

   Serial.begin(115200);

   // start the swarm
   ftSwarm.verbose(true);
   FtSwarmSerialNumber_t local = ftSwarm.begin( false );
   printf("Press any key to start setup...\n");
   

}

void loop() {
  
  int keys = 0;
  uart_get_buffered_data_len( UART_NUM_0, (size_t*)&keys);
  if ( keys > 0 ) {
   ftSwarm.setup();
   ESP.restart();    
  }

  delay(250);
}
