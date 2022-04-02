#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <driver/ledc.h>
#include <driver/gpio.h>
#include <driver/adc.h>

#include <esp_now.h>
#include <esp_log.h>

#include <FastLED.h>

#include "SwOSHW.h"
#include "SwOSCom.h"
#include "ftSwarm.h"

const char IOTYPE[FTSWARM_MAXIOTYPE][10] = { "INPUT", "ACTOR", "BUTTON", "JOYSTICK", "LED", "SERVO", "OLED", "GYRO", "HC165" };

const char SENSORICON[FTSWARM_MAXSENSOR][20] = {
  "00_digital.svg",
  "01_analog.svg",
  "02_switch.svg",
  "03_reedswitch.svg",
  "04_voltage.svg",
  "05_resistor.svg",
  "06_ntc.svg",
  "07_ldr.svg",
  "08_trailsensor.svg",
  "09_colorsensor.svg",
  "10_ultrasonic.svg"
};

const char SENSORTYPE[FTSWARM_MAXSENSOR][20] = { "DIGITAL", "ANALOG", "SWITCH", "REEDSWITCH", "VOLTMETER", "OHMMETER", "THERMOMETER", "LDR", "TRAILSENSOR", "COLORSENSOR", "ULTRASONIC" };

const char ACTORICON[FTSWARM_MAXACTOR][20] = {
  "16_xmotor.svg",
  "17_tractor.svg",
  "18_encoder.svg",
  "19_lamp.svg"
};

const char ACTORTYPE[FTSWARM_MAXACTOR][20] = { "XMOTOR", "TRACTORMOTOR", "ENCODERMOTOR", "LAMP" };

/***************************************************
 *
 *   SwOSObj - Base class for all SwOS objects.
 *
 ***************************************************/

SwOSObj::SwOSObj( const char *name) {
  _alias = NULL;
  _name = (char *) malloc( strlen(name)+1 );
  strcpy( _name, name );
}

SwOSObj::~SwOSObj() {
  if (_name)  free( _name );
  if (_alias) free( _alias );
}

void SwOSObj::setAlias( const char *alias ) {

  if (_alias!=NULL) { free( (void*) _alias); }
  _alias = (char *) malloc(strlen(alias)+1);
  strcpy( _alias, alias );
}

void SwOSObj::setName( const char *name ) {

  if (_name) { free( (void*) _name); }
  _name = (char *) malloc(strlen(name)+1);
  strcpy( _name, name );
  
}

bool SwOSObj::equals( const char *name ) {
  if (strcmp(_name, name) == 0 ) { return true; }
  else if ( ( _alias != NULL) && (strcmp(_alias, name) == 0 ) ) { return true; }
  else { return false; }
}

char * SwOSObj::getName( ) {
  return _name;
}

char * SwOSObj::getAlias( ) {
  return _alias;
}


 void SwOSObj::jsonize( JSONize *json, uint8_t id) {

   // display name
   if (_alias) {
     json->variable("name", _alias);
   } else {
     json->variable("name", _name);
   }

   // unique ID
   char str[50];
   sprintf(str, "%d-%s", id, _name);   json->variable("id", str);

}

/***************************************************
 *
 *   SwOSIO - Base class for all sensors or actors.
 *
 ***************************************************/

SwOSIO::SwOSIO( const char *name, uint8_t port, SwOSCtrl *ctrl ) : SwOSObj( name ) {

  // store local port and controler 
  _port  = port;
  _ctrl  = ctrl;

  char str[10];
  if (_port<255) {
    // normal stuff
    sprintf(str, "%s%d", name, _port+1 );
    setName( str ); 
  } else {
    // avoid servo256
    setName( name );
  } 
 
}

SwOSIO::SwOSIO( const char *name, SwOSCtrl *ctrl ) : SwOSIO( name, 255, ctrl ) {
}

void SwOSIO::jsonize( JSONize *json, uint8_t id) {
  SwOSObj::jsonize(json, id);
  json->variable("type", IOTYPE[getIOType()]);
  json->variable("icon", getIcon() );
}

/***************************************************
 *
 *   SwOSInput
 *
 *    ftSwarm13/ftSwarmControl  ftSwarm15
 *
 * A1 GPIO39/ADC1_CHANNEL_3   GPIO39/ADC1_CHANNEL_3
 * A2 GPIO25/           GPIO32/ADC1_CHANNEL_4
 * A3 GPIO26/           GPIO33/ADC1_CHANNEL_5
 * A4 GPIO27/           GPIO34/ADC1_CHANNEL_6
 *
 ***************************************************/

SwOSInput::SwOSInput(const char *name, uint8_t port, SwOSCtrl *ctrl ) : SwOSIO( name, port, ctrl ) {

  // initialize some vars to undefined
  _lastRawValue = 0;
  _toggle       = FTSWARM_NOTOGGLE;
  _sensorType   = FTSWARM_DIGITAL;

  // initialize local HW
  if ( _ctrl->isLocal() ) _setupLocal();

}

