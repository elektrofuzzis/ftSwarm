#include <Arduino.h>
#include "SwOS.h"

void setup() {

    Serial.begin(115200);

    FtSwarmSerialNumber_t local = ftSwarm.begin( );

    FtSwarmMotor *mini = new FtSwarmMotor( "MiniMotor" );
    mini->setSpeed( 200 );

    FtSwarmLamp *led = new FtSwarmLamp("LED");
    led->on();

    delay(2000);
    mini->setSpeed(0);
    led->off();
/*
    firmware();
    ESP.restart();
*/

}

void loop () {

    delay(250);

}
