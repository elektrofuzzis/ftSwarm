/*
 * SwOSHW.cpp
 *
 * internal implementation of all ftSwarm HW. Use ftSwarm-classes in ftSwarm.h to access your HW!
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <driver/ledc.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <driver/i2c.h>
#include <driver/pcnt.h>
#include <esp_now.h>
#include <esp_log.h>
#include <freertos/semphr.h>

#include "SwOS.h"
#include "SwOSHW.h"
#include "SwOSCom.h"
#include "ftDuino.h"

const char IOTYPE[FTSWARM_MAXIOTYPE][15] = { "INPUT", "DIGITALINPUT", "ANALOGINPUT", "ACTOR", "BUTTON", "JOYSTICK", "LED", "SERVO", "OLED", "GYRO", "HC165", "I2C", "CAM",  "COUNTER", "FREQUENCYMETER" };

const char EMPTYSTRING[] = "";

const char SENSORICON[FTSWARM_MAXSENSOR][21] = {
  "00_digital.svg",
  "01_analog.svg",
  "02_switch.svg",
  "03_reedswitch.svg",
  "21_lightbarrier.svg",
  "04_voltage.svg",
  "05_resistor.svg",
  "06_ntc.svg",
  "07_ldr.svg",
  "08_trailsensor.svg",
  "09_colorsensor.svg",
  "10_ultrasonic.svg",
  "26_cam.svg",
  "25_counter.svg",
  "27_rotaryEncoder.svg",
  "28_frequency.svg"
};

const char SENSORTYPE[FTSWARM_MAXSENSOR][20] = { "DIGITAL", "ANALOG", "SWITCH", "REEDSWITCH", "LIGHTBARRIER", "VOLTMETER", "OHMMETER", "THERMOMETER", "LDR", "TRAILSENSOR", "COLORSENSOR", "ULTRASONIC", "CAM", "COUNTER", "ROTARY", "FREQUENCYMETER" };

const char ACTORICON[FTSWARM_MAXACTOR][20] = {
  "16_xmotor.svg",
  "20_xmmotor.svg",
  "17_tractor.svg",
  "18_encoder.svg",
  "19_lamp.svg",
  "23_valve.svg",
  "22_compressor.svg",
  "24_buzzer.svg"
};

const char ACTORTYPE[FTSWARM_MAXACTOR][20] = { "MOTOR", "XMMOTOR", "TRACTORMOTOR", "ENCODERMOTOR", "LAMP", "VALVE", "COMPRESSOR", "BUZZER", "STEPPER" };

const uint32_t LEDCOLOR0[MAXSTATE] = { CRGB::Blue, CRGB::Yellow, CRGB::Green, CRGB::Red, CRGB::Cyan, CRGB::Aquamarine };
const uint32_t LEDCOLOR1[MAXSTATE] = { CRGB::Blue, CRGB::Yellow, CRGB::Green, CRGB::Red, CRGB::Cyan, CRGB::Aquamarine };

const char     OLEDMSG[MAXSTATE][20] = { "booting", "connecting wifi", "online", "ERROR - check logs", "waiting on HW", "It's me!" };

#define FTDUINOADDR               0x20
#define FTDUINO_CMD_READ          0x00
#define FTDUINO_CMD_SETSENSORTYPE 0x01
#define FTDUINO_CMD_SETACTORTYPE  0x02

// reference to local ftPwrDrive
ftPwrDrive *pwrDrive = NULL;

// reference to local ftDuino
FtDuino *ftDuino = NULL;

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

void SwOSObj::loadAliasFromNVS( nvs_handle_t my_handle ) {

  size_t size = MAXIDENTIFIER;
  char alias[MAXIDENTIFIER];

  if ( ESP_OK == nvs_get_str( my_handle, getName(), alias, &size ) ) {
    setAlias( alias );
  }
}


void SwOSObj::saveAliasToNVS( nvs_handle_t my_handle ) {

  nvs_set_str( my_handle, getName(), getAlias() );
  
}

void SwOSObj::setAlias( const char *alias ) {

  // free memory?
  if ( _alias != NULL ) { free( (void*) _alias ); }

  // nothing?
  if ( (!alias) || (alias[0]=='\0') ) {
    _alias = NULL;
    return;
  }

  // store
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
  if (!_alias) {
    return (char *) EMPTYSTRING;
  } else {
    return _alias;
  }
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
  json->variable("type", (char *) IOTYPE[getIOType()]);
  json->variable("icon", (char *) getIcon() );
}

void SwOSIO::onTrigger( int32_t value ) {
  ESP_LOGE( LOGFTSWARM, "IO is unable to handle trigger events." );
}

char *SwOSIO::subscribe( char *IOName, uint32_t hysteresis ) {
  
  _hysteresis = hysteresis;
  _isSubscribed = true;

  // only if I don't know my external name, store it
  if (!_subscribedIOName) {
    _subscribedIOName = (char *)malloc( strlen(IOName)+1 );
    strcpy( _subscribedIOName, IOName );
  }
  
  // return my internal name to outside
  return _subscribedIOName;

} 

void SwOSIO::unsubscribe() {
  _isSubscribed = false;
  if (_subscribedIOName) free( _subscribedIOName );
  _subscribedIOName = NULL;
}


/***************************************************
 *
 *   SwOSEventHandler
 *
 ***************************************************/

SwOSEventHandler::SwOSEventHandler( ) {
  _actor        = NULL;
  _parameter    = 0;
  _usePortValue = true;
}

SwOSEventHandler::SwOSEventHandler( SwOSIO *actor, boolean usePortValue, int32_t parameter ) {
  _actor        = actor;
  _usePortValue = usePortValue;
  _parameter    = parameter;
}

void SwOSEventHandler::trigger( int32_t portValue ) {

  if ( _actor ) {
    if (_usePortValue) _actor->onTrigger( portValue );
    else               _actor->onTrigger( _parameter );
  }
}

/***************************************************
 *
 *   SwOSEventHandlers
 *
 ***************************************************/

SwOSEventHandlers::SwOSEventHandlers( ) {

  for (uint8_t i=0; i<FTSWARM_MAXTRIGGER; i++ ) _event[i] = NULL;

};

SwOSEventHandlers::~SwOSEventHandlers() {

  for (uint8_t i=0; i<FTSWARM_MAXTRIGGER; i++ ) {
    if (_event[i]) delete _event[i];
  }
  
}

void SwOSEventHandlers::registerEvent( FtSwarmTrigger_t triggerEvent, SwOSIO *actor, boolean usePortValue, int32_t parameter ) {

  // if there is already a registered event, delete it
  if (_event[triggerEvent]) delete _event[triggerEvent];

  _event[triggerEvent] = new SwOSEventHandler( actor, usePortValue, parameter );
  
};

void SwOSEventHandlers::unregisterEvent( FtSwarmTrigger_t triggerEvent ) {

  // if there is already a registered event, delete it
  if (_event[triggerEvent]) delete _event[triggerEvent];

  _event[triggerEvent] = NULL;
  
};

void SwOSEventHandlers::trigger( FtSwarmTrigger_t triggerEvent, int32_t portValue ) {

  if ( _event[triggerEvent] ) {
    _event[triggerEvent]->trigger( portValue );
  }
}

/***************************************************
 *
 *   SwOSEventInput
 *
 ***************************************************/

SwOSEventInput::~SwOSEventInput() {
  if (!_events) delete _events;
}

void SwOSEventInput::registerEvent( FtSwarmTrigger_t triggerEvent, SwOSIO *actor, boolean usePortValue, int32_t p1 ) {

  if (!_events) _events = new SwOSEventHandlers( );

  _events->registerEvent( triggerEvent, actor, usePortValue, p1 );
  
}

void SwOSEventInput::unregisterEvent( FtSwarmTrigger_t triggerEvent ) {

  if (!_events) return;

  _events->unregisterEvent( triggerEvent );
  
}

void SwOSEventInput::trigger( FtSwarmTrigger_t triggerEvent, int32_t portValue ) {

  if ( _events ) _events->trigger( triggerEvent, portValue );

}

/***************************************************
 *
 *   SwOSInput
 *
 ***************************************************/

// handle different plattforms
#if CONFIG_IDF_TARGET_ESP32S3
  #define xGPIO_NUM_22    GPIO_NUM_NC
  #define xGPIO_NUM_25    GPIO_NUM_NC
  #define xADC1_CHANNEL_8 ADC1_CHANNEL_8
  #define xGPIO_NUM_41    GPIO_NUM_41
  #define xGPIO_NUM_42    GPIO_NUM_42
  #define xGPIO_NUM_45    GPIO_NUM_45
  #define xGPIO_NUM_46    GPIO_NUM_46
  #define xGPIO_NUM_47    GPIO_NUM_47
  #define xGPIO_NUM_48    GPIO_NUM_48
#else
  #define xGPIO_NUM_22    GPIO_NUM_22
  #define xGPIO_NUM_25    GPIO_NUM_25
  #define xADC1_CHANNEL_8 ADC1_CHANNEL_MAX
  #define xGPIO_NUM_41    GPIO_NUM_NC
  #define xGPIO_NUM_42    GPIO_NUM_NC
  #define xGPIO_NUM_45    GPIO_NUM_NC
  #define xGPIO_NUM_46    GPIO_NUM_NC
  #define xGPIO_NUM_47    GPIO_NUM_NC
  #define xGPIO_NUM_48    GPIO_NUM_NC
  #define ADC1_CHANNEL_8  ADC1_CHANNEL_MAX
  #define ADC1_CHANNEL_9  ADC1_CHANNEL_MAX
#endif

const int8_t GPIO_INPUT[9][8][3] = 
  { /* FTSWARMJST_1V0 */       { { GPIO_NUM_33, ADC_UNIT_1, ADC1_CHANNEL_5},   { xGPIO_NUM_25, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_26, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_27, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX} },
    /* FTSWARMCONTROL_1V3 */   { { GPIO_NUM_39, ADC_UNIT_1, ADC1_CHANNEL_3},   { xGPIO_NUM_25, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_26, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_27, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX} },
    /* FTSWARMJST_1V15 */      { { GPIO_NUM_39, ADC_UNIT_1, ADC1_CHANNEL_3},   { GPIO_NUM_32,  ADC_UNIT_1, ADC1_CHANNEL_4},   { GPIO_NUM_33, ADC_UNIT_1, ADC1_CHANNEL_5},   { GPIO_NUM_34, ADC_UNIT_1, ADC1_CHANNEL_6},   { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX} },
    /* FTSWARMRS_2V0 */        { { GPIO_NUM_1,  ADC_UNIT_1, ADC1_CHANNEL_0},   { GPIO_NUM_2,   ADC_UNIT_1, ADC1_CHANNEL_1},   { GPIO_NUM_8,  ADC_UNIT_1, ADC1_CHANNEL_7},   { GPIO_NUM_9,  ADC_UNIT_1, ADC1_CHANNEL_8},   { GPIO_NUM_11, ADC_UNIT_2, ADC2_CHANNEL_0},   { GPIO_NUM_13, ADC_UNIT_2, ADC2_CHANNEL_2},   { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX} },
    /* FTSWARMRS_2V1 */        { { GPIO_NUM_1,  ADC_UNIT_1, ADC1_CHANNEL_0},   { GPIO_NUM_2,   ADC_UNIT_1, ADC1_CHANNEL_1},   { GPIO_NUM_19, ADC_UNIT_2, ADC1_CHANNEL_8},   { GPIO_NUM_20, ADC_UNIT_2, ADC2_CHANNEL_9},   { GPIO_NUM_11, ADC_UNIT_2, ADC2_CHANNEL_0},   { GPIO_NUM_13, ADC_UNIT_2, ADC2_CHANNEL_2},   { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX} },
    /* FTSWARMCAM_2V11 */      { { GPIO_NUM_33, ADC_UNIT_1, ADC1_CHANNEL_5},   { GPIO_NUM_12,  ADC_UNIT_1, ADC1_CHANNEL_1},   { GPIO_NUM_13, ADC_UNIT_2, ADC1_CHANNEL_8},   { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX} },
    /* FTSWARMPWRDRIVE_1V14 */ { { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX} },
    /* FTSWARMDUINO_1V14 */    { { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX}, { GPIO_NUM_NC, ADC_UNIT_1, ADC1_CHANNEL_MAX} },
    /* FTSWARMXL_1V00 */       { { GPIO_NUM_1,  ADC_UNIT_1, ADC1_CHANNEL_0},   { GPIO_NUM_2,   ADC_UNIT_1, ADC1_CHANNEL_1},   { GPIO_NUM_3, ADC_UNIT_1, ADC1_CHANNEL_2},    { GPIO_NUM_4,  ADC_UNIT_1, ADC1_CHANNEL_3},   { GPIO_NUM_5, ADC_UNIT_1, ADC1_CHANNEL_4},    { GPIO_NUM_8, ADC_UNIT_1, ADC1_CHANNEL_7},    { GPIO_NUM_10, ADC_UNIT_1, ADC1_CHANNEL_9},   { GPIO_NUM_9, ADC_UNIT_1, ADC1_CHANNEL_8}    }
  };   

SwOSInput::SwOSInput(const char *name, uint8_t port, SwOSCtrl *ctrl, FtSwarmSensor_t sensorType ) : SwOSIO( name, port, ctrl ), SwOSEventInput( ) {
  
  // initialize some vars to undefined
  _sensorType   = sensorType;

}


void SwOSInput::_setupLocal() {
  // initialize local HW

  _GPIO = (gpio_num_t) GPIO_INPUT[_ctrl->getCPU()][_port][0];

  gpio_config_t io_conf = {};

  if ( _GPIO != GPIO_NUM_NC) {

    // initialize digital  port
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pin_bit_mask = 1ULL << _GPIO;
    gpio_config(&io_conf);

  }
  
}

char * SwOSInput::getIcon() { 
  return (char *) SENSORICON[ _sensorType ]; 
}; 

void SwOSInput::setSensorTypeLocal( FtSwarmSensor_t sensorType ) {

}

void SwOSInput::setSensorType( FtSwarmSensor_t sensorType ) {

  _sensorType   = sensorType;

  if ( _ctrl->isLocal() ) {

    setSensorTypeLocal( sensorType ); 

  } else {

    // send SN, SETSENSORTYPE, _port, sensorType
    SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_SETSENSORTYPE );
    cmd.data.sensorCmd.index        = _port;
    cmd.data.sensorCmd.sensorType   = _sensorType;
    cmd.send( );

  }

}

void SwOSInput::subscription() {

  // test, if input is subscribed
  if (!_isSubscribed) return;

  if ( ( (_lastRawValue > _lastsubscribedValue) && (_lastRawValue-_lastsubscribedValue) > _hysteresis ) ||
       ( (_lastsubscribedValue > _lastRawValue ) && (_lastsubscribedValue-_lastRawValue) > _hysteresis ) ) {
       printf("S: %s %d\n", _subscribedIOName, _lastRawValue);
       _lastsubscribedValue = _lastRawValue;
  }
}


int32_t SwOSInput::getValueI32() {
  return _lastRawValue;
}

float SwOSInput::getValueF() {
  return (float)_lastRawValue;
}


/***************************************************
 *
 *   SwOSDigitalInput
 *
 ***************************************************/

SwOSDigitalInput::SwOSDigitalInput(const char *name, uint8_t port, SwOSCtrl *ctrl ) : SwOSInput( name, port, ctrl, FTSWARM_DIGITAL ) {
  
  // initialize local HW
  if ( _ctrl->isLocal() ) _setupLocal();

}

