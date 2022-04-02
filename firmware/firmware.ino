
#include "ftSwarm.h"
#include <FastLED.h>
#include "SwOSSwarm.h"

#define LOCAL 100

FtSwarmButton   *button;
FtSwarmJoystick *joystick;

void setup() {

  Serial.begin(115200);

  ftSwarm.begin( true, true );

/*
  new FtSwarmVoltmeter( LOCAL, FTSWARM_A1 );
  new FtSwarmOhmmeter( LOCAL, FTSWARM_A2 );
  new FtSwarmThermometer( LOCAL, FTSWARM_A3 );
  new FtSwarmLDR( LOCAL, FTSWARM_A4 );

  new FtSwarmTractorMotor( LOCAL, FTSWARM_M1 );
*/

 // ftSwarm.setup();

 button = new FtSwarmButton( LOCAL,FTSWARM_S1);
 joystick = new FtSwarmJoystick( LOCAL, FTSWARM_JOY2 );

}

void loop() {

  printf("S1: %d\n", button->getState() );
  printf("FB %d, LR %d\n", joystick->getFB(), joystick->getLR() );

  delay( 250 );

}
