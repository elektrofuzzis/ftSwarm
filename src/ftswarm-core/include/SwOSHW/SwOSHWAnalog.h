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
#include "SwOSHWBaseCtrl.h"
#include "SwOSFilter.h"

#include <esp_adc_cal.h>

/***************************************************
 *
 *   SwOSAnalogInput
 *
 ***************************************************/

 class SwOSAnalogInput : public SwOSInput {

  protected:
    int8_t                         ADCChannel = ADC1_CHANNEL_MAX;
    int8_t                         ADCUnit    = GPIO_NUM_NC;
    esp_adc_cal_characteristics_t *adc_chars  = NULL;
    SwOSFilter                    *filter     = NULL;
	
    bool isXMeter();
    virtual void setupLocal();
    virtual void setSensorTypeLocal( FtSwarmSensor_t sensorType );  // set sensor type

  public:
 
	  SwOSAnalogInput(const char *name, uint8_t port, SwOSCtrl *ctrl );
    ~SwOSAnalogInput();
  
    // administrative stuff
	  virtual FtSwarmIOType_t getIOType() { return FTSWARM_ANALOGINPUT; };
    virtual void jsonize( JSONize *json, uint8_t id);

    // delete all existing filtes
    virtual void deleteFilter( void );

    // add a new filter
    virtual void addFilter( SwOSFilter *filter );

    // read sensor
	  virtual void     read();
    virtual void setReading( int32_t newValue );

    // external commands
    virtual void   setValue( int32_t value );                    // set value by an external call
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

 class SwOSJoystick : public SwOSIO, SwOSEventInput {
  protected:
    adc1_channel_t ADCChannelLR, ADCChannelFB;
    int16_t        lastLR, lastFB;
    int16_t        lastSubscribedLR, lastSubscribedFB;
    int16_t        zeroLR, zeroFB;
    int16_t        lastRawLR, lastRawFB;
  
    // local HW procedures
    virtual void setupLocal(); // initializes local HW
    
  public:
    SwOSEventInput triggerLR, triggerFB;
    
    // constructors
    SwOSJoystick(const char *name, uint8_t port, SwOSCtrl *ctrl, int16_t zeroLR, int16_t zeroFB );
  
    // administrative stuff
    virtual FtSwarmIOType_t getIOType() { return FTSWARM_JOYSTICK; };
    virtual FtSwarmIcon_t getIcon() { return FTSWARM_11_JOYSTICK; };
    virtual void jsonize( JSONize *json, uint8_t id);
    
    // Test, if I', an Sensor
    virtual bool            isSensor( void ) { return true; }
  
    // read
    virtual void subscription();
    virtual void read();
  
    // commands
    virtual void getValue( int16_t* FB, int16_t* LR ) { *FB = lastFB; *LR = lastLR; };
    virtual void setValue( int16_t  FB, int16_t  lastLR );
    virtual void calibrate( int16_t *zeroLR, int16_t *zeroFB );  // uses actual readings to calibrate
  };