/*
 * SwOSHWAnalog.h
 *
 * Analog inputs hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include "SwOSHWBaseIO.h"
#include "SwOSHWDigital.h"
#include "SwOSHWBaseCtrl.h"
#include "SwOSFilter.h"

#include <driver/adc.h>
#include <esp_adc_cal.h>
#include "hal/adc_types.h"

/***************************************************
 *
 *   SwOSAnalogInput
 *
 ***************************************************/

 class SwOSAnalogInput : public SwOSInput {

  protected:
    int8_t                        ADCChannel  = ADC1_CHANNEL_MAX;
    int8_t                        ADCUnit     = GPIO_NUM_NC;
    adc_atten_t                   attenuation = ADC_ATTEN_DB_12;
    esp_adc_cal_characteristics_t *adc_chars  = NULL;
    SwOSFilter                    *filter     = NULL;
	
    bool isXMeter();
    virtual void setupLocal();

  public:
 
	  SwOSAnalogInput(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType, uint8_t flags );
    ~SwOSAnalogInput();
  
    // administrative stuff
	  virtual void serialize( Serialize *serialize );
    virtual bool isGPIOInput( void );  
    virtual bool isAnalogInput( void ) { return true; };

    // delete all existing filtes
    virtual void deleteFilter( void );

    // delete a specific filter type
    virtual void deleteFilter( SwOSFilter_t filterType );

    // add a new filter
    virtual void addFilter( SwOSFilter *filter );

    // find first filter of type ft
    virtual SwOSFilter *getFilter( SwOSFilter_t ft );

    // read sensor
	  virtual void operate();

    // push my state to a buffer
    virtual uint8_t pushState( uint8_t *buffer ) { return pushState16( buffer ); };

    // pop my state from a buffer
    virtual uint8_t popState( uint8_t *buffer )  { return popState16( buffer ); };


    // external commands
    virtual float  getVoltage();
    virtual float  getResistance();
    virtual float  getKelvin();
    virtual float  getCelcius();
    virtual float  getFahrenheit();

};


/***************************************************
 *
 *   SwOSJoystick
 *
 ***************************************************/

 class SwOSJoystick : public SwOSIO {

  public:

    SwOSDigitalInput* button = NULL;
    SwOSAnalogInput*  lr     = NULL;
    SwOSAnalogInput*  fb     = NULL;
              
    // constructors
    SwOSJoystick(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSDigitalInput* button, SwOSAnalogInput* lr, SwOSAnalogInput* fb, uint8_t flags );
  
    // administrative stuff
    virtual void serialize( Serialize *serialize );
    // not an input due to firmware check during events. Might be a TODO in other cases
    // virtual bool isInput( void ) { return true; };

    virtual char* subscribe( char *IOName, uint32_t hysteresis ); // subscribe sensor to display value changes as console outputs 
	  virtual void  unsubscribe();                                  // clear subscription
  
    // commands
    virtual void getValue( int16_t* FB, int16_t* LR ) { *FB = fb->getValueI32(); *LR = lr->getValueI32(); };

    // start calibration
    virtual void deleteFilters( void );

    // start calibration
    virtual void addFilters( void );

  };