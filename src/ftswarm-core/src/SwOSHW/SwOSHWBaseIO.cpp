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
#include "SwOSHW/SwOSHWDuino.h"
#include "SwOSLog.h"

const char EMPTYSTRING[] = "";

const char IO_ICON[SWOSIO_MAXIOTYPE][20] = 
  { "digital.svg", 
    "switch.svg",
    "reedswitch.svg", 
    "lightbarrier.svg", 
    "button.svg",
    "analog.svg", 
    "voltage.svg", 
    "resistor.svg",
    "ntc.svg",
    "ldr.svg",
    "joystick.svg",
    "motor.svg",
    "xmotor.svg",
    "xmmotor.svg", // todo better icon xmmotor
    "tractor.svg",
    "encoder.svg",
    "lamp.svg",
    "valve.svg",
    "compressor.svg",
    "buzzer.svg",
    "motor.svg", // todo Icon Stepper
    "counter.svg", 
    "rotaryencoder.svg", 
    "frequency.svg",
    "digital.svg", // todo lidar icon
    "cam.svg",
    "servo.svg",
    "pixel.svg",
    "undef.svg",  // no oled icon
    "undef.svg",  // no i2c icon
    "undef.svg",   // todo gyro icon
    "undef.svg",   // no hc165 icon
    "power.svg",
    "colorsensor.svg", 
    "trailsensor.svg",
    "ultrasonic.svg"
  };

SwOSUIClass_t UI_CLASS[SWOSIO_MAXIOTYPE] = 
  { UICLASS_SENSOR, 
    UICLASS_SENSOR,
    UICLASS_SENSOR, 
    UICLASS_SENSOR, 
    UICLASS_SENSOR,
    UICLASS_SENSOR, 
    UICLASS_SENSOR, 
    UICLASS_SENSOR,
    UICLASS_SENSOR,
    UICLASS_SENSOR,
    UICLASS_JOYSTICK,
    UICLASS_MOTOR,
    UICLASS_MOTOR,
    UICLASS_MOTOR,
    UICLASS_MOTOR,
    UICLASS_MOTOR,
    UICLASS_MOTOR, // ToDo Lamp slider positive only
    UICLASS_ONOFF,
    UICLASS_ONOFF,
    UICLASS_ONOFF,
    UICLASS_MOTOR,
    UICLASS_SENSOR,
    UICLASS_SENSOR,
    UICLASS_SENSOR,
    UICLASS_SENSOR,
    UICLASS_CAM,
    UICLASS_SERVO,
    UICLASS_PIXEL,
    UICLASS_NONE,    // no oled 
    UICLASS_NONE,    // no i2c
    UICLASS_NONE,    // todo gyro
    UICLASS_NONE,    // no hc165
    UICLASS_SENSOR, 
    UICLASS_SENSOR,
    UICLASS_SENSOR
  };
  

// reference to local ftPwrDrive
FtPwrDrive *ftPwrDrive = NULL;

// reference to local ftDuino
SwOSDuino *ftDuino = NULL;

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

void SwOSObj::loadFromNVS( nvs_handle_t my_handle ) {

  size_t size = MAXIDENTIFIER;
  char alias[MAXIDENTIFIER];

  if ( ESP_OK == nvs_get_str( my_handle, getName(), alias, &size ) ) {
    setAlias( alias );
  }
}

void SwOSObj::saveToNVS( nvs_handle_t my_handle ) {

  nvs_set_str( my_handle, getName(), getAlias() );
  
}

