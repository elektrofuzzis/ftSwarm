/*
 * SwOSHWAnalog.cpp
 *
 * Analog inputs hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOSHW/SwOSHWAnalog.h"
#include "SwOSHW/SwOSHWHAL.h"
#include "SwOSHW/SwOSHWBaseCtrl.h"
#include "SwOSHW/SwOSHWXXCtrl.h"
#include "SwOSFilter.h"

/***************************************************
 *
 *   SwOSAnalogInput
 *
 ***************************************************/

 SwOSAnalogInput::SwOSAnalogInput(const char *name, uint8_t port, SwOSCtrl *ctrl ) : SwOSInput( name, port, ctrl, FTSWARM_DIGITAL ) {
  
  // initialize local HW
  if ( _ctrl->isLocal() ) _setupLocal( );

}

void SwOSAnalogInput::_setupLocal() {
  // initialize local HW

  SwOSInput::_setupLocal( );

  adc_atten_t attenuation;

  // local init
  _ADCUnit      = GPIO_INPUT[(int8_t)_ctrl->getCPU()][(int8_t) _port].adc_unit;
  _ADCChannel   = GPIO_INPUT[(int8_t)_ctrl->getCPU()][(int8_t) _port].adc_channel;
  attenuation   = GPIO_INPUT[(int8_t)_ctrl->getCPU()][ _port].attenuation;

  if ( ( _ADCUnit ==  ADC_UNIT_1) && ( _ADCChannel != ADC1_CHANNEL_MAX ) ) {
    // set ADC to 12 bits, scale 3.9V
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten( (adc1_channel_t) _ADCChannel, attenuation );
  }

  #if CONFIG_IDF_TARGET_ESP32S3
  if ( ( _ADCUnit == ADC_UNIT_2 ) && ( _ADCChannel != ADC2_CHANNEL_MAX ) ) {
    adc2_config_channel_atten( (adc2_channel_t) _ADCChannel, attenuation );
  }
  #endif

  filter = new SwOSSpike( 10, 60 );
  filter->addFilter( new SwOSMovingAverage( 3) );

}

SwOSAnalogInput::~SwOSAnalogInput( ) {

  if (filter) delete filter;

}

void SwOSAnalogInput::deleteFilter( void ) {
  
  if (filter) delete filter;
  filter = NULL;

}

void SwOSAnalogInput::addFilter( SwOSFilter *filter ) {

  if (this->filter) this->filter->addFilter( filter );
  else this->filter = filter;

}

bool SwOSAnalogInput::isXMeter() {

  return ( ( _sensorType == FTSWARM_VOLTMETER ) ||
           ( _sensorType == FTSWARM_OHMMETER ) ||
           ( _sensorType == FTSWARM_THERMOMETER ) );

}

void SwOSAnalogInput::setSensorTypeLocal( FtSwarmSensor_t sensorType ) {

  // analog calibration if needed
  if ( ( isXMeter() ) && (!_adc_chars) ) {
    _adc_chars = (esp_adc_cal_characteristics_t*) calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize((adc_unit_t) _ADCUnit, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 0, _adc_chars);
  }

}

float SwOSAnalogInput::getVoltage() {

  if ( _ctrl->getCPU() == FTSWARMDUINO_1V141 )
    return ( (float) _lastRawValue ) / 1000;
  else
    return ( (float) _lastRawValue ) / 1000 * (129.0/82.0);
}

float SwOSAnalogInput::getResistance() {

  if ( _ctrl->getCPU() == FTSWARMDUINO_1V141 ) return ( (float) _lastRawValue ) / 1000;

  // y = 8E-15x^6 - 2E-10x^5 + 1E-06x^4 - 0,0057x^3 + 13,847x^2 - 17791x + 9E+06

  // avoid hangup
  if ( _lastRawValue == 0 ) return 0;

  float adc = (float) _lastRawValue;

  float r = exp(-15)*pow(adc,float(6)) - 2*exp(-10)*pow(adc,5) + exp(-6)*pow(adc,4) - 0.0057*pow(adc,3) + 13.847*pow(adc,2) - 17791*adc + exp(6);

  return r;
  
}

#define NULLKELVIN 273.15

float SwOSAnalogInput::getKelvin() {

  // Tref = 25°C
  // Rref = 1500
  // B    = 3900
  //
  // all temperatures in Kelvin
  //
  //                 1
  // T = ----------------------------
  //     ln(R/RRef)/B + 1 / Tref

  float r = getResistance();

  // avoid some trouble
  if ( r <= 0 ) return -NULLKELVIN;

  float t = 1 / ( ( log( r / 1500 ) / 3900 ) + ( 1 / ( 25+NULLKELVIN ) ) );
  
  return t;
}

