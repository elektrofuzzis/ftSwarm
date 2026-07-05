/*
 * Firmware.ino
 *
 * ftSwarm Firmware
 * 
 * (C) 2021-26 Christian Bergschneider & Stefan Fuss
 * 
 */

#include <ftSwarm.h>

void setup( ) {

  firmware();
  ESP.restart();

}

void loop() {


  delay(250);

}