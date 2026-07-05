// MotorSwitchSwarm
//
// Simple ftSwarm starter application using multiple controllers. Details at https://elektrofuzzis.github.io/ftSwarm.
//
// (C) 2022-2026 Christian Bergschneider, Stefan Fuss

#include <ftSwarm.h>

// serial number of the second controller - change it to your 2nd device serial number
#define REMOTE 2

FtSwarmSwitch *sw;
FtSwarmMotor  *mot;

void setup( ) {

  // start the swarm
  FtSwarmSerialNumber_t local = ftSwarm.begin( );
	
  // get switch and motor instances
  sw  = new FtSwarmSwitch( REMOTE, FTSWARM_A1 );
  mot = new FtSwarmMotor( local, FTSWARM_M1 );

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