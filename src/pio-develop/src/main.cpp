#include <Arduino.h>
#include <ftSwarmPwrDrive.h>
#include "SwOSFirmware.h"
#include <FastLED.h>
#include "SwOSSwarm.h"

#define FIRMWARE

#ifdef FIRMWARE

void setup() {

  Serial.begin(115200);

  // start the swarm
  // FtSwarmSerialNumber_t local = ftSwarm.begin( true );
  myOSSwarm.begin( true );

  firmware();
  ESP.restart();

}

void loop () {
  
}

#else 

class FtTimer {
  private:
    int _startTime;
    bool _running = true;
  public:
    FtTimer() { _startTime = millis(); };
    int valueM( void ) {
      return ( millis() - _startTime);
    }
    float valueS( void ) {
      return valueM() * 1000;
    }
    bool triggerDone( float seconds ) {
      if (!_running) return false;
      int c = (int) ( seconds * 1000 );
      if ( millis() > ( c - _startTime ) ) {
        _running = false;
        return true;
      } else {
        return false;
      }
    }
    void reset(void) {
      _startTime = millis();
    }
};

FtSwarmPixel *rave[16];
FtSwarmMotor *dragon[2];
FtSwarmMotor *tree;
FtSwarmAnalogInput *poti[2];
FtSwarmSwitch *limitUp;
FtSwarmSwitch *limitDown;
FtSwarmLamp *smoke;
FtSwarmLamp *treeLED;
FtSwarmLamp *dragonLED;
FtSwarmDigitalInput *panel[3];
FtSwarmMotor *carMotor;
FtSwarmAnalogInput *carSensor[2];
FtSwarmServo *jaw[2];

void setup() {

  Serial.begin(115200);

  // start the swarm
  ftSwarm.verbose(true);
  FtSwarmSerialNumber_t local = ftSwarm.begin( false );

  // initialize
  printf("120\n");
  for (uint8_t i=0; i<16; i++) rave[i] = new FtSwarmPixel( 120, i+2 );
  tree      = new FtSwarmMotor( 120, FTSWARM_M1 ); 
  treeLED   = new FtSwarmLamp( 120, FTSWARM_M2 );
  
  printf("121\n");
  dragon[0] = new FtSwarmMotor( 121, FTSWARM_M1 );
  jaw[0]    = new FtSwarmServo( 121, FTSWARM_SERVO1 );
  poti[0]   = new FtSwarmAnalogInput( 121, FTSWARM_A1 );
  limitUp   = new FtSwarmSwitch( 121, FTSWARM_A2 );
  limitDown = new FtSwarmSwitch( 121, FTSWARM_A3 );
  
  printf("122\n");
  dragon[1] = new FtSwarmMotor( 122, FTSWARM_M1 );
  jaw[1]    = new FtSwarmServo( 122, FTSWARM_SERVO1 );
  poti[1]   = new FtSwarmAnalogInput( 122, FTSWARM_A1 );
  smoke     = new FtSwarmLamp( 122, FTSWARM_M4 );

  printf("123\n");
  dragonLED = new FtSwarmLamp( 123, FTSWARM_M1 );
  for ( uint8_t i=0; i<3; i++ ) panel[i] = new FtSwarmDigitalInput( 123, i );

/*
  carMotor  = new FtSwarmMotor( 200, FTSWARM_M1 );
  carSensor[0] = new FtSwarmAnalogInput( 200, FTSWARM_A1 );
  carSensor[1] = new FtSwarmAnalogInput( 200, FTSWARM_A1 );
*/

  // smoke switch/
  panel[0]->onTrigger( FTSWARM_TRIGGERUP,   smoke, 255 );
  panel[0]->onTrigger( FTSWARM_TRIGGERDOWN, smoke, 0 );

    // tree switch
  panel[1]->onTrigger( FTSWARM_TRIGGERUP,   tree, 32 );
  panel[1]->onTrigger( FTSWARM_TRIGGERDOWN, tree, 0 );

}

#define dragonSpeed 32

