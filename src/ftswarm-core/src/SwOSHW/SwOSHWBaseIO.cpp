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

const char EMPTYSTRING[] = "";

FtSwarmIcon_t IO_ICON[SWOSIO_MAXIOTYPE] = 
  { FTSWARM_00_DIGITAL, 
    FTSWARM_02_SWITCH,
    FTSWARM_03_REEDSWITCH, 
    FTSWARM_21_LIGHTBARRIER, 
    FTSWARM_12_BUTTON,
    FTSWARM_01_ANALOG, 
    FTSWARM_04_VOLTAGE, 
    FTSWARM_05_RESISTOR,
    FTSWARM_06_NTC,
    FTSWARM_07_LDR,
    FTSWARM_11_JOYSTICK,
    FTSWARM_13_MOTOR,
    FTSWARM_16_XMOTOR,
    FTSWARM_20_XMMOTOR,
    FTSWARM_17_TRACTOR,
    FTSWARM_18_ENCODER,
    FTSWARM_19_LAMP,
    FTSWARM_23_VALVE,
    FTSWARM_22_COMPRESSOR,
    FTSWARM_24_BUZZER,
    FTSWARM_13_MOTOR,
    FTSWARM_25_COUNTER, 
    FTSWARM_27_ROTARYENCODER, 
    FTSWARM_28_FREQUENCY,
    FTSWARM_00_DIGITAL, // TODO LIDAR Icon
    FTSWARM_26_CAM,
    FTSWARM_14_SERVO,
    FTSWARM_15_RGBLED,
    FTSWARM_XX_UNDEF,  // TODO OLED Icon
    FTSWARM_XX_UNDEF,  // TODO I2C Icon
    FTSWARM_29_GYRO
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

void SwOSIO::lock( void ) {
  if (ctrl) ctrl->lock();
}

void SwOSIO::unlock( void ) {
  if (ctrl) ctrl->unlock();
}

FtSwarmIcon_t SwOSIO::getIcon() {

  if ( ioType == SWOSIO_UNDEF ) return FTSWARM_XX_UNDEF;
  return IO_ICON[ioType];

}

void SwOSIO::jsonize( JSONize *json, uint8_t id) {
  SwOSObj::jsonize(json, id);
  json->variableUI32("type", getIOType() );
  json->variableUI32("icon", getIcon() );
  json->variableB( "active", ( _alias != NULL ) || isInUse() );
}

void SwOSIO::onTrigger( int32_t value ) {
  ESP_LOGE( LOGFTSWARM, "IO is unable to handle trigger events." );
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

SwOSEventHandler::SwOSEventHandler( ) {
  actor        = NULL;
  parameter    = 0;
  usePortValue = true;
}

SwOSEventHandler::SwOSEventHandler( SwOSIO *actor, boolean usePortValue, int32_t parameter ) {
  this->actor        = actor;
  this->usePortValue = usePortValue;
  this->parameter    = parameter;
}

void SwOSEventHandler::trigger( int32_t portValue ) {

  if ( actor ) {
    if (usePortValue) actor->onTrigger( portValue );
    else              actor->onTrigger( parameter );
  }
}

/***************************************************
 *
 *   SwOSEventHandlers
 *
 ***************************************************/

SwOSEventHandlers::SwOSEventHandlers( ) {

  for (uint8_t i=0; i<FTSWARM_MAXTRIGGER; i++ ) event[i] = NULL;

};

SwOSEventHandlers::~SwOSEventHandlers() {

  for (uint8_t i=0; i<FTSWARM_MAXTRIGGER; i++ ) {
    if (event[i]) delete event[i];
  }
  
}

void SwOSEventHandlers::registerEvent( FtSwarmTrigger_t triggerEvent, SwOSIO *actor, boolean usePortValue, int32_t parameter ) {

  // if there is already a registered event, delete it
  if (event[triggerEvent]) delete event[triggerEvent];

  event[triggerEvent] = new SwOSEventHandler( actor, usePortValue, parameter );
  
};

void SwOSEventHandlers::unregisterEvent( FtSwarmTrigger_t triggerEvent ) {

  // if there is already a registered event, delete it
  if (event[triggerEvent]) delete event[triggerEvent];

  event[triggerEvent] = NULL;
  
};

void SwOSEventHandlers::trigger( FtSwarmTrigger_t triggerEvent, int32_t portValue ) {

  if ( event[triggerEvent] ) {
    event[triggerEvent]->trigger( portValue );
  }
}

/***************************************************
 *
 *   SwOSEventInput
 *
 ***************************************************/

SwOSEventInput::~SwOSEventInput() {
  if (!events) delete events;
}

void SwOSEventInput::registerEvent( FtSwarmTrigger_t triggerEvent, SwOSIO *actor, boolean usePortValue, int32_t p1 ) {

  if (!events) events = new SwOSEventHandlers( );

  events->registerEvent( triggerEvent, actor, usePortValue, p1 );
  
}

void SwOSEventInput::unregisterEvent( FtSwarmTrigger_t triggerEvent ) {

  if (!events) return;

  events->unregisterEvent( triggerEvent );
  
}

void SwOSEventInput::trigger( FtSwarmTrigger_t triggerEvent, int32_t portValue ) {

  if ( events ) events->trigger( triggerEvent, portValue );

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
