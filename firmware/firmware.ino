/*
 * firmware.ino
 *
 * ftSwarm Firmware
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */
 
 #include "ftSwarm.h"

void setup() {

  Serial.begin(115200);

  ftSwarm.verbose(true);
  ftSwarm.begin( );
  ftSwarm.setup();

  ESP.restart();

}

void loop() {

}
