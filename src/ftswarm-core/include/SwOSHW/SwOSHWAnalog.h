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

#include <esp_adc_cal.h>

/***************************************************
 *
 *   SwOSAnalogInput
 *
 ***************************************************/

 class SwOSAnalogInput : public SwOSInput {

  protected:
    int8_t            _ADCChannel = ADC1_CHANNEL_MAX;
    int8_t            _ADCUnit    = GPIO_NUM_NC;
    esp_adc_cal_characteristics_t *_adc_chars = NULL;
	
    bool isXMeter();
    virtual void _setupLocal();
    virtual void setSensorTypeLocal( FtSwarmSensor_t sensorType );  // set sensor type

  public:
 
	  SwOSAnalogInput(const char *name, uint8_t port, SwOSCtrl *ctrl );
  
    // administrative stuff
	  virtual FtSwarmIOType_t getIOType() { return FTSWARM_ANALOGINPUT; };
    virtual void jsonize( JSONize *json, uint8_t id);

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
    adc1_channel_t _ADCChannelLR, _ADCChannelFB;
    int16_t        _lastLR, _lastFB;
    int16_t        _lastSubscribedLR, _lastSubscribedFB;
    int16_t        _zeroLR, _zeroFB;
    int16_t        _lastRawLR, _lastRawFB;
  
    // local HW procedures
    virtual void _setupLocal(); // initializes local HW
    
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
    virtual void getValue( int16_t* FB, int16_t* LR ) { *FB = _lastFB; *LR = _lastLR; };
    virtual void setValue( int16_t  FB, int16_t  lastLR );
    virtual void calibrate( int16_t *zeroLR, int16_t *zeroFB );  // uses actual readings to calibrate
  };