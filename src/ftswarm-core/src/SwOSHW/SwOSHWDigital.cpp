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
#include "SwOSHW/SwOSHWLocal.h"


/***************************************************
 *
 *   SwOSDigitalInput
 *
 ***************************************************/

 SwOSDigitalInput::SwOSDigitalInput(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType, uint8_t flags ) : SwOSInput( name, port, ctrl, ioType, flags ) {
  
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

  // ftPwrDrive
  if ( ( ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (ftPwrDrive) ) {
    return;
  }
  
  if ( ioType == SWOSIO_BUTTON ) return; // all done

  gpio_config_t io_conf = {};
  
  // initialize A2 pullup
  #if PUA2 != GPIO_NUM_NC
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pin_bit_mask = 1ULL << PUA2;
    gpio_config(&io_conf);
    gpio_set_level( (gpio_num_t) PUA2, 0 );
  #endif
  
  // initialize A1 pulldown
  #if USTX != GPIO_NUM_NC
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << USTX;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    gpio_set_level( (gpio_num_t) USTX, 0 );
  #endif

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

void SwOSDigitalInput::operate() {
  
  // no work on remote sensors
  if (!ctrl->isLocal()) return;

  // ftPwrDrive
  if ( ( ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && ( ftPwrDrive ) ) { 
    
    if (port == SWOS_NOPORT) 
      setReading( ( ftPwrDrive->lastState[0] & EMERCENCYSTOP ) > 0, FTSWARM_NOTRIGGER ); 
    else                     
      setReading( ( ftPwrDrive->lastState[port] & ENDSTOP ) > 0, FTSWARM_NOTRIGGER ); 

    return;
  }

  // ftDuino
  if ( ( ctrl->getCPU() == FTSWARMDUINO_1V141 ) && ( ftDuino ) ) { 
    setReading( ftDuino->input[port], FTSWARM_NOTRIGGER ); 
    return; 
  }

  if ( ioType == SWOSIO_BUTTON ) {
    #if FTSWARM_HAL_HC165 > 0
    if (hc165) setReading( ( ( hc165->getValue( ) & (1<<port) ) >0 ), FTSWARM_NOTRIGGER );
    #endif
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

  setReading( newValue, FTSWARM_NOTRIGGER );

}

void SwOSDigitalInput::setReading( int32_t newValue, FtSwarmTrigger_t secondTriggerEvent ) {

  bool changes = (lastRawValue != newValue);
  FtSwarmTrigger_t trigger = FTSWARM_NOTRIGGER;

  // check if it's toggled?
  if ( changes ) { 
  
    if (newValue) { toggle = FTSWARM_TOGGLEUP;   trigger = FTSWARM_TRIGGERUP;   }
    else          { toggle = FTSWARM_TOGGLEDOWN; trigger = FTSWARM_TRIGGERDOWN; }

  }

  SwOSInput::setReading( newValue, trigger );

}

FtSwarmToggle_t SwOSDigitalInput::getToggle() {
  // check toggle state and reset it

  FtSwarmToggle_t rtoggle = toggle;
  toggle = FTSWARM_NOTOGGLE;
  
  return rtoggle;

}

void SwOSDigitalInput::serialize( Serialize *serialize ) {

  serialize->startObject( );
  SwOSIO::serialize( serialize );
  serialize->item( SERIALIZE_LITERAL_VALUE, getValueI32() ); 
  serialize->endObject();
  
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
