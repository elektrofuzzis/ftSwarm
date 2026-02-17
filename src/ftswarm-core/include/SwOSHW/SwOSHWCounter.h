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

#include "driver/pcnt.h"

 /***************************************************
 *
 *   SwOSCounter
 *
 ***************************************************/

 class SwOSCounter : public SwOSInput {

  protected:

    gpio_num_t CONTROL  = GPIO_NUM_NC;
    uint8_t portControl = SWOS_NOPORT;
    pcnt_unit_t unit    = PCNT_UNIT_MAX;

    virtual void setupLocal();

  public:
 
	  SwOSCounter(const char *name, uint8_t port1, uint8_t port2, SwOSCtrl *ctrl, bool hidden );
  
    // administrative stuff
    virtual void serialize( Serialize *serialize );
    virtual bool isCounter( void ) { return true; };
    // virtual void registerEvent( int32_t value );

    // read sensor
	  virtual void operate();

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

    gpio_num_t CONTROL     = GPIO_NUM_NC;
    uint8_t    portControl = SWOS_NOPORT;

    int64_t       lastTick = 0;
    QueueHandle_t freqQueue = NULL;

    virtual void setupLocal();

  public:
 
	  SwOSFrequencymeter(const char *name, uint8_t port1, uint8_t port2, SwOSCtrl *ctrl, bool hidden );
    ~SwOSFrequencymeter();
  
    // administrative stuff
    virtual void serialize( Serialize *serialize );

    // read sensor
	  virtual void operate();
    virtual void setValue( int32_t value );                 // set value by an external call
    
};