void SwOSObj::setAlias( const char *alias ) {

  // no change?
  if ( ( _alias ) && ( strcmp( _alias, alias ) == 0 ) ) return;

  // free memory?
  if ( _alias != NULL ) { free( (void*) _alias ); _alias = NULL; }

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
 *   SwOSIO - Base class for all IOs.
 *
 ***************************************************/

SwOSIO::SwOSIO( const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType ) : SwOSObj( name ) {

  // store local port and controller 
  this->port   = port;
  this->ctrl   = ctrl;
  this->ioType = ioType;

  setName( name );
 
}

bool SwOSIO::isOnline( void ) { 
  return ctrl->isOnline();
};

void SwOSIO::loadFromNVS( nvs_handle_t my_handle ) {

  uint8_t blob[MAXIDENTIFIER+2];
  size_t  len = MAXIDENTIFIER+2;

  if (nvs.version == 2) {
    // compatibility to old version
    if ( ESP_OK != nvs_get_str( my_handle, getName(), (char *)&blob[1], &len ) ) return;
    blob[0] = ioType;

  } else {
    // read ioType & alias in a blob
    if ( ESP_OK != nvs_get_blob( my_handle, getName(), blob, &len ) ) return;

  }

  setAlias( (char *) &blob[1] );
  ctrl->changeIOType( ctrl->getIndex(this), (SwOSIOType_t) blob[0] );
    
}

void SwOSIO::saveToNVS( nvs_handle_t my_handle ) {

  uint8_t blob[MAXIDENTIFIER+2];

  bzero( blob, MAXIDENTIFIER+2 );
  
  uint8_t len = strlen( getAlias() );

  blob[0] = ioType;
  memcpy( &blob[1], getAlias(), len );

  nvs_set_blob( my_handle, getName(), blob, len+2 );
  
}


void SwOSIO::lock( void ) {
  if (ctrl) ctrl->lock();
}

void SwOSIO::unlock( void ) {
  if (ctrl) ctrl->unlock();
}

const char *SwOSIO::getIcon() {

  if ( ioType == SWOSIO_UNDEF ) return IO_ICON[0];
  return IO_ICON[ioType];

}

SwOSUIClass_t SwOSIO::getUIClass() {

  if ( ioType == SWOSIO_UNDEF ) return UICLASS_NONE;
  return UI_CLASS[getIOType()];

}

void SwOSIO::jsonize( JSONize *json, uint8_t id) {
  SwOSObj::jsonize(json, id);
  json->variableUI32("type", getUIClass() );
  json->variable("icon", (char *) getIcon() );
  json->variableB( "active", ( _alias != NULL ) || isInUse() );
}

void SwOSIO::onTrigger( int32_t value ) {
  SWARM_LOG_ERROR( "IO is unable to handle trigger events." );
}

char *SwOSIO::subscribe( char *IOName, uint32_t hysteresis ) {
  
  this->hysteresis = hysteresis;
  isSubscribed = true;

  // only if I don't know my external name, store it
  if (!subscribedIOName) {
    subscribedIOName = (char *)malloc( strlen(IOName)+1 );
    strcpy( subscribedIOName, IOName );
  }
  
  // return my internal name to outside
  return subscribedIOName;

} 

void SwOSIO::unsubscribe() {
  isSubscribed = false;
  if (subscribedIOName) free( subscribedIOName );
  subscribedIOName = NULL;
}

/***************************************************
 *
 *   SwOSEventHandler
 *
 ***************************************************/

SwOSEventHandler::~SwOSEventHandler( ) {
  
  if (next) delete next;
  
}

SwOSEventHandler::SwOSEventHandler( FtSwarmTrigger_t triggerEvent, SwOSIO *actor, int32_t parameter  ) {
  this->trigger   = triggerEvent;
  this->actor     = actor;
  this->parameter = parameter;
}

/***************************************************
 *
 *   SwOSEventInput
 *
 ***************************************************/

SwOSEventInput::~SwOSEventInput() {

  if (eventList) delete eventList;

}

void SwOSEventInput::registerEvent( FtSwarmTrigger_t triggerEvent, SwOSIO *actor, int32_t parameter ) {

  // first event?
  if (!eventList) {
    eventList = new SwOSEventHandler( triggerEvent, actor, parameter );
    return;
  }

  // check list
  SwOSEventHandler *e = eventList;
  while (e) {

    // same event, replace parameter 
    if ( ( e->trigger == triggerEvent ) && ( e->actor == actor ) ) {
      e->parameter = parameter;
      e->active = true;
      return;
    }

    // EOL?
    if (e->next) e = e->next;
    else {
      e->next = new SwOSEventHandler( triggerEvent, actor, parameter );
      return;
    }

}
  
}

void SwOSEventInput::unregisterEvent( FtSwarmTrigger_t triggerEvent, SwOSIO *actor ) {

  SwOSEventHandler *e = eventList;

  while (e) {

    if ( ( e->trigger == triggerEvent ) && ( e->actor == actor ) ) e->active = false;
    
    e = e->next;

  }
  
}

void SwOSEventInput::trigger( FtSwarmTrigger_t triggerEvent, int32_t value ) {

  SwOSEventHandler *e = eventList;

  while (e) {

    // same trigger type & actor?
    if ( ( e->actor ) && ( e->trigger == triggerEvent ) ) {

      // send port value? or static parameter ?
      e->actor->onTrigger( (e->trigger == FTSWARM_TRIGGERVALUE)?value:e->parameter );

    }
    
    // next one
    e = e->next;
    
  }

}

/***************************************************
 *
 *   SwOSInput
 *
 ***************************************************/  

void SwOSInput::setupLocal() {
  // initialize local HW

  // ftDuino
  if ( ( ctrl->getCPU() == FTSWARMDUINO_1V141 ) && (ftDuino) ) {
    ftDuino->setIOType( port, ioType );
    return;
  }

  if ( ioType == SWOSIO_BUTTON ) GPIO = GPIO_NUM_NC;
  else                           GPIO = GPIO_INPUT[ctrl->getCPU()][port].io;

  gpio_config_t io_conf = {};

  if ( GPIO != GPIO_NUM_NC) {

    // initialize digital  port
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pin_bit_mask = 1ULL << GPIO;
    gpio_config(&io_conf);

  }
  
} 

void SwOSInput::subscription() {

  // test, if input is subscribed
  if (!isSubscribed) return;

  if ( ( (lastRawValue > lastsubscribedValue) && (lastRawValue-lastsubscribedValue) > hysteresis ) ||
       ( (lastsubscribedValue > lastRawValue ) && (lastsubscribedValue-lastRawValue) > hysteresis ) ) {
       printf("S: %s %d\n", subscribedIOName, lastRawValue);
       lastsubscribedValue = lastRawValue;
  }
}


int32_t SwOSInput::getValueI32() {
  return lastRawValue;
}

float SwOSInput::getValueF() {
  return (float)lastRawValue;
}

uint8_t SwOSInput::pushState( uint8_t *buffer ) { 
  
  memcpy( buffer, &lastRawValue, sizeof( lastRawValue ) );
  return sizeof( lastRawValue );

};

uint8_t SwOSInput::popState( uint8_t *buffer ) { 

  int32_t newValue;

  
  memcpy( &newValue, buffer, sizeof( newValue ) );
  setReading( newValue );

  return sizeof( newValue );
  
};
