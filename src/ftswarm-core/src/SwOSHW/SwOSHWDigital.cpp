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

// local HC165
SwOSHC165 *hc165 = NULL;

/***************************************************
 *
 *   SwOSDigitalInput
 *
 ***************************************************/

 SwOSDigitalInput::SwOSDigitalInput(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType ) : SwOSInput( name, port, ctrl, ioType ) {
  
  if ( ioType == SWOSIO_BUTTON ) normallyOpen = false;

  // initialize local HW
  if (ctrl->isLocal()) {
      setupLocal();
  }

}

void SwOSDigitalInput::setupLocal() {
  // initialize local HW

  SwOSInput::setupLocal( );

  // ftDuino
  if ( ( ctrl->getCPU() == FTSWARMDUINO_1V141 ) && (ftDuino) ) {
    ftDuino->setIOType( port, ioType );
    return;
  }

  if ( ioType == SWOSIO_BUTTON ) return; // all done

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

  // set A1 pullup if available
  if ( ( PUA2 != GPIO_NUM_NC ) && ( ioType != SWOSIO_ULTRASONIC ) ) {
    gpio_set_level( (gpio_num_t) PUA2, true );
  }

}

bool SwOSDigitalInput::isGPIOInput( void ) { 
  
  return ( ( ctrl->getCPU() != FTSWARMDUINO_1V141 ) && 
           ( ctrl->getCPU() != FTSWARMPWRDRIVE_1V141 ) && 
           ( ioType != SWOSIO_BUTTON ) 
         );

};

void SwOSDigitalInput::read() {
  
  // no work on remote sensors
  if (!ctrl->isLocal()) return;

  // ftDuino?
  if ( ( ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && ( ftPwrDrive ) ) { setReading( ftDuino->input[port] ); return; }
  if ( ( ctrl->getCPU() == FTSWARMDUINO_1V141 )    && ( ftDuino ) )    { setReading( ftDuino->input[port] ); return; }

  if ( ioType == SWOSIO_BUTTON ) {
    if (hc165) setReading( ( ( hc165->getValue( ) & (1<<port) ) >0 ) );
    return;
  }

  // GPIO-based

  // existing port?
  if (GPIO == GPIO_NUM_NC ) return;

  uint32_t newValue;

  // read new data
  newValue = gpio_get_level( (gpio_num_t) GPIO );

  // normally open: change logic
  if (normallyOpen) newValue = 1-newValue;

  setReading( newValue );

}

void SwOSDigitalInput::setReading( int32_t newValue ) {

  bool changes = (lastRawValue != newValue);

  // check if it's toggled?
  if ( changes ) { 
    if (newValue) { toggle = FTSWARM_TOGGLEUP;   trigger( FTSWARM_TRIGGERUP, newValue ); }
    else          { toggle = FTSWARM_TOGGLEDOWN; trigger( FTSWARM_TRIGGERDOWN, newValue ); }
    trigger( FTSWARM_TRIGGERVALUE, newValue );
  }

  // store new data
  lastRawValue = newValue;  

  // subscription only if needed
  if ( changes ) subscription();

}

void SwOSDigitalInput::setValue( int32_t value ) {

  // no work on real local HW
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
  json->variableI32("value", getValueI32() ); 
  json->endObject();
  
}

void SwOSDigitalInput::setParameter( int32_t parameter ) {

  if ( ctrl->isLocal() ) {
    
    this->normallyOpen = (bool) parameter;

  } else {

    // send 
    SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_SETPARAMETER );
    cmd.data.parameterCmd.index     = ctrl->getIndex( this );
    cmd.data.parameterCmd.parameter = parameter;
    cmd.send( );

  }

}

/***************************************************
 *
 *   SwOSHC165
 *
 ***************************************************/

SwOSHC165::SwOSHC165(const char *name, SwOSCtrl *ctrl) : SwOSIO(name, ctrl, SWOSIO_HC165 ) {

  // initialize local HW
  if (ctrl->isLocal()) setupLocal();

}

void SwOSHC165::setupLocal() {
  // initialize local HW

  switch ( ctrl->getCPU() ) {
    case FTSWARMCONTROL_1V3:  CS   = GPIO_NUM_14;
                              LD   = GPIO_NUM_15;
                              CLK  = GPIO_NUM_12;
                              MISO = GPIO_NUM_35;
                              break;

    default:                  CS = LD = CLK = MISO = GPIO_NUM_NC;
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

  // no work on remote HW
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

