// MotorSwitchAlias
//
// Simple ftSwarm starter application using alias names. Details at https://elektrofuzzis.github.io/ftSwarm.
//
// (C) 2022-2026 Christian Bergschneider, Stefan Fuss

#include <ftSwam.h>

FtSwarmSwitch *sw;
FtSwarmMotor  *mot;

void setup( ) {

  // start the swarm
  FtSwarmSerialNumber_t local = ftSwarm.begin( );
	
  // get switch and motor instances
  sw  = new FtSwarmSwitch( "switch" );
  mot = new FtSwarmMotor( "motor" );

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