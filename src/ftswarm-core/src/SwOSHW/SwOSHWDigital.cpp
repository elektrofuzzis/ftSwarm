/*
 * SwOSHWDigital.h
 *
 * Digital input hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#include "SwOSHW/SwOSHWDigital.h"
#include "SwOSHW/SwOSHWBaseCtrl.h"
#include "SwOSHW/SwOSHWHAL.h"

 /***************************************************
 *
 *   SwOSDigitalInput
 *
 ***************************************************/

 SwOSDigitalInput::SwOSDigitalInput(const char *name, uint8_t port, SwOSCtrl *ctrl ) : SwOSInput( name, port, ctrl, FTSWARM_DIGITAL ) {
  
  // initialize local HW
  if (_ctrl->isLocal()) {
    if ( _ctrl->isI2CSwarmCtrl() ) {
      // _setupI2C();
    } else {
      _setupLocal();
    }
  }

}

void SwOSDigitalInput::_setupLocal() {
  // initialize local HW

  SwOSInput::_setupLocal( );

  // local init
  _PUA2         = GPIO_NUM_NC;
  _USTX         = GPIO_NUM_NC;

  if ( _port == 0 ) _USTX = USTCPUA[_ctrl->getCPU()][0];
  if ( ( _ctrl->getType() == FTSWARM ) && ( _port == 1 ) ) { _PUA2 = USTCPUA[_ctrl->getCPU()][1]; }      

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

