/*
 * SwOSBaseIO.cpp
 *
 * Basic classes for hardware impelmentation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOS.h"

#include "SwOSHW/SwOSHWBaseCtrl.h"
#include "SwOSHW/SwOSHWHAL.h"

const char IOTYPE[FTSWARM_MAXIOTYPE][15] = { 
  "INPUT", 
  "DIGITALINPUT", 
  "ANALOGINPUT", 
  "ACTOR", 
  "BUTTON", 
  "JOYSTICK", 
  "LED", 
  "SERVO", 
  "OLED", 
  "GYRO", 
  "HC165", 
  "I2C", 
  "CAM",  
  "COUNTER", 
  "COUNTER", 
  "COUNTER" };

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

// reference to local ftPwrDrive
ftPwrDrive *pwrDrive = NULL;

// reference to local ftDuino
FtDuino *ftDuino = NULL;

#define FTDUINOADDR               0x20
#define FTDUINO_CMD_READ          0x00
#define FTDUINO_CMD_SETSENSORTYPE 0x01
#define FTDUINO_CMD_SETACTORTYPE  0x02

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

  // store local port and controller 
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

void SwOSIO::lock( void ) {
  if (_ctrl) _ctrl->lock();
}

void SwOSIO::unlock( void ) {
  if (_ctrl) _ctrl->unlock();
}

void SwOSIO::jsonize( JSONize *json, uint8_t id) {
  SwOSObj::jsonize(json, id);
  json->variable("type", (char *) IOTYPE[getIOType()]);
  json->variable("icon", (char *) getIcon() );
  json->variableB( "active", ( _alias != NULL ) || isInUse() );
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