void SwOSInput::_setupLocal() {
  // initialize local HW

  // local init
  _ADCChannel   = ADC1_CHANNEL_MAX;
  _PUA2         = GPIO_NUM_NC;
  _USTX         = GPIO_NUM_NC;

  // assign port to GPIO
  if (_ctrl->getCPU()==FTSWARM_1V15) { // 1v5
    switch (_port) {
    case 0:
      _GPIO = GPIO_NUM_39;
      _ADCChannel = ADC1_CHANNEL_3;
      _USTX = GPIO_NUM_15;
      break;
    case 1:
      _GPIO = GPIO_NUM_32;
      _ADCChannel = ADC1_CHANNEL_4;
      if ( _ctrl->getType() == FTSWARM) { _PUA2 = GPIO_NUM_14; }
      break;
    case 2:
      _GPIO = GPIO_NUM_33;
      _ADCChannel = ADC1_CHANNEL_5;
      break;
    case 3:
      _GPIO = GPIO_NUM_34;
      _ADCChannel = ADC1_CHANNEL_6;
      break;
    default:
      break;
    }
  } else if (_ctrl->getCPU()==FTSWARM_1V3) {
    switch (_port) {
    case 0:
      _GPIO = GPIO_NUM_39;
      _ADCChannel = ADC1_CHANNEL_3;
      _USTX = GPIO_NUM_15;
      break;
    case 1:
      _GPIO = GPIO_NUM_25;
      if ( _ctrl->getType() == FTSWARM) { _PUA2 = GPIO_NUM_14; }
      break;
    case 2:
      _GPIO = GPIO_NUM_26;
      break;
    case 3:
      _GPIO = GPIO_NUM_27;
      break;
    default:
      break;
    }
  } else { // 1v0
    switch (_port) {
    case 0:
      _GPIO = GPIO_NUM_33;
      _ADCChannel = ADC1_CHANNEL_5;
      break;
    case 1:
      _GPIO = GPIO_NUM_25;
      break;
    case 2:
      _GPIO = GPIO_NUM_26;
      break;
    case 3:
      _GPIO = GPIO_NUM_27;
      break;
    default:
      break;
    }
  }

  if ( _ADCChannel != ADC1_CHANNEL_MAX ) {
    // set ADC to 12 bits, scale 3.9V
    adc1_config_width(ADC_WIDTH_BIT_12);
    // adc1_config_channel_atten( _ADCChannel, ADC_ATTEN_DB_11);
  }

  // initialize digital  port
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pin_bit_mask = 1ULL << _GPIO;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);

  // initialize A2 pullup
  if (_PUA2 != GPIO_NUM_NC) {
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << _PUA2;
    gpio_config(&io_conf);
    gpio_set_level( _PUA2, 0 );
  }

  // initialize A1 pulldown
  if (_USTX != GPIO_NUM_NC) {
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << _USTX;
    gpio_config(&io_conf);
    gpio_set_level( _USTX, 0 );
  }

}

char * SwOSInput::getIcon() { 
  return (char *) SENSORICON[ _sensorType ]; 
}; 

bool SwOSInput::isDigitalSensor() {

  return ( ( _sensorType == FTSWARM_DIGITAL ) ||
           ( _sensorType == FTSWARM_SWITCH ) ||
           ( _sensorType == FTSWARM_REEDSWITCH ) ||
           ( _sensorType == FTSWARM_TRAILSENSOR ) );

}

bool SwOSInput::isXMeter() {

  return ( ( _sensorType == FTSWARM_VOLTMETER ) ||
           ( _sensorType == FTSWARM_OHMMETER ) ||
           ( _sensorType == FTSWARM_THERMOMETER ) );

}

void SwOSInput::setSensorType( FtSwarmSensor_t sensorType, bool normallyOpen ) {

  _sensorType   = sensorType;
  _normallyOpen = normallyOpen;

  if (_ctrl->isLocal()) {

    // analog calibration if needed
    if ( ( isXMeter() ) && (!_adc_chars) ) {
      _adc_chars = (esp_adc_cal_characteristics_t*) calloc(1, sizeof(esp_adc_cal_characteristics_t));
      esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 0, _adc_chars);
    }

    // set A1 pullup if available
    if ( ( _PUA2 != GPIO_NUM_NC ) && ( _sensorType != FTSWARM_ULTRASONIC ) ) {
      gpio_set_level( _PUA2, true );
    }
  
  } else {
    
    // send SN, SETSENSORTYPE, _port, sensorType
    SwOSCom cmd( _ctrl->mac, _ctrl->serialNumber, _ctrl->getType(), CMD_SETSENSORTYPE );
    cmd.data.sensorCmd.index        = _port;
    cmd.data.sensorCmd.sensorType   = _sensorType;
    cmd.data.sensorCmd.normallyOpen = _normallyOpen;
    cmd.send( );

  }

}

void SwOSInput::read() {

  // nothing todo on remote sensors
  if (!_ctrl->isLocal()) return;

  // local reading
  if ( ( !isDigitalSensor() ) && ( _ADCChannel != ADC1_CHANNEL_MAX) ) {
    
    _lastRawValue = adc1_get_raw( _ADCChannel );

    if ( isXMeter() ) {
      // XMeter: cast to mV
      _lastRawValue = esp_adc_cal_raw_to_voltage( _lastRawValue, _adc_chars ); 
    }    
    
  } else {
    
    // read new data
    uint32_t newValue = gpio_get_level( _GPIO );
    
    // normally open: change logic
    if (_normallyOpen) newValue = 1-newValue;
    
    // check if it's toggled?
    if ( _lastRawValue != newValue ) { 
      if (newValue) _toggle = FTSWARM_TOGGLEUP;
      else          _toggle = FTSWARM_TOGGLEDOWN;
    }
    
    // store new data
    _lastRawValue = newValue;  
    
  }

}

uint32_t SwOSInput::getValueUI32() {
  return _lastRawValue;
}

float SwOSInput::getValueF() {
  return (float)_lastRawValue;
}

float SwOSInput::getVoltage() {
  return ( (float) _lastRawValue ) / 1000 * (129.0/82.0);
}

float SwOSInput::getResistance() {

  // R1 = 2k
  // R2 = ?
  // R3 = 47k
  // R4 = 82 k
  // Ue = 5V
  //
  //        R1(R3+R4)+R4
  // R2 = ----------------
  //      R4(Ue/Ua)-R1- R3
  //
  //         340000
  // R2 = -----------
  //      82(5/Ue)-49

  // avoid hangup
  if ( _lastRawValue == 0 ) return 0;

  float Ue = ( (float)_lastRawValue ) / 1000;

  float r = 340000.0 / ( ( 410.0 / Ue ) - 49.0 );

  return r;
  
}

#define NULLKELVIN 273.15

