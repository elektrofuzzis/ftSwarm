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
#include "SwOSHW/SwOSHWDuino.h"
#include "SwOSHW/SwOSHWLocal.h"
#include "SwOSLog.h"

const char EMPTYSTRING[] = "";

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
    UICLASS_MOTOR,
    UICLASS_SERVO,
    UICLASS_NONE
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

SwOSObj::SwOSObj( const char *name, uint8_t flags) {

  _alias = NULL;
  _name = STRDUP( name );

  this->flags = flags;

}

SwOSObj::~SwOSObj() {
  if (_name)  free( _name );
  if (_alias) free( _alias );
}

void SwOSObj::loadFromNVS( nvs_handle_t myHandle ) {

  size_t size = MAXIDENTIFIER;
  char alias[MAXIDENTIFIER];

  if ( ESP_OK == nvs_get_str( myHandle, getName(), alias, &size ) ) {
    setAlias( alias );
  }
}

void SwOSObj::printNVS( nvs_handle_t myHandle ) {

  size_t size = MAXIDENTIFIER;
  char alias[MAXIDENTIFIER];

  if ( ESP_OK == nvs_get_str( myHandle, getName(), alias, &size ) ) {
    printf( "%s alias: %s\n", getName(), alias );
  }

}

void SwOSObj::saveToNVS( nvs_handle_t myHandle ) {

  nvs_set_str( myHandle, getName(), getAlias() );
  
}

void SwOSObj::setAlias( const char *alias ) {

  // no change?
  if ( ( _alias ) && ( strcmp( _alias, alias ) == 0 ) ) return;

  // free memory?
  if ( _alias != NULL ) { free( _alias ); _alias = NULL; }

  // nothing?
  if ( (!alias) || (alias[0]=='\0') ) {
    _alias = NULL;
    return;
  }

  // store
  _alias = STRDUP( alias );

}

void SwOSObj::setName( const char *name ) {

  if (_name) { free( _name ); }
  _name = STRDUP( name );
  
}

bool SwOSObj::equals( const char *name ) {

  if (strcmp(_name, name) == 0 ) { return true; }
  else if ( ( _alias != NULL) && (strcmp(_alias, name) == 0 ) ) { return true; }
  else { return false; }

}

const char* SwOSObj::getName( ) {
  return _name;
}


const char* SwOSObj::getAlias( ) {
  if (!_alias) {
    return (char *) EMPTYSTRING;
  } else {
    return _alias;
  }
}

const char* SwOSObj::getAliasOrName( ) {
  if (!_alias) {
    return _name;
  } else {
    return _alias;
  }
}

void SwOSObj::serialize( Serialize *serialize) {

   // alias
   if (_alias) serialize->item( SERIALIZE_LITERAL_ALIAS, _alias );

   // name
   serialize->item( SERIALIZE_LITERAL_NAME, _name );

}

/***************************************************
 *
 *   SwOSIO - Base class for all IOs.
 *
 ***************************************************/

SwOSIO::SwOSIO( const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType, uint8_t flags ) : SwOSObj( name, flags ) {

  // store local port and controller 
  this->port   = port;
  this->ctrl   = ctrl;
  this->ioType = ioType;

  setName( name );
 
}

SwOSIO::~SwOSIO() {

  #if FTSWARM_HAL_OLEDS > 0
  if ( subscribedScreenIO ) subscribedScreenIO->unregister( this );
  #endif

}

bool SwOSIO::isOnline( void ) { 
  return ctrl->isOnline();
};

SwOSLabel_t SwOSIO::getLabel( void ) {

  if ( ( !ctrl->isLocal() ) || ( !ctrl->hasOLED() ) ) return SWOSLABEL_UNDEF;

  if ( ioType == SWOSIO_BUTTON )        return ( SwOSLabel_t )   port;
  if ( ioType == SWOSIO_JOYSTICK_POTI ) return ( SwOSLabel_t ) ( port - FTSWARM_HAL_FIRSTJPOTI + 8);

  return SWOSLABEL_UNDEF;

}

