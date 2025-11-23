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

const char IO_ICON[SWOSIO_MAXIOTYPE][10] = 
  { "0.svg",          // digital 
    "1.svg",          // switch
    "2.svg",          // reedswitch 
    "3.svg",          // lightbarrier 
    "4.svg",          // button
    "5.svg",          // analog 
    "6.svg",          // voltage 
    "7.svg",          // resistor
    "8.svg",          // ntc
    "9.svg",          // ldr
    "A.svg",          // joystick
    "B.svg",          // motor
    "C.svg",          // xsmotor
    "D.svg",          // xmmotor - todo better icon xmmotor
    "E.svg",          // tractor
    "F.svg",          // encoder
    "G.svg",          // lamp
    "H.svg",          // valve
    "I.svg",          // compressor
    "J.svg",          // buzzer
    "B.svg",          // stepper - todo Icon Stepper
    "K.svg",          // counter 
    "L.svg",          // rotaryencoder 
    "M.svg",          // frequency
    "0.svg",          // todo lidar icon
    "N.svg",          // cam
    "O.svg",          // servo
    "P.svg",          // pixel
    "0.svg",          // no oled icon
    "0.svg",          // no i2c icon
    "0.svg",          // todo gyro icon
    "0.svg",          // no hc165 icon
    "Q.svg",          // power
    "R.svg",          // colorsensor 
    "S.svg",          // trailsensor
    "T.svg",          // ultrasonic
    "0.svg",          // no joystick icon
    "B.svg",          // TODO wheelDrive
    "B.svg",          // TODO MiniMotor
    "B.svg",          // TODO SMotor
    "B.svg",          // TODO PowerMotor
    "B.svg",          // TODO MMotor
    "B.svg"           // TODO RCMotor
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
    UICLASS_SENSOR,
    UICLASS_MOTOR,
    UICLASS_MOTOR,
    UICLASS_MOTOR,
    UICLASS_MOTOR,
    UICLASS_MOTOR,
    UICLASS_MOTOR
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

void SwOSObj::serialize( Serialize *serialize) {

   // display name
   if (_alias) {
     serialize->item( SERIALIZE_LITERAL_NAME, _alias );

   } else {
    serialize->item( SERIALIZE_LITERAL_NAME, _name );

   }

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

void SwOSIO::serialize( Serialize *serialize ) {
  SwOSObj::serialize( serialize );
  serialize->item( SERIALIZE_LITERAL_TYPE, getUIClass() );
  serialize->item( SERIALIZE_LITERAL_ICON, getIcon() );
  serialize->item( SERIALIZE_LITERAL_ACTIVE, ( _alias != NULL ) || isInUse() );
}

void SwOSIO::onTrigger( int32_t value ) {
  SWARM_LOG_ERROR( "IO is unable to handle trigger events." );
}

char *SwOSIO::subscribe( char *IOName, uint32_t hysteresis ) {

  printf("subscribe %s\n", getName() );

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

  if (actor) actor->give();
  if (next)  delete next;
  
}

SwOSEventHandler::SwOSEventHandler( FtSwarmTrigger_t triggerEvent, SwOSIO *actor, int32_t parameter  ) {
  this->trigger        = triggerEvent;
  this->actor          = actor;
  this->parameter      = parameter;

  actor->take();

}

/***************************************************
 *
 *   SwOSEventInput
 *
 ***************************************************/

SwOSEventInput::~SwOSEventInput() {

  if (eventList) delete eventList;

}

bool SwOSEventInput::addEvent( FtSwarmTrigger_t triggerEvent, SwOSIO *actor, int32_t parameter ) {

  // first event?
  if (!eventList) {
    eventList = new SwOSEventHandler( triggerEvent, actor, parameter );
    return true;
  }

  // check list
  SwOSEventHandler *e = eventList;
  while (e) {

    // same event, replace parameter 
    if ( ( e->trigger == triggerEvent ) && ( e->actor == actor ) ) {
      e->parameter = parameter;
      return true;
    }

    // EOL?
    if (e->next) e = e->next;
    else {
      e->next = new SwOSEventHandler( triggerEvent, actor, parameter );
      return true;
    }

  }

  return false;
  
}

void SwOSEventInput::deleteEvents( void ) {

  if ( !eventList ) return;
  
  delete eventList;
  eventList = NULL;

}

bool SwOSEventInput::deleteEvent( FtSwarmTrigger_t triggerEvent, SwOSIO *actor ) {

  SwOSEventHandler *e   = eventList;
  SwOSEventHandler *old = NULL;

  // empty list
  if (!eventList) return false;

  // first element fits
  if ( ( eventList->trigger == triggerEvent ) && ( eventList->actor == actor ) ) {
    old = eventList;
    eventList = eventList->next;
    old->next = NULL;
    delete old;
    return true;
  }

  while (e->next) {

    // just check if the next element is the one to kill
    if ( ( e->next->trigger == triggerEvent ) && ( e->next->actor == actor ) ) {
      old = e->next;
      e->next = e->next->next;
      old->next = NULL;
      delete old;
      return true;
    }
    
    e = e->next;

  }

  return false;
  
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

  // ftPwrDrive
  if ( ( ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (ftPwrDrive) ) {
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

void SwOSInput::setReading( int32_t newValue ) {

  bool changes = (lastRawValue != newValue);
    
  // send changed value event?
  if ( (eventList) && ( changes ) ) trigger( FTSWARM_TRIGGERVALUE, newValue );

  // store new data
  lastRawValue = newValue;  

  if (changes) subscription();

}

void SwOSInput::serializeEvents( Serialize *serialize ) {

  SwOSEventHandler *e = eventList;

  char sensor[2*MAXIDENTIFIER+2];
  char actor[2*MAXIDENTIFIER+2];
  
  // calc my name
  strcpy( sensor, getAlias());
  if (!sensor) sprintf(sensor, "%s.%s", ctrl->getName(), getName() );

  while (e) {

    if (e->actor) {

      // calc actor's name
      strcpy( actor, e->actor->getAlias() );
      if (!actor) sprintf(actor, "%s.%s", e->actor->getCtrl()->getName(), e->actor->getName() );

      serialize->startObject( );
      serialize->item( SERIALIZE_LITERAL_SENSOR, sensor );
      serialize->item( SERIALIZE_LITERAL_ACTOR,  actor );
      serialize->item( SERIALIZE_LITERAL_TRIGGER, e->trigger );
      serialize->item( SERIALIZE_LITERAL_VALUE,   e->parameter );
      serialize->endObject( );

      e = e->next;

    }

  }

}