float SwOSInput::getKelvin() {

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

float SwOSInput::getCelcius() {
  // ToDo cast to temperature
  return getKelvin() - NULLKELVIN;
}

float SwOSInput::getFahrenheit() {
  // ToDo cast to temperature
  return getCelcius() * 9 / 5 + 32;
}

FtSwarmToggle_t SwOSInput::getToggle() {
  // check toggle state and reset it
  FtSwarmToggle_t toggle = _toggle;
  _toggle = FTSWARM_NOTOGGLE;
  return toggle;
}

void SwOSInput::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variable("sensorType", _sensorType);
  json->variable("subType", SENSORTYPE[_sensorType]);
  
  if ( isDigitalSensor() ) {
    json->variable("value", getValueUI32() );
  } else if ( _sensorType == FTSWARM_VOLTMETER ) {
    json->variableVolt("value", getVoltage() );
  } else if ( _sensorType == FTSWARM_OHMMETER ) {
    json->variableOhm("value", getResistance() );
  } else if ( _sensorType == FTSWARM_THERMOMETER ) {
    json->variableCelcius("value", getCelcius() );
  } else {
    json->variable("value", getValueUI32() );
  }
  json->endObject();
}

void SwOSInput::setValue( uint32_t value ) {

  // nothing ToDo on local HW
  if (_ctrl->isLocal()) return;

  // check if it's toggled?
  if ( _lastRawValue != value) { 
    if   (value) _toggle = FTSWARM_TOGGLEUP;
    else         _toggle = FTSWARM_TOGGLEDOWN;
  }
  
  _lastRawValue = value;

}


/***************************************************
 *
 *   SwOSActor
 *
 ***************************************************/

SwOSActor::SwOSActor(const char *name, uint8_t port, SwOSCtrl *ctrl):SwOSIO(name, port, ctrl ){

  _motionType = FTSWARM_COAST;
  _power      = 0;
  _actorType  = FTSWARM_XMOTOR;

  // initialize local HW
  if (ctrl->isLocal()) _setupLocal();

}

char * SwOSActor::getIcon() { 
  return (char *) ACTORICON[ _actorType ]; 
}; 


void SwOSActor::_setupLocal() {
  // initialize local HW

  // assign port to GPIO. All CPU versions use same ports.
  switch (_port) {
  case 0:
    _IN1 = GPIO_NUM_13;
    _IN2 = GPIO_NUM_4;
    break;
  case 1:
    _IN1 = GPIO_NUM_2;
    _IN2 = GPIO_NUM_0;
    break;
  default:
    break;
  }

  // set digital ports _in1 & in2 to output
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << _IN1) | (1ULL << _IN2);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);

  gpio_set_level( _IN1, 0 );
  gpio_set_level( _IN2, 1 );

  // ledc channels
  _channelIN1 = (ledc_channel_t) (2*_port);
  _channelIN2 = (ledc_channel_t) (2*_port + 1);

  // use Timer 0
  ledc_timer_config_t ledc_timer = {
    .speed_mode       = LEDC_LOW_SPEED_MODE,
    .duty_resolution  = LEDC_TIMER_8_BIT,
    .timer_num        = LEDC_TIMER_0,
    .freq_hz          = 60,  // Set output frequency to 60 Hz
    .clk_cfg          = LEDC_AUTO_CLK
  };
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

  // register 1st channel
  ledc_channel_config_t ledc_channel = {
    .gpio_num       = _IN1,
    .speed_mode     = LEDC_LOW_SPEED_MODE,
    .channel        = _channelIN1,
    .intr_type      = LEDC_INTR_DISABLE,
    .timer_sel      = LEDC_TIMER_0,
    .duty           = 0, // Set duty to 0%
    .hpoint         = 0
  };
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

  // register 2nd Channel
  ledc_channel.gpio_num = _IN2;
  ledc_channel.channel  = _channelIN2;
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

}

void SwOSActor::setMotionType( FtSwarmMotion_t motionType ) {

  _motionType = motionType;

  if (!_ctrl->isLocal()) _setRemote();  
  else                   _setLocal();
  
}

void SwOSActor::setPower( int16_t power ) {

  // XMotor: set COAST or ON automatically
  if ( _actorType == FTSWARM_XMOTOR ) {
    if ( ( _power != 0 ) && ( power == 0 ) ) _motionType = FTSWARM_COAST;
    if ( ( _power == 0 ) && ( power != 0 ) ) _motionType = FTSWARM_ON;
  }
  
  if      (power>255)  _power =  255;
  else if (power<-255) _power = -255;
  else                 _power = power;

  if (!_ctrl->isLocal()) _setRemote();  
  else                   _setLocal();
}

void SwOSActor::_setRemote() {
  
  SwOSCom cmd( _ctrl->mac, _ctrl->serialNumber, _ctrl->getType(), CMD_SETACTORPOWER );
  cmd.data.actorPowerCmd.index = _port;
  cmd.data.actorPowerCmd.motionType = _motionType;
  cmd.data.actorPowerCmd.power = _power;
  cmd.send( );
}

void SwOSActor::_setLocal() {

  uint8_t duty1, duty2;

  // calculate duty
  switch (_motionType) {
  case FTSWARM_COAST:
    // SLEEP HIGH, IN1 LOW, IN2 LOW
    duty1 = 0;
    duty2 = 0;
    break;
  case FTSWARM_BRAKE:
    // SLEEP HIGH, IN1 HIGH, IN2 HIGH
    duty1 = MAXSPEED;
    duty2 = MAXSPEED;
    break;
  case FTSWARM_ON:
    if ( _power <  0) {
      // SLEEP HIGH, IN1 PWM, IN2 HIGH
      duty1 = _power;
      duty2 = 0;
    } else {
      // SLEEP HIGH, IN1 HIGH, IN2 PWM
      duty1 = 0;
      duty2 = _power;
    }
    break;
  default:
    duty1 = 0;
    duty2 = 0;
    }

  // set duty
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, _channelIN1, duty1));
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, _channelIN2, duty2));

  // update duty
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, _channelIN1));
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, _channelIN2));

}

