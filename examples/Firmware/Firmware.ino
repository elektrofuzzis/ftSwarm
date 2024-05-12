/*
 * Firmware.ino
 *
 * ftSwarm Firmware
 * 
 * (C) 2021-24 Christian Bergschneider & Stefan Fuss
 * 
 */

#include <Arduino.h>
#include <ftSwarmXL.h>

void setup( ) {

  Serial.begin(115200);

  firmware();
  ESP.restart();

}

void loop() {


  delay(250);

}