void SwOSDigitalInput::_setupLocal() {
  // initialize local HW

  SwOSInput::_setupLocal( );

  // local init
  _PUA2         = GPIO_NUM_NC;
  _USTX         = GPIO_NUM_NC;
 
  switch (_ctrl->getCPU() ) {
    case FTSWARMRS_2V1:  if ( _port == 0 ) _USTX = xGPIO_NUM_42;
                       if ( ( _ctrl->getType() == FTSWARM ) && ( _port == 2 ) ) { _PUA2 = xGPIO_NUM_41; }                       
                       break;
    
    case FTSWARMRS_2V0:  if ( _port == 0 ) _USTX = xGPIO_NUM_42;
                       if ( ( _ctrl->getType() == FTSWARM ) && ( _port == 2 ) ) { _PUA2 = xGPIO_NUM_41; }                       
                       break;
    
    case FTSWARMJST_1V15: if ( _port == 0 ) _USTX = GPIO_NUM_15;
                       if ( ( _ctrl->getType() == FTSWARM ) && ( _port == 2 ) ) { _PUA2 = GPIO_NUM_14; }
                       break;
    
    case FTSWARMCONTROL_1V3:  if ( _port == 0 ) _USTX = GPIO_NUM_15;
                       if ( ( _ctrl->getType() == FTSWARM ) && ( _port == 2 ) ) { _PUA2 = GPIO_NUM_14; }
                       break;

    default:           /* eg. PwrDrive, XL, Duino */
                       break;
  }

  gpio_config_t io_conf = {};
  
  // initialize A2 pullup
  if (_PUA2 != GPIO_NUM_NC) {
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pin_bit_mask = 1ULL << _PUA2;
    gpio_config(&io_conf);
    gpio_set_level( (gpio_num_t) _PUA2, 0 );
  }

  // initialize A1 pulldown
  if (_USTX != GPIO_NUM_NC) {
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << _USTX;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    gpio_set_level( (gpio_num_t) _USTX, 0 );
  }

}

void SwOSDigitalInput::setSensorType( FtSwarmSensor_t sensorType, bool normallyOpen ) {

  // due to send norallyOpen to remote controllers, don't call super class

  _sensorType   = sensorType;
  _normallyOpen = normallyOpen;

  if (_ctrl->isLocal()) { 
    
    setSensorTypeLocal( sensorType );

  } else {

    // send SN, SETSENSORTYPE, _port, sensorType
    SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_SETSENSORTYPE );
    cmd.data.sensorCmd.index        = _port;
    cmd.data.sensorCmd.sensorType   = _sensorType;
    cmd.data.sensorCmd.normallyOpen = _normallyOpen;
    cmd.send( );

  }

}

void SwOSDigitalInput::setSensorTypeLocal( FtSwarmSensor_t sensorType ) {

  // set A1 pullup if available
  if ( ( _PUA2 != GPIO_NUM_NC ) && ( _sensorType != FTSWARM_ULTRASONIC ) ) {
    gpio_set_level( (gpio_num_t) _PUA2, true );
  }

  // ftDuino
  if ( ( _ctrl->getCPU() == FTSWARMDUINO_1V141 ) && (ftDuino) ) ftDuino->setSensorType( _port, sensorType );

}

void SwOSDigitalInput::read() {
  
  // nothing todo on remote sensors
  if (!_ctrl->isLocal()) return;

  // i2c sensor is read via a block control by <controller>.read
  if (_ctrl->isI2CSwarmCtrl()) return; 

  // existing port?
  if (_GPIO == GPIO_NUM_NC ) return;

  uint32_t newValue;

  // read new data
  newValue = gpio_get_level( (gpio_num_t) _GPIO );

  setReading( newValue );

}

void SwOSDigitalInput::setReading( int32_t newValue ) {
    
  // normally open: change logic
  if (_normallyOpen) newValue = 1-newValue;
    
  // check if it's toggled?
  if ( _lastRawValue != newValue ) { 
    if (newValue) { _toggle = FTSWARM_TOGGLEUP;   trigger( FTSWARM_TRIGGERUP, newValue ); }
    else          { _toggle = FTSWARM_TOGGLEDOWN; trigger( FTSWARM_TRIGGERDOWN, newValue ); }
  }

  // send changed value event?
  if ( (_events) && ( _lastRawValue != newValue ) ) trigger( FTSWARM_TRIGGERVALUE, newValue );

  // store new data
  _lastRawValue = newValue;  

  subscription();

}

void SwOSDigitalInput::setValue( int32_t value ) {

  // nothing ToDo on real local HW
  if ( ( _ctrl->isLocal()) && (!_ctrl->isI2CSwarmCtrl() ) ) return;

  // check if it's toggled?
  if ( _lastRawValue != value) { 

    if   (value) { _toggle = FTSWARM_TOGGLEUP;   trigger( FTSWARM_TRIGGERUP, value ); }
    else         { _toggle = FTSWARM_TOGGLEDOWN; trigger( FTSWARM_TRIGGERDOWN, value ); }

    // trigger value event
    trigger( FTSWARM_TRIGGERVALUE, value );
    
  }
  
  _lastRawValue = value;

  subscription();

}

FtSwarmToggle_t SwOSDigitalInput::getToggle() {
  // check toggle state and reset it
  FtSwarmToggle_t toggle = _toggle;
  _toggle = FTSWARM_NOTOGGLE;
  return toggle;
}

void SwOSDigitalInput::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variableUI32("sensorType", _sensorType);
  json->variable("subType",    (char *) SENSORTYPE[_sensorType]);

  json->variableI32("value", getValueI32() );
  
  json->endObject();
}

/***************************************************
 *
 *   SwOSCounter
 *
 ***************************************************/

SwOSCounter::SwOSCounter(const char *name, uint8_t port1, uint8_t port2, SwOSCtrl *ctrl ) : SwOSInput( name, port1, ctrl, FTSWARM_COUNTER ) {

  _portControl = port2;
  
  // initialize local HW
  if ( _ctrl->isLocal() ) _setupLocal();

}

void SwOSCounter::_setupLocal() {
  // initialize local HW

  // setup _GPIO / Counter Input
  SwOSInput::_setupLocal( );

  // setup _CONTROL Input if needed
  if ( _portControl != 255 ) { 
    
    _CONTROL = (gpio_num_t) GPIO_INPUT[_ctrl->getCPU()][_portControl][0];

    gpio_config_t io_conf = {};

    if ( _CONTROL != GPIO_NUM_NC) {

      // initialize digital  port
      io_conf.intr_type = GPIO_INTR_DISABLE;
      io_conf.mode = GPIO_MODE_INPUT;
      io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
      io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
      io_conf.pin_bit_mask = 1ULL << _CONTROL;
      gpio_config(&io_conf);

    }

  }

  // Calculate used unit
  _unit = pcnt_unit_t( _port );

  // configure Channel 0
  pcnt_config_t pcnt_config = {
    // Set PCNT input signal and control GPIOs
    .pulse_gpio_num = _GPIO,
    .ctrl_gpio_num  = _CONTROL,
    // What to do when control input is low or high?
    .lctrl_mode = PCNT_MODE_KEEP, // Reverse counting direction if low
    .hctrl_mode = PCNT_MODE_KEEP,    // Keep the primary counter mode if high
    // What to do on the positive / negative edge of pulse input?
    .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge
    .neg_mode = PCNT_COUNT_DIS,   // Keep the counter value on the negative edge
    .unit = _unit,
    .channel = PCNT_CHANNEL_0,
    };
  
  // rotary encoder?
  if ( _CONTROL != GPIO_NUM_NC) {

    // 1st Channel
    pcnt_config.lctrl_mode = PCNT_MODE_REVERSE, // Reverse counting direction if low
    pcnt_config.pos_mode = PCNT_COUNT_DEC;
    pcnt_config.neg_mode = PCNT_COUNT_INC;
    pcnt_unit_config(&pcnt_config);

    // 2nd Channel
    pcnt_config.pulse_gpio_num = _CONTROL;
    pcnt_config.ctrl_gpio_num = _GPIO;
    pcnt_config.channel = PCNT_CHANNEL_1;
    pcnt_config.pos_mode = PCNT_COUNT_INC;
    pcnt_config.neg_mode = PCNT_COUNT_DEC;
    pcnt_unit_config(&pcnt_config);

  } else {
    // simple counter
    pcnt_unit_config(&pcnt_config);
  }
  
  /* Configure and enable the input filter */
  pcnt_set_filter_value(_unit, 100);
  pcnt_filter_enable(_unit);

  /* Initialize PCNT's counter */
  pcnt_counter_pause(_unit);
  pcnt_counter_clear(_unit);

  /* Everything is set up, now go to counting */
  pcnt_counter_resume(_unit);

}

void SwOSCounter::read( void ) {

  // nothing todo on remote sensors
  if (!_ctrl->isLocal()) return;

  // i2c sensor is read via a block control by <controller>.read
  if (_ctrl->isI2CSwarmCtrl()) return; 

  // just in case hw isn't initialized
  if ( _unit == PCNT_UNIT_MAX ) return;

  int16_t newValue;
  pcnt_get_counter_value(_unit, &newValue);

  // store new data
  _lastRawValue = newValue;  

  subscription();

}

void SwOSCounter::resetCounter( void ) {

  if ( _ctrl->isLocal() ) {

    pcnt_counter_clear( _unit );

  } else {

    SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_RESETCOUNTER );
    cmd.data.CounterCmd.index = _port;
    cmd.send( );

  }

  _lastRawValue = 0;

  subscription();

}

void SwOSCounter::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variableUI32("sensorType", _sensorType);
  json->variable("subType", (char *) SENSORTYPE[_sensorType]);
  json->variableI32("value", getValueI32() );
  json->endObject();
}

/***************************************************
 *
 *   SwOSFrequencymeter
 *
 ***************************************************/

static void IRAM_ATTR freq_isr_handler(void* arg) {

  QueueHandle_t freqQueue = (QueueHandle_t) arg;
  int64_t t = esp_timer_get_time();
  xQueueSendFromISR(freqQueue, &t, NULL);

}

SwOSFrequencymeter::SwOSFrequencymeter(const char *name, uint8_t port1, uint8_t port2, SwOSCtrl *ctrl ) : SwOSInput( name, port1, ctrl, FTSWARM_FREQUENCYMETER ) {

  _portControl = port2;
  
  // initialize local HW
  if ( _ctrl->isLocal() ) _setupLocal();

}

SwOSFrequencymeter::~SwOSFrequencymeter( ) {
  
  if ( _freqQueue ) {
    gpio_isr_handler_remove( _GPIO );
    vQueueDelete( _freqQueue );
    _freqQueue = NULL;
  }

}

void SwOSFrequencymeter::_setupLocal() {
  // initialize local HW

  // setup _GPIO / Counter Input
  SwOSInput::_setupLocal( );

  // local variables
  _freqQueue = xQueueCreate(10, sizeof(int64_t));
  _lastTick = esp_timer_get_time();

  // setup interupt handling
  gpio_set_intr_type( _GPIO, GPIO_INTR_POSEDGE );
  gpio_install_isr_service( 0 );
  gpio_isr_handler_add( _GPIO, freq_isr_handler, (void*) _freqQueue );

}

void SwOSFrequencymeter::read( void ) {

  // nothing todo on remote sensors
  if (!_ctrl->isLocal()) return;

  // i2c sensor is read via a block control by <controller>.read
  if (_ctrl->isI2CSwarmCtrl()) return; 

  // not initialized?
  if ( !_freqQueue ) return;

  int64_t tick, dt, dt1;
  bool hasEvents = false;
  
  // now read the latest data
  while ( xQueueReceive( _freqQueue, &tick, 0 ) == pdTRUE ) {
    dt1 = tick - _lastTick;
    if ( dt1 > 1000LL )  {
      dt = dt1;
      _lastTick = tick;
      hasEvents = true;
    }
  }

  if ( (hasEvents) && (dt != 0LL) ) {
    // I got some data out of the queue, so I calc the frequency
    float newValueF = 1/( ((float)dt) / 1000000.0 );
    _lastRawValue = newValueF; 
  
  } else if ( esp_timer_get_time() - _lastTick > 1000000LL ) {
    // no tick for more than a second
    _lastRawValue = 0;
  }

  subscription();

}

void SwOSFrequencymeter::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variableUI32("sensorType", _sensorType);
  json->variable("subType", (char *) SENSORTYPE[_sensorType]);
  json->variableI32("value", getValueI32() );
  json->endObject();
}

/***************************************************
 *
 *   SwOSAnalogInput
 *
 ***************************************************/

SwOSAnalogInput::SwOSAnalogInput(const char *name, uint8_t port, SwOSCtrl *ctrl ) : SwOSInput( name, port, ctrl, FTSWARM_DIGITAL ) {
  
  // initialize local HW
  if ( _ctrl->isLocal() ) _setupLocal();

}

void SwOSAnalogInput::_setupLocal() {
  // initialize local HW

  SwOSInput::_setupLocal( );

  // local init
  _ADCUnit      = GPIO_INPUT[(int8_t)_ctrl->getCPU()][(int8_t) _port][1];
  _ADCChannel   = GPIO_INPUT[(int8_t)_ctrl->getCPU()][(int8_t) _port][2];

  if ( ( _ADCUnit ==  ADC_UNIT_1) && ( _ADCChannel != ADC1_CHANNEL_MAX ) ) {
    // set ADC to 12 bits, scale 3.9V
    adc1_config_width(ADC_WIDTH_BIT_12);
  }

}

bool SwOSAnalogInput::isXMeter() {

  return ( ( _sensorType == FTSWARM_VOLTMETER ) ||
           ( _sensorType == FTSWARM_OHMMETER ) ||
           ( _sensorType == FTSWARM_THERMOMETER ) );

}