void SwOSActor::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variable("subType", ACTORTYPE[ _actorType ] );
  json->variable("motiontype", getMotionType() );
  json->variable("power",      getPower() );
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

SwOSJoystick::SwOSJoystick(const char *name, uint8_t port,SwOSCtrl *ctrl) : SwOSIO( name, port, ctrl ) {

  // set read values to undefined
  _lastLR = 0;
  _lastFB = 0;

  // initialize local HW
  if (_ctrl->isLocal()) _setupLocal();
}

void SwOSJoystick::_setupLocal() {
  // initialize local HW

  // assign port to GPIO
  _ADCChannelLR = ADC1_CHANNEL_MAX;
  _ADCChannelFB = ADC1_CHANNEL_MAX;

  if ( _ctrl->getCPU() == FTSWARM_1V0 ) {
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
  } else if ( _ctrl->getCPU() == FTSWARM_1V3 ) {
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

void SwOSJoystick::read() {

  // nothing ToDO with remote HW
  if (!_ctrl->isLocal()) return;

  // local HW only
  if (_ADCChannelLR != ADC1_CHANNEL_MAX ) { _lastLR = adc1_get_raw( _ADCChannelLR ); }
  if (_ADCChannelFB != ADC1_CHANNEL_MAX ) { _lastFB = adc1_get_raw( _ADCChannelFB ); }

}

void SwOSJoystick::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variable("valueLR", _lastLR);
  json->variable("valueFB", _lastFB);
  json->variable("button", static_cast<SwOSSwarmControl *>(_ctrl)->button[6+_port]->getState());
  json->endObject();
}

void SwOSJoystick::setValue( int16_t FB, int16_t LR ) {
  _lastLR = LR;
  _lastFB = FB;
}

/***************************************************
 *
 *   SwOSLED
 *
 ***************************************************/

// LED Representation in FastLED library
CRGB leds[MAXPIXEL];
uint8_t usedPixels = 0;
bool ledsInitialized = false;

SwOSLED::SwOSLED(const char *name, uint8_t port, SwOSCtrl *ctrl) : SwOSIO( name, port, ctrl ) {

  if (ctrl->isLocal()) _setupLocal();

}

void SwOSLED::_setupLocal() {

  // leds[] need to be initialized only once.
  if (!ledsInitialized) {

    if ( _ctrl->getCPU() != FTSWARM_1V15) {
      FastLED.addLeds<WS2812, 32, GRB>(leds, MAXPIXEL).setCorrection( TypicalLEDStrip );  
    } else {
      FastLED.addLeds<WS2812, 26, GRB>(leds, MAXPIXEL).setCorrection( TypicalLEDStrip );  
    }
    
    setBrightness( BRIGHTNESSDEFAULT );
    ledsInitialized = true;
  }

  // initialize pixel
  if ( _port < MAXPIXEL ) {
    // leds[_port] = CRGB::Black;
    setColor( CRGB::Black );
  }

}

void SwOSLED::setColor(uint32_t color) {

  // store new color
  _color = color;

  // apply local or remote
  if (_ctrl->isLocal()) _setColorLocal();
  else                  _setRemote();
}

void SwOSLED::_setRemote() {
  
  SwOSCom cmd( _ctrl->mac, _ctrl->serialNumber, _ctrl->getType(), CMD_SETLED );
  cmd.data.ledCmd.index = _port;
  cmd.data.ledCmd.color = _color;
  cmd.data.ledCmd.brightness = _brightness;
  cmd.send( );
}

void SwOSLED::_setColorLocal() {

  // set color
  if (_port < MAXPIXEL ) {
    leds[_port] = _color;
    FastLED.show();
  }

}

void SwOSLED::setBrightness(uint8_t brightness) {

  // store new brightness
  _brightness = brightness;

  // apply local or remote
  if (_ctrl->isLocal()) _setBrightnessLocal();
  else                  _setRemote();

}

void SwOSLED::_setBrightnessLocal() {

  // set brightness
  if (_port < MAXPIXEL ) {
    // TODO: brightness per pixel
    // leds[_port].fadeLightBy( brightness );
    FastLED.setBrightness( _brightness );
    FastLED.show();
  }

}

void SwOSLED::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variable("brightness", _brightness);
  json->variableX("color",     _color);
  json->endObject();
}

/***************************************************
 *
 *   SwOSServo
 *
 ***************************************************/

SwOSServo::SwOSServo(const char *name, SwOSCtrl *ctrl) : SwOSIO( name, ctrl ) {

  // initialize local HW
  if (ctrl->isLocal()) _setupLocal();
}

void SwOSServo::_setupLocal() {
  // initialize local HW

  // assign port to GPIO. All CPU versions use same ports.
  switch ( _ctrl->getCPU() ) {
    case FTSWARM_1V3: _SERVO = GPIO_NUM_33; break;
    default:          _SERVO = GPIO_NUM_25; break;
  }

  // set digital port  to output
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << _SERVO);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);

  // ledc channels
  _channelSERVO = (ledc_channel_t) 4;

  // use Timer 1
    ledc_timer_config_t ledc_timer = {
      .speed_mode       = LEDC_LOW_SPEED_MODE,
      .duty_resolution  = LEDC_TIMER_10_BIT,
      .timer_num        = LEDC_TIMER_1,
      .freq_hz          = 40,  // Set output frequency to 40Hz
      .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // register channel
    ledc_channel_config_t ledc_channel = {
      .gpio_num       = _SERVO,
      .speed_mode     = LEDC_LOW_SPEED_MODE,
      .channel        = _channelSERVO,
      .intr_type      = LEDC_INTR_DISABLE,
      .timer_sel      = LEDC_TIMER_0,
      .duty           = 0, // Set duty to 0%
      .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    // set coast
    _setLocal();

}

void SwOSServo::_setLocal() {

  // calc duty
  float p = _offset + _position;
  if (p <   0 ) p =   0;
  if (p > 255 ) p = 255;

  // maxduty 255 = 3ms = 123 ticks
  float ticks = p/256*123;

  // set duty
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, _channelSERVO, (uint16_t)ticks));

  // update duty
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, _channelSERVO));

}

