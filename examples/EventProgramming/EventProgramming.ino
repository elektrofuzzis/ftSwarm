// EventProgramming
//
// Simple ftSwarm starter application using event programming. Details at https://elektrofuzzis.github.io/ftSwarm.
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
  mot = new FtSwarmMiniMotor( "motor" );

  sw.onTrigger( FTSWARM_TRIGGERUP,   FTSWARM_ASSIGN, FTSWARM_CONSTANT, FTSWARM_CONSTANT, mot, 100 );
  sw.onTrigger( FTSWARM_TRIGGERDOWN, FTSWARM_ASSIGN, FTSWARM_CONSTANT, FTSWARM_CONSTANT, mot, 0 );

}

void loop( ) {
	
  // wait some time
  delay(100);

}