void SwOSAnalogInput::_setSensorTypeLocal( FtSwarmSensor_t sensorType ) {

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

  uint32_t newValue;

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
  json->variable("subType",    (char *) SENSORTYPE[_sensorType]);

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
 *   SwOSActor
 *
 ***************************************************/

SwOSActor::SwOSActor(const char *name, uint8_t port, SwOSCtrl *ctrl):SwOSIO(name, port, ctrl ){

  // ftPwrDrive has a bitmap motor representation, so precalc the Mx values
  _pwrDriveMotor = 1 << _port;

  // initialize local HW
  if (_ctrl->isLocal()) {
    if ( _ctrl->isI2CSwarmCtrl() ) {
      _setupI2C();
    } else {
      _setupLocal();
    }
  }

}

SwOSActor::~SwOSActor() {
  if (_ctrl->isLocal() ) setSpeed(0);
  if ( ledc_channel ) free( ledc_channel );
}

char * SwOSActor::getIcon() { 
  return (char *) ACTORICON[ _actorType ]; 
}; 

void SwOSActor::_setupI2C() {
  
}

const gpio_num_t GPIO_ACTOR[9][8][2] = 
  { /* FTSWARMJST_1V0 */      { { GPIO_NUM_13,  GPIO_NUM_4  }, { GPIO_NUM_2,   GPIO_NUM_0},   { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC} },
    /* FTSWARMCONTROL_1V3 */  { { GPIO_NUM_13,  GPIO_NUM_4  }, { GPIO_NUM_2,   GPIO_NUM_0},   { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC} },
    /* FTSWARMJST_1V15 */     { { GPIO_NUM_13,  GPIO_NUM_4  }, { GPIO_NUM_2,   GPIO_NUM_0},   { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC} },
    /* FTSWARMRS_2V0 */       { { GPIO_NUM_14,  GPIO_NUM_21 }, { xGPIO_NUM_45, xGPIO_NUM_46}, { GPIO_NUM_NC, GPIO_NUM_8},  { GPIO_NUM_NC, GPIO_NUM_9},  { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC} },
    /* FTSWARMRS_2V1 */       { { GPIO_NUM_14,  GPIO_NUM_21 }, { xGPIO_NUM_45, xGPIO_NUM_46}, { GPIO_NUM_NC, GPIO_NUM_8},  { GPIO_NUM_NC, GPIO_NUM_9},  { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC} },
    /* FTSWARMCAM_2V11 */     { { GPIO_NUM_14,  GPIO_NUM_15 }, { GPIO_NUM_2,   GPIO_NUM_4},   { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC} },
    /* FTSWARMPWRDRIVE_1V14*/ { { GPIO_NUM_NC,  GPIO_NUM_NC},  { GPIO_NUM_NC,  GPIO_NUM_NC},  { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC} },
    /* FTSWARMDUINO_1V14*/    { { GPIO_NUM_NC,  GPIO_NUM_NC},  { GPIO_NUM_NC,  GPIO_NUM_NC},  { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC} },
    /* FTSWARMXL_1V00*/       { { GPIO_NUM_16,  GPIO_NUM_15},  { GPIO_NUM_14,  GPIO_NUM_13},  { GPIO_NUM_11, GPIO_NUM_12}, { GPIO_NUM_42, GPIO_NUM_41}, { GPIO_NUM_40, GPIO_NUM_39}, { GPIO_NUM_38, GPIO_NUM_37}, { GPIO_NUM_19, GPIO_NUM_20}, { GPIO_NUM_35, GPIO_NUM_36} }
  };   

void SwOSActor::_setupLocal() {
  // initialize local HW

  // set HW Pins
  _IN1 = GPIO_ACTOR[_ctrl->getCPU()][_port][0];
  _IN2 = GPIO_ACTOR[_ctrl->getCPU()][_port][1];
  
  // set digital ports _in1 & in2 to output
  gpio_config_t io_conf = {
    .pin_bit_mask = 0,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };
  if ( _IN1 != GPIO_NUM_NC ) io_conf.pin_bit_mask = io_conf.pin_bit_mask | (1ULL << _IN1);
  if ( _IN2 != GPIO_NUM_NC ) io_conf.pin_bit_mask = io_conf.pin_bit_mask | (1ULL << _IN2);
  gpio_config(&io_conf);
  
  // set motor driver off
  if ( _IN1 != GPIO_NUM_NC ) gpio_set_level( _IN1, 0 );
  if ( _IN2 != GPIO_NUM_NC ) gpio_set_level( _IN2, 0 );

  // use Timer 0
  ledc_timer_config_t ledc_timer = {
    .speed_mode       = LEDC_LOW_SPEED_MODE,
    .duty_resolution  = LEDC_TIMER_12_BIT,
    .timer_num        = LEDC_TIMER_0,
    .freq_hz          = 600,  // Set output frequency to 60 Hz
    .clk_cfg          = LEDC_AUTO_CLK,
  };
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

  // just prepare led channel, don't register yet
  ledc_channel = (ledc_channel_config_t *) calloc( sizeof( ledc_channel_config_t ), 1 );
  ledc_channel->gpio_num       = GPIO_NUM_NC;
  ledc_channel->speed_mode     = LEDC_LOW_SPEED_MODE;
  ledc_channel->channel        = (ledc_channel_t) (_port);
  ledc_channel->intr_type      = LEDC_INTR_DISABLE;
  ledc_channel->timer_sel      = LEDC_TIMER_0;
  ledc_channel->duty           = 0; 
  ledc_channel->hpoint         = 0;
  ledc_channel->flags.output_invert = 1;

}

void SwOSActor::setMotionType( FtSwarmMotion_t motionType ) {

  _motionType = motionType;

  if   (!_ctrl->isLocal())           _setRemote();  
  else if ( _ctrl->isI2CSwarmCtrl()) _setLocalI2C();
  else                               _setLocalLHW();
  
}

void SwOSActor::setActorType( FtSwarmActor_t actorType, bool highResolution, bool dontSendToRemote ) { 

  _actorType = actorType;
  _highResolution = highResolution;

  if (_ctrl->isLocal()) { 
    
  } else if (!dontSendToRemote) {    
    // send SN, SETSENSORTYPE, _port, sensorType
    SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_SETACTORTYPE );
    cmd.data.actorTypeCmd.index        = _port;
    cmd.data.actorTypeCmd.actorType    = _actorType;
    cmd.data.actorTypeCmd.highResolution = highResolution;
    cmd.send( );

  }
  
};   

void SwOSActor::setAcceleration( uint32_t rampUpT,  uint32_t rampUpY ) {

  _rampUpT = rampUpT;
  _rampUpY = rampUpY;

  if (!_ctrl->isLocal()) _setRemote();

}

void SwOSActor::getAcceleration( uint32_t *rampUpT,  uint32_t *rampUpY ) {
  *rampUpY = _rampUpY;
  *rampUpT = _rampUpT;
}

void SwOSActor::setSpeed( int16_t speed ) {

  // if no change is needed, return
  if ( speed == _speed ) return;

  // Motor, OnOff-Actors: set COAST or ON automatically
  if ( ( _actorType != FTSWARM_XMMOTOR ) && ( _actorType != FTSWARM_TRACTOR ) && ( _actorType != FTSWARM_ENCODER ) ) {
    if ( ( _speed != 0 ) && ( speed == 0 ) ) _motionType = FTSWARM_COAST;
    if ( ( _speed == 0 ) && ( speed != 0 ) ) _motionType = FTSWARM_ON;
  }
  
  // limit speed values
  int16_t maxSpeed = MAXSPEED256;
  if (_highResolution) maxSpeed = MAXSPEED4096;
  if      (speed> maxSpeed) _speed =  maxSpeed;
  else if (speed<-maxSpeed) _speed = -maxSpeed;
  else                      _speed =  speed;

  // set speed values
  if   (!_ctrl->isLocal())           _setRemote();  
  else if ( _ctrl->isI2CSwarmCtrl()) _setLocalI2C();
  else                               _setLocalLHW();
}

void SwOSActor::_setRemote() {
  
  SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_SETACTORSPEED  );
  cmd.data.actorSpeedCmd.index      = _port;
  cmd.data.actorSpeedCmd.motionType = _motionType;
  cmd.data.actorSpeedCmd.speed      = _speed;
  cmd.data.actorSpeedCmd.rampUpT    = _rampUpT;
  cmd.data.actorSpeedCmd.rampUpY    = _rampUpY;
  cmd.send( );

}

void SwOSActor::_setLocalI2C() {

  if ( ( _ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && ( pwrDrive ) ) {
    pwrDrive->setMaxSpeed( _pwrDriveMotor, _speed );
  }

  if ( ( _ctrl->getCPU() == FTSWARMDUINO_1V141 ) && ( ftDuino ) ) {
    ftDuino->setMotor( _port, _motionType, _speed);
  }

}

void SwOSActor::setPWM( int16_t in1, int16_t in2, gpio_num_t pwm, uint32_t duty ) {

  // calc duty based on _highResolution
  uint32_t duty1 = duty;
  if ( (!_highResolution) && (duty) ) duty1 = ( duty1 << 4 ) + 0xF;

  // 1st step, check if the old pwm pin is different to the new one
  if ( ledc_channel->gpio_num != pwm ) {
  
    // reconfigure old pin
    if ( ledc_channel->gpio_num != GPIO_NUM_NC ) ESP_ERROR_CHECK( gpio_reset_pin( (gpio_num_t) ledc_channel->gpio_num ) );
  
    // set ledc_channel to new pin
    ledc_channel->gpio_num = pwm;
    if ( ledc_channel->gpio_num != GPIO_NUM_NC ) ESP_ERROR_CHECK( ledc_channel_config( ledc_channel ) );
  
  }

  // 2rd step: set pwm
  if ( ledc_channel->gpio_num != GPIO_NUM_NC ) {

    if ( _rampUpT | _rampUpY ) {

      uint32_t oldDuty = ledc_get_duty( LEDC_LOW_SPEED_MODE, ledc_channel->channel );
      ESP_ERROR_CHECK( ledc_set_fade( LEDC_LOW_SPEED_MODE, ledc_channel->channel, oldDuty, ( oldDuty > duty1 ) ? LEDC_DUTY_DIR_DECREASE : LEDC_DUTY_DIR_INCREASE, _rampUpT, _rampUpY, duty1 ) );
      ESP_ERROR_CHECK( ledc_update_duty( LEDC_LOW_SPEED_MODE, ledc_channel->channel ) );
    
    } else {
      ESP_ERROR_CHECK( ledc_set_duty( LEDC_LOW_SPEED_MODE, ledc_channel->channel, duty1 ) );
      ESP_ERROR_CHECK( ledc_update_duty( LEDC_LOW_SPEED_MODE, ledc_channel->channel ) );
    }

  }

  // 3rd step: set static levels if applicable
  if ( _IN1 != GPIO_NUM_NC ) gpio_set_level( _IN1, in1 );
  if ( _IN2 != GPIO_NUM_NC ) gpio_set_level( _IN2, in2 );

}

void SwOSActor::_setLocalLHW() {

  // just in case of non-existent HW
  if ( ( _IN1 == GPIO_NUM_NC ) || ( _IN2 == GPIO_NUM_NC ) ) return;

  // calculate duty
  switch (_motionType) {
    case FTSWARM_COAST: // SLEEP HIGH, IN1 LOW, IN2 LOW
                        setPWM( 0, 0, GPIO_NUM_NC, 0 );
                        break;
  
    case FTSWARM_BRAKE: // SLEEP HIGH, IN1 HIGH, IN2 HIGH
                        setPWM( 1, 1, GPIO_NUM_NC, 0 );
                        break;

    case FTSWARM_ON:    if ( _speed <  0) {
                          // SLEEP HIGH, IN1 PWM, IN2 HIGH
                          setPWM( 0, 1, _IN1, abs(_speed) );
                        } else {
                          // SLEEP HIGH, IN1 HIGH, IN2 PWM
                          setPWM( 1, 0, _IN2, abs(_speed) );
                        }
                        break;

    default:            // SLEEP HIGH, IN1 LOW, IN2 LOW
                        setPWM( 0, 0, GPIO_NUM_NC, 0 );
                        break;
    }  

  // update duty
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t) (_port) ) );
  
}

void SwOSActor::setDistance( long distance, bool relative, bool dontSendToRemote ) {

  if   (!_ctrl->isLocal()) {

    if (!dontSendToRemote) {
      // send remote
      SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_SETSTEPPERDISTANCE  );
      cmd.data.actorStepperCmd.index = _port;
      cmd.data.actorStepperCmd.paraml = distance;
      cmd.data.actorStepperCmd.paramb = relative;
      cmd.send( );

    }

  } else if ( ( _ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (pwrDrive ) ) {
    // set local
    if (relative) pwrDrive->setRelDistance( _pwrDriveMotor, distance );
    else          pwrDrive->setAbsDistance( _pwrDriveMotor, distance );

    _distance = pwrDrive->getStepsToGo( _pwrDriveMotor );

  }

}

long SwOSActor::getDistance( void ) {
  return _distance;
}

void SwOSActor::startStop( bool start ) {

  if (!_ctrl->isLocal() )  {
    // send remote
    SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_STEPPERSTARTSTOP  );
    cmd.data.actorStepperCmd.index  = _port;
    cmd.data.actorStepperCmd.paramb = start;
    cmd.send( );

  } else if ( ( _ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (pwrDrive ) ) {

    if (start) pwrDrive->startMoving( _pwrDriveMotor );
    else       pwrDrive->stopMoving( _pwrDriveMotor );
    
  }

}

void SwOSActor::setPosition( long position, bool dontSendToRemote ) {

  _position = position;

  if   (!_ctrl->isLocal()) {

    if (!dontSendToRemote) {
    
      // send remote
      SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_SETSTEPPERPOSITION  );
      cmd.data.actorStepperCmd.index = _port;
      cmd.data.actorStepperCmd.paraml = position;
      cmd.send( );

    }

  } else if ( ( _ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (pwrDrive ) ) {
    // set local
    pwrDrive->setPosition( _pwrDriveMotor, position );
  }

}

long SwOSActor::getPosition( void ) {
  return _position;
}

void SwOSActor::homing( long maxDistance ) {

  if   (!_ctrl->isLocal()) {
    // send remote
    SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_STEPPERHOMING );
    cmd.data.actorStepperCmd.index = _port;
    cmd.data.actorStepperCmd.paraml = maxDistance;
    cmd.send( );

  } else if ( ( _ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (pwrDrive ) ) {
    // set local
    pwrDrive->homing( _pwrDriveMotor, maxDistance );
  }

}

void SwOSActor::setHomingOffset( long offset ) {

  if   (!_ctrl->isLocal()) {
    // send remote
    SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_SETSTEPPERHOMINGOFFSET );
    cmd.data.actorStepperCmd.index = _port;
    cmd.data.actorStepperCmd.paraml = offset;
    cmd.send( );

  } else if ( ( _ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (pwrDrive ) ) {
    // set local
    pwrDrive->homingOffset( _pwrDriveMotor, offset );
  }

}

void SwOSActor::jsonize( JSONize *json, uint8_t id) {

  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variable("subType",    (char *) ACTORTYPE[ _actorType ] );
  json->variableUI32("motiontype", getMotionType() );
  json->variableI16 ("speed",      getSpeed() );
  json->variableB( "highResolution", _highResolution );
  json->endObject();
}

void SwOSActor::onTrigger( int32_t value ) {

  setSpeed( (int16_t) value );
}

void SwOSActor::read( void ) {

}

void SwOSActor::setIsHoming( bool isHoming ) {
  _isHoming = isHoming;
}

bool SwOSActor::isHoming( void ) {
  return _isHoming;
}

void SwOSActor::setIsRunning( bool isRunning ) {
  _isRunning = isRunning;
}

bool SwOSActor::isRunning( void ) {
  return _isRunning;
}

