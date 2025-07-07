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
#include "SwOSFilter.h"
#include "SwOSHW/SwOSHWDigital.h"

/***************************************************
 *
 *   SwOSAnalogInput
 *
 ***************************************************/

 SwOSAnalogInput::SwOSAnalogInput(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType ) : SwOSInput( name, port, ctrl, ioType ) {
  
  // initialize local HW
  if ( ctrl->isLocal() ) setupLocal( );

}

void SwOSAnalogInput::setupLocal() {
  // initialize local HW

  // ftDuino
  if ( ( ctrl->getCPU() == FTSWARMDUINO_1V141 ) && (ftDuino) ) {
    ftDuino->setIOType( port, ioType );
    return;
  }

  SwOSInput::setupLocal( );

  // local init
  ADCUnit     = GPIO_INPUT[(int8_t)ctrl->getCPU()][(int8_t) port].adc_unit;
  ADCChannel  = GPIO_INPUT[(int8_t)ctrl->getCPU()][(int8_t) port].adc_channel;
  attenuation = GPIO_INPUT[(int8_t)ctrl->getCPU()][ port].attenuation;

  if ( ( ADCUnit ==  ADC_UNIT_1) && ( ADCChannel != ADC1_CHANNEL_MAX ) ) {
    // set ADC to 12 bits, scale 3.9V
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten( (adc1_channel_t) ADCChannel, attenuation );
  }

  #if CONFIG_IDF_TARGET_ESP32S3
  if ( ( ADCUnit == ADC_UNIT_2 ) && ( ADCChannel != ADC2_CHANNEL_MAX ) ) {
    adc2_config_channel_atten( (adc2_channel_t) ADCChannel, attenuation );
  }
  #endif

  // analog calibration if needed
  if ( ( isXMeter() ) && (!adc_chars) ) {
    adc_chars = (esp_adc_cal_characteristics_t*) calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize((adc_unit_t) ADCUnit, attenuation, ADC_WIDTH_BIT_12, 0, adc_chars);
  }

  filter = new SwOSSpike( 10, 60 );
  filter->addFilter( new SwOSMovingAverage( 3) );

}

SwOSAnalogInput::~SwOSAnalogInput( ) {

  if (filter) delete filter;

}

bool SwOSAnalogInput::isGPIOInput( void ) { 
  
  return ( ctrl->getCPU() != FTSWARMDUINO_1V141 ); 

};  

void SwOSAnalogInput::deleteFilter( void ) {
  
  if (filter) delete filter;
  filter = NULL;

}

void SwOSAnalogInput::addFilter( SwOSFilter *filter ) {

  if (this->filter) this->filter->addFilter( filter );
  else this->filter = filter;

}

bool SwOSAnalogInput::isXMeter() {

  return ( ( ioType == SWOSIO_VOLTMETER ) ||
           ( ioType == SWOSIO_OHMMETER ) ||
           ( ioType == SWOSIO_THERMOMETER ) );

}

float SwOSAnalogInput::getVoltage() {

  if ( ctrl->getCPU() == FTSWARMDUINO_1V141 )
    return ( (float) lastRawValue ) / 1000;
  else if ( ioType == SWOSIO_POWER )
    return ( (float) lastRawValue ) / 1000 * (129.0/82.0) * 1.7; // ToDo correct power calculation
  else
    return ( (float) lastRawValue ) / 1000 * (129.0/82.0);
}

float SwOSAnalogInput::getResistance() {

  if ( ctrl->getCPU() == FTSWARMDUINO_1V141 ) return ( (float) lastRawValue ) / 1000;

  // y = 8E-15x^6 - 2E-10x^5 + 1E-06x^4 - 0,0057x^3 + 13,847x^2 - 17791x + 9E+06

  // avoid hangup
  if ( lastRawValue == 0 ) return 0;

  float adc = (float) lastRawValue;

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

  return getKelvin() - NULLKELVIN;

}

float SwOSAnalogInput::getFahrenheit() {

  return getCelcius() * 9 / 5 + 32;

}

void SwOSAnalogInput::read() {

  // remote: no work
  if (!ctrl->isLocal()) return;
  
  // ftDuino?
  if ( ( ctrl->getCPU() == FTSWARMDUINO_1V141 ) && ( ftDuino ) ) { setReading( ftDuino->input[port] ); return; }

  // non existing port?
  if ( ( GPIO == GPIO_NUM_NC ) || ( ADCChannel == ADC1_CHANNEL_MAX) ) return;

  int32_t newValue;

  if ((adc_unit_t)ADCUnit == ADC_UNIT_1) newValue = adc1_get_raw( (adc1_channel_t )ADCChannel );

  #if CONFIG_IDF_TARGET_ESP32S3
  if ((adc_unit_t)ADCUnit == ADC_UNIT_2) {
    int raw;
    adc2_get_raw( (adc2_channel_t )ADCChannel, ADC_WIDTH_12Bit, &raw );
    newValue = raw;
  }
  #endif

  if ( isXMeter() ) {
    // XMeter: cast to mV
    newValue = esp_adc_cal_raw_to_voltage( newValue, adc_chars ); 
  }

  if (filter) newValue = filter->fx(newValue); 

  setReading( newValue );

}

void SwOSAnalogInput::setReading( int32_t newValue ) {
    
  // send changed value event?
  if ( (eventList) && ( lastRawValue != newValue ) ) trigger( FTSWARM_TRIGGERVALUE, newValue );

  // store new data
  lastRawValue = newValue;  

  subscription();

}

