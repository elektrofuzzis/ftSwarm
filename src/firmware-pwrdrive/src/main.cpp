#include <Arduino.h>
#include "ftSwarmPwrDrive.h"

void setup() {

    Serial.begin(115200);

    firmware();
    ESP.restart();

}

void loop () {

    delay(250);

}