void SwOSServo::_setRemote() {
  
  SwOSCom cmd( _ctrl->mac, _ctrl->serialNumber, _ctrl->getType(), CMD_SETSERVO );
  cmd.data.servoCmd.index = _port;
  cmd.data.servoCmd.position = _position;
  cmd.data.servoCmd.offset   = _offset;
  cmd.send( );
}

void SwOSServo::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variable("offset",   _offset);
  json->variable("position", _position);
  json->endObject();
}

void SwOSServo::setPosition( int16_t position ) {
  _position = position;

  // apply local or remote
  if (_ctrl->isLocal()) _setLocal();
  else                  _setRemote();

}

void SwOSServo::setOffset( int16_t offset ) {
  _offset = offset;
 
  // apply local or remote
  if (_ctrl->isLocal()) _setLocal();
  else                  _setRemote();

}

/***************************************************
 *
 *   SwOSOLED
 *
 ***************************************************/

SwOSOLED::SwOSOLED(const char *name, SwOSCtrl *ctrl) : SwOSIO( name, ctrl ) {

}


/***************************************************
 *
 *   SwOSGyro
 *
 ***************************************************/

SwOSGyro::SwOSGyro(const char *name, SwOSCtrl *ctrl) : SwOSIO( name, ctrl ) {

}

/***************************************************
 *
 *   SwOSButton
 *
 ***************************************************/

SwOSButton::SwOSButton(const char *name, uint8_t port, SwOSCtrl *ctrl) : SwOSIO( name, port, ctrl ) {

  _toggle = FTSWARM_NOTOGGLE;

}

void SwOSButton::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variable("state", _lastState );
  json->endObject();
}
void SwOSButton::setState( bool state ) {
  
  if ( state != _lastState ) {
    if (state) _toggle = FTSWARM_TOGGLEUP;
    else       _toggle = FTSWARM_TOGGLEDOWN;
  }
  
  _lastState = state; 

  };

bool SwOSButton::getState() { 
  return _lastState;
};

FtSwarmToggle_t SwOSButton::getToggle() {
  FtSwarmToggle_t toggle = _toggle;
  _toggle = FTSWARM_NOTOGGLE;
  return toggle;
}

/***************************************************
 *
 *   SwOSHC165
 *
 ***************************************************/

SwOSHC165::SwOSHC165(const char *name, SwOSCtrl *ctrl) : SwOSIO(name, ctrl) {

  // initialize local HW
  if (ctrl->isLocal()) _setupLocal();

}

void SwOSHC165::_setupLocal() {
  // initialize local HW

  if ( _ctrl->getCPU() != FTSWARM_1V3 ) {
    _CS = _LD = _CLK = _MISO = GPIO_NUM_NC;
    return;
  }

  // Port Mapping
  _CS   = GPIO_NUM_14;
  _LD   = GPIO_NUM_15;
  _CLK  = GPIO_NUM_12;
  _MISO = GPIO_NUM_35;

  // initialize ports
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  io_conf.pin_bit_mask = (1ULL<<_CS) | (1ULL<<_LD) | (1ULL<<_CLK) ;
  gpio_config(&io_conf);

  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pin_bit_mask = 1ULL<<_MISO ;
  gpio_config(&io_conf);

  // set levels
  gpio_set_level( _CS, 1 );
  gpio_set_level( _LD, 1 );
  gpio_set_level( _CLK, 1 );

}

void SwOSHC165::read( ) {

  // remote HW nothing todo
  if (!_ctrl->isLocal()) return;

  // invalid configuration?
  if (_LD == GPIO_NUM_NC ) {
    return;
  }

  // parallel load
  gpio_set_level( _LD, 0 );
  gpio_set_level( _LD, 1 );

  // enable
  gpio_set_level( _CS, 0 );

  // load
  _lastValue = 0;
  for ( uint8_t i=0; i<8; i++ ) {

    // get value
    _lastValue = ( _lastValue << 1 ) | (!gpio_get_level( _MISO ));

    // one tick
    gpio_set_level( _CLK, 0 );
    gpio_set_level( _CLK, 1 );

  }

}

/***************************************************
 *
 *   SwOSCtrl - Base class for controllers.
 *
 ***************************************************/

SwOSCtrl::SwOSCtrl( uint16_t SN, const uint8_t *macAddress, bool local, FtSwarmVersion_t CPU, FtSwarmVersion_t HAT, bool IAmAKelda ):SwOSObj() {

  // copy master data
  _IAmAKelda = IAmAKelda;
  serialNumber = SN;
  if (macAddress) { memcpy( &mac, macAddress, 6 ); }
  _local = local;
  _CPU = CPU;
  _HAT = HAT;

  // define common hardware
  for (uint8_t i=0; i<4; i++) { input[i] = new SwOSInput("A", i, this ); }
  for (uint8_t i=0; i<2; i++) { actor[i] = new SwOSActor("M", i, this ); }
  gyro  = NULL;

}

SwOSCtrl::~SwOSCtrl() {
  
  for (uint8_t i=0; i<4; i++) { if ( input[i] ) delete( input[i] ); }
  for (uint8_t i=0; i<2; i++) { if ( actor[i] ) delete( actor[i] ); }
  if (gyro) delete( gyro );
  
}