float SwOSAnalogInput::getCelcius() {
  // ToDo cast to temperature
  return getKelvin() - NULLKELVIN;
}

float SwOSAnalogInput::getFahrenheit() {
  // ToDo cast to temperature
  return getCelcius() * 9 / 5 + 32;
}

void SwOSAnalogInput::read() {

  // nothing todo on remote sensors
  if (!_ctrl->isLocal()) return;
  if (_ctrl->isI2CSwarmCtrl()) return;  // sensor is read via a block control by <controller>.read

  // non existing port?
  if ( ( _GPIO == GPIO_NUM_NC ) || ( _ADCChannel == ADC1_CHANNEL_MAX) ) return;

  int32_t newValue;

  if ((adc_unit_t)_ADCUnit == ADC_UNIT_1) newValue = adc1_get_raw( (adc1_channel_t )_ADCChannel );

  #if CONFIG_IDF_TARGET_ESP32S3
  if ((adc_unit_t)_ADCUnit == ADC_UNIT_2) {
    int raw;
    adc2_get_raw( (adc2_channel_t )_ADCChannel, ADC_WIDTH_12Bit, &raw );
    newValue = raw;
  }
  #endif

  if ( isXMeter() ) {
    // XMeter: cast to mV
    newValue = esp_adc_cal_raw_to_voltage( newValue, _adc_chars ); 
  }

  if (filter) newValue = filter->fx(newValue); 

  setReading( newValue );

}

void SwOSAnalogInput::setReading( int32_t newValue ) {
    
  // send changed value event?
  if ( (_events) && ( _lastRawValue != newValue ) ) trigger( FTSWARM_TRIGGERVALUE, newValue );

  // store new data
  _lastRawValue = newValue;  

  subscription();

}

void SwOSAnalogInput::setValue( int32_t value ) {

  // nothing ToDo on real local HW
  if ( ( _ctrl->isLocal()) && (!_ctrl->isI2CSwarmCtrl() ) ) return;

  // check if it's toggled?
  if ( _lastRawValue != value) { 

    // trigger value event
    trigger( FTSWARM_TRIGGERVALUE, value );
    
  }
  
  _lastRawValue = value;

  subscription();

}

void SwOSAnalogInput::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variableUI32("sensorType", _sensorType);
  json->variableUI32("subType",    _sensorType);

  if ( _sensorType == FTSWARM_VOLTMETER ) {
    json->variableVolt("value", getVoltage() );
  } else if ( _sensorType == FTSWARM_OHMMETER ) {
    json->variableOhm("value", getResistance() );
  } else if ( _sensorType == FTSWARM_THERMOMETER ) {
    json->variableCelcius("value", getCelcius() );
  } else {
    json->variableI32("value", getValueI32() );
  }
  
  json->endObject();
}

/***************************************************
 *
 *   SwOSJoystick
 *
 *   Port Mapping:
 *
 *        1v0           1v3           1v15
 *   JOY1 LR  GPIO39/ADC1_CHANNEL_3 GPIO33/ADC1_CHANNEL_5 n/a
 *   JOY1 FB  GPIO36/ADC1_CHANNEL_0 GPIO36/ADC1_CHANNEL_0 n/a
 *   JOY2 LR  GPIO32/ADC1_CHANNEL_4 GPIO32/ADC1_CHANNEL_4 n/a
 *   JOY2 FB  GPIO34/ADC1_CHANNEL_6 GPIO34/ADC1_CHANNEL_6 n/a
 *
 ***************************************************/

 SwOSJoystick::SwOSJoystick(const char *name, uint8_t port,SwOSCtrl *ctrl, int16_t zeroLR, int16_t zeroFB ) : SwOSIO( name, port, ctrl ) {

  // set read values to undefined
  _lastLR = 0;
  _lastFB = 0;
  _lastSubscribedLR = 0;
  _lastSubscribedFB = 0;
  _zeroLR = 0;
  _zeroFB = 0;
  _lastRawLR = -1;
  _lastRawFB = -1;

  // initialize local HW
  if (_ctrl->isLocal()) {
    _zeroLR = zeroLR;
    _zeroFB = zeroFB;
    _setupLocal();
  }
  
}