FtSwarmTriggerParameter SwOSIO::evalOperand( FtSwarmOperand_t v, int32_t sensor, int32_t delta, int32_t actor, FtSwarmTriggerParameter parameter ) {

  FtSwarmTriggerParameter result = { 0 };

  switch (v) {
    case FTSWARM_CONSTANT:    result = parameter;        break;
    case FTSWARM_SENSORVALUE: result.setValue( sensor ); break;
    case FTSWARM_SENSORDELTA: result.setValue( delta );  break;
    case FTSWARM_ACTORVALUE:  result.setValue( actor );  break;
  }

  return result;

}

FtSwarmTriggerParameter SwOSIO::evalTriggerMath( SwOSTriggerMath triggerMath, int32_t sensor, int32_t delta, int32_t actor, FtSwarmTriggerParameter parameter, int32_t minValue, int32_t maxValue ) {

  // get operands
  FtSwarmTriggerParameter v1 = evalOperand( triggerMath.bits.v1, sensor, delta, actor, parameter );
  FtSwarmTriggerParameter v2 = evalOperand( triggerMath.bits.v2, sensor, delta, actor, parameter );

  // calculate
  FtSwarmTriggerParameter r = { 0 };
  switch (triggerMath.bits.op) {
    case FTSWARM_ASSIGN:   r = v1; break;
    case FTSWARM_ADD:      r.setValue( v1.getValue() + v2.getValue() ); break;
    case FTSWARM_SUBTRACT: r.setValue( v1.getValue() - v2.getValue() ); break;
    case FTSWARM_MULTIPLY: r.setValue( v1.getValue() * v2.getValue() ); break;
  }
 
  // check on bounderies
  if ( r.getValue() < minValue ) r.setValue( minValue );
  if ( r.getValue() > maxValue ) r.setValue( maxValue );

  // done
  return r;
  
}

void SwOSIO::loadFromNVS( nvs_handle_t myHandle ) {

  uint8_t blob[MAXIDENTIFIER+3];
  size_t  len = MAXIDENTIFIER+3;

  if (nvs.version == 2) {
    // compatibility to old version
    if ( ESP_OK != nvs_get_str( myHandle, getName(), (char *)&blob[2], &len ) ) return;
    blob[0] = ioType;
    blob[1] = false;

  } else {
    // read ioType & alias in a blob
    if ( ESP_OK != nvs_get_blob( myHandle, getName(), blob, &len ) ) return;

  }

  setAlias( (char *) &blob[2] );
  ctrl->changeIOType( ctrl->getIndex(this), (SwOSIOType_t) blob[0], blob[1] );
    
}

void SwOSIO::printNVS( nvs_handle_t myHandle ) {

  uint8_t blob[MAXIDENTIFIER+3];
  size_t  len = MAXIDENTIFIER+3;

  // read ioType & alias in a blob
  if ( ESP_OK != nvs_get_blob( myHandle, getName(), blob, &len ) ) return;

  printf("%s alias: %s iotype: %d flags: %X\n", getName(), (char *) &blob[2], (SwOSIOType_t) blob[0], blob[1] );

    
}

void SwOSIO::saveToNVS( nvs_handle_t myHandle ) {

  uint8_t blob[MAXIDENTIFIER+3];

  bzero( blob, MAXIDENTIFIER+3 );
  
  uint8_t len = strlen( getAlias() );

  blob[0] = ioType;
  blob[1] = getFlags();
  memcpy( &blob[2], getAlias(), len );

  nvs_set_blob( myHandle, getName(), blob, len+3 );
  
}

void SwOSIO::lock( void ) {
  if (ctrl) ctrl->lock();
}

void SwOSIO::unlock( void ) {
  if (ctrl) ctrl->unlock();
}

SwOSUIClass_t SwOSIO::getUIClass() {

  if ( ioType == SWOSIO_UNDEF ) return UICLASS_NONE;
  return UI_CLASS[getIOType()];

}

void SwOSIO::getUID( SwOSIOUID *uid ) {

  if (!uid) return;

  uid->serialNumber = getCtrl()->serialNumber;
  uid->ioType       = getIOType();
  uid->port         = getPort();

}

void SwOSIO::getUniqueName( char *name ) {

  if (_alias) strcpy( name, _alias );
  else sprintf( name, "%s.%s", ctrl->getAliasOrName(), _name );

}

