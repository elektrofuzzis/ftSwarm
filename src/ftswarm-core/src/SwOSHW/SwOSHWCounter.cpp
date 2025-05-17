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

 SwOSCounter::SwOSCounter(const char *name, uint8_t port1, uint8_t port2, SwOSCtrl *ctrl ) : SwOSInput( name, port1, ctrl, SWOSIO_COUNTER ) {

  portControl = port2;

  // rotary?
  if ( port2 < SWOS_NOPORT ) ioType = SWOSIO_ROTARYENCODER;
  
  // initialize local HW
  if ( ctrl->isLocal() ) setupLocal();

}

void SwOSCounter::setupLocal() {
  // initialize local HW

  // setup _GPIO / Counter Input
  SwOSInput::setupLocal( );

  // setup _CONTROL Input if needed.
  // counter: portControl = SWOS_NOPORT -> no _CONTROL
  // encode:  if counter post is the highest input port, portControl = ctrl->inputs  -> no _CONTROL
  if ( portControl < MAXIOS[ ctrl->getCPU() ].inputs ) { 

    CONTROL = (gpio_num_t) GPIO_INPUT[ctrl->getCPU()][portControl].io;

    gpio_config_t io_conf = {};

    if ( CONTROL != GPIO_NUM_NC) {

      // initialize digital  port
      io_conf.intr_type    = GPIO_INTR_DISABLE;
      io_conf.mode         = GPIO_MODE_INPUT;
      io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
      io_conf.pull_up_en   = GPIO_PULLUP_DISABLE;
      io_conf.pin_bit_mask = 1ULL << CONTROL;
      gpio_config(&io_conf);

    }

  }

  // Calculate used unit
  unit = pcnt_unit_t( port );

  // configure Channel 0
  pcnt_config_t pcnt_config = {
    // Set PCNT input signal and control GPIOs
    .pulse_gpio_num = GPIO,
    .ctrl_gpio_num  = CONTROL,
    // What to do when control input is low or high?
    .lctrl_mode = PCNT_MODE_KEEP, // Reverse counting direction if low
    .hctrl_mode = PCNT_MODE_KEEP,    // Keep the primary counter mode if high
    // What to do on the positive / negative edge of pulse input?
    .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge
    .neg_mode = PCNT_COUNT_DIS,   // Keep the counter value on the negative edge
    .unit = this->unit,
    .channel = PCNT_CHANNEL_0,
    };
  
  // rotary encoder?
  if ( CONTROL != GPIO_NUM_NC) {

    // 1st Channel
    pcnt_config.lctrl_mode = PCNT_MODE_REVERSE, // Reverse counting direction if low
    pcnt_config.pos_mode = PCNT_COUNT_DEC;
    pcnt_config.neg_mode = PCNT_COUNT_INC;
    pcnt_unit_config(&pcnt_config);

    // 2nd Channel
    pcnt_config.pulse_gpio_num = CONTROL;
    pcnt_config.ctrl_gpio_num = GPIO;
    pcnt_config.channel = PCNT_CHANNEL_1;
    pcnt_config.pos_mode = PCNT_COUNT_INC;
    pcnt_config.neg_mode = PCNT_COUNT_DEC;
    pcnt_unit_config(&pcnt_config);

  } else {
    // simple counter
    pcnt_unit_config(&pcnt_config);
  }
  
  /* Configure and enable the input filter */
  pcnt_set_filter_value(unit, 100);
  pcnt_filter_enable(unit);

  /* Initialize PCNT's counter */
  pcnt_counter_pause(unit);
  pcnt_counter_clear(unit);

  /* Everything is set up, now go to counting */
  pcnt_counter_resume(unit);

}

void SwOSCounter::read( void ) {

  // nothing todo on remote sensors
  if ( !ctrl->isLocal() ) return;

  // i2c sensor is read via a block control by <controller>.read
  if ( ctrl->isI2CSwarmCtrl() ) return; 

  // just in case hw isn't initialized
  if ( unit == PCNT_UNIT_MAX ) return;

  int16_t newValue;
  pcnt_get_counter_value( unit, &newValue );

  // store new data
  lastRawValue = newValue;  

  subscription();

}

