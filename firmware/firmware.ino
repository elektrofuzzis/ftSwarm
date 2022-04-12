

#include "ftSwarm.h"
#include <FastLED.h>
#include "SwOSSwarm.h"

FtSwarmSerialNumber_t LOCAL;

FtSwarmLED *LED1;
FtSwarmLED *LED2;

void setup() {

  Serial.begin(115200);

  ftSwarm.verbose(true);
  LOCAL = ftSwarm.begin( );
  ftSwarm.setup();

  new FtSwarmVoltmeter( LOCAL, FTSWARM_A1 );
  new FtSwarmOhmmeter( LOCAL, FTSWARM_A2 );
  new FtSwarmThermometer( LOCAL, FTSWARM_A3 );
  new FtSwarmLDR( LOCAL, FTSWARM_A4 );

  new FtSwarmTractorMotor( LOCAL, FTSWARM_M1 );
 
  LED1 = new FtSwarmLED( LOCAL, FTSWARM_LED1 );
  LED2 = new FtSwarmLED( LOCAL, FTSWARM_LED2 );

}

void loop() {

  LED1->setColor( CRGB::Blue );
  LED2->setColor( CRGB::Red );

  delay( 250 );

  LED1->setColor( CRGB::Red);
  LED2->setColor( CRGB::Green );

  delay( 250 );

  LED1->setColor( CRGB::Green);
  LED2->setColor( CRGB::Yellow );

  delay( 250 );

  LED1->setColor( CRGB::Yellow);
  LED2->setColor( CRGB::Blue );

  delay(250);

}
