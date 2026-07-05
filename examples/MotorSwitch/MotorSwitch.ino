// MotorSwitch
//
// Simple ftSwarm starter application. Details at https://elektrofuzzis.github.io/ftSwarm.
//
// (C) 2022-2026 Christian Bergschneider, Stefan Fuss

#include <ftSwarm.h>

FtSwarmSwitch *sw;
FtSwarmMiniMotor *mot;

void setup( ) {

  // start the swarm
  FtSwarmSerialNumber_t local = ftSwarm.begin( );
	
  // get switch and motor instances
  sw  = new FtSwarmSwitch( local, FTSWARM_A1 );
  mot = new FtSwarmMiniMotor( local, FTSWARM_M1 );

}

void loop( ) {

  // check if switch is pressed or released
  if ( sw->isPressed() )
    mot->setSpeed(100);
  else
    mot->setSpeed(0);
	
  // wait some time
  delay(100);

}
