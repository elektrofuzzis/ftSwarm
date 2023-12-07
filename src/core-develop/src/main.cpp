#include <Arduino.h>
#include "SwOS.h"

FtSwarmCounter *cnt;
FtSwarmFrequencymeter *frq;
FtSwarmLDR *notaus;

void setup() {

    Serial.begin(115200);
    firmware();
    ESP.restart();

}

void loop () {

    delay(250);

}
