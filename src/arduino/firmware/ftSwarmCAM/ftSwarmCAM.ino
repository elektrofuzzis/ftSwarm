
/*
 * ftSwarCAM.ino
 *
 * ftSwarm Firmware
 * 
 * (C) 2021-23 Christian Bergschneider & Stefan Fuss
 * 
 */

 /*  needed Arduino IDE settings:

    Board: "ESP32SCAM"
    Core Debug Level: "None"
    Events Runs On: 1
    Arduino Runs On: 1
    PSRAM: "enabled"

*/
 
#include "ftSwarmCAM.h"

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
