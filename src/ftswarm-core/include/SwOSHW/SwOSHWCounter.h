/*
 * SwOSHWCounter.h
 *
 * Counter inputs hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include "SwOSHWBaseIO.h"

#include <driver/pcnt.h>
 
 /***************************************************
 *
 *   SwOSCounter
 *
 ***************************************************/

 class SwOSCounter : public SwOSInput {

  protected:

    gpio_num_t _CONTROL  = GPIO_NUM_NC;
    uint8_t _portControl = 255;
    pcnt_unit_t _unit = PCNT_UNIT_MAX;

    virtual void _setupLocal();

  public:
 
	  SwOSCounter(const char *name, uint8_t port1, uint8_t port2, SwOSCtrl *ctrl );
  
    // administrative stuff
	  virtual FtSwarmIOType_t getIOType();
    virtual void jsonize( JSONize *json, uint8_t id);

    // read sensor
	  virtual void read();

    // external commands
    virtual void resetCounter( void );
    virtual void setValue( int32_t value );                 // set value by an external call

};

/***************************************************
 *
 *   SwOSFrequencymeter
 *
 ***************************************************/

class SwOSFrequencymeter : public SwOSInput {

  protected:

    gpio_num_t _CONTROL  = GPIO_NUM_NC;
    uint8_t    _portControl = 255;

    int64_t                  _lastTick = 0;
    QueueHandle_t _freqQueue = NULL;

    virtual void _setupLocal();

  public:
 
	  SwOSFrequencymeter(const char *name, uint8_t port1, uint8_t port2, SwOSCtrl *ctrl );
    ~SwOSFrequencymeter();
  
    // administrative stuff
	  virtual FtSwarmIOType_t getIOType() { return FTSWARM_FREQUENCYINPUT; };
    virtual void jsonize( JSONize *json, uint8_t id);

    // read sensor
	  virtual void read();
    virtual void setValue( int32_t value );                 // set value by an external call
    
};