void SwOSAnalogInput::setValue( int32_t value ) {

  // stop, if it's not local HW
  if ( ( ctrl->isLocal()) && (!ctrl->isI2CSwarmCtrl() ) ) return;

  // check if it's toggled?
  if ( lastRawValue != value) { 

    // trigger value event
    trigger( FTSWARM_TRIGGERVALUE, value );
    
  }
  
  lastRawValue = value;

  subscription();

}

void SwOSAnalogInput::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);

  if ( ( ioType == SWOSIO_VOLTMETER ) || ( ioType == SWOSIO_POWER ) ){
    json->variableVolt("value", getVoltage() );
  } else if ( ioType == SWOSIO_OHMMETER ) {
    json->variableOhm("value", getResistance() );
  } else if ( ioType == SWOSIO_THERMOMETER ) {
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

 SwOSJoystick::SwOSJoystick(const char *name, uint8_t port,SwOSCtrl *ctrl, int16_t zeroLR, int16_t zeroFB ) : SwOSIO( name, port, ctrl, SWOSIO_JOYSTICK ) {

  // set read values to undefined
  this->lastLR = 0;
  this->lastFB = 0;
  this->lastSubscribedLR = 0;
  this->lastSubscribedFB = 0;
  this->zeroLR = 0;
  this->zeroFB = 0;
  this->lastRawLR = -1;
  this->lastRawFB = -1;

  // initialize local HW
  if (ctrl->isLocal()) {
    this->zeroLR = zeroLR;
    this->zeroFB = zeroFB;
    setupLocal();
  }
  
}

void SwOSJoystick::setupLocal() {
  // initialize local HW

  // assign port to GPIO
  ADCChannelLR = ADC1_CHANNEL_MAX;
  ADCChannelFB = ADC1_CHANNEL_MAX;

  if ( ctrl->getCPU() == FTSWARMCONTROL_1V3 ) {
    switch (port) {
    case 0:
      ADCChannelLR = ADC1_CHANNEL_5;
      ADCChannelFB = ADC1_CHANNEL_0;
      break;
    case 1:
      ADCChannelLR = ADC1_CHANNEL_4;
      ADCChannelFB = ADC1_CHANNEL_6;
      break;
    default: break;
    }
  }

  // set ADC to 12 bits, scale 3.9V
  adc1_config_width(ADC_WIDTH_BIT_12);
  if (ADCChannelLR != ADC1_CHANNEL_MAX ) {
    adc1_config_channel_atten( ADCChannelLR, ADC_ATTEN_DB_11);
    adc1_config_channel_atten( ADCChannelFB, ADC_ATTEN_DB_11);
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
  if (!isSubscribed) return;

  if ( ( hasChanged( lastLR, lastSubscribedLR, hysteresis ) ) ||
       ( hasChanged( lastFB, lastSubscribedFB, hysteresis ) ) ) {
    printf("S: %s %d %d\n", subscribedIOName, lastLR, lastFB );
    lastSubscribedLR = lastLR;
    lastSubscribedFB = lastFB;

  }

}

void SwOSJoystick::read() {

  int16_t x;

  // remote: no work
  if (!ctrl->isLocal()) return;

  x = readChannel( ADCChannelLR, zeroLR, &lastRawLR, port );
  if ( x != lastLR ) triggerLR.trigger( FTSWARM_TRIGGERVALUE, x );
  lastLR = x;

  x = readChannel( ADCChannelFB, zeroFB, &lastRawFB, port );
  if ( x != lastFB ) triggerFB.trigger( FTSWARM_TRIGGERVALUE, x );
  lastFB = x;

  subscription();

}

void SwOSJoystick::setValue( int16_t FB, int16_t LR ) {

  if ( lastLR != LR )  triggerLR.trigger( FTSWARM_TRIGGERVALUE, LR );
  if ( lastFB != FB )  triggerFB.trigger( FTSWARM_TRIGGERVALUE, FB );

  lastLR = LR;
  lastFB = FB;
  
}


void SwOSJoystick::calibrate( int16_t *zeroLR, int16_t *zeroFB ) {

  /// remote: no work
  if (!ctrl->isLocal()) return;
  if ( (ADCChannelLR == ADC1_CHANNEL_MAX ) || ( ADCChannelFB == ADC1_CHANNEL_MAX )) return;

  // get 3 values
  int16_t lr[3], fb[3];
  for ( uint8_t i=0; i<3; i++ ) {
    lr[i] =  adc1_get_raw( ADCChannelLR ); 
    fb[i] =  adc1_get_raw( ADCChannelFB ); 
    vTaskDelay( 25 / portTICK_PERIOD_MS );
  }

  // and calculate mean value
  *zeroLR = this->zeroLR = ( lr[0] + lr[1] + lr[2] ) / 3;
  *zeroFB = this->zeroFB = ( fb[0] + fb[1] + fb[2] ) / 3;
  
}

void SwOSJoystick::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variableI16("valueLr", lastLR );
  json->variableI16("valueFb", lastFB );

  SwOSDigitalInput *button = (SwOSDigitalInput*) ctrl->getIO( SWOSIO_BUTTON, FTSWARM_J1 + port );
  if (button) json->variableB( "button", button->getValueI32() );
  
  json->endObject();

}