void SwOSIO::serialize( Serialize *serialize ) {
  SwOSObj::serialize( serialize );
  serialize->item( SERIALIZE_LITERAL_UICLASS, getUIClass() );
  serialize->item( SERIALIZE_LITERAL_IOTYPE, getIOType() );
  serialize->item( SERIALIZE_LITERAL_ACTIVE, ( _alias != NULL ) || isInUse() );
}

void SwOSIO::onTrigger( SwOSTriggerMath triggerMath, int32_t sensor, int32_t delta, FtSwarmTriggerParameter parameter ) {
  SWARM_LOG_ERROR( TRANSLATE( "IO is unable to handle trigger events.", "IO kann Trigger-Events nicht verarbeiten." ) );
}

char *SwOSIO::subscribe( const char *IOName, uint32_t hysteresis ) {

  printf("subscribe %s\n", getName() );

  this->hysteresis = hysteresis;
  isSubscribed = true;

  // only if I don't know my external name, store it
  if (!subscribedIOName) subscribedIOName = STRDUP( IOName );
  
  // return my internal name to outside
  return subscribedIOName;

} 

void SwOSIO::subscribe( FtSwarmScreenIO *screenIO ) {
  subscribedScreenIO = screenIO;
}

void SwOSIO::unsubscribe() {
  isSubscribed = false;
  if (subscribedIOName) free( subscribedIOName );
  subscribedIOName = NULL;
}

void SwOSIO::unsubscribe( FtSwarmScreenIO *screenIO ) {

  // to avoid races, test on same object
  if ( subscribedScreenIO == screenIO ) subscribedScreenIO = NULL;

}

void SwOSIO::setLabelText( char *text ) {

  #if FTSWARM_HAL_OLEDS > 0
  if ( subscribedScreenIO ) subscribedScreenIO->setText( text );
  #endif

}

void SwOSIO::sendEffect( FtSwarmTriggerParameter effect ) {

  SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_SETEFFECT );
  cmd.data.effectCmd.index = ctrl->getIndex(this);
  cmd.data.effectCmd.effect = effect;
  cmd.send( );

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

