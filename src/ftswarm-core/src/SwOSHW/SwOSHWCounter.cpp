/*
 * SwOSHWCounter.cpp
 *
 * Counter inputs hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOSHW/SwOSHWCounter.h"
#include "SwOSHW/SwOSHWBaseCtrl.h"
#include "SwOSHW/SwOSHWHAL.h"
 
 /***************************************************
 *
 *   SwOSCounter
 *
 ***************************************************/

 SwOSCounter::SwOSCounter(const char *name, uint8_t port1, uint8_t port2, SwOSCtrl *ctrl ) : SwOSInput( name, port1, ctrl, FTSWARM_COUNTER ) {

  _portControl = port2;

  // rotary?
  if ( port2 < SWOS_NOPORT ) _sensorType = FTSWARM_ROTARYENCODER;
  
  // initialize local HW
  if ( _ctrl->isLocal() ) _setupLocal();

}

void SwOSCounter::_setupLocal() {
  // initialize local HW

  // setup _GPIO / Counter Input
  SwOSInput::_setupLocal( );

  // setup _CONTROL Input if needed.
  // counter: _portControl = SWOS_NOPORT -> no _CONTROL
  // encode:  if counter post is the highest input port, _portControl = _ctrl->inputs  -> no _CONTROL
  if ( _portControl < _ctrl->inputs ) { 

    _CONTROL = (gpio_num_t) GPIO_INPUT[_ctrl->getCPU()][_portControl].io;

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

FtSwarmIOType_t SwOSCounter::getIOType() { 

  if ( _sensorType == FTSWARM_ROTARYENCODER )
    return FTSWARM_ROTARYINPUT;
  else
    return FTSWARM_COUNTERINPUT; 
};


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
    cmd.data.counterCmd.index = _port;
    cmd.send( );

  }

  _lastRawValue = 0;

  subscription();

}

void SwOSCounter::setValue( int32_t value ) {

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

void SwOSCounter::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variableUI32("sensorType", _sensorType);
  json->variableUI32("subType", _sensorType);
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

void SwOSFrequencymeter::setValue( int32_t value ) {

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

void SwOSFrequencymeter::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variableUI32("sensorType", _sensorType);
  json->variableUI32("subType", _sensorType );
  json->variableI32("value", getValueI32() );
  json->endObject();
}