bool SwOSCtrl::cmdAlias( const char *obj, const char *alias) {

  char device[30];
  uint8_t pos = 0;
  uint8_t port;

  // scan for first digit
  while ( (!isdigit(obj[pos]) ) && (obj[pos]!='\0') ) pos++;

  // device not found?
  if (( pos==0) || ( pos>30) ) return false;

  // get device
  uint8_t i;
  for (i=0; i<pos;i++) { device[i] = toupper(obj[i]); }
  device[i]='\0';

  // get port
  port = atoi(&obj[pos]);

  // check on trailing digits only
  while ( obj[pos]!='\0') {
    if (!isdigit(obj[pos]) ) return false;
    pos++;
  }

  // now start interpreting
  if (strcmp( "FTSWARM", device) == 0)                                  { setAlias( alias );            return true; }
  else if ( ( strcmp(device, "A") == 0 )     && (port < 4) )            { input[port]->setAlias(alias); return true; }
  else if ( ( strcmp(device, "M") == 0 )     && (port < 2) )            { actor[port]->setAlias(alias); return true; }
  else if ( ( strcmp(device, "GYRO") == 0 )  && (port = 99) && (gyro) ) { gyro->setAlias(alias);        return true; }

  // specific hardware?
  return cmdAlias( device, port, alias );

}

bool SwOSCtrl::cmdAlias( char *device, uint8_t port, const char *alias) {

  // own hardware options are already interpreted
  return false;

}

SwOSIO *SwOSCtrl::getIO( char *name) {

  for (uint8_t i=0;i<4;i++) { if (input[i]->equals(name) ) { return input[i]; } }
  for (uint8_t i=0;i<2;i++) { if (actor[i]->equals(name) ) { return actor[i]; } }
  if ((gyro) && (gyro->equals(name) ) ) { return gyro; }

  return NULL;
}

SwOSIO *SwOSCtrl::getIO( FtSwarmIOType_t ioType, FtSwarmPort_t port) {

  switch (ioType) {
    case FTSWARM_INPUT: return ( (port<4)?input[ port ]:NULL);
    case FTSWARM_ACTOR: return ( (port<2)?actor[ port ]:NULL);
    case FTSWARM_GYRO:  return gyro;
    default: return NULL;   
  }

}

char* SwOSCtrl::myType() {
  return (char *) "UNDEFINED";
}

FtSwarmControler_t SwOSCtrl::getType() {
  return FTSWARM_NOCTRL;
}

void SwOSCtrl::read() {

  for (uint8_t i=0; i<4; i++) { input[i]->read(); }

}

const char *SwOSCtrl::version( FtSwarmVersion_t v) {
  switch (v) {
  case FTSWARM_NOVERSION: return "??";
  case FTSWARM_1V0:       return "1v0";
  case FTSWARM_1V3:       return "1v3";
  case FTSWARM_1V15:      return "1v15";
  default:                return  "??";
  }
}

const char *SwOSCtrl::getVersionHAT() {
  return version(_HAT);
}

const char *SwOSCtrl::getVersionCPU() {
  return version(_CPU);
}

char *SwOSCtrl::getHostname( void ) {

  if (_alias) {
    return _alias ;
  } else {
    return _name;
  }

}

void SwOSCtrl::jsonize( JSONize *json, uint8_t id) {

  json->variable( "name", getHostname());
  json->variable( "id", id);
  json->variable( "serialNumber", serialNumber);

  char hostinfo[250];
  sprintf(hostinfo, "%s S/N: %lu HAT%s CPU%s Kelda: %d", myType(), (unsigned long) serialNumber, getVersionHAT(), getVersionCPU(), _IAmAKelda );
  json->variable( "hostinfo", hostinfo);
  json->variable( "type", strdup( myType() ) );

}

bool SwOSCtrl::apiActorCmd( char *id, int cmd ) {
  // send a actor command (from api)

  // search IO
  for (uint8_t i=0; i<2; i++) {

    if ( actor[i]->equals(id) ) {
      // found
      actor[i]->setMotionType( (FtSwarmMotion_t) cmd );
      return true;
    }
  }

  return false;
}

bool SwOSCtrl::apiActorPower( char *id, int power ) {
  // send a actor command (from api)

  // search IO
  for (uint8_t i=0; i<2; i++) {

    if ( actor[i]->equals(id) ) {
      // found
      actor[i]->setPower( power );
      return true;
    }
  }

  return false;
}

bool SwOSCtrl::apiLED( char *id, int brightness, int color ) {
  // send a LED command (from api)

  // will be defined in SwOSSwarmJST
  return false;

}

bool SwOSCtrl::apiServo( char * id, int offset, int position ) {
  // send a Servo command (from api)

  // will be defined in SwOSSwarmJST
  return false;

}

bool SwOSCtrl::maintenanceMode() {
  return false;
}

void SwOSCtrl::setState( int state ) { 
    // visualizes controler's state

}

bool SwOSCtrl::OnDataRecv(SwOSCom *com ) {

  if (!com) return false;

  switch (com->data.cmd) {
    case CMD_SETSENSORTYPE:
      input[com->data.sensorCmd.index]->setSensorType( com->data.sensorCmd.sensorType, com->data.sensorCmd.normallyOpen );
      return true;
    case CMD_SETACTORPOWER:
      actor[com->data.actorPowerCmd.index]->setValue( com->data.actorPowerCmd.motionType, com->data.actorPowerCmd.power );
      return true;
  }

  return false;

}

SwOSCom *SwOSCtrl::state2Com( void ) {

  uint8_t mac[] = {0,0,0,0,0,0}; // dummy mac

  SwOSCom *com = new SwOSCom( mac, serialNumber, getType(), CMD_STATE );

  int16_t FB, LR;

  for (uint8_t i=0; i<4; i++ ) { com->data.stateCmdControl.input[i].rawValue   = input[i]->getValueUI32(); };
  for (uint8_t i=0; i<2; i++ ) { com->data.stateCmdControl.actor[i].motionType = actor[i]->getMotionType(); com->data.stateCmdControl.actor[i].power = actor[i]->getPower(); };

  return com;

}

void SwOSCtrl::setup( void ) {
  
}


/***************************************************
 *
 *   SwOSSwarmJST
 *
 ***************************************************/

