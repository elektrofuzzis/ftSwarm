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
  if (ctrl->isLocal()) {
    if ( ctrl->isI2CSwarmCtrl() ) {
      // _setupI2C();
    } else {
      setupLocal();
    }
  }

}

void SwOSDigitalInput::setupLocal() {
  // initialize local HW

  SwOSInput::setupLocal( );

  // local init
  PUA2         = GPIO_NUM_NC;
  USTX         = GPIO_NUM_NC;

  if ( port == 0 ) USTX = USTCPUA[ctrl->getCPU()][0];
  if ( ( ctrl->getType() == FTSWARM ) && ( port == 1 ) ) { PUA2 = USTCPUA[ctrl->getCPU()][1]; }      

  gpio_config_t io_conf = {};
  
  // initialize A2 pullup
  if (PUA2 != GPIO_NUM_NC) {
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pin_bit_mask = 1ULL << PUA2;
    gpio_config(&io_conf);
    gpio_set_level( (gpio_num_t) PUA2, 0 );
  }

  // initialize A1 pulldown
  if (USTX != GPIO_NUM_NC) {
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << USTX;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    gpio_set_level( (gpio_num_t) USTX, 0 );
  }

}

void SwOSDigitalInput::setSensorType( FtSwarmSensor_t sensorType, bool normallyOpen ) {

  // due to send norallyOpen to remote controllers, don't call super class

  this->sensorType   = sensorType;
  this->normallyOpen = normallyOpen;

  if (ctrl->isLocal()) { 
    
    setSensorTypeLocal( sensorType );

  } else {

    // send SN, SETSENSORTYPE, port, sensorType
    SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_SETSENSORTYPE );
    cmd.data.sensorCmd.index        = port;
    cmd.data.sensorCmd.sensorType   = sensorType;
    cmd.data.sensorCmd.normallyOpen = normallyOpen;
    cmd.send( );

  }

}

void SwOSDigitalInput::setSensorTypeLocal( FtSwarmSensor_t sensorType ) {

  // set A1 pullup if available
  if ( ( PUA2 != GPIO_NUM_NC ) && ( sensorType != FTSWARM_ULTRASONIC ) ) {
    gpio_set_level( (gpio_num_t) PUA2, true );
  }

  // ftDuino
  if ( ( ctrl->getCPU() == FTSWARMDUINO_1V141 ) && (ftDuino) ) ftDuino->setSensorType( port, sensorType );

}

void SwOSDigitalInput::read() {
  
  // nothing todo on remote sensors
  if (!ctrl->isLocal()) return;

  // i2c sensor is read via a block control by <controller>.read
  if (ctrl->isI2CSwarmCtrl()) return; 

  // existing port?
  if (GPIO == GPIO_NUM_NC ) return;

  uint32_t newValue;

  // read new data
  newValue = gpio_get_level( (gpio_num_t) GPIO );

  setReading( newValue );

}

void SwOSDigitalInput::setReading( int32_t newValue ) {
    
  // normally open: change logic
  if (normallyOpen) newValue = 1-newValue;
    
  // check if it's toggled?
  if ( lastRawValue != newValue ) { 
    if (newValue) { toggle = FTSWARM_TOGGLEUP;   trigger( FTSWARM_TRIGGERUP, newValue ); }
    else          { toggle = FTSWARM_TOGGLEDOWN; trigger( FTSWARM_TRIGGERDOWN, newValue ); }
  }

  // send changed value event?
  if ( (events) && ( lastRawValue != newValue ) ) trigger( FTSWARM_TRIGGERVALUE, newValue );

  // store new data
  lastRawValue = newValue;  

  subscription();

}

void SwOSDigitalInput::setValue( int32_t value ) {

  // nothing ToDo on real local HW
  if ( ( ctrl->isLocal()) && (!ctrl->isI2CSwarmCtrl() ) ) return;

  // check if it's toggled?
  if ( lastRawValue != value) { 

    if   (value) { toggle = FTSWARM_TOGGLEUP;   trigger( FTSWARM_TRIGGERUP, value ); }
    else         { toggle = FTSWARM_TOGGLEDOWN; trigger( FTSWARM_TRIGGERDOWN, value ); }

    // trigger value event
    trigger( FTSWARM_TRIGGERVALUE, value );
    
  }
  
  lastRawValue = value;

  subscription();

}

