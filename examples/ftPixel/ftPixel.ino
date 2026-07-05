/*
 * ftPixel.ino
 *
 * ftSwarm example using the onboard RGB leds
 * 
 * (C) 2021-2026 Christian Bergschneider & Stefan Fuss
 * 
 */

#include <ftSwarm.h>
#include <FastLED.h>

FtSwarmPixel *led1;
FtSwarmPixel *led2;

void setup( ) {
  
  // start the swarm
  FtSwarmSerialNumber_t local = ftSwarm.begin( );
  
  // get led instances
  led1 = new FtSwarmPixel( local, FTSWARM_LED1 );
  led2 = new FtSwarmPixel( local, FTSWARM_LED2 );
  
}

void loop( ) {

  Serial.println("colors...");
  led1->setColor( CRGB::Blue );
  led2->setColor( CRGB::Cyan);
  delay(500);
  led1->setColor( CRGB::Red);
  led2->setColor( CRGB::Orange);
  delay(500);
  led1->setColor( CRGB::Green);
  led2->setColor( CRGB::Yellow);
  delay(500);

  Serial.println("brightness");
  for (uint8_t i=240; i!=0; i-=16) {
    led1->setBrightness(i);
    delay(500);
  }
  
  led1->setBrightness(255);

}