SwOSSwarmJST::SwOSSwarmJST( uint16_t SN, const uint8_t *macAddress, bool local, FtSwarmVersion_t CPU, FtSwarmVersion_t HAT, bool IAmAKelda ):SwOSCtrl( SN, macAddress, local, CPU, HAT, IAmAKelda ) {

  char buffer[32];
  sprintf( buffer, "%s%d", myType(), SN);
  setName( buffer );
  
  // define specific hardware
  for (uint8_t i=0; i<2; i++) { led[i] = new SwOSLED("LED", i, this); }
  servo = new SwOSServo("SERVO", this);

}

SwOSSwarmJST::~SwOSSwarmJST() {
  
  for ( uint8_t i=0; i<2; i++ ) { if ( led[i] ) delete led[i]; }
  if ( servo ) delete servo;

}

bool SwOSSwarmJST::cmdAlias( char *device, uint8_t port, const char *alias) {

  // test on my specific hardware
  if ( ( strcmp(device, "LED") == 0 )   && (port < 2) )        { led[port]->setAlias(alias); return true; }
  else if ( ( strcmp(device, "SERVO") == 0 ) && (port = 99) )  { servo->setAlias(alias);     return true; }
  else return false;

}

SwOSIO *SwOSSwarmJST::getIO( char *name) {

  // check on base class hardware
  SwOSIO *IO = SwOSCtrl::getIO(name);
  if ( IO != NULL ) { return IO; }

  // check on specific hardware
  for (uint8_t i=0;i<2;i++) { if (led[i]->equals(name) ) { return led[i]; } }
  if (servo->equals(name) ) { return servo; }

  return NULL;

}

SwOSIO *SwOSSwarmJST::getIO( FtSwarmIOType_t ioType, FtSwarmPort_t port) {

  // check on base class hardware
  SwOSIO *IO = SwOSCtrl::getIO(ioType, port);
  if ( IO != NULL ) { return IO; }

  switch (ioType) {
    case FTSWARM_LED:   return ( (port<2)?led[port]:NULL);
    case FTSWARM_SERVO: return servo; 
  }

  return NULL;

}

char* SwOSSwarmJST::myType() {
  return (char *) "ftSwarm";
}

FtSwarmControler_t SwOSSwarmJST::getType() {
  return FTSWARM;
}

void SwOSSwarmJST::jsonize( JSONize *json, uint8_t id) {

  json->startObject();
  SwOSCtrl::jsonize(json, id);
  json->startArray( "io" );

  for (uint8_t i=0; i<4; i++) { input[i]->jsonize( json, id ); }
  for (uint8_t i=0; i<2; i++) { actor[i]->jsonize( json, id ); }
  for (uint8_t i=0; i<2; i++) { led[i]->jsonize( json, id ); }
  if (servo) { servo->jsonize( json, id ); }
  if (gyro)  { gyro->jsonize(json, id); }

  json->endArray();
  json->endObject();
  
}

bool SwOSSwarmJST::apiLED( char *id, int brightness, int color ) {
  // send a LED command (from api)

  // search IO
  for (uint8_t i=0; i<2; i++) {

    if ( led[i]->equals(id) ) {
      // found
      led[i]->setBrightness( brightness );
      led[i]->setColor( color );
      return true;
    }
  }

  return false;

}

bool SwOSSwarmJST::apiServo( char *id, int offset, int position ) {
  // send a Servo command (from api)

  // do I have a servo and does the name/alias fit?
  if (!servo) { return false; }
  if (!servo->equals(id)) { return false; }

  // everything is fine, set values
  servo->setOffset( offset );
  servo->setPosition( position );

  return true;

}

void SwOSSwarmJST::setState( int state ) { 
  // visualizes controler's state

  if (led[0]) led[0]->setColor(state);
  if (led[1]) led[1]->setColor(state);
  
}

SwOSCom *SwOSSwarmJST::state2Com(void ) {

  SwOSCom *com = SwOSCtrl::state2Com( ); 
  for (uint8_t i=0; i<2; i++ ) { com->data.stateCmdJST.led[i].brightness   = led[i]->getBrightness();   com->data.stateCmdJST.led[i].color   = led[i]->getColor(); };
  return com;
}

bool SwOSSwarmJST::recvState( SwOSCom *com ) {
  
  for (uint8_t i=0; i<4; i++ ) { input[i]->setValue( com->data.stateCmdJST.input[i].rawValue ); };
  for (uint8_t i=0; i<2; i++ ) { actor[i]->setValue( com->data.stateCmdJST.actor[i].motionType, com->data.stateCmdJST.actor[i].power ); };
  for (uint8_t i=0; i<2; i++ ) { led[i]->setValue( com->data.stateCmdJST.led[i].brightness,     com->data.stateCmdJST.led[i].color ); };
      
  return true;
 
} 

bool SwOSSwarmJST::OnDataRecv(SwOSCom *com ) {

  if (!com) return false;

  // check if SwOSCrtl knows the cmd
  if ( SwOSCtrl::OnDataRecv( com ) ) return true;

  switch ( com->data.cmd ) {
    case CMD_STATE: 
      return recvState( com );
    case CMD_SETLED: 
      led[com->data.ledCmd.index]->setValue( com->data.ledCmd.brightness, com->data.ledCmd.color );
      return true;
    case CMD_SETSERVO:
      if (servo) { 
          servo->setOffset( com->data.servoCmd.offset);
          servo->setPosition( com->data.servoCmd.position );
      }
      return true;
  }

  return false;

}


/***************************************************
 *
 *   SwOSSwarmControl
 *
 ***************************************************/