FtSwarmToggle_t SwOSDigitalInput::getToggle() {
  // check toggle state and reset it

  FtSwarmToggle_t rtoggle = toggle;
  toggle = FTSWARM_NOTOGGLE;
  
  return rtoggle;

}

void SwOSDigitalInput::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variableUI32("sensorType", sensorType);
  json->variableUI32("subType", sensorType);

  json->variableI32("value", getValueI32() );
  
  json->endObject();
}

/***************************************************
 *
 *   SwOSButton
 *
 ***************************************************/

 SwOSButton::SwOSButton(const char *name, uint8_t port, SwOSCtrl *ctrl) : SwOSIO( name, port, ctrl ), SwOSEventInput( ) {

  toggle = FTSWARM_NOTOGGLE;

}

void SwOSButton::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variableB("state", lastState );
  json->endObject();
}

void SwOSButton::setState( bool state, bool clearToggle ) {
  
  if ( state != lastState ) {

    if (state) { 
      toggle = FTSWARM_TOGGLEUP; 
      if ( ctrl->isLocal() ) trigger( FTSWARM_TRIGGERUP, state );
    
    } else {
      toggle = FTSWARM_TOGGLEDOWN;
      if ( ctrl->isLocal() ) trigger( FTSWARM_TRIGGERDOWN, state );
    }
  }

  if (clearToggle) toggle = FTSWARM_NOTOGGLE;
  
  lastState = state; 

  };

bool SwOSButton::getState() { 
  return lastState;
};

FtSwarmToggle_t SwOSButton::getToggle() {

  FtSwarmToggle_t rtoggle = toggle;
  toggle = FTSWARM_NOTOGGLE;

  return rtoggle;

}

/***************************************************
 *
 *   SwOSHC165
 *
 ***************************************************/

SwOSHC165::SwOSHC165(const char *name, SwOSCtrl *ctrl) : SwOSIO(name, ctrl) {

  // initialize local HW
  if (ctrl->isLocal()) setupLocal();

}

void SwOSHC165::setupLocal() {
  // initialize local HW

  switch ( ctrl->getCPU() ) {
    case FTSWARMJST_1V0: CS   = GPIO_NUM_19;
                      LD   = GPIO_NUM_18;
                      CLK  = GPIO_NUM_14;
                      MISO = GPIO_NUM_12;
                      break; 
    case FTSWARMCONTROL_1V3: CS   = GPIO_NUM_14;
                      LD   = GPIO_NUM_15;
                      CLK  = GPIO_NUM_12;
                      MISO = GPIO_NUM_35;
                      break;
    default:          CS = LD = CLK = MISO = GPIO_NUM_NC;
                      return;
  }

  // initialize ports
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  io_conf.pin_bit_mask = (1ULL<<CS) | (1ULL<<LD) | (1ULL<<CLK) ;
  gpio_config(&io_conf);

  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pin_bit_mask = 1ULL<<MISO ;
  gpio_config(&io_conf);

  // set levels
  gpio_set_level( CS, 1 );
  gpio_set_level( LD, 1 );
  gpio_set_level( CLK, 1 );

}

void SwOSHC165::read( ) {

  // remote HW nothing todo
  if (!ctrl->isLocal()) return;

  // invalid configuration?
  if (LD == GPIO_NUM_NC ) {
    return;
  }

  // parallel load
  gpio_set_level( LD, 0 );
  gpio_set_level( LD, 1 );

  // enable
  gpio_set_level( CS, 0 );

  // load
  lastValue = 0;
  for ( uint8_t i=0; i<8; i++ ) {

    // get value
    lastValue = ( lastValue << 1 ) | (!gpio_get_level( MISO ));

    // one tick
    gpio_set_level( CLK, 0 );
    gpio_set_level( CLK, 1 );

  }

}

