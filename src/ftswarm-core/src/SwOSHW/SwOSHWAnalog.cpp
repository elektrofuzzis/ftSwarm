/*
 * SwOSHWAnalog.cpp
 *
 * Analog inputs hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOSHW/SwOSHWAnalog.h"
#include "SwOSHW/SwOSHWBaseCtrl.h"
#include "SwOSFilter.h"
#include "SwOSHW/SwOSHWDigital.h"

/***************************************************
 *
 *   SwOSAnalogInput
 *
 ***************************************************/

SwOSAnalogInput::SwOSAnalogInput(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType, uint8_t flags ) : SwOSInput( name, port, ctrl, ioType, flags ) {

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
  ADCUnit     = INPUT_ADC_UNIT[port];
  ADCChannel  = INPUT_ADC_CHANNEL[port];
  attenuation = (adc_atten_t) INPUT_ATTENUATION[port];

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

  switch (ioType) {

    case SWOSIO_POWER:  filter = new SwOSSpike( 10, 60 );
                        filter->addFilter( new SwOSMovingAverage( 10 ) );
                        break;

    case SWOSIO_RCPOTI: filter = new SwOSMovingAverage(5);
                        filter->addFilter(new SwOSSHR(2) );
                        break;
  
    default:            filter = new SwOSSpike( 10, 60 );
                        filter->addFilter( new SwOSMovingAverage( 3 ) );
                        break;

  }

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

void SwOSAnalogInput::deleteFilter( SwOSFilter_t filterType ) {

  while ( ( filter ) && ( filter->getType() == filterType ) ) {
    SwOSFilter *obsolete = filter;
    filter = filter->nextFilter;
    obsolete->nextFilter = NULL;
    delete obsolete;
  }

  if (filter) filter->deleteFilter( filterType );

}

void SwOSAnalogInput::addFilter( SwOSFilter *filter ) {

  if (this->filter) this->filter->addFilter( filter );
  else this->filter = filter;

}

SwOSFilter *SwOSAnalogInput::getFilter( SwOSFilter_t ft ) {

  SwOSFilter *f = filter;
  
  while (f) {
    if ( f->getType() == ft ) break;
  }

  return f;

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
    return round( ( 0.00001158854430694 * lastRawValue * lastRawValue -0.03759004605973 * lastRawValue + 34.9580282817 ) * 10 ) / 10;
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

void SwOSAnalogInput::operate() {

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

void SwOSAnalogInput::serialize( Serialize *serialize ) {
  serialize->startObject( );
  SwOSIO::serialize( serialize );

  if ( ( ioType == SWOSIO_VOLTMETER ) || ( ioType == SWOSIO_POWER ) ){
    serialize->item( SERIALIZE_LITERAL_VALUE, getVoltage(), 3, "V" );
  } else if ( ioType == SWOSIO_OHMMETER ) {
    serialize->item( SERIALIZE_LITERAL_VALUE, getResistance(), 0, "Ohm" );
  } else if ( ioType == SWOSIO_THERMOMETER ) {
    serialize->item( SERIALIZE_LITERAL_VALUE, getCelcius(), 1, "°C" );
  } else {
    serialize->item( SERIALIZE_LITERAL_VALUE, getValueI32() );
  }
  
  serialize->endObject();
}


// SwOSJoystick

SwOSJoystick::SwOSJoystick(const char *name, uint8_t port,SwOSCtrl *ctrl, SwOSDigitalInput* button, SwOSAnalogInput* lr, SwOSAnalogInput* fb, uint8_t flags ) : SwOSIO( name, port, ctrl, SWOSIO_JOYSTICK, flags ) {

  this->button = button;
  this->lr     = lr;
  this->fb     = fb;

  // initialize local HW
  if (ctrl->isLocal()) {

    lr->addFilter( new SwOSFJoystick( nvs.calibration[port*2+1].minValue, nvs.calibration[port*2+1].midValue, nvs.calibration[port*2].maxValue ) );
    if ( port) lr->addFilter( new SwOSMultiply( -1 ) ); 

    fb->addFilter( new SwOSFJoystick( nvs.calibration[port*2].minValue, nvs.calibration[port*2].midValue, nvs.calibration[port*2].maxValue) );
    if (!port) fb->addFilter( new SwOSMultiply( -1 ) ); 

  }
  
}

void SwOSJoystick::serialize( Serialize *serialize ) {
  serialize->startObject( );
  SwOSIO::serialize( serialize );

  serialize->item( SERIALIZE_LITERAL_VALUELR, lr->getValueI32() );
  serialize->item( SERIALIZE_LITERAL_VALUEFB, fb->getValueI32() );

  if (button) serialize->item( SERIALIZE_LITERAL_VALUE, button->getValueI32() );
  
  serialize->endObject();

}

char* SwOSJoystick::subscribe( char *IOName, uint32_t hysteresis ) {

  if( button ) button->subscribe( button->getName(), 0 ) ;
  if( lr )     lr->subscribe( lr->getName(), 0 ) ;
  if( fb )     fb->subscribe( fb->getName(), 0 ) ;

  return SwOSIO::subscribe( IOName, hysteresis );
  
}

void  SwOSJoystick::unsubscribe() {

  if (button) button->unsubscribe();
  if (lr)     lr->unsubscribe();
  if (fb)     fb->unsubscribe();

  SwOSIO::unsubscribe();

}
