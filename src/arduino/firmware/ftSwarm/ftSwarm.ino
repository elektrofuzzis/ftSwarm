
/*
 * ftSwarm.ino
 *
 * ftSwarm Firmware
 * 
 * (C) 2021-23 Christian Bergschneider & Stefan Fuss
 * 
 */

/*  needed Arduino IDE settings:

    Board: "ESP32 Dev Module"
    Core Debug Level: "None"
    Events Runs On: 0
    Arduino Runs On: 1
    PSRAM: "enabled"

*/
 
#include "ftSwarm.h"

void setup() {

  Serial.begin(115200);

  ftSwarm.verbose(true);
  FtSwarmSerialNumber_t local = ftSwarm.begin( false );

  firmware();
  ESP.restart();
   
}

void loop() {
  
  delay(250);

}
