#include "ftSwarm.h"

FtSwarmSwitch *sw1;
FtSwarmSwitch *sw2;
FtSwarmSwitch *sw3;
FtSwarmSwitch *sw4;
FtSwarmMotor  *mot;

void setup( ) {

  Serial.begin(115200);

   // start the swarm
   ftSwarm.verbose(true);
   FtSwarmSerialNumber_t local = ftSwarm.begin( );

   // get switch and motor instances
   sw1  = new FtSwarmSwitch( local, FTSWARM_A1 );
   sw2  = new FtSwarmSwitch( local, FTSWARM_A2 );
   sw3  = new FtSwarmSwitch( local, FTSWARM_A3 );
   sw4  = new FtSwarmSwitch( local, FTSWARM_A4 );
   mot = new FtSwarmMotor( local, FTSWARM_M2 );

}

void loop( ) {

   printf("%d %d %d %d\n", sw1->getState(), sw2->getState(), sw3->getState(), sw4->getState() );

   // wait some time
   delay(300);

}
