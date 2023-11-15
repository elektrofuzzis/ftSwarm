#include <Arduino.h>
#include "ftSwarm.h"
#include "SwOSFirmware.h"

void setup() {

  Serial.begin(115200);

  firmware();
  ESP.restart();

}

void loop () {

  delay(250);

}
