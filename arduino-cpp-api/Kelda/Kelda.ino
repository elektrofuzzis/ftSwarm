#include "ftSwarmMotor.h"
#include "ftSwarmSwitch.h"

ftSwarmMotor M1( "123", FTSWARM_M1 );

ftSwarmSwitch J1( "234", FTSWARM_J1 );
ftSwarmSwitch F1( "234", FTSWARM_F1 );

void setup() {
  // put your setup code here, to run once:

  Serial.print( M1.getLastError() );
}

void loop() {
  // put your main code here, to run repeatedly:

  F1.getState();

}
