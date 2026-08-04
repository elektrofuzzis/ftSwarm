/*
 * SwOSEffect.cpp
 *
 * Effect definitions for ftSwarm
 * 
 * (C) 2026 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOSEffect.h"

/***************************
 * 
 * SwOSBlink
 * 
 ***************************/

SwOSBlink::SwOSBlink( FtSwarmTriggerParameter parameter ) {

  this->parameter = parameter;

  state = IDLE;

  signal = parameter.blink.signal;

  duty1 = uint16_t ( ( (float) parameter.blink.period ) * ( 0.25 * ( parameter.blink.duty + 1 ) ) ) * 4;
  duty2 = parameter.blink.period *4 - duty1;
  duty3 = parameter.blink.pause * parameter.blink.period * 4;

  tickCounter = 0;
  signalCounter = 0;

}

bool SwOSBlink::tick( uint32_t duty, uint8_t nextState, uint8_t *p ) {

  tickCounter++;

  if ( tickCounter >= duty ) {

    tickCounter = 0;
    state = nextState;
    *p = state - DUTY1;
    return true;

  }

  return false;

}

bool SwOSBlink::operate( uint8_t *p ) {

  switch ( state ) {

    case IDLE:  // start: set first DUTY1
                state = DUTY1;
                *p = 0;
                return true;

    case DUTY1: // First duty cycle of a pulse 
                // increment tickCounter and test for DUTY2
                return tick( duty1, DUTY2, p );

    case DUTY2: // Second duty cycle of a pulse
                if ( tick( duty2, DUTY1, p ) ) {

                  // 2nd duty is done, run next signal
                  signalCounter++;

                  // all signals done: start with DUTY3
                  if ( signalCounter >= signal ) {
                    signalCounter = 0;
                    state = DUTY3;
                    *p = 2;
                  }

                  return true;

                }

                return false;

    case DUTY3: // Test if DUTY3 is done and start DUTY1 again
                return tick( duty3, DUTY1, p );

  }

  return false;
  
}

/***************************
 * 
 * SwOSPixelBlink
 * 
 ***************************/

static CRGB FTSWARM_EFFECT_COLOR[FTSWARM_EFFECT_COLOR_MAX] = { CRGB::Black, CRGB::Red, CRGB::Green, CRGB::Blue, CRGB::Yellow, CRGB::Orange, CRGB::Cyan, CRGB::Pink, CRGB::Magenta, CRGB::White };

SwOSPixelBlink::SwOSPixelBlink( FtSwarmTriggerParameter parameter ) : SwOSBlink( parameter ) {

  color[0] = p2CRGB( parameter.blink.p1 );
  color[1] = p2CRGB( parameter.blink.p2 );
  color[2] = p2CRGB( parameter.blink.p3 );

}

CRGB SwOSPixelBlink::p2CRGB( uint8_t c ) {

  // out of range: take black
  if ( c >= FTSWARM_EFFECT_COLOR_MAX ) return FTSWARM_EFFECT_COLOR[ FTSWARM_EFFECT_COLOR_BLACK ];

  // in range
  return FTSWARM_EFFECT_COLOR[ c ];

}

bool SwOSPixelBlink::operate( CRGB *color ) {

  uint8_t p;
  bool result = SwOSBlink::operate( &p );

  if ( p <= 3 ) *color = this->color[p];
  return result;

}

/***************************
 * 
 * SwOSLampBlink
 * 
 ***************************/

SwOSLampBlink::SwOSLampBlink( FtSwarmTriggerParameter parameter ) : SwOSBlink( parameter ) {

  brightness[0] = p2Brightness( parameter.blink.p1 );
  brightness[1] = p2Brightness( parameter.blink.p2 );
  brightness[2] = p2Brightness( parameter.blink.p3 );

}

int16_t SwOSLampBlink::p2Brightness( uint8_t p ) {

  return ( (float) p ) * 6.25;

}

bool SwOSLampBlink::operate( int16_t *brightness ) {

  uint8_t p;
  bool result = SwOSBlink::operate( &p );

  if ( p <= 3 ) *brightness = brightness[p];
  return result;

}