void SwOSJoystick::_setupLocal() {
  // initialize local HW

  // assign port to GPIO
  _ADCChannelLR = ADC1_CHANNEL_MAX;
  _ADCChannelFB = ADC1_CHANNEL_MAX;

  if ( _ctrl->getCPU() == FTSWARMJST_1V0 ) {
    switch (_port) {
    case 0:
      _ADCChannelLR = ADC1_CHANNEL_3;
      _ADCChannelFB = ADC1_CHANNEL_0;
      break;
    case 1:
      _ADCChannelLR = ADC1_CHANNEL_4;
      _ADCChannelFB = ADC1_CHANNEL_6;
      break;
    default: break;
    }
  } else if ( _ctrl->getCPU() == FTSWARMCONTROL_1V3 ) {
    switch (_port) {
    case 0:
      _ADCChannelLR = ADC1_CHANNEL_5;
      _ADCChannelFB = ADC1_CHANNEL_0;
      break;
    case 1:
      _ADCChannelLR = ADC1_CHANNEL_4;
      _ADCChannelFB = ADC1_CHANNEL_6;
      break;
    default: break;
    }
  }

  // set ADC to 12 bits, scale 3.9V
  adc1_config_width(ADC_WIDTH_BIT_12);
  if (_ADCChannelLR != ADC1_CHANNEL_MAX ) {
    adc1_config_channel_atten( _ADCChannelLR, ADC_ATTEN_DB_11);
    adc1_config_channel_atten( _ADCChannelFB, ADC_ATTEN_DB_11);
  }

}

int16_t readChannel( adc1_channel_t channel, int16_t zero, int16_t *lastRaw, uint8_t port) {

  // with correct channels only
  if ( channel == ADC1_CHANNEL_MAX ) return 0;

  // get two readings and calc mean value
  int16_t newRaw = ( adc1_get_raw( channel ) + adc1_get_raw( channel ) ) /2;

  // hysteresis
  if ( ( abs( newRaw - *lastRaw ) < 40 ) && ( lastRaw >= 0 ) ) { newRaw = *lastRaw; }
  *lastRaw = newRaw;

  // calc result
  int16_t result = ( newRaw - zero ) / 20;

  if ( result >  100 ) result =  100;
  if ( result < -100 ) result = -100;

  // change directions on right joystick
  if (port>0) result = -result;

  return result;
  
}

bool hasChanged( int16_t value1, int16_t value2, int16_t hysteresis ) {
  
  return abs( value1 - value2 ) > hysteresis;

}

void SwOSJoystick::subscription() {

  // test, if input is subscribed
  if (!_isSubscribed) return;

  if ( ( hasChanged( _lastLR, _lastSubscribedLR, _hysteresis ) ) ||
       ( hasChanged( _lastFB, _lastSubscribedFB, _hysteresis ) ) ) {
    printf("S: %s %d %d\n", _subscribedIOName, _lastLR, _lastFB );
    _lastSubscribedLR = _lastLR;
    _lastSubscribedFB = _lastFB;

  }

}

void SwOSJoystick::read() {

  int16_t x;

  // nothing ToDO with remote HW
  if (!_ctrl->isLocal()) return;

    x = readChannel( _ADCChannelLR, _zeroLR, &_lastRawLR, _port );
    if ( x != _lastLR ) triggerLR.trigger( FTSWARM_TRIGGERVALUE, x );
    _lastLR = x;

    x = readChannel( _ADCChannelFB, _zeroFB, &_lastRawFB, _port );
    if ( x != _lastFB ) triggerFB.trigger( FTSWARM_TRIGGERVALUE, x );
    _lastFB = x;

    subscription();

}

void SwOSJoystick::setValue( int16_t FB, int16_t LR ) {

  if ( _lastLR != LR )  triggerLR.trigger( FTSWARM_TRIGGERVALUE, LR );
  if ( _lastFB != FB )  triggerFB.trigger( FTSWARM_TRIGGERVALUE, FB );

  _lastLR = LR;
  _lastFB = FB;
  
}


void SwOSJoystick::calibrate( int16_t *zeroLR, int16_t *zeroFB ) {
  
  // nothing ToDO with remote HW
  if (!_ctrl->isLocal()) return;
  if ( (_ADCChannelLR == ADC1_CHANNEL_MAX ) || ( _ADCChannelFB == ADC1_CHANNEL_MAX )) return;

  // get 3 values
  int16_t lr[3], fb[3];
  for ( uint8_t i=0; i<3; i++ ) {
    lr[i] =  adc1_get_raw( _ADCChannelLR ); 
    fb[i] =  adc1_get_raw( _ADCChannelFB ); 
    vTaskDelay( 25 / portTICK_PERIOD_MS );
  }

  // and calculate mean value
  *zeroLR = _zeroLR = ( lr[0] + lr[1] + lr[2] ) / 3;
  *zeroFB = _zeroFB = ( fb[0] + fb[1] + fb[2] ) / 3;
  
}

void SwOSJoystick::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variableI16("valueLr", _lastLR );
  json->variableI16("valueFb", _lastFB );
  json->variableB("button", static_cast<SwOSSwarmControl *>(_ctrl)->button[6+_port]->getState());
  json->endObject();
}