SwOSSwarmControl::SwOSSwarmControl( uint16_t SN, const uint8_t *macAddress, bool local, FtSwarmVersion_t CPU, FtSwarmVersion_t HAT, bool IAmAKelda ):SwOSCtrl( SN, macAddress, local, CPU, HAT, IAmAKelda ) {

  char buffer[32];
  sprintf( buffer, "%s%d", myType(), SN);
  setName( buffer );

  // define specific hardware
  for (uint8_t i=0; i<4; i++) { button[i]   = new SwOSButton("S", i, this); }
  for (uint8_t i=0; i<2; i++) { button[4+i] = new SwOSButton("F", i, this); }
  for (uint8_t i=0; i<2; i++) { button[6+i] = new SwOSButton("J", i, this); }
  for (uint8_t i=0; i<2; i++) { joystick[i] = new SwOSJoystick("JOY", i, this); }
  hc165 = new SwOSHC165("HC165", this);
  oled  = new SwOSOLED("OLED", this);
}


SwOSSwarmControl::~SwOSSwarmControl() {
  
  for ( uint8_t i=0; i<8; i++ ) { if ( button[i] ) delete button[i]; }
  for ( uint8_t i=0; i<2; i++ ) { if ( joystick[i] ) delete joystick[i]; }
  if ( hc165 ) delete hc165;
  if ( oled  ) delete oled;
  
}

bool SwOSSwarmControl::cmdAlias( char *device, uint8_t port, const char *alias) {

  // just test on specific hardware
  if      ( ( strcmp(device, "S") == 0 )    && (port < 4) )  { button[port]->setAlias(alias);   return true; }
  else if ( ( strcmp(device, "F") == 0 )    && (port < 2) )  { button[port+4]->setAlias(alias); return true; }
  else if ( ( strcmp(device, "J") == 0 )    && (port < 2) )  { button[port+6]->setAlias(alias); return true; }
  else if ( ( strcmp(device, "JOY") == 0 )  && (port < 2) )  { joystick[port]->setAlias(alias); return true; }
  else if ( ( strcmp(device, "OLED") == 0 ) && (port = 99) ) { oled->setAlias(alias);           return true; }
  else return false;

}

SwOSIO *SwOSSwarmControl::getIO( char *name) {

  // check on base base class hardware
  SwOSIO *IO = SwOSCtrl::getIO(name);
  if ( IO != NULL ) { return IO; }

  // check on specific hardware
  for (uint8_t i=0;i<8;i++) { if (button[i]->equals(name) )   { return button[i]; } }
  for (uint8_t i=0;i<2;i++) { if (joystick[i]->equals(name) ) { return joystick[i]; } }
  if (oled->equals(name) ) { return oled; }

  return NULL;

}

SwOSIO *SwOSSwarmControl::getIO( FtSwarmIOType_t ioType, FtSwarmPort_t port) {

  // check on base base class hardware
  SwOSIO *IO = SwOSCtrl::getIO(ioType, port);
  if ( IO != NULL ) { return IO; }
  
  switch (ioType) {
    case FTSWARM_JOYSTICK: return ( (port<2)?joystick[port]:NULL);
    case FTSWARM_BUTTON:   return ( (port<8)?button[port]:NULL);
    case FTSWARM_OLED:     return oled;
  }

  return NULL;
  
}

char* SwOSSwarmControl::myType() {
  return (char *) "ftSwarmControl";
}

FtSwarmControler_t SwOSSwarmControl::getType() {
  return FTSWARMCONTROL;
}

void SwOSSwarmControl::jsonize( JSONize *json, uint8_t id) {

  json->startObject();

  SwOSCtrl::jsonize(json, id);

  json->startArray( "io" );


  for (uint8_t i=0; i<4; i++) { input[i]->jsonize( json, id ); }
  for (uint8_t i=0; i<2; i++) { actor[i]->jsonize( json, id ); }
  for (uint8_t i=0; i<6; i++) { button[i]->jsonize( json, id ); }
  for (uint8_t i=0; i<2; i++) { joystick[i]->jsonize( json, id ); }
  if (gyro)  { gyro->jsonize(json, id); }

  json->endArray();
  json->endObject();
}

void SwOSSwarmControl::read() {

  SwOSCtrl::read();

  // get joystick readings
  for (uint8_t i=0; i<2; i++) {
    joystick[i]->read();
  }

  // get buttons via hc165
  hc165->read();

  // transfer result to buttons
  uint8_t v;
  v = hc165->getValue( );
  for (uint8_t i=0; i<8; i++) { button[i]->setState( v & (1<<i) ); }

}

void SwOSSwarmControl::setState( int state ) { 
    // visualizes controler's state

    // TODO show state on OLED

}

SwOSCom *SwOSSwarmControl::state2Com( void ) {

  SwOSCom *com = SwOSCtrl::state2Com();

  int16_t FB, LR;
  for (uint8_t i=0; i<2; i++ ) { 
    joystick[i]->getValue( &FB, &LR );
    com->data.stateCmdControl.joystick[i].FB = FB;
    com->data.stateCmdControl.joystick[i].LR = LR;
  };
  
  com->data.stateCmdControl.hc165 = hc165->getValue();

  return com;
}

bool SwOSSwarmControl::recvState( SwOSCom *com ) {
  
  for (uint8_t i=0; i<4; i++ ) { input[i]->setValue( com->data.stateCmdControl.input[i].rawValue ); };
  for (uint8_t i=0; i<2; i++ ) { actor[i]->setValue( com->data.stateCmdControl.actor[i].motionType, com->data.stateCmdControl.actor[i].power ); };
  for (uint8_t i=0; i<2; i++ ) { joystick[i]->setValue( com->data.stateCmdControl.joystick[i].FB, com->data.stateCmdControl.joystick[i].LR ); };
      
  // set HC165 value & buttons
  uint8_t hc = com->data.stateCmdControl.hc165;
  hc165->setValue( hc );
  for (uint8_t i=0; i<8; i++) { button[i]->setState( hc && 1<<i ); }

  return true;
 
} 

bool SwOSSwarmControl::OnDataRecv(SwOSCom *com ) {

  if (!com) return false;

  // check if SwOSCrtl knows the cmd
  if ( SwOSCtrl::OnDataRecv( com ) ) return true;

  switch ( com->data.cmd ) {
    case CMD_STATE: return recvState( com );
  }

  return false;

}