SwOSEventHandler::SwOSEventHandler( SwOSTriggerMath triggerMath, SwOSIO *actor, FtSwarmTriggerParameter parameter  ) {
  this->triggerMath = triggerMath;
  this->actor       = actor;
  this->parameter   = parameter;

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

SwOSTriggerMath genTriggerMath( FtSwarmTrigger_t triggerEvent, FtSwarmOperator_t op, FtSwarmOperand_t v1, FtSwarmOperand_t v2) {

  SwOSTriggerMath tm;
  tm.bits.trigger = triggerEvent;
  tm.bits.op      = op;
  tm.bits.v1      = v1;
  tm.bits.v2      = v2;

  return tm;

}

bool SwOSEventInput::addEvent( FtSwarmTrigger_t triggerEvent, FtSwarmOperator_t op, FtSwarmOperand_t v1, FtSwarmOperand_t v2, SwOSIO *actor, FtSwarmTriggerParameter parameter ) {

  // first event?
  if (!eventList) {
    eventList = new SwOSEventHandler( genTriggerMath( triggerEvent, op, v1, v2 ), actor, parameter );
    return true;
  }

  // check list
  SwOSEventHandler *e = eventList;
  while (e) {

    // same event, replace parameter 
    if ( ( e->triggerMath.bits.trigger == triggerEvent ) && 
         ( e->triggerMath.bits.op      == op ) && 
         ( e->actor == actor ) ) {
      e->parameter = parameter;
      return true;
    }

    // EOL?
    if (e->next) e = e->next;
    else {
      e->next = new SwOSEventHandler( genTriggerMath( triggerEvent, op, v1, v2 ), actor, parameter );
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

bool SwOSEventInput::deleteEvent( FtSwarmTrigger_t triggerEvent, FtSwarmOperator_t op, SwOSIO *actor ) {

  SwOSEventHandler *e   = eventList;
  SwOSEventHandler *old = NULL;

  // empty list
  if (!eventList) return false;

  // first element fits
  if ( ( eventList->triggerMath.bits.trigger == triggerEvent ) && 
       ( eventList->triggerMath.bits.op      == op ) &&
       ( eventList->actor == actor ) ) {
    old = eventList;
    eventList = eventList->next;
    old->next = NULL;
    delete old;
    return true;
  }

  while (e->next) {

    // just check if the next element is the one to kill
    if ( ( e->next->triggerMath.bits.trigger == triggerEvent ) &&
         ( e->next->triggerMath.bits.op      == op ) &&
         ( e->next->actor == actor ) ) {
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

void SwOSEventInput::trigger( FtSwarmTrigger_t triggerEvent, int32_t sensor, int32_t delta ) {

  SwOSEventHandler *e = eventList;

  while (e) {

    // same trigger type & actor?
    if ( ( e->actor ) && ( e->triggerMath.bits.trigger == triggerEvent ) ) {

      e->actor->onTrigger( e->triggerMath, sensor, delta, e->parameter );

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
  else                           GPIO = INPUT_GPIO[port];

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

uint8_t SwOSInput::pushState8( uint8_t *buffer ) { 

  int8_t value = lastRawValue;
  
  memcpy( buffer, &value, sizeof( value ) );
  return sizeof( value );

};

uint8_t SwOSInput::pushState16( uint8_t *buffer ) { 
  
  int16_t value = lastRawValue;
  
  memcpy( buffer, &value, sizeof( value ) );
  return sizeof( value );

};

uint8_t SwOSInput::pushState32( uint8_t *buffer ) { 
  
  int32_t value = lastRawValue;
  
  memcpy( buffer, &value, sizeof( value ) );
  return sizeof( value );

};

uint8_t SwOSInput::popState8( uint8_t *buffer ) { 

  int8_t newValue;
  
  memcpy( &newValue, buffer, sizeof( newValue ) );
  setReading( newValue, FTSWARM_NOTRIGGER );

  return sizeof( newValue );
  
};

uint8_t SwOSInput::popState16( uint8_t *buffer ) { 

  int16_t newValue;
  
  memcpy( &newValue, buffer, sizeof( newValue ) );
  setReading( newValue, FTSWARM_NOTRIGGER );

  return sizeof( newValue );
  
};

uint8_t SwOSInput::popState32( uint8_t *buffer ) { 

  int32_t newValue;
  
  memcpy( &newValue, buffer, sizeof( newValue ) );
  setReading( newValue, FTSWARM_NOTRIGGER );

  return sizeof( newValue );
  
};

bool SwOSInput::isDirty( void ) {

  dirty--;

  if ( dirty <= 0 ) {
    dirty = 10;
    return true;
  }

  return false;

}

void SwOSInput::setReading( int32_t newValue, FtSwarmTrigger_t secondTriggerEvent ) {

  bool changes = (lastRawValue != newValue);

  // set dirty flag
  if ( changes ) dirty = 0;

  // store new data
  int32_t delta = newValue - lastRawValue;
  lastRawValue  = newValue;  

  if (changes) {

    #if FTSWARM_HAL_OLEDS > 0
    if ( subscribedScreenIO ) subscribedScreenIO->setValue( newValue ) ;

    // trigger event - if not blocked
    if ( !screenManager.blockEvents ) {
    #endif
      // printf("trigger %s\n", getName() );
      this->trigger( FTSWARM_TRIGGERVALUE, newValue, delta ); 
      if ( secondTriggerEvent != FTSWARM_NOTRIGGER ) this->trigger( secondTriggerEvent, newValue, delta ); 
    #if FTSWARM_HAL_OLEDS > 0
    }
    #endif
  
    // send data to subscriber?
    subscription();

  }

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
      serialize->item( SERIALIZE_LITERAL_SENSOR,   sensor );
      serialize->item( SERIALIZE_LITERAL_ACTOR,    actor );
      serialize->item( SERIALIZE_LITERAL_TRIGGER,  e->triggerMath.bits.trigger );
      serialize->item( SERIALIZE_LITERAL_OPERATOR, e->triggerMath.bits.op );
      serialize->item( SERIALIZE_LITERAL_OPERAND1, e->triggerMath.bits.v1 );
      serialize->item( SERIALIZE_LITERAL_OPERAND2, e->triggerMath.bits.v2 );
      serialize->item( SERIALIZE_LITERAL_VALUE,    e->parameter.raw );
      serialize->endObject( );

      e = e->next;

    }

  }

}