void SwOSActor::setValue( long distance, long position, bool isHoming, bool isRunning ) {

  //printf("setValue: %s %ld %d %d\n", getName(), distance, position, isHoming, isRunning );
  _distance = distance;
  _position = position;
  _isHoming = isHoming;
  _isRunning = isRunning;
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

/***************************************************
 *
 *   SwOSPixel
 *
 ***************************************************/

  // LED Representation in FastLED library
  CRGB led[MAXLEDS];
  uint8_t usedPixels = 0;
  bool ledsInitialized = false;

SwOSPixel::SwOSPixel(const char *name, uint8_t port, SwOSCtrl *ctrl) : SwOSIO( name, port, ctrl ) {

  if (ctrl->isLocal()) _setupLocal();

}

void SwOSPixel::_setupLocal() {

  // leds[] need to be initialized only once.
  if (!ledsInitialized) {

    // assign port to GPIO.
    switch ( _ctrl->getCPU() ) {
      #if CONFIG_IDF_TARGET_ESP32S3
        case FTSWARMPWRDRIVE_1V141:
        case FTSWARMDUINO_1V141:
        case FTSWARMXL_1V00:
        case FTSWARMRS_2V1:
        case FTSWARMRS_2V0:  FastLED.addLeds<WS2812, xGPIO_NUM_48, GRB>(led, MAXLEDS).setCorrection( TypicalLEDStrip ); break;
      #endif
      case FTSWARMJST_1V15:  FastLED.addLeds<WS2812, GPIO_NUM_26, GRB>(led, MAXLEDS).setCorrection( TypicalLEDStrip ); break;
      default:               FastLED.addLeds<WS2812, GPIO_NUM_33, GRB>(led, MAXLEDS).setCorrection( TypicalLEDStrip ); break;
    }

    setBrightness( BRIGHTNESSDEFAULT );
    ledsInitialized = true;
  }

  // initialize pixel
  if ( _port < MAXLEDS ) {
    // leds[_port] = CRGB::Black;
    setColor( FtSwarmColor::Black );
  }

}

void SwOSPixel::setColor(uint32_t color) {

  // store new color
  _color = color;

  // apply local or remote
  if (_ctrl->isLocal()) _setColorLocal();
  else                  _setRemote();
}

void SwOSPixel::_setRemote() {
  
  SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_SETLED );
  cmd.data.ledCmd.index = _port;
  cmd.data.ledCmd.color = _color;
  cmd.data.ledCmd.brightness = _brightness;
  cmd.send( );
}

void SwOSPixel::_setColorLocal() {

  // set color
  if (_port < MAXLEDS ) {
    led[_port] = _color;
    FastLED.show();
  }

}

void SwOSPixel::setBrightness(uint8_t brightness) {

  // store new brightness
  _brightness = brightness;

  // apply local or remote
  if (_ctrl->isLocal()) _setBrightnessLocal();
  else                  _setRemote();

}

void SwOSPixel::_setBrightnessLocal() {

  // set brightness
  if (_port < MAXLEDS ) {
    // TODO: brightness per pixel
    // leds[_port].fadeLightBy( brightness );
    FastLED.setBrightness( _brightness );
    FastLED.show();
  }

}

void SwOSPixel::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variableUI8  ("brightness", _brightness);
  json->variableUI32X("color",     _color);
  json->endObject();
}

void SwOSPixel::onTrigger( int32_t value ) {

  setColor( (uint32_t) value );
  
}

/***************************************************
 *
 *   SwOSServo
 *
 ***************************************************/

SwOSServo::SwOSServo(const char *name, uint8_t port, SwOSCtrl *ctrl) : SwOSIO( name, port, ctrl ) {

  // initialize local HW
  if (ctrl->isLocal()) _setupLocal();
}

void SwOSServo::_setupLocal() {
  // initialize local HW

  // assign port to GPIO. All CPU versions use same ports.
  switch ( _ctrl->getCPU() ) {
    case FTSWARMRS_2V1:
      if (_port == 0 ) _SERVO = xGPIO_NUM_47;
      else             _SERVO = GPIO_NUM_15;
      break;

    case FTSWARMRS_2V0: 
      _SERVO = xGPIO_NUM_47;
      break;

    case FTSWARMCONTROL_1V3: 
      _SERVO = GPIO_NUM_33;
      break;

    default:
      _SERVO = xGPIO_NUM_25;
      break;
      
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
  _channelSERVO = (ledc_channel_t) (4 + _port);

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

void SwOSServo::_setRemote( ) {
  
  SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_SETSERVO );
  cmd.data.servoCmd.index    = _port;
  cmd.data.servoCmd.position = _position;
  cmd.data.servoCmd.offset   = _offset;
  cmd.send( );
}

void SwOSServo::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variableI16("offset",   _offset);
  json->variableI16("position", _position);
  json->endObject();
}

void SwOSServo::setPosition( int16_t position, bool dontSendToRemote ) {
  _position = position;

  // apply local or remote
  if (_ctrl->isLocal())       _setLocal();
  else if (!dontSendToRemote) _setRemote();

}

void SwOSServo::setOffset( int16_t offset, bool dontSendToRemote ) {
  _offset = offset;
 
  // apply local or remote
  if (_ctrl->isLocal())       _setLocal();
  else if (!dontSendToRemote) _setRemote();

}

void SwOSServo::onTrigger( int32_t value ) {

  setPosition( (int16_t) value, false );

}

/***************************************************
 *
 *   SwOSGyro
 *
 ***************************************************/

SwOSGyro::SwOSGyro(const char *name, SwOSCtrl *ctrl) : SwOSIO( name, ctrl ) {

  if (ctrl->isLocal()) _setupLocal();

}

void SwOSGyro::_setupLocal() {

  // need an internal I²C interface
  TwoWire internalI2C = TwoWire(1);
  internalI2C.begin( 4, 5 );

  // create gyro object
  _gyro = new LSM6DSRSensor(&internalI2C, LSM6DSR_I2C_ADD_H);

  // pull INT1 to low to enable I2C
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << GPIO_NUM_16);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);
  gpio_set_level( GPIO_NUM_16, 0 );
  delay(200);

  // start gyro
  _gyro->begin();
  _gyro->Enable_X();
  _gyro->Enable_G();

}

void SwOSGyro::jsonize( JSONize *json, uint8_t id) {
  
}

void SwOSGyro::read() {

  _gyro->Get_X_Axes(_accelerometer);
  _gyro->Get_G_Axes(_gyroscope);

  
}

/***************************************************
 *
 *   SwOSButton
 *
 ***************************************************/

SwOSButton::SwOSButton(const char *name, uint8_t port, SwOSCtrl *ctrl) : SwOSIO( name, port, ctrl ), SwOSEventInput( ) {

  _toggle = FTSWARM_NOTOGGLE;

}

void SwOSButton::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variableB("state", _lastState );
  json->endObject();
}

void SwOSButton::setState( bool state, bool clearToggle ) {
  
  if ( state != _lastState ) {

    if (state) { 
      _toggle = FTSWARM_TOGGLEUP; 
      if ( _ctrl->isLocal() ) trigger( FTSWARM_TRIGGERUP, state );
    
    } else {
      _toggle = FTSWARM_TOGGLEDOWN;
      if ( _ctrl->isLocal() ) trigger( FTSWARM_TRIGGERDOWN, state );
    }
  }

  if (clearToggle) _toggle = FTSWARM_NOTOGGLE;
  
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

  switch ( _ctrl->getCPU() ) {
    case FTSWARMJST_1V0: _CS   = GPIO_NUM_19;
                      _LD   = GPIO_NUM_18;
                      _CLK  = GPIO_NUM_14;
                      _MISO = GPIO_NUM_12;
                      break; 
    case FTSWARMCONTROL_1V3: _CS   = GPIO_NUM_14;
                      _LD   = GPIO_NUM_15;
                      _CLK  = GPIO_NUM_12;
                      _MISO = GPIO_NUM_35;
                      break;
    default:          _CS = _LD = _CLK = _MISO = GPIO_NUM_NC;
                      return;
  }

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
 *   I2C Slave
 *
 ***************************************************/

#define I2C_DATA_LENGTH 512                        /*!< Data buffer length of test buffer */
#define I2C_SLAVE_TX_BUF_LEN (2 * I2C_DATA_LENGTH)  /*!< I2C slave tx buffer size */
#define I2C_SLAVE_RX_BUF_LEN (2 * I2C_DATA_LENGTH)  /*!< I2C slave rx buffer size */
#define I2C_SLAVE_NUM I2C_NUM_1

void SwOSI2C::read(  ) {
 
  int bytes_read = 0;
  uint8_t data[4];
  
  bytes_read = i2c_slave_read_buffer(I2C_SLAVE_NUM, data, 4, portMAX_DELAY);

  if ( bytes_read == 1 ) {
    // read a register
    uint8_t result = 0;
    if (data[0]<MAXI2CREGISTERS) result = _myRegister[data[0]];
    i2c_reset_tx_fifo(I2C_SLAVE_NUM);
    i2c_slave_write_buffer(I2C_SLAVE_NUM, &result, 1, portMAX_DELAY);
      
  } else if ( bytes_read == 2 ) {
    // write a register
    if (data[0]<MAXI2CREGISTERS) {
      _myRegister[data[0]] = data[1];
    }
  }

  if ( bytes_read > 0 ) {
    trigger( FTSWARM_TRIGGERI2CREAD, 0 );
  }

}

void SwOSI2C::_setupLocal(uint8_t I2CAddress) {

  int i2c_slave_port = I2C_SLAVE_NUM;
  i2c_config_t conf_slave;

  switch ( _ctrl->getCPU() ) {
    case FTSWARMRS_2V1:
      // ftSwarmRS final
      conf_slave.sda_io_num = GPIO_NUM_8;
      conf_slave.scl_io_num = GPIO_NUM_9;
      break;
    case FTSWARMRS_2V0:
      // ftSwarmRS 
      conf_slave.sda_io_num = GPIO_NUM_4;
      conf_slave.scl_io_num = GPIO_NUM_5;
      break;
    default:
      // ftSwarm & ftSwarmControl 
      conf_slave.sda_io_num = GPIO_NUM_21;
      conf_slave.scl_io_num = xGPIO_NUM_22;
      break;
    }
    
    conf_slave.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;
    conf_slave.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf_slave.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf_slave.mode = I2C_MODE_SLAVE;
    conf_slave.slave.addr_10bit_en = 0;
    conf_slave.slave.slave_addr = I2CAddress;
    i2c_param_config(i2c_slave_port, &conf_slave);
    i2c_driver_install(i2c_slave_port,  I2C_MODE_SLAVE, I2C_SLAVE_RX_BUF_LEN, I2C_SLAVE_TX_BUF_LEN, 0);

}


SwOSI2C::SwOSI2C( const char *name, SwOSCtrl *ctrl, uint8_t I2CAddress):SwOSIO( name, ctrl ) {

  memset(_myRegister, 0, sizeof(_myRegister));
  
  if (ctrl->isLocal()) _setupLocal(I2CAddress);

}

void SwOSI2C::setRegister( uint8_t reg, uint8_t value ) {

  // check on boundaries
  if (reg>=MAXI2CREGISTERS) return;
  
  _myRegister[reg] = value;

  if (_ctrl->isLocal()) _setLocal( reg, value );
  else                  _setRemote( reg, value );

}

void SwOSI2C::_setRemote( uint8_t reg, uint8_t value ) {
  
  SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_I2CREGISTER );
  cmd.data.I2CRegisterCmd.reg   = reg;
  cmd.data.I2CRegisterCmd.value = value;
  cmd.send( );
}

void SwOSI2C::_setLocal( uint8_t reg, uint8_t value ) {
  _myRegister[reg] = value;
  trigger( FTSWARM_TRIGGERI2CWRITE, 0 );
}

uint8_t SwOSI2C::getRegister( uint8_t reg ) {

  if (reg>=MAXI2CREGISTERS) return 0;
  else return _myRegister[reg];

}

/***************************************************
 *
 *   SwOSOLED
 *
 ***************************************************/

#define YELLOWPIXELS 16


SwOSOLED::SwOSOLED(const char *name, SwOSCtrl *ctrl, uint8_t displayType) : SwOSIO( name, ctrl ) {

  if ( _ctrl->isLocal() ) { 
    displayType = displayType; 
    _setupLocal(); 
  }

}

void SwOSOLED::_setupLocal() {

  _display = new Adafruit_SSD1306 (128, 64, &Wire, -1);
  if ( !_display->begin(SSD1306_SWITCHCAPVCC, 0x3C ) ) {
    delete _display;
    _display = NULL;
    ESP_LOGE( LOGFTSWARM, "Couldn't initialize OLED display." );
    return;
  }

  _display->clearDisplay();
  // _display->dim(true);
     
  // set useful default values
  _display->setTextColor(true, false);   // Draw white text
  _display->cp437(true);       // Use full 256 char 'Code Page 437' font

  _display->setTextSize(3,3);            // Logo
  write( (char *) "ftSwarm", getWidth()/2, 0, FTSWARM_ALIGNCENTER, true );

  _display->setTextSize(1,1);            // hostname & version
  char line[100];
  sprintf( line, "%s %s", _ctrl->getHostname(), SWOSVERSION );
  write( line, getWidth()/2, 32, FTSWARM_ALIGNCENTER, true );

  // additional default values
  _display->setTextSize(1, 1);           // Normal 1:1 pixel scale
  _display->setCursor(0, 0);             // Start at top-left corner

  dim(true);

  display();

}

void SwOSOLED::display(void) {

  if (_display) _display->display();
  
}

void SwOSOLED::invertDisplay(bool i) {

  if (_display) _display->invertDisplay( i );
  
}

void SwOSOLED::fillScreen(bool white) {
  
  drawRect( 0, YELLOWPIXELS, getWidth(), getHeight()-YELLOWPIXELS, true, white );
  
} 

void SwOSOLED::dim(bool dim) {

  // origin adafruit code fails with some displays
  // if (_display) _display->dim( dim );
  setContrast( dim ? 1 : 0x8F );
  
}

void SwOSOLED::setContrast(uint8_t contrast) {

  // avoid problemns with setContrast and Display Types != 1
  if (_displayType != 1 ) return;

  // send set contrast
  Wire.beginTransmission( 0x3C );
  Wire.write( (uint8_t) 0 );
  Wire.write( 0x81 );
  Wire.endTransmission();

  // send contast value
  Wire.beginTransmission( 0x3C );
  Wire.write( (uint8_t) 0 );
  Wire.write( contrast );
  Wire.endTransmission();
  
}

void SwOSOLED::drawPixel(int16_t x, int16_t y, bool white ) {
  
  if (_display) _display->drawPixel( x, y + YELLOWPIXELS, (white)?(SSD1306_WHITE):(0) );
  
}

void SwOSOLED::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool white) {
  
  if (_display) {
    if      (x0==x1) _display->drawFastVLine( x0, y0 + YELLOWPIXELS, y1 - y0,               (white)?(SSD1306_WHITE):(0) ); 
    else if (y0==y1) _display->drawFastHLine( x0, y0 + YELLOWPIXELS, x1 - x0,               (white)?(SSD1306_WHITE):(0) ); 
    else             _display->drawLine(      x0, y0 + YELLOWPIXELS, x1, y1 + YELLOWPIXELS, (white)?(SSD1306_WHITE):(0) );
  }
  
} 

void SwOSOLED::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, bool fill, bool white) {

  if (_display) {
    if (fill) _display->fillRect( x, y + YELLOWPIXELS, w, h, (white)?(SSD1306_WHITE):(0) );
    else      _display->drawRect( x, y + YELLOWPIXELS, w, h, (white)?(SSD1306_WHITE):(0) );
  }
  
}

void SwOSOLED::drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, bool fill, bool white) {
  
   if (_display) {
    if (fill) _display->fillRoundRect( x0, y0 + YELLOWPIXELS, w, h, radius, (white)?(SSD1306_WHITE):(0)); 
    else      _display->drawRoundRect( x0, y0 + YELLOWPIXELS, w, h, radius, (white)?(SSD1306_WHITE):(0)); 
  }
  
} 


void SwOSOLED::drawCircle(int16_t x0, int16_t y0, int16_t r, bool fill, bool white) {
  
  if (_display) {
    if (fill) _display->fillCircle( x0, y0 + YELLOWPIXELS, r, (white)?(SSD1306_WHITE):(0)); 
    else      _display->drawCircle( x0, y0 + YELLOWPIXELS, r, (white)?(SSD1306_WHITE):(0)); 
  }
  
} 