void closeDragon( void ) {

  jaw[0]->setPosition(0);
  jaw[1]->setPosition(0);
  dragon[0]->setSpeed( -dragonSpeed );
  dragon[1]->setSpeed( -dragonSpeed );

  while (!limitDown->hasToggledUp()) {
    delay(50);
  }

  dragon[0]->setSpeed(0);
  dragon[1]->setSpeed(0);

}

void openDragon( void ) {

  FtTimer smokeTimer;
  FtTimer jewTimer;
  FtTimer openTimer;
  smoke->on();

  dragon[0]->setSpeed( dragonSpeed );
  dragon[1]->setSpeed( dragonSpeed );

  while (1) {

    if ( smokeTimer.triggerDone(4) ) {
      smoke->off();
    }

    if ( openTimer.triggerDone( 0.1 ) ) {
      int pos = openTimer.valueM();
      jaw[0]->setPosition( pos );
      jaw[1]->setPosition( pos );
      openTimer.reset();
    }

    if ( limitUp->hasToggledUp() ) {
      dragon[0]->setSpeed(0);
      dragon[1]->setSpeed(0);
    }

    delay(50);

  }  

  smoke->off();
  dragon[0]->setSpeed(0);
  dragon[1]->setSpeed(0);

}

void oneColor( uint32_t c ) {

  for ( uint8_t i=0; i<16; i++ ) rave[i]->setColor( c );

}

void flash( uint8_t times, int on, int off, uint8_t brightness ) {

  uint8_t oldBrightness = rave[0]->getBrightness( );

  for ( uint8_t t=0; t<times; t++) {
    rave[0]->setBrightness(0);
    delay(off);
    rave[0]->setBrightness(brightness);
    delay(on);
  }

  rave[0]->setBrightness(oldBrightness);

}

void fade( int w, uint8_t start, uint8_t stop ) {

  for (uint8_t i=start; i<stop; i++ ) {
    rave[0]->setBrightness(i);
    delay(w);
  }

}

void rainbow( int w ) {

  oneColor( CRGB::Black);
  delay(w);

  for (uint8_t i=0; i<4; i++) {
    rave[0+i]->setColor(CRGB::Green);
    rave[4+i]->setColor(CRGB::Yellow);
    rave[8+i]->setColor(CRGB::Red);
    rave[12+i]->setColor(CRGB::Blue);
    delay(w);
  }

}

void raveHorn ( void ) {

  rainbow(1000);

  rave[0]->setBrightness(0);
  oneColor( CRGB::Green);
  fade(5, 0, 255);
  oneColor( CRGB::Orange);
  fade(5, 0, 255);
  oneColor( CRGB::Red);
  fade(5, 0, 255);
  oneColor( CRGB::Violet);
  fade(5, 0, 255);
  oneColor( CRGB::Blue);
  fade(5, 0, 255);

}

void loop () {

/*
  if (panel[1]->hasToggledUp() )   openDragon();
  if (panel[1]->hasToggledDown() ) closeDragon();
*/

//  printf("%d %d %d\n", panel[0]->getState(), panel[1]->getState(), panel[2]->getState() );
  
  delay(50);

  raveHorn();
  
}

#include "ftPwrDrive/ftPwrDrive.h"

ftPwrDrive drive(32, 5, 4);

void setup() {

  Serial.begin(115200);
  Serial.printf("Hello world!\n");

  drive.stopMovingAll(0xF);

  printf("---\n");
  drive.setMaxSpeed( M1, 10 );
  printf( "setMaxSpeed: %ld\n", drive.getMaxSpeed( M1 ) );

  printf("---\n");
  drive.setRelDistance( M1, 100000L );
  printf( "setRelDistance: %ld\n", drive.getStepsToGo( M1 ) );

  uint8_t state[4];

  for ( uint8_t i=0; i<10; i++ ) {

    if (i==2) {
      printf("---\n");
      drive.startMoving( M1 );
    }

    printf("---\n");
    drive.getStateAll( &state[0], &state[1], &state[2], &state[3]);
    printf("state: %02X %d %02X %02X\n", state[0], drive.isMoving( M1 ), state[1], ( ( state[2] &0x02 ) > 0 ) );
    delay(500);
  }

}

void loop() {

  delay(50);

}


#endif