void SwOSCounter::resetCounter( void ) {

  if ( ctrl->isLocal() ) {

    pcnt_counter_clear( unit );

  } else {

    SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_RESETCOUNTER );
    cmd.data.counterCmd.index = port;
    cmd.send( );

  }

  lastRawValue = 0;

  subscription();

}

void SwOSCounter::setValue( int32_t value ) {

  // nothing ToDo on real local HW
  if ( ( ctrl->isLocal()) && (!ctrl->isI2CSwarmCtrl() ) ) return;

  // check if it's toggled?
  if ( lastRawValue != value) { 

    // trigger value event
    trigger( FTSWARM_TRIGGERVALUE, value );
    
  }
  
  lastRawValue = value;

  subscription();

}

void SwOSCounter::jsonize( JSONize *json, uint8_t id) {

  json->startObject();
  SwOSIO::jsonize(json, id);
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
  int64_t       t         = esp_timer_get_time();

  xQueueSendFromISR(freqQueue, &t, NULL);

}

SwOSFrequencymeter::SwOSFrequencymeter(const char *name, uint8_t port1, uint8_t port2, SwOSCtrl *ctrl ) : SwOSInput( name, port1, ctrl, SWOSIO_FREQUENCYMETER ) {

  portControl = port2;
  
  // initialize local HW
  if ( ctrl->isLocal() ) setupLocal();

}

SwOSFrequencymeter::~SwOSFrequencymeter( ) {
  
  if ( freqQueue ) {
    gpio_isr_handler_remove( GPIO );
    vQueueDelete( freqQueue );
    freqQueue = NULL;
  }

}

void SwOSFrequencymeter::setupLocal() {
  // initialize local HW

  // setup _GPIO / Counter Input
  SwOSInput::setupLocal( );

  // local variables
  freqQueue = xQueueCreate(10, sizeof(int64_t));
  lastTick = esp_timer_get_time();

  // setup interupt handling
  gpio_set_intr_type( GPIO, GPIO_INTR_POSEDGE );
  gpio_install_isr_service( 0 );
  gpio_isr_handler_add( GPIO, freq_isr_handler, (void*) freqQueue );

}

void SwOSFrequencymeter::read( void ) {

  // nothing todo on remote sensors
  if (!ctrl->isLocal()) return;

  // i2c sensor is read via a block control by <controller>.read
  if (ctrl->isI2CSwarmCtrl()) return; 

  // not initialized?
  if ( !freqQueue ) return;

  int64_t tick, dt, dt1;
  bool hasEvents = false;
  
  // now read the latest data
  while ( xQueueReceive( freqQueue, &tick, 0 ) == pdTRUE ) {
    dt1 = tick - lastTick;
    if ( dt1 > 1000LL )  {
      dt = dt1;
      lastTick  = tick;
      hasEvents = true;
    }
  }

  if ( (hasEvents) && (dt != 0LL) ) {
    // I got some data out of the queue, so I calc the frequency
    float newValueF = 1/( ((float)dt) / 1000000.0 );
    lastRawValue = newValueF; 
  
  } else if ( esp_timer_get_time() - lastTick > 1000000LL ) {
    // no tick for more than a second
    lastRawValue = 0;
  }

  subscription();

}

void SwOSFrequencymeter::setValue( int32_t value ) {

  // nothing ToDo on real local HW
  if ( ( ctrl->isLocal()) && (!ctrl->isI2CSwarmCtrl() ) ) return;

  // check if it's toggled?
  if ( lastRawValue != value) { 

    // trigger value event
    trigger( FTSWARM_TRIGGERVALUE, value );
    
  }
  
  lastRawValue = value;

  subscription();

}

void SwOSFrequencymeter::jsonize( JSONize *json, uint8_t id) {

  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variableI32("value", getValueI32() );
  json->endObject();

}