void SwOSOLED::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool fill, bool white) {
  
  if (_display) {
    if (fill) _display->fillTriangle( x0, y0 + YELLOWPIXELS, x1, y1 + YELLOWPIXELS, x2, y2 + YELLOWPIXELS, (white)?(SSD1306_WHITE):(0)); 
    else      _display->drawTriangle( x0, y0 + YELLOWPIXELS, x1, y1 + YELLOWPIXELS, x2, y2 + YELLOWPIXELS, (white)?(SSD1306_WHITE):(0)); 
  }

} 

void SwOSOLED::setCursor(int16_t x, int16_t y) {
  
  if (_display) _display->setCursor( x, y  + YELLOWPIXELS);
  
}

void SwOSOLED::getCursor(int16_t *x, int16_t *y) {
  
  if (_display) {
    *x = _display->getCursorX( );
    *y = _display->getCursorY( );
  }
  
}

void SwOSOLED::setTextColor(bool c, bool bg) {
  
  if (_display) _display->setTextColor( (c)?(SSD1306_WHITE):(0), (bg)?(SSD1306_WHITE):(0) );
  
}

void SwOSOLED::setTextWrap(bool w) {
  
  if (_display) _display->setTextWrap( w );
  
}

void SwOSOLED::setRotation(uint8_t r) {
  
  if (_display) _display->stopscroll( );
  
}

uint8_t SwOSOLED::getRotation(void)  {
  
  if (_display) return _display->getRotation( ); else return 0;
  
}

void SwOSOLED::setTextSize(uint8_t sx, uint8_t sy) {
  
  _textSizeX = sx;
  _textSizeY = sy;
  
  if (_display) _display->setTextSize( sx, sy );
  
} 

void SwOSOLED::getTextSize( uint8_t *sx, uint8_t *sy ) {
  *sx = _textSizeX;
  *sy = _textSizeY;
}

void SwOSOLED::drawChar(int16_t x, int16_t y, unsigned char c, bool color, bool bg, uint8_t size_x, uint8_t size_y) {
  
  if (_display) _display->drawChar( x, y + YELLOWPIXELS, c, (color)?(SSD1306_WHITE):(0), (bg)?(SSD1306_WHITE):(0), size_x, size_y );

} 

void SwOSOLED::write( char *str ) {
  
  if (_display) _display->write(str);
  
}

void SwOSOLED::write( char *str, int16_t x, int16_t y, FtSwarmAlign_t align, bool fill ) { 

  // no display...
  if (!_display) return;

  int16_t x1, y1, x2, y2;
  uint16_t w, h;
  getTextBounds( str, 0, 0, &x1, &y1, &w, &h );

  switch (align) {
    case FTSWARM_ALIGNLEFT:   x2 = x;       y2 = y; break;
    case FTSWARM_ALIGNCENTER: x2 = x - w/2; y2 = y; break;
    case FTSWARM_ALIGNRIGHT:  x2 = x - w;   y2 = y; break;
    default:                  x2 = x;       y2 = y; break;
  }

  if (fill) _display->fillRect( x2, y2 + YELLOWPIXELS, w, h, 0 );
  setCursor( x2, y2 );
  write( str );
  
}

void SwOSOLED::getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {

  if (_display) {
    _display->getTextBounds( string, x, y + YELLOWPIXELS, x1, y1, w, h );
    *y1 -=  + YELLOWPIXELS;
  }
  
} 

int16_t SwOSOLED::getWidth(void)  {
  
  if (_display) return _display->width( ); else return 0;
  
}

int16_t SwOSOLED::getHeight(void) {

  if (_display) return _display->height( ) - YELLOWPIXELS; else return 0;
  
}

/***************************************************
 *
 *   SwOSCAM - Camera
 *
 ***************************************************/


SwOSCAM::SwOSCAM(const char *name, SwOSCtrl *ctrl ) : SwOSIO( name, ctrl ) {

  if ( _ctrl->isLocal() ) { 
    _setupLocal(); 
  }

}

// CAMERA Pins
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void SwOSCAM::_setupLocal() {

camera_config_t config;
config.ledc_channel = LEDC_CHANNEL_5;
config.ledc_timer = LEDC_TIMER_3;
config.pin_d0 = Y2_GPIO_NUM;
config.pin_d1 = Y3_GPIO_NUM;
config.pin_d2 = Y4_GPIO_NUM;
config.pin_d3 = Y5_GPIO_NUM;
config.pin_d4 = Y6_GPIO_NUM;
config.pin_d5 = Y7_GPIO_NUM;
config.pin_d6 = Y8_GPIO_NUM;
config.pin_d7 = Y9_GPIO_NUM;
config.pin_xclk = XCLK_GPIO_NUM;
config.pin_pclk = PCLK_GPIO_NUM;
config.pin_vsync = VSYNC_GPIO_NUM;
config.pin_href = HREF_GPIO_NUM;
config.pin_sccb_sda = SIOD_GPIO_NUM;
config.pin_sccb_scl = SIOC_GPIO_NUM;
config.pin_pwdn = PWDN_GPIO_NUM;
config.pin_reset = RESET_GPIO_NUM;
config.xclk_freq_hz = 20000000;
config.frame_size = FRAMESIZE_UXGA;
config.pixel_format = PIXFORMAT_JPEG; // for streaming
config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
config.fb_location = CAMERA_FB_IN_PSRAM;
config.jpeg_quality = 12;
config.fb_count = 1;
if(config.pixel_format == PIXFORMAT_JPEG){
  if(psramFound()){
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    // Limit the frame size when PSRAM is not available
    config.frame_size = FRAMESIZE_SVGA;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }
} else {
  // Best option for face detection/recognition
  config.frame_size = FRAMESIZE_240X240;
}

// camera init
esp_err_t err = esp_camera_init(&config);
if (err != ESP_OK) {
  Serial.printf("Camera init failed with error 0x%x", err);
  return;
}

sensor_t * s = esp_camera_sensor_get();
// drop down frame size for higher initial frame rate
if(config.pixel_format == PIXFORMAT_JPEG){
  s->set_framesize(s, FRAMESIZE_QVGA);
}

}

void SwOSCAM::jsonize( JSONize *json, uint8_t id) {

  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variable("url", (char *) "/stream" );
  json->variableUI8( "framesize",  _framesize );
  json->variableUI8( "quality",    _quality );
  json->variableI16( "brightness", _brightness );
  json->variableI16( "contrast",   _contrast );
  json->variableI16( "saturation", _saturation );
  json->variableB( "H-Mirror", _hMirror );
  json->variableB( "v-Flip",   _vFlip );
  json->endObject();

}

void SwOSCAM::setRemote( void ) {
  
}

