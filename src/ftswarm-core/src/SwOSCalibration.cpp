/*
 * SwOSCalibration.h
 *
 * Joystick & RCServo calibration used by serial and OLED firmware
 * 
 * (C) 2021-26 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOSCalibration.h"
#include "SwOSFilter.h"
#include "SwOSHW/SwOSHWAnalog.h"
#include "SwOSHW/SwOSHWDigital.h"
#include "SwOSSwarm.h"
#include "SwOSLog.h"

void initCalibration( SwOSJoyCalibration_t *calibration ) {
  calibration->minValue = 1000;
  calibration->maxValue = 3000;
}

bool testCalibration( int32_t value, SwOSJoyCalibration_t *calibration, char visualizer[], uint8_t p1, uint8_t p2 ) {

  if ( value == FILTER_INVALID ) return false;

  bool change = false;

  if (value < calibration->minValue ) { change = true; calibration->minValue = value; visualizer[p1] = '+'; }
  if (value > calibration->maxValue ) { change = true; calibration->maxValue = value; visualizer[p2] = '+'; }

  return change;

}

bool calibrateJoysticks( bool useOLED, SwOSJoyCalibration_t calibration[4] ) {

  SwOSDigitalInput*    s1 = (SwOSDigitalInput*)myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_S1 );
  SwOSDigitalInput*    s4 = (SwOSDigitalInput*)myOSSwarm.Ctrl[0]->getIO( SWOSIO_BUTTON, FTSWARM_S4 );
  SwOSJoystick*        joy[2];
  char                 visualizer[15];
  SwOSJoyCalibration_t newCalibration[4];

  // get direct readings
  joy[0] = (SwOSJoystick *)myOSSwarm.Ctrl[0]->getIO("JOY1");
  joy[1] = (SwOSJoystick *)myOSSwarm.Ctrl[0]->getIO("JOY2");

  // drop joystick filters
  joy[0]->fb->deleteFilter( SWOS_FILTER_JOYSTICK );
  joy[0]->lr->deleteFilter( SWOS_FILTER_JOYSTICK );
  joy[1]->fb->deleteFilter( SWOS_FILTER_JOYSTICK );
  joy[1]->lr->deleteFilter( SWOS_FILTER_JOYSTICK );

  joy[0]->fb->deleteFilter( SWOS_FILTER_MULTIPLY );
  joy[1]->lr->deleteFilter( SWOS_FILTER_MULTIPLY );

  // init calibration
  for ( uint8_t i=0; i<4; i++ ) initCalibration( &newCalibration[i] );

  // since some filters are dropped, we need to wait for new values
  delay(100);

  // 1st step: rotate the stick to get min/max values

  strcpy( visualizer, "---- ----" );
  printf("\nPlease rotate both joysticks.\nClick S1 when all - changed to + or S4 to abort. %s", visualizer ); flushStdIO();

  while ( true ) {

    // abort?
    if ( s4->getToggle() == FTSWARM_TOGGLEUP ) { 
      
      joy[0]->fb->addFilter( new SwOSFJoystick( calibration[0].minValue, calibration[0].midValue, calibration[0].maxValue ) );
      joy[0]->lr->addFilter( new SwOSFJoystick( calibration[1].minValue, calibration[1].midValue, calibration[1].maxValue ) );
      joy[0]->fb->addFilter( new SwOSMultiply( -1 ) );

      joy[1]->fb->addFilter( new SwOSFJoystick( calibration[2].minValue, calibration[2].midValue, calibration[2].maxValue ) );
      joy[1]->lr->addFilter( new SwOSFJoystick( calibration[3].minValue, calibration[3].midValue, calibration[3].maxValue ) );
      joy[1]->lr->addFilter( new SwOSMultiply( -1 ) );

      return false; 
    }

    // finish?
    if ( ( s1->getToggle() == FTSWARM_TOGGLEUP ) && ( strcmp( visualizer, "++++ ++++" ) == 0 ) ) { break; }

    if ( testCalibration( joy[0]->lr->getValueI32(), &newCalibration[0], visualizer, 0, 3 ) ||
         testCalibration( joy[0]->fb->getValueI32(), &newCalibration[1], visualizer, 1, 2 ) ||
         testCalibration( joy[1]->lr->getValueI32(), &newCalibration[2], visualizer, 8, 5 ) ||
         testCalibration( joy[1]->fb->getValueI32(), &newCalibration[3], visualizer, 7, 6 ) ) {
      printf("\b\b\b\b\b\b\b\b\b%s", visualizer); flushStdIO();
    }
    
    // wait for new values
    delay(25);

  }

  // 2nd step get mid / released positions

  printf("\nPlease release both joysticks or press S4 to abort.\n");

  int32_t lastValue[4] = { FILTER_INVALID, FILTER_INVALID, FILTER_INVALID, FILTER_INVALID };
  int32_t newValue[4]  = { FILTER_INVALID, FILTER_INVALID, FILTER_INVALID, FILTER_INVALID };
  uint8_t nTimes = 0;

  while (nTimes < 3) {

    // abort?
    if ( s4->getToggle() == FTSWARM_TOGGLEUP ) { 
      joy[0]->fb->addFilter( new SwOSFJoystick( calibration[0].minValue, calibration[0].midValue, calibration[0].maxValue ) );
      joy[0]->lr->addFilter( new SwOSFJoystick( calibration[1].minValue, calibration[1].midValue, calibration[1].maxValue ) );
      joy[1]->fb->addFilter( new SwOSFJoystick( calibration[2].minValue, calibration[2].midValue, calibration[2].maxValue ) );
      joy[1]->lr->addFilter( new SwOSFJoystick( calibration[3].minValue, calibration[3].midValue, calibration[3].maxValue ) );
      return false; 
    }

    // catch new values
    newValue[0] = joy[0]->fb->getValueI32();
    newValue[1] = joy[0]->lr->getValueI32();
    newValue[2] = joy[1]->fb->getValueI32();
    newValue[3] = joy[1]->lr->getValueI32();

    // are the new values the same values as last time?
    bool stable = true;
    for (uint8_t i=0; i<4; i++) {
      stable == stable || ( lastValue[i] != FILTER_INVALID ) || ( newValue[i] != FILTER_INVALID ) || ( lastValue[i] == newValue[i] );
      lastValue[i] = newValue[i];
    }

    // if all is clear, increment counter, otherwise reset it
    if ( stable ) nTimes++; else nTimes = 0;

    // wait for new values
    delay(25);
    
  }

  // all done, copy values 
  for (uint8_t i=0; i<4; i++) {
    newCalibration[i].midValue = newValue[i];
  }
  memcpy( calibration, newCalibration, 4 * sizeof( SwOSJoyCalibration_t ) );

  // set filters  
  joy[0]->fb->addFilter( new SwOSFJoystick( calibration[0].minValue, calibration[0].midValue, calibration[0].maxValue ) );
  joy[0]->lr->addFilter( new SwOSFJoystick( calibration[1].minValue, calibration[1].midValue, calibration[1].maxValue ) );
  joy[1]->fb->addFilter( new SwOSFJoystick( calibration[2].minValue, calibration[2].midValue, calibration[2].maxValue ) );
  joy[1]->lr->addFilter( new SwOSFJoystick( calibration[3].minValue, calibration[3].midValue, calibration[3].maxValue ) );

  return true;

}