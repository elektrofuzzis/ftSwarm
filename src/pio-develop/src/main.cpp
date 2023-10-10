#include <Arduino.h>
#include <ftSwarmPwrDrive.h>

#include "SwOSFirmware.h"

void setup() {

  Serial.begin(115200);

  // start the swarm
  ftSwarm.verbose(true);
  FtSwarmSerialNumber_t local = ftSwarm.begin( false );

  firmware();
  ESP.restart();

}

void loop () {}