void SwOSCAM::streaming( bool onOff, bool dontSendToRemote ) {
  
  _streaming = onOff;

  // apply local or remote
  if (_ctrl->isLocal())       1==1;  // ToDo Streaming
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setFramesize( framesize_t framesize, bool dontSendToRemote ) {
  
  _framesize = framesize;

  // apply local or remote
  if (_ctrl->isLocal())       esp_camera_sensor_get()->set_framesize( esp_camera_sensor_get(), framesize );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setQuality( int quality, bool dontSendToRemote ) {
  
  _quality = quality;

  // apply local or remote
  if (_ctrl->isLocal())       esp_camera_sensor_get()->set_quality( esp_camera_sensor_get(), quality );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setBrightness( int brightness, bool dontSendToRemote ) {
  
  _brightness = brightness;

  // apply local or remote
  if (_ctrl->isLocal())       esp_camera_sensor_get()->set_brightness( esp_camera_sensor_get(), brightness );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setContrast( int contrast, bool dontSendToRemote ) {
  
  _contrast = contrast;

  // apply local or remote
  if (_ctrl->isLocal())       esp_camera_sensor_get()->set_contrast( esp_camera_sensor_get(), contrast );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setSaturation( int saturation, bool dontSendToRemote ) {
  
  _saturation = saturation;

  // apply local or remote
  if (_ctrl->isLocal())       esp_camera_sensor_get()->set_saturation( esp_camera_sensor_get(), saturation );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setSpecialEffect( int specialEffect, bool dontSendToRemote ) {
  
  _specialEffect = specialEffect;

  // apply local or remote
  if (_ctrl->isLocal())       esp_camera_sensor_get()->set_special_effect( esp_camera_sensor_get(), specialEffect );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setWbMode( int wbMode, bool dontSendToRemote ) {
  
  _wbMode = wbMode;

  // apply local or remote
  if (_ctrl->isLocal())       esp_camera_sensor_get()->set_wb_mode( esp_camera_sensor_get(), wbMode );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setVFlip( bool vFlip, bool dontSendToRemote ) {
  
  _vFlip = vFlip; 

  // apply local or remote
  if (_ctrl->isLocal())   esp_camera_sensor_get()->set_vflip( esp_camera_sensor_get(), vFlip );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setHMirror( bool hMirror, bool dontSendToRemote ) {
  
  _hMirror = hMirror; 

  // apply local or remote
  if (_ctrl->isLocal())   esp_camera_sensor_get()->set_hmirror( esp_camera_sensor_get(), hMirror );
  else if (!dontSendToRemote) setRemote();

}


/***************************************************
 *
 *   SwOSCtrl - Base class for controllers.
 *
 ***************************************************/

SwOSCtrl::SwOSCtrl( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmAKelda, FtSwarmExtMode_t extentionPort  ):SwOSObj() {

  // copy master data
  this->IAmAKelda = IAmAKelda;
  serialNumber = SN;
  this->macAddr.set( macAddr );
  _local = local;
  _CPU = CPU;

  // # of inputs & actors
  switch (CPU) {
    case FTSWARMRS_2V0:
    case FTSWARMRS_2V1:         inputs = 6; actors = 2; leds = 2;
                                if ( extentionPort == FTSWARM_EXT_OUTPUT ) actors = 4; 
                                break;
    case FTSWARMCAM_2V11:       inputs = 3; actors = 2; leds = 0; break;
    case FTSWARMDUINO_1V141:    inputs = 8; actors = 4; leds = 2; break;
    case FTSWARMPWRDRIVE_1V141: inputs = 5; actors = 4; leds = 2; break;
    case FTSWARMXL_1V00:        inputs = 8; actors = 8; leds = 2; break;
    default:                    inputs = 4; actors = 2; leds = 0; break;
  }

  // define common hardware
  for (uint8_t i=0; i<MAXINPUTS; i++) { 
    
    if ( i < inputs ) {

      if (CPU == FTSWARMPWRDRIVE_1V141 ) {

        if (i==4) {
          input[i] = new SwOSDigitalInput("EM", 255, this );
        } else {
          input[i] = new SwOSDigitalInput("ES", i, this );
        }

      } else {

        input[i] = new SwOSDigitalInput("A", i, this );
      
      }

    } else {
      input[i] = NULL;
    }

  }

  for (uint8_t i=0; i<MAXACTORS; i++) { 
    
    if ( i< actors ) {
    
      if (CPU == FTSWARMPWRDRIVE_1V141 ) {
        actor[i] = new SwOSActor("Stepper", i, this );
    
      } else {
        actor[i] = new SwOSActor("M", i, this );
    
      } 
    
    } else {
      actor[i] = NULL;
    
    }

  }

  for (uint8_t i=0; i<MAXLEDS; i++) { 
    if ( i < leds ) led[i] = new SwOSPixel("LED", i, this);
    else            led[i] = NULL;
  }

}

SwOSCtrl::~SwOSCtrl() {
  
  if ( _subscribedCtrlName ) delete _subscribedCtrlName;
  
  for (uint8_t i=0; i<MAXINPUTS; i++) { if ( input[i] ) delete( input[i] ); }
  for (uint8_t i=0; i<MAXACTORS; i++) { if ( actor[i] ) delete( actor[i] ); }
  for (uint8_t i=0; i<MAXLEDS;   i++) { if ( led[i] )   delete( led[i] ); }
  
}

void SwOSCtrl::halt( void ) {

  for (uint8_t i=0; i<MAXACTORS; i++) { if ( actor[i] ) actor[i]->setSpeed(0); }

}

char *SwOSCtrl::subscribe( char *ctrlName  ) {
  
  _isSubscribed = true;

  if (_subscribedCtrlName) delete _subscribedCtrlName;

  // only if I don't know my external name, store it
  if (!_subscribedCtrlName) {
    _subscribedCtrlName = (char *)malloc( strlen(ctrlName)+1 );
    strcpy( _subscribedCtrlName, ctrlName );
  }

  return _subscribedCtrlName;

}

void SwOSCtrl::unsubscribe( bool cascade ) {

  _isSubscribed = false;

  if (!cascade) return;

  for (uint8_t i=0; i<MAXINPUTS; i++) { if ( input[i] ) input[i]->unsubscribe(); }
  for (uint8_t i=0; i<MAXACTORS; i++) { if ( actor[i] ) actor[i]->unsubscribe(); }
  for ( uint8_t i=0; i<MAXLEDS; i++ ) { if ( led[i] ) led[i]->unsubscribe(); }
  
}

void SwOSCtrl::factorySettings( void ) {

  setAlias("");
  for (uint8_t i=0; i<MAXINPUTS; i++) { if ( input[i] ) input[i]->setAlias( "" ); }
  for (uint8_t i=0; i<MAXACTORS; i++) { if ( actor[i] ) actor[i]->setAlias( "" ); }
  for (uint8_t i=0; i<MAXLEDS; i++)   { if ( led[i] )   led[i]->setAlias( "" ); }

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
  port = atoi(&obj[pos])-1;

  // check on trailing digits only
  while ( obj[pos]!='\0') {
    if (!isdigit(obj[pos]) ) return false;
    pos++;
  }

  // now start interpreting
  if (strcmp( "FTSWARM", device) == 0)                                           { setAlias( alias );            return true; }
  else if ( ( strcmp(device, "A") == 0 )   && (port < inputs) )                  { input[port]->setAlias(alias); return true; }
  else if ( ( strcmp(device, "M") == 0 )   && (port < actors) )                  { actor[port]->setAlias(alias); return true; }
  else if ( ( strcmp(device, "LED") == 0 ) && (port < MAXLEDS) && (led[port] ) ) { led[port]->setAlias(alias);   return true; }
  
  // specific hardware?
  return cmdAlias( device, port, alias );

}

bool SwOSCtrl::cmdAlias( char *device, uint8_t port, const char *alias) {

  // own hardware options are already interpreted
  return false;

}

SwOSIO *SwOSCtrl::getIO( const char *name) {

  for ( uint8_t i=0; i<inputs;  i++) { if ( ( input[i] ) && ( input[i]->equals(name) ) ) { return input[i]; } }
  for ( uint8_t i=0; i<actors;  i++) { if ( ( actor[i] ) && ( actor[i]->equals(name) ) ) { return actor[i]; } }
  for ( uint8_t i=0; i<MAXLEDS; i++) { if ( ( led[i] )   && ( led[i]->equals(name) ) )   { return led[i]; } }

  return NULL;
}

SwOSIO *SwOSCtrl::getIO( FtSwarmIOType_t ioType, FtSwarmPort_t port) {

  switch (ioType) {
    case FTSWARM_COUNTER:
    case FTSWARM_INPUT: 
    case FTSWARM_DIGITALINPUT:
    case FTSWARM_ANALOGINPUT : return ( (port<inputs)?input[ port ]:NULL);
    
    case FTSWARM_ACTOR:        return ( (port<actors)?actor[ port ]:NULL);
    
    case FTSWARM_PIXEL:        return ( (port<MAXLEDS)?led[ port ]:NULL);
    
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

  // don't send packets to myself, so I need to now last reading time
  _lastContact = millis();

  for (uint8_t i=0; i<inputs; i++) { if (input[i]) input[i]->read(); }
  for (uint8_t i=0; i<actors; i++) { if (actor[i]) actor[i]->read(); }

}

bool SwOSCtrl::isI2CSwarmCtrl( void ) {

  return ( _CPU == FTSWARMPWRDRIVE_1V141 ) || ( _CPU == FTSWARMDUINO_1V141 );

}

const char *SwOSCtrl::version( FtSwarmVersion_t v) {
  switch (v) {
  case FTSWARM_NOVERSION:     return "??";
  case FTSWARMJST_1V0:        return "1.0";
  case FTSWARMCONTROL_1V3:    return "1.3";
  case FTSWARMJST_1V15:       return "1.15";
  case FTSWARMRS_2V0:         return "2.0";
  case FTSWARMRS_2V1:         return "2.1.0";
  case FTSWARMCAM_2V11:       return "2.1.1";
  case FTSWARMDUINO_1V141:  
  case FTSWARMPWRDRIVE_1V141: return "1.4.1";
  case FTSWARMXL_1V00:        return "1.0.0";
  default:                    return  "??";
  }
}

const char *SwOSCtrl::getVersionCPU() {
  return version(_CPU);
}

char *SwOSCtrl::getHostname( void ) {

  if ( (_alias) && (_alias[0]!='\0') ) {
    return _alias ;
  } else {
    return _name;
  }

}

bool SwOSCtrl::changeIOType( uint8_t port, FtSwarmIOType_t oldIOType, FtSwarmIOType_t newIOType ) {

  // not in standard, maybe overloaded
  return false;

}

void SwOSCtrl::jsonize( JSONize *json, uint8_t id) {

  json->startObject();
  json->variable( "name", getHostname());
  json->variableUI8( "id", id);
  json->variableUI16( "serialNumber", serialNumber);
  json->variable( "type", myType() );
  
  json->startArray( "io" );
  jsonizeIO( json, id );
  json->endArray();

  json->endObject();


}

void SwOSCtrl::jsonizeIO( JSONize *json, uint8_t id ) {
  
  for (uint8_t i=0; i<inputs; i++) { if ( input[i] ) input[i]->jsonize( json, id ); }
  for (uint8_t i=0; i<actors; i++) { if ( actor[i] ) actor[i]->jsonize( json, id ); }
  for (uint8_t i=0; i<leds;   i++) { if ( led[i]   ) led[i]->jsonize( json, id ); }

}

bool SwOSCtrl::apiActorCmd( char *id, int cmd ) {
  // send a actor command (from api)

  // search IO
  for (uint8_t i=0; i<actors; i++) {

    if ( actor[i]->equals(id) ) {
      // found
      actor[i]->setMotionType( (FtSwarmMotion_t) cmd );
      return true;
    }
  }

  return false;
}

bool SwOSCtrl::apiActorSpeed( char *id, int speed ) {
  // send a actor command (from api)

  // search IO
  for (uint8_t i=0; i<actors; i++) {

    if ( actor[i]->equals(id) ) {
      // found
      actor[i]->setSpeed( speed );
      return true;
    }
  }

  return false;
}

bool SwOSCtrl::apiLEDBrightness( char *id, int brightness ) {
  // send a LED command (from api)

  // search IO
  for (uint8_t i=0; i<MAXLEDS; i++) {

    if ( (led[i]) && ( led[i]->equals(id) ) ) {
      // found
      led[i]->setBrightness( brightness );
      return true;
    }
  }

  return false;

}

bool SwOSCtrl::apiLEDColor( char *id, int color ) {
  // send a LED command (from api)

  // search IO
  for (uint8_t i=0; i<MAXLEDS; i++) {

    if ( (led[i]) && ( led[i]->equals(id) ) ) {
      // found
      led[i]->setColor( color );
      return true;
    }
  }

  return false;

}

bool SwOSCtrl::apiServoOffset( char * id, int offset ) {
  // send a Servo command (from api)

  // will be defined in SwOSSwarmJST
  return false;

}

bool SwOSCtrl::apiServoPosition( char * id, int position ) {
  // send a Servo command (from api)

  // will be defined in SwOSSwarmJST
  return false;

}

bool SwOSCtrl::apiCAMStreaming( char *id, bool onOff ) {
  // set CAM framzesize / resolution

  // will be implemented in SwOSSwarmCAM
  return false;

}

bool SwOSCtrl::apiCAMFramesize( char *id, int framesize ) {
  // set CAM framzesize / resolution

  // will be implemented in SwOSSwarmCAM
  return false;

}

bool SwOSCtrl::apiCAMQuality( char *id, int quality ) { 
  // set CAM quality

  // will be implemented in SwOSSwarmCAM
  return false;

}

bool SwOSCtrl::apiCAMBrightness( char *id, int brightness ) { 
  // set CAM brightness

  // will be implemented in SwOSSwarmCAM
  return false;

}

bool SwOSCtrl::apiCAMContrast( char *id, int contrast ) { 
  // set CAM contrast

  // will be implemented in SwOSSwarmCAM
  return false;

}

bool SwOSCtrl::apiCAMSaturation( char *id, int saturation ) { 
  // set CAM saturation

  // will be implemented in SwOSSwarmCAM
  return false;

}

bool SwOSCtrl::apiCAMSpecialEffect( char *id, int effect ) { 
  // set CAM special effect

  // will be implemented in SwOSSwarmCAM
  return false;

}

bool SwOSCtrl::apiCAMWbMode( char *id, int wbMode ) { 
  // set CAM wbMode

  // will be implemented in SwOSSwarmCAM
  return false;

}

bool SwOSCtrl::apiCAMVFlip( char *id,bool vFlip ) { 
  // set CAM V-Flip 

  // will be implemented in SwOSSwarmCAM
  return false;

}

bool SwOSCtrl::apiCAMHMirror( char *id,bool hMirror ) { 
  // set CAM hMirror

  // will be implemented in SwOSSwarmCAM
  return false;

}


bool SwOSCtrl::maintenanceMode() {
  return false;
}

void SwOSCtrl::setState( SwOSState_t state, uint8_t members, char *SSID ) {
  // visualizes controler's state like booting, error,...
  
  if (led[0]) led[0]->setColor( LEDCOLOR0[state] );
  if (led[1]) led[1]->setColor( LEDCOLOR1[state] );

}

void SwOSCtrl::identify( void ) {
  
  if (_local) {
    setState( IDENTIFY );
  
  } else {
    SwOSCom identify( macAddr, serialNumber, CMD_IDENTIFY );
    identify.send();

  }

}

unsigned long SwOSCtrl::networkAge( void ) { 

  unsigned long age = millis() - _lastContact;

  return age;

}

bool SwOSCtrl::OnDataRecv(SwOSCom *com ) {

  if (!com) return false;

  _lastContact = millis();

  switch (com->data.cmd) {
    case CMD_STATE: 
      return recvState( com );

    case CMD_SETLED: 
      if (led[com->data.ledCmd.index]) {
        led[com->data.ledCmd.index]->setBrightness( com->data.ledCmd.brightness );
        led[com->data.ledCmd.index]->setColor( com->data.ledCmd.color );
      }
      return true;

    case CMD_SETSENSORTYPE:
      if ( input[com->data.sensorCmd.index]->getIOType() == FTSWARM_DIGITALINPUT) { 
        ((SwOSDigitalInput *)input[com->data.sensorCmd.index])->setSensorType( com->data.sensorCmd.sensorType, com->data.sensorCmd.normallyOpen );
      } else {
        ((SwOSAnalogInput *)input[com->data.sensorCmd.index])->setSensorType( com->data.sensorCmd.sensorType );
      }
      return true;

    case CMD_SETACTORSPEED:
      actor[com->data.actorSpeedCmd.index]->setMotionType( com->data.actorSpeedCmd.motionType );
      actor[com->data.actorSpeedCmd.index]->setAcceleration( com->data.actorSpeedCmd.rampUpT, com->data.actorSpeedCmd.rampUpY );
      actor[com->data.actorSpeedCmd.index]->setSpeed( com->data.actorSpeedCmd.speed );
      
      return true;

    case CMD_RESETCOUNTER:
       if ( ( input[com->data.CounterCmd.index] ) && ( input[com->data.CounterCmd.index]->getIOType() == FTSWARM_COUNTERINPUT ) )
          static_cast<SwOSCounter *>(input[com->data.CounterCmd.index])->resetCounter();
       return true;

    case CMD_SETSTEPPERDISTANCE:
      actor[com->data.actorStepperCmd.index]->setDistance( com->data.actorStepperCmd.paraml, com->data.actorStepperCmd.paramb, true );
      return true;

    case CMD_SETSTEPPERPOSITION:
      actor[com->data.actorStepperCmd.index]->setPosition( com->data.actorStepperCmd.paraml, true );
      return true;

    case CMD_STEPPERHOMING:
      actor[com->data.actorStepperCmd.index]->homing( com->data.actorStepperCmd.paraml );
      return true;

    case CMD_SETSTEPPERHOMINGOFFSET:
      actor[com->data.actorStepperCmd.index]->setHomingOffset( com->data.actorStepperCmd.paraml );
      return true;

    case CMD_STEPPERSTARTSTOP:
      actor[com->data.actorStepperCmd.index]->startStop( com->data.actorStepperCmd.paramb );
      return true;

    case CMD_SETACTORTYPE:
      actor[com->data.actorTypeCmd.index]->setActorType( com->data.actorTypeCmd.actorType, com->data.actorTypeCmd.highResolution, true );
      return true;

    case CMD_IDENTIFY:
      identify();
      return true;

    case CMD_USEREVENT:
      if ( com->data.userEventCmd.trigger ) {

        // send trigger event to local procedure
        if ( xQueueSend( myOSNetwork.userEvent, com, ESPNOW_MAXDELAY ) != pdTRUE ) {
          ESP_LOGE( LOGFTSWARM, "Can't send data to user event." );
        }

      } else {
        
        // got some user event data
        if (_isSubscribed) {
          printf("S: %s", _subscribedCtrlName );
          for ( uint8_t i=0; i<com->data.userEventCmd.size; i++ ) printf(" %02X", com->data.userEventCmd.payload[i]);
          printf("\n");
        }
      }
      
      return true;

    case CMD_ALIAS:
      // get all entries in datagramm
      for (uint8_t i=0; i<MAXALIAS; i++) {
        if (com->data.aliasCmd.alias[i].name[0] != '\0') {
          // entry isn't empty
          if ( strcmp( com->data.aliasCmd.alias[i].name, "HOSTNAME" ) == 0 ) {
            // hostname
            setAlias( com->data.aliasCmd.alias[i].alias );
          } else {
            // an IO port?
            cmdAlias( com->data.aliasCmd.alias[i].name, com->data.aliasCmd.alias[i].alias );
          }
        }
      }
      return true;

    case CMD_CHANGEIOTYPE:
      // change IO Type
      changeIOType( com->data.changeIOTypeCmd.index, com->data.changeIOTypeCmd.oldIOType, com->data.changeIOTypeCmd.newIOType );
      return true;

  }

  return false;

}

SwOSCom *SwOSCtrl::state2Com( MacAddr destination ) {

  SwOSCom *com = new SwOSCom( destination, serialNumber, CMD_STATE );

  int16_t FB, LR;

  for (uint8_t i=0; i<inputs; i++ ) { com->data.stateCmd.inputValue[i] = input[i]->getValueI32(); };

  return com;

}

bool SwOSCtrl::recvState( SwOSCom *com ) {
  
  for (uint8_t i=0; i<inputs; i++ ) { input[i]->setValue( com->data.stateCmd.inputValue[i] ); };
  return true;
 
} 

void SwOSCtrl::registerMe( SwOSCom *com ){

  if (!com) return;

  // meta data
  com->data.registerCmd.ctrlType   = getType();
  com->data.registerCmd.versionCPU = getCPU();
  com->data.registerCmd.IAmAKelda  = IAmAKelda;

  // extention port
  com->data.registerCmd.extentionPort = nvs.extentionPort;

  if ( getType() != FTSWARMCONTROL ) com->data.registerCmd.leds = leds;

}

void SwOSCtrl::saveAliasToNVS( nvs_handle_t my_handle ) {

  SwOSObj::saveAliasToNVS( my_handle );
  for (uint8_t i=0; i<inputs; i++ )  input[i]->saveAliasToNVS( my_handle );
  for (uint8_t i=0; i<actors; i++ )  actor[i]->saveAliasToNVS( my_handle );
  for (uint8_t i=0; i<MAXLEDS; i++ ) if (led[i]) led[i]->saveAliasToNVS( my_handle );
  
}

void SwOSCtrl::loadAliasFromNVS( nvs_handle_t my_handle ) {

  SwOSObj::loadAliasFromNVS( my_handle );
  for (uint8_t i=0; i<inputs; i++ )  input[i]->loadAliasFromNVS( my_handle );
  for (uint8_t i=0; i<actors; i++ )  actor[i]->loadAliasFromNVS( my_handle );
  for (uint8_t i=0; i<MAXLEDS; i++ ) if (led[i]) led[i]->loadAliasFromNVS( my_handle );
}

void SwOSCtrl::_sendAlias( SwOSCom *alias ) {

    // hostname
  alias->sendBuffered( (char *)"HOSTNAME", getAlias() ); 

  // input
  for (uint8_t i=0; i<inputs;i++) alias->sendBuffered( input[i]->getName(), input[i]->getAlias() ); 
  
  // actor
  for (uint8_t i=0; i<actors;i++) alias->sendBuffered( actor[i]->getName(), actor[i]->getAlias() ); 
  
  // LED
  for (uint8_t i=0; i<MAXLEDS;i++) 
    if (led[i]) alias->sendBuffered( led[i]->getName(), led[i]->getAlias() ); 

}

void SwOSCtrl::sendAlias( MacAddr destination ) {

  SwOSCom alias( destination, serialNumber, CMD_ALIAS );
  _sendAlias( &alias );
  alias.flushBuffer( );
  
}

/***************************************************
 *
 *   SwOSSwarmI2CCtrl
 *
 ***************************************************/

SwOSSwarmI2CCtrl::SwOSSwarmI2CCtrl( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmAKelda ): SwOSCtrl (SN, macAddr, local, CPU,  IAmAKelda, FTSWARM_EXT_OFF ) {

  Wire.begin( 5, 4);

}

SwOSSwarmI2CCtrl::~SwOSSwarmI2CCtrl() {

}

/***************************************************
 *
 *   SwOSSwarmPwrDrive - ftPwrDrive implementation as I2C Slave
 *
 ***************************************************/

SwOSSwarmPwrDrive::SwOSSwarmPwrDrive( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmAKelda ): SwOSSwarmI2CCtrl (SN, macAddr, local, CPU,  IAmAKelda ) {

  char buffer[32];
  sprintf( buffer, "ftSwarm%d", SN);
  setName( buffer );

  if (local) pwrDrive = new ftPwrDrive( 32, 5, 4 );
  
}

SwOSSwarmPwrDrive::SwOSSwarmPwrDrive( SwOSCom *com ):SwOSSwarmPwrDrive( com->data.sourceSN, com->macAddr, false, com->data.registerCmd.versionCPU, com->data.registerCmd.IAmAKelda ) {
  
}

SwOSSwarmPwrDrive::~SwOSSwarmPwrDrive() {

}

char* SwOSSwarmPwrDrive::myType() {
  return (char *) "FTSWARMPWRDRIVE";
}

FtSwarmControler_t SwOSSwarmPwrDrive::getType() {
  return FTSWARMPWRDRIVE;
}

SwOSCom *SwOSSwarmPwrDrive::state2Com( MacAddr destination ) {

  SwOSCom *com = new SwOSCom( destination, serialNumber, CMD_STATE );

  for (uint8_t i=0; i<inputs; i++ ) { 
    com->data.stepperStateCmd.inputValue[i] = input[i]->getValueI32();
  };
  
  for (uint8_t i=0; i<actors; i++ ) {
    com->data.stepperStateCmd.isHoming[i] = actor[i]->isHoming();
    com->data.stepperStateCmd.isRunning[i] = actor[i]->isRunning(); 
    com->data.stepperStateCmd.position[i] = actor[i]->getPosition();
    com->data.stepperStateCmd.distance[i] = actor[i]->getDistance();
  }

  return com;

}

bool SwOSSwarmPwrDrive::recvState( SwOSCom *com ) {
  
  for (uint8_t i=0; i<inputs; i++ ) { input[i]->setValue( com->data.stepperStateCmd.inputValue[i] ); };
  for (uint8_t i=0; i<actors; i++ ) {
    actor[i]->setPosition( com->data.stepperStateCmd.position[i], true );
    // actor[i]->setDistance( com->data.stepperStateCmd.distance[i], false, true );
    actor[i]->setIsHoming( com->data.stepperStateCmd.isHoming[i] );
    actor[i]->setIsRunning( com->data.stepperStateCmd.isRunning[i] );
  }
  return true;
 
} 

void SwOSSwarmPwrDrive::read( void ) {

  uint8_t state[4];
  long    position[4];
  long    distance[4];

  // don't send packets to myself, so I need to now last reading time
  _lastContact = millis();
  
  // I'm alive
  pwrDrive->Watchdog( 500 );

  // get input & motor states
  pwrDrive->getStateAll( &state[0], &state[1], &state[2], &state[3] );
  pwrDrive->getPositionAll( &position[0], &position[1], &position[2], &position[3] );
  pwrDrive->getStepsToGoAll( &distance[0], &distance[1], &distance[2], &distance[3] );
  
  for ( uint8_t i=0; i<4; i++ ) {

    // flag 1 motor is running, flag 2 endstop, flag 3 EMS, flag 4 homing
    input[i]->setValue( ( ( state[i] & 0x02 ) > 0 ) );
    
    // MERKER
    actor[i]->setValue( distance[i], position[i], ( ( state[i] & 0x08 ) > 0 ), state[i] & 0x01 );
  
  }
  
  input[4]->setValue( ( state[0] & 0x04 ) >> 2 );

}


void SwOSSwarmPwrDrive::setMicrostepMode( uint8_t mode, bool dontSendToRemote ) {
  // set microstep mode
  
  _microstepMode = mode;

  if   (!isLocal()) {

    if (!dontSendToRemote) {
      // send remote
      SwOSCom cmd( macAddr, serialNumber, CMD_SETMICROSTEPMODE );
      cmd.data.CtrlCmd.microstepMode = mode;
      cmd.send( );
    }

  } else if ( ( getCPU() == FTSWARMPWRDRIVE_1V141 ) && (pwrDrive ) ) {
    // set local
    pwrDrive->setMicrostepMode( mode );

  }

}

uint8_t SwOSSwarmPwrDrive::getMicrostepMode( void ) {
  // get microstep mode
  // FILLSTEP, HALFSTEP, QUARTERSTEP, EIGTHSTEP, SIXTEENTHSTEP

  return _microstepMode;

}

bool SwOSSwarmPwrDrive::OnDataRecv( SwOSCom *com ) {

  if (!com) return false;

  if ( com->data.cmd == CMD_SETMICROSTEPMODE ) {
      setMicrostepMode( com->data.CtrlCmd.microstepMode, true );
      return true;
  } else {
      return SwOSCtrl::OnDataRecv(com);
  }

  return true;

}

/***************************************************
 *
 *   SwOSSwarmDuino - ftDuino implementation as I2C Slave
 *
 ***************************************************/

SwOSSwarmDuino::SwOSSwarmDuino( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmAKelda ): SwOSSwarmI2CCtrl (SN, macAddr, local, CPU,  IAmAKelda ) {

  char buffer[32];
  sprintf( buffer, "ftSwarm%d", SN);
  setName( buffer );

  ftDuino = new FtDuino();

}

SwOSSwarmDuino::~SwOSSwarmDuino() {

  if (ftDuino) delete ftDuino;

}

char* SwOSSwarmDuino::myType() {
  return (char *) "FTSWARMDUINO";
}

FtSwarmControler_t SwOSSwarmDuino::getType() {
  return FTSWARMDUINO;
}

void SwOSSwarmDuino::read() {

  uint16_t value[8];

  if ( ftDuino ) {
    
    // get input mesurements from ftDuino
    ftDuino->getState( value );
    
    // set input measurements
    for (uint8_t i=0; i<8; i++) { 
      if ( input[i] ) input[i]->setReading( value[i] );
    }

    // errors during I2C communication?
    if ( ftDuino->getError() != 0 ) setState( ERROR );
  }

}

/***************************************************
 *
 *   SwOSSwarmXX
 *
 ***************************************************/

SwOSSwarmXX::SwOSSwarmXX( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmAKelda, FtSwarmExtMode_t extentionPort ) : SwOSCtrl (SN, macAddr, local, CPU, IAmAKelda, extentionPort ) {

  gyro = NULL;
  I2C  = NULL;

  if ( local ) {
    
    switch (CPU) {

      case FTSWARMJST_1V0:      Wire.begin( 13, 12 ); break;

      case FTSWARMCONTROL_1V3: 
      case FTSWARMJST_1V15:     Wire.begin( 21, 22 ); break;

      case FTSWARMXL_1V00:      Wire.begin( 5, 4 ); break;

      case FTSWARMRS_2V0: 
      case FTSWARMRS_2V1:       if ( ( nvs.extentionPort != FTSWARM_EXT_I2C_SLAVE ) && ( nvs.extentionPort != FTSWARM_EXT_OFF ) ) Wire.begin( 8, 9 ); 
                                break;
  
      default:                  break;

    }

  }

  if ( nvs.extentionPort == FTSWARM_EXT_I2C_SLAVE ) { I2C = new SwOSI2C ( "I2C", this, nvs.I2CAddr ); };

}

SwOSSwarmXX::~SwOSSwarmXX() {

  if (gyro) delete( gyro );
  if (I2C)  delete( I2C );

}

void SwOSSwarmXX::unsubscribe( void ) {

  if (gyro) gyro->unsubscribe();
  if (I2C)  I2C->unsubscribe();

}

void SwOSSwarmXX::factorySettings( void ) {

  SwOSCtrl::factorySettings();
  if (gyro) gyro->setAlias("");
  
}

SwOSIO *SwOSSwarmXX::getIO( FtSwarmIOType_t ioType, FtSwarmPort_t port) {

  SwOSIO *result = SwOSCtrl::getIO( ioType, port );
  if (result) return result;

  if (ioType == FTSWARM_GYRO ) return gyro;
  else                         return NULL;

}


SwOSIO *SwOSSwarmXX::getIO( const char *name ) {

  SwOSIO *result = SwOSCtrl::getIO( name );
  if (result) return result;

  if      (!gyro)               return NULL;
  else if (gyro->equals(name) ) return gyro;
  else                          return NULL;

}

bool SwOSSwarmXX::OnDataRecv(SwOSCom *com ) {

  if (!com) return false;

  if ( ( com->data.cmd == CMD_I2CREGISTER ) && (I2C) ) {
      I2C->setRegister( com->data.I2CRegisterCmd.reg, com->data.I2CRegisterCmd.value );
      return true;
  } else {
      return SwOSCtrl::OnDataRecv(com);
  }

  return true;

}


void SwOSSwarmXX::_sendAlias( SwOSCom *alias ) {

  SwOSCtrl::_sendAlias( alias );

  // gyro
  if (gyro) alias->sendBuffered( gyro->getName(), gyro->getAlias() ); 

}

bool isInputType( FtSwarmIOType_t ioType ) {

  return ( ioType == FTSWARM_DIGITALINPUT ) ||
         ( ioType == FTSWARM_ANALOGINPUT ) ||
         ( ioType == FTSWARM_COUNTERINPUT ) ||
         ( ioType == FTSWARM_FREQUENCYINPUT );
}

bool SwOSSwarmXX::changeIOType( uint8_t port, FtSwarmIOType_t oldIOType, FtSwarmIOType_t newIOType ) {

  // check on compatible types
  if (!isInputType( oldIOType) ) return false;
  if (!isInputType( newIOType) ) return false;

  // register the new one
  SwOSInput *io;
  switch ( newIOType ) {
    case FTSWARM_DIGITALINPUT:   io = new SwOSDigitalInput("A", port, this ); break;
    case FTSWARM_ANALOGINPUT:    io = new SwOSAnalogInput("A", port, this ); break;
    case FTSWARM_COUNTERINPUT:   io = new SwOSCounter("A", port, 255, this ); break;
    case FTSWARM_FREQUENCYINPUT: io = new SwOSFrequencymeter("A", port, 255, this ); break;
    default: return false;
  }

  // if old port exits, transfer needed properties and kill it
  if (input[port]) {
    char alias[MAXIDENTIFIER];
    strcpy( alias, input[port]->getAlias() );
    io->setAlias( alias );
    delete input[port];
  }

  // assign new port
  input[port] = io;

  // if it's an remote port, change remote site as well
  if ( !isLocal() ) {
    SwOSCom IOType( macAddr, serialNumber, CMD_CHANGEIOTYPE );
    IOType.data.changeIOTypeCmd.index     = port;
    IOType.data.changeIOTypeCmd.oldIOType = oldIOType;
    IOType.data.changeIOTypeCmd.newIOType = newIOType;
    IOType.send();
  }

  return true;

}

/***************************************************
 *
 *   SwOSSwarmJST
 *
 ***************************************************/

SwOSSwarmJST::SwOSSwarmJST( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmAKelda, uint8_t xLeds, FtSwarmExtMode_t extentionPort ):SwOSSwarmXX( SN, macAddr, local, CPU, IAmAKelda, extentionPort ) {

  char buffer[32];
  sprintf( buffer, "ftSwarm%d", SN);
  setName( buffer );

  // additional  leds?
  for ( uint8_t i = leds; i < xLeds; i++ ) {
    led[i] = new SwOSPixel("LED", i, this);
  }
  leds = xLeds;

  // define specific hardware
  for (uint8_t i=0; i<MAXSERVOS; i++) servo[i] = NULL;
  switch ( CPU ) {
    case FTSWARMRS_2V1:     servo[0] = new SwOSServo("SERVO", 0, this);
                            servo[1] = new SwOSServo("SERVO", 1, this);
                            servos = 2;
                            break;

    case FTSWARMRS_2V0:
    case FTSWARMJST_1V15:   servo[0] = new SwOSServo("SERVO", 0, this);
                            servos = 1;
                            break;

    default:                servos = 0;
                            break;
  }

}

SwOSSwarmJST::SwOSSwarmJST( SwOSCom *com ):SwOSSwarmJST( com->data.sourceSN, com->macAddr, false, com->data.registerCmd.versionCPU, com->data.registerCmd.IAmAKelda, com->data.registerCmd.leds, com->data.registerCmd.extentionPort ) {
  
}

SwOSSwarmJST::~SwOSSwarmJST() {
  
  for ( uint8_t i=0; i<MAXSERVOS; i++ ) { if ( servo[i] ) delete servo[i]; }

}

void SwOSSwarmJST::unsubscribe( void ) {
  
  for ( uint8_t i=0; i<MAXSERVOS; i++ ) { if ( servo[i] ) servo[i]->unsubscribe(); }

}

void SwOSSwarmJST::factorySettings( void ) {

  SwOSSwarmXX::factorySettings();
  for (uint8_t i=0; i<MAXSERVOS; i++) { if ( servo[i] ) servo[i]->setAlias( "" ); }
  
}

bool SwOSSwarmJST::cmdAlias( char *device, uint8_t port, const char *alias) {

  // test on my specific hardware
  if      ( ( strcmp(device, "SERVO") == 0 ) && (port < MAXSERVOS ) && (servo[port])) { servo[port]->setAlias(alias); return true; }
  else if ( ( strcmp(device, "GYRO") == 0 )  && (port = 255) && (gyro) )              { gyro->setAlias(alias);        return true; }
  else return false;

}

SwOSIO *SwOSSwarmJST::getIO( const char *name) {

  // check on base class hardware
  SwOSIO *IO = SwOSSwarmXX::getIO(name);
  if ( IO != NULL ) { return IO; }

  // check on specific hardware
  for (uint8_t i=0;i<MAXSERVOS;i++) { if ( (servo[i]) && ( servo[i]->equals(name) ) ) { return servo[i]; } }

  return NULL;

}

SwOSIO *SwOSSwarmJST::getIO( FtSwarmIOType_t ioType, FtSwarmPort_t port) {

  // check on base class hardware
  SwOSIO *IO = SwOSSwarmXX::getIO(ioType, port);
  if ( IO != NULL ) { return IO; }

  if ( ioType == FTSWARM_SERVO) return ( (port<MAXSERVOS)?servo[port]:NULL);

  return NULL;

}

char* SwOSSwarmJST::myType() {
  return (char *) "ftSwarm";
}

FtSwarmControler_t SwOSSwarmJST::getType() {
  return FTSWARM;
}

void SwOSSwarmJST::jsonizeIO( JSONize *json, uint8_t id) {

  SwOSSwarmXX::jsonizeIO(json, id);

  for (uint8_t i=0; i<servos; i++) { if ( servo[i] ) servo[i]->jsonize( json, id ); } 
  if (gyro)  { gyro->jsonize(json, id); }

}

bool SwOSSwarmJST::apiServoOffset( char *id, int offset ) {
  // send a Servo command (from api)

  // search IO
  for (uint8_t i=0; i<MAXSERVOS; i++) {

    if ( (servo[i]) && ( servo[i]->equals(id) ) ) {
      // found
      servo[i]->setOffset( offset, false );
      return true;
    }
  }

  return false;

}

bool SwOSSwarmJST::apiServoPosition( char *id, int position ) {
  // send a Servo command (from api)

  // search IO
  for (uint8_t i=0; i<MAXSERVOS; i++) {

    if ( (servo[i]) && ( servo[i]->equals(id) ) ) {
      // found
      servo[i]->setPosition( position, false );
      return true;
    }
  }

  return false;

}

bool SwOSSwarmJST::OnDataRecv(SwOSCom *com ) {

  if (!com) return false;

  // check if SwOSCrtl knows the cmd
  if ( SwOSSwarmXX::OnDataRecv( com ) ) return true;

  switch ( com->data.cmd ) {
    case CMD_SETSERVO:
      if (servo[com->data.servoCmd.index]) {
        servo[com->data.servoCmd.index]->setOffset( com->data.servoCmd.offset, true );
        servo[com->data.servoCmd.index]->setPosition( com->data.servoCmd.position, true );
      }
      return true;
  }

  return false;

}

void SwOSSwarmJST::_sendAlias( SwOSCom *alias ) {

  SwOSSwarmXX::_sendAlias( alias );

  // servo
  for (uint8_t i=0; i<MAXSERVOS;i++) if (servo[i]) alias->sendBuffered( servo[i]->getName(), servo[i]->getAlias() ); 

}

void SwOSSwarmJST::saveAliasToNVS( nvs_handle_t my_handle ) {

  SwOSSwarmXX::saveAliasToNVS( my_handle );

  if (gyro) gyro->saveAliasToNVS( my_handle );
  
}

void SwOSSwarmJST::loadAliasFromNVS( nvs_handle_t my_handle ) {

  SwOSSwarmXX::loadAliasFromNVS( my_handle );

  if (gyro) gyro->loadAliasFromNVS( my_handle );
  for (uint8_t i=0; i<MAXSERVOS; i++ ) if (servo[i]) servo[i]->loadAliasFromNVS( my_handle );
  
}

/***************************************************
 *
 *   SwOSSwarmControl
 *
 ***************************************************/

SwOSSwarmControl::SwOSSwarmControl( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmAKelda, int16_t zero[2][2], uint8_t displayType ):SwOSSwarmXX( SN, macAddr, local, CPU,  IAmAKelda, FTSWARM_EXT_OFF ) {

  char buffer[32];
  sprintf( buffer, "ftSwarm%d", SN);
  setName( buffer );

  // define specific hardware
  for (uint8_t i=0; i<4; i++) { button[i]   = new SwOSButton("S", i, this); }
  for (uint8_t i=0; i<2; i++) { button[4+i] = new SwOSButton("F", i, this); }
  for (uint8_t i=0; i<2; i++) { button[6+i] = new SwOSButton("J", i, this); }
  for (uint8_t i=0; i<2; i++) { 
    if (zero) joystick[i] = new SwOSJoystick("JOY", i, this, zero[i][0], zero[i][1]); 
    else      joystick[i] = new SwOSJoystick("JOY", i, this, 0, 0 ); 
  }
  hc165 = new SwOSHC165("HC165", this);
  oled  = new SwOSOLED("OLED", this, displayType);

}

SwOSSwarmControl::SwOSSwarmControl( SwOSCom *com ):SwOSSwarmControl( com->data.sourceSN, com->macAddr, false, com->data.registerCmd.versionCPU, com->data.registerCmd.IAmAKelda, NULL, 1 ) {
  
}

SwOSSwarmControl::~SwOSSwarmControl() {
  
  if ( oled  ) delete oled;

}

void SwOSSwarmControl::unsubscribe( void ) {
  
  for ( uint8_t i=0; i<8; i++ ) { if ( button[i] ) button[i]->unsubscribe(); }
  for ( uint8_t i=0; i<2; i++ ) { if ( joystick[i] ) joystick[i]->unsubscribe(); }
  if ( oled  ) oled->unsubscribe();

}

void SwOSSwarmControl::factorySettings( void ) {

  SwOSSwarmXX::factorySettings();
  for (uint8_t i=0; i<8; i++) { if ( button[i] ) button[i]->setAlias( "" ); }
  if (hc165) hc165->setAlias("");

  if (oled)  oled->setAlias("");

}

bool SwOSSwarmControl::cmdAlias( char *device, uint8_t port, const char *alias) {

  // just test on specific hardware
  if      ( ( strcmp(device, "S") == 0 )    && (port < 4) )   { button[port]->setAlias(alias);   return true; }
  else if ( ( strcmp(device, "F") == 0 )    && (port < 2) )   { button[port+4]->setAlias(alias); return true; }
  else if ( ( strcmp(device, "J") == 0 )    && (port < 2) )   { button[port+6]->setAlias(alias); return true; }
  else if ( ( strcmp(device, "JOY") == 0 )  && (port < 2) )   { joystick[port]->setAlias(alias); return true; }

  else if ( ( strcmp(device, "OLED") == 0 ) && (port = 255) ) { oled->setAlias(alias);           return true; }

  else return false;

}

SwOSIO *SwOSSwarmControl::getIO( const char *name ) {

  // check on base base class hardware
  SwOSIO *IO = SwOSSwarmXX::getIO(name);
  if ( IO != NULL ) { return IO; }

  // check on specific hardware
  for (uint8_t i=0;i<8;i++) { if (button[i]->equals(name) )   { return button[i]; } }
  for (uint8_t i=0;i<2;i++) { if (joystick[i]->equals(name) ) { return joystick[i]; } }

  if ( (oled) && ( oled->equals(name) ) ) { return oled; }

  return NULL;

}

SwOSIO *SwOSSwarmControl::getIO( FtSwarmIOType_t ioType, FtSwarmPort_t port) {

  // check on base base class hardware
  SwOSIO *IO = SwOSSwarmXX::getIO(ioType, port);
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

void SwOSSwarmControl::jsonizeIO( JSONize *json, uint8_t id) {

  SwOSSwarmXX::jsonizeIO(json, id);

  for (uint8_t i=0; i<6; i++) { button[i]->jsonize( json, id ); }
  for (uint8_t i=0; i<2; i++) { joystick[i]->jsonize( json, id ); } 
  if (gyro)  { gyro->jsonize(json, id); }

}


void SwOSSwarmControl::read() {

  SwOSSwarmXX::read();

  // get joystick readings
  for (uint8_t i=0; i<2; i++) {
    joystick[i]->read();
  }

  // get buttons via hc165
  hc165->read();

  // transfer result to buttons
  uint8_t v;
  v = hc165->getValue( );
  for (uint8_t i=0; i<8; i++) { button[i]->setState( v & (1<<i), _firstRead ); }

  // _firstRead allows to suppress a toggle event on buttons during startup
  _firstRead = false;

}

void SwOSSwarmControl::setState( SwOSState_t state, uint8_t members, char *SSID ) {
  // visualizes controler's state like booting, error,...


  if (!oled) return;

  // rember old values
  uint8_t sx, sy;
  oled->getTextSize( &sx, &sy );
  int16_t cx, cy;
  oled->getCursor( &cx, &cy );
  
  int16_t w = oled->getWidth();

  // clear status bar
  oled->drawRect( 0, -YELLOWPIXELS, w, YELLOWPIXELS, true, false );

  // status message
  if ( ( state == RUNNING ) && (SSID) ) {
    char _SSID[15];
    strncpy( _SSID, SSID, 14 );
    oled->write( _SSID, w/2, -YELLOWPIXELS, FTSWARM_ALIGNCENTER, false );
  } else {
    oled->write( (char *) OLEDMSG[state], w/2, -YELLOWPIXELS, FTSWARM_ALIGNCENTER, false );
  }

  // members
  if ( members > 0) {
    char m[10];
    sprintf( m, "%d", members );
    oled->write( m, w, -YELLOWPIXELS, FTSWARM_ALIGNRIGHT, false );
  }

  // Kelda
  if (IAmAKelda) oled->write( (char *) "K", 0, -YELLOWPIXELS, FTSWARM_ALIGNLEFT, false );

  // cool line
  oled->drawLine( 0, -5, w, -5, true );

  // restore values
  oled->setCursor( cx, cy );
  oled->setTextSize( sx, sy );

  // show on display
  oled->display();
   
}

SwOSCom *SwOSSwarmControl::state2Com( MacAddr destination ) {

  SwOSCom *com = SwOSSwarmXX::state2Com( destination );

  int16_t FB, LR;
  for (uint8_t i=0; i<2; i++ ) { 
    joystick[i]->getValue( &FB, &LR );
    com->data.stateCmd.FB[i] = FB;
    com->data.stateCmd.LR[i] = LR;
  };
  
  com->data.stateCmd.hc165 = hc165->getValue();

  return com;
}

bool SwOSSwarmControl::recvState( SwOSCom *com ) {

  SwOSSwarmXX::recvState( com );
  
  for (uint8_t i=0; i<2; i++ ) { joystick[i]->setValue( com->data.stateCmd.FB[i], com->data.stateCmd.LR[i] ); };
      
  // set HC165 value & buttons
  uint8_t hc = com->data.stateCmd.hc165;
  hc165->setValue( hc );
  for (uint8_t i=0; i<8; i++) { button[i]->setState( hc & (1<<i) ); }

  return true;
 
} 

bool SwOSSwarmControl::OnDataRecv(SwOSCom *com ) {

  if (!com) return false;

  // check if SwOSCrtl knows the cmd
  if ( SwOSSwarmXX::OnDataRecv( com ) ) return true;

  return false;

}

void SwOSSwarmControl::_sendAlias( SwOSCom *alias ) {

  SwOSSwarmXX::_sendAlias( alias );

  // buttons
  for (uint8_t i=0; i<8;i++) alias->sendBuffered( button[i]->getName(), button[i]->getAlias() ); 

  // joystick
  for (uint8_t i=0; i<2;i++) alias->sendBuffered( joystick[i]->getName(), joystick[i]->getAlias() ); 

  // oled
  if (oled) alias->sendBuffered( oled->getName(), oled->getAlias() ); 

}


void SwOSSwarmControl::saveAliasToNVS( nvs_handle_t my_handle ) {

  SwOSSwarmXX::saveAliasToNVS( my_handle );

  if (oled) oled->saveAliasToNVS( my_handle );

  for (uint8_t i=0; i<8; i++ ) button[i]->saveAliasToNVS( my_handle );
  for (uint8_t i=0; i<2; i++ ) joystick[i]->saveAliasToNVS( my_handle );
  
}


void SwOSSwarmControl::loadAliasFromNVS( nvs_handle_t my_handle ) {

  SwOSSwarmXX::loadAliasFromNVS( my_handle );

  if (oled) oled->loadAliasFromNVS( my_handle );
  
  for (uint8_t i=0; i<8; i++ ) button[i]->loadAliasFromNVS( my_handle );
  for (uint8_t i=0; i<2; i++ ) joystick[i]->loadAliasFromNVS( my_handle );
  
}

boolean SwOSSwarmControl::getRemoteControl( void ) {
  return _remoteControl;
}

void SwOSSwarmControl::setRemoteControl( boolean remoteControl ) {
  _remoteControl = remoteControl;
}


/***************************************************
 *
 *   SwOSSwarmCAM
 *
 ***************************************************/

SwOSSwarmCAM::SwOSSwarmCAM( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmAKelda ):SwOSCtrl( SN, macAddr, local, CPU,  IAmAKelda, FTSWARM_EXT_OFF ) {

  char buffer[32];
  sprintf( buffer, "ftSwarm%d", SN);
  setName( buffer );

  if (local) cam = new SwOSCAM( "CAM", this );

}

SwOSSwarmCAM::SwOSSwarmCAM( SwOSCom *com ):SwOSSwarmCAM( com->data.sourceSN, com->macAddr, false, com->data.registerCmd.versionCPU, com->data.registerCmd.IAmAKelda ) {
 
}

SwOSSwarmCAM::~SwOSSwarmCAM() {

  if (cam) delete cam;
  
}

bool SwOSSwarmCAM::cmdAlias( char *device, uint8_t port, const char *alias) {

  if ( ( strcmp(device, "CAM") == 0 ) && (port = 255) ) { 
    cam->setAlias(alias); 
    return true; 
  }

  return false;

}

SwOSIO *SwOSSwarmCAM::getIO( const char *name ) {

  // check on base base class hardware
  SwOSIO *IO = SwOSCtrl::getIO(name);
  if ( IO != NULL ) { return IO; }

  if ( (cam) && ( cam->equals(name) ) ) { return cam; }

  return NULL;

}

SwOSIO *SwOSSwarmCAM::getIO( FtSwarmIOType_t ioType, FtSwarmPort_t port) {

  // check on base base class hardware
  SwOSIO *IO = SwOSCtrl::getIO(ioType, port);
  if ( IO != NULL ) { return IO; }

  if (ioType == FTSWARM_CAM ) return cam;

  return NULL;
  
}

char* SwOSSwarmCAM::myType() {
  return (char *) "ftSwarmCAM";
}

FtSwarmControler_t SwOSSwarmCAM::getType() {
  return FTSWARMCAM;
}

void SwOSSwarmCAM::jsonizeIO( JSONize *json, uint8_t id) {

  SwOSCtrl::jsonizeIO(json, id);

  if (cam) cam->jsonize( json, id ); 

}


void SwOSSwarmCAM::setState( SwOSState_t state, uint8_t members, char *SSID ) {
  // visualizes controler's state like booting, error,...
   
}


bool SwOSSwarmCAM::OnDataRecv(SwOSCom *com ) {

  if (!com) return false;

  // check if SwOSCrtl knows the cmd
  if ( SwOSCtrl::OnDataRecv( com ) ) return true;

  // ToDO: add CAM Commands

  return false;

}

void SwOSSwarmCAM::saveAliasToNVS( nvs_handle_t my_handle ) {

  SwOSCtrl::saveAliasToNVS( my_handle );
  if (cam) cam->saveAliasToNVS( my_handle );
  
}


void SwOSSwarmCAM::loadAliasFromNVS( nvs_handle_t my_handle ) {

  SwOSCtrl::loadAliasFromNVS( my_handle );
  if (cam) cam->loadAliasFromNVS( my_handle );
  
}


bool SwOSSwarmCAM::apiCAMStreaming( char *id, bool onOff ) {
  // set CAM framzesize / resolution

  if (cam) cam->streaming( onOff, false );
  return true;

}

bool SwOSSwarmCAM::apiCAMFramesize( char *id, int framesize ) {
  // set CAM framzesize / resolution

  if (cam) cam->setFramesize( (framesize_t) framesize, false );
  return true;

}

bool SwOSSwarmCAM::apiCAMQuality( char *id, int quality ) { 
  // set CAM quality

  if (cam) cam->setQuality( quality, false );
  return true;

}

bool SwOSSwarmCAM::apiCAMBrightness( char *id, int brightness ) { 
  // set CAM brightness

  if (cam) cam->setBrightness( brightness, false );
  return true;

}

bool SwOSSwarmCAM::apiCAMContrast( char *id, int contrast ) { 
  // set CAM contrast

  if (cam) cam->setContrast( contrast, false );
  return true;

}

bool SwOSSwarmCAM::apiCAMSaturation( char *id, int saturation ) { 
  // set CAM saturation

  if (cam) cam->setSaturation( saturation, false );
  return true;

}

bool SwOSSwarmCAM::apiCAMSpecialEffect( char *id, int specialEffect ) { 
  // set CAM special effect

  if (cam) cam->setSpecialEffect( specialEffect, false );
  return true;

}

bool SwOSSwarmCAM::apiCAMWbMode( char *id, int wbMode ) { 
  // set CAM wbMode

  if (cam) cam->setWbMode( wbMode, false );
  return true;

}

bool SwOSSwarmCAM::apiCAMVFlip( char *id, bool vFlip ) { 
  // set CAM V-Flip 

  if (cam) cam->setVFlip( vFlip, false );
  return true;

}

bool SwOSSwarmCAM::apiCAMHMirror( char *id, bool hMirror ) { 
  // set CAM H-Mirror

  if (cam) cam->setHMirror( hMirror, false );
  return true;

}
