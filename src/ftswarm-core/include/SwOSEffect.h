/*
 * SwOSEffect.h
 *
 * Effect definitions for ftSwarm
 * 
 * (C) 2026 Christian Bergschneider & Stefan Fuss
 *
 */

#pragma once

#include "SwOS.h"

typedef enum { 
  FTSWARM_EFFECT_COLOR_BLACK, 
  FTSWARM_EFFECT_COLOR_RED, 
  FTSWARM_EFFECT_COLOR_GREEN, 
  FTSWARM_EFFECT_COLOR_BLUE, 
  FTSWARM_EFFECT_COLOR_YELLOW, 
  FTSWARM_EFFECT_COLOR_ORANGE,
  FTSWARM_EFFECT_COLOR_CYAN, 
  FTSWARM_EFFECT_COLOR_PINK, 
  FTSWARM_EFFECT_COLOR_MAGENTA, 
  FTSWARM_EFFECT_COLOR_WHITE,
  FTSWARM_EFFECT_COLOR_MAX
} FtSwarmEffectColor_t;

class SwOSBlink {

  private:

    static const uint8_t IDLE  = 0;
    static const uint8_t DUTY1 = 1;
    static const uint8_t DUTY2 = 2;
    static const uint8_t DUTY3 = 3;

    uint16_t signal;  // Number of impulses in a cycle
    uint16_t duty1;   // duty cycle pulse on
    uint16_t duty2;   // duty cycle pulse off
    uint16_t duty3;   // duty cycle pause

    uint8_t  state;
    uint16_t tickCounter;
    uint16_t signalCounter;

    FtSwarmTriggerParameter parameter;

    bool tick( uint32_t duty, uint8_t nextState, uint8_t *p );

  public:

    SwOSBlink( FtSwarmTriggerParameter parameter );

    bool operate( uint8_t *value );

};

class SwOSPixelBlink : public SwOSBlink {

  protected:

    CRGB color[3];
    CRGB p2CRGB( uint8_t c );

  public:

    SwOSPixelBlink( FtSwarmTriggerParameter parameter );
    bool operate( CRGB *color );

};

class SwOSLampBlink : public SwOSBlink {

  protected:

    int16_t brightness[3];
    int16_t p2Brightness( uint8_t p );

  public:

    SwOSLampBlink( FtSwarmTriggerParameter parameter );
    bool operate( int16_t *brightness );

};

