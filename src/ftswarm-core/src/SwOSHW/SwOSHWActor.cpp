/*
 * SwOActor.cpp
 *
 * Actor hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOSHW/SwOSHWActor.h"
#include "SwOSHW/SwOSHWHAL.h"
#include "SwOSHW/SwOSHWBaseCtrl.h"

#include "SwOSCom.h"

/***************************************************
 *
 *   SwOSMotor
 *
 ***************************************************/

 SwOSMotor::SwOSMotor(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType ):SwOSIO(name, port, ctrl, ioType ){

  // initialize local HW
  if (ctrl->isLocal()) {
    setupLocal();
  }

}

void SwOSMotor::setMotionType( FtSwarmMotion_t motionType ) {

  this->motionType = motionType;
  
}

void SwOSMotor::setSpeed( int16_t speed ) {

  // if no change is needed, return
  if ( speed == this->speed ) return;

  // Motor, OnOff-Actors: set COAST or ON automatically
  if ( autoCoast() ) {
    if ( ( this->speed != 0 ) && ( speed == 0 ) ) motionType = FTSWARM_COAST;
    if ( ( this->speed == 0 ) && ( speed != 0 ) ) motionType = FTSWARM_ON;
  }
  
  // limit speed values
  int16_t maxSpeed = MAXSPEED4096;
  if      (speed> maxSpeed) this->speed =  maxSpeed;
  else if (speed<-maxSpeed) this->speed = -maxSpeed;
  else                      this->speed =  speed;

}

void SwOSMotor::apply(void) {

  // set speed values
  if   (!ctrl->isLocal()) setRemote();
  else                    setLocal();

}

void SwOSMotor::serialize( Serialize *serialize ) {

  serialize->startObject( );
  SwOSIO::serialize( serialize );
  serialize->item( SERIALIZE_LITERAL_SPEED, getSpeed() );
  serialize->endObject();
}

void SwOSMotor::onTrigger( int32_t value ) {

  setSpeed( (int16_t) value );
  apply();

}

void SwOSMotor::read( void ) {

}

/***************************************************
 *
 *   SwOSDCMotor
 *
 ***************************************************/

SwOSDCMotor::SwOSDCMotor(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType ):SwOSMotor(name, port, ctrl, ioType ){

  // initialize local HW
  if (ctrl->isLocal()) {
    setupLocal();
  }

}

SwOSDCMotor::~SwOSDCMotor() {

  if ( ctrl->isLocal() ) { setSpeed(0); apply(); }
  if ( ledc_channel ) free( ledc_channel );

}

void SwOSDCMotor::setupLocal() {
  // initialize local HW

  // ftDuino
  if ( ( ctrl->getCPU() == FTSWARMDUINO_1V141 ) && (ftDuino) ) {
    ftDuino->setIOType( port, ioType );
    return;
  }

  // set HW Pins
  IN1 = GPIO_ACTOR[ctrl->getCPU()][port][0];
  IN2 = GPIO_ACTOR[ctrl->getCPU()][port][1];
  
  // set digital ports IN1 & in2 to output
  gpio_config_t io_conf = {
    .pin_bit_mask = 0,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };
  if ( IN1 != GPIO_NUM_NC ) io_conf.pin_bit_mask = io_conf.pin_bit_mask | (1ULL << IN1);
  if ( IN2 != GPIO_NUM_NC ) io_conf.pin_bit_mask = io_conf.pin_bit_mask | (1ULL << IN2);
  gpio_config(&io_conf);
  
  // set motor driver off
  if ( IN1 != GPIO_NUM_NC ) gpio_set_level( IN1, 0 );
  if ( IN2 != GPIO_NUM_NC ) gpio_set_level( IN2, 0 );

  // use Timer 0
  ledc_timer_config_t ledc_timer = {
    .speed_mode       = LEDC_LOW_SPEED_MODE,
    .duty_resolution  = LEDC_TIMER_12_BIT,
    .timer_num        = LEDC_TIMER_0,
    .freq_hz          = 600,  // Set output frequency to 60 Hz
    .clk_cfg          = LEDC_AUTO_CLK,
  };
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

  // just prepare led channel, don't register yet
  ledc_channel = (ledc_channel_config_t *) calloc( sizeof( ledc_channel_config_t ), 1 );
  ledc_channel->gpio_num       = IN1;
  ledc_channel->speed_mode     = LEDC_LOW_SPEED_MODE;
  ledc_channel->channel        = (ledc_channel_t) (port);
  ledc_channel->intr_type      = LEDC_INTR_DISABLE;
  ledc_channel->timer_sel      = LEDC_TIMER_0;
  ledc_channel->duty           = 0; 
  ledc_channel->hpoint         = 0;
  ledc_channel->flags.output_invert = 1;

}

void SwOSDCMotor::setPWM( int16_t xin1, int16_t xin2, gpio_num_t pwm, uint32_t duty ) {

  // calc duty 
  uint32_t duty1 = duty;

  // 1st step, check if the old pwm pin is different to the new one

  if ( ledc_channel->gpio_num != pwm ) {
  
    // reconfigure old pin
    if ( ledc_channel->gpio_num != GPIO_NUM_NC ) ESP_ERROR_CHECK( gpio_reset_pin( (gpio_num_t) ledc_channel->gpio_num ) );
  
    // set ledc_channel to new pin
    ledc_channel->gpio_num = pwm;
    if ( ledc_channel->gpio_num != GPIO_NUM_NC ) ESP_ERROR_CHECK( ledc_channel_config( ledc_channel ) );
  
  }

  // 2rd step: set pwm
  if ( ledc_channel->gpio_num != GPIO_NUM_NC ) {

    if ( rampUpT | rampUpY ) {
      // acceleration
      uint32_t oldDuty = ledc_get_duty( LEDC_LOW_SPEED_MODE, ledc_channel->channel );
      ESP_ERROR_CHECK( ledc_set_fade( LEDC_LOW_SPEED_MODE, ledc_channel->channel, oldDuty, ( oldDuty > duty1 ) ? LEDC_DUTY_DIR_DECREASE : LEDC_DUTY_DIR_INCREASE, rampUpT, rampUpY, duty1 ) );
      ESP_ERROR_CHECK( ledc_update_duty( LEDC_LOW_SPEED_MODE, ledc_channel->channel ) );
    
    } else {
      // pwm signal
      ESP_ERROR_CHECK( ledc_set_duty( LEDC_LOW_SPEED_MODE, ledc_channel->channel, duty1 ) );
      ESP_ERROR_CHECK( ledc_update_duty( LEDC_LOW_SPEED_MODE, ledc_channel->channel ) );
    }

  }

  // 3rd step: set static levels if applicable
  if ( IN1 != GPIO_NUM_NC ) gpio_set_level( IN1, xin1 );
  if ( IN2 != GPIO_NUM_NC ) gpio_set_level( IN2, xin2 );

}

void SwOSDCMotor::setLocal() {

  // just in case of non-existent HW
  if ( ( IN1 == GPIO_NUM_NC ) || ( IN2 == GPIO_NUM_NC ) ) return;

  // calculate duty
  switch (motionType) {
    case FTSWARM_COAST: // SLEEP HIGH, IN1 LOW, IN2 LOW
                        setPWM( 0, 0, IN1, 0 );
                        break;
  
    case FTSWARM_BRAKE: // SLEEP HIGH, IN1 HIGH, IN2 HIGH
                        setPWM( 1, 1, IN1, 0 );
                        break;

    case FTSWARM_ON:    if ( speed <  0) {
                          // SLEEP HIGH, IN1 PWM, IN2 HIGH
                          setPWM( 0, 1, IN1, abs(speed) );
                        } else {
                          // SLEEP HIGH, IN1 HIGH, IN2 PWM
                          setPWM( 1, 0, IN2, abs(speed) );
                        }
                        break;

    default:            // SLEEP HIGH, IN1 LOW, IN2 LOW
                        setPWM( 0, 0, IN1, 0 );
                        break;
  }  
  
}


void SwOSDCMotor::setAcceleration( uint32_t rampUpT,  uint32_t rampUpY ) {

  this->rampUpT = rampUpT;
  this->rampUpY = rampUpY;

}

void SwOSDCMotor::getAcceleration( uint32_t *rampUpT,  uint32_t *rampUpY ) {
  *rampUpY = this->rampUpY;
  *rampUpT = this->rampUpT;
}

void SwOSDCMotor::setRemote() {
  
  SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_SETACTORSPEED  );
  cmd.data.actorSpeedCmd.index      = ctrl->getIndex( this );
  cmd.data.actorSpeedCmd.motionType = motionType;
  cmd.data.actorSpeedCmd.speed      = speed;
  cmd.data.actorSpeedCmd.rampUpT    = rampUpT;
  cmd.data.actorSpeedCmd.rampUpY    = rampUpY;
  cmd.send( );

}

/***************************************************
 *
 *   SwOSStepper
 *
 ***************************************************/

SwOSStepper::SwOSStepper(const char *name, uint8_t port, SwOSCtrl *ctrl ):SwOSMotor(name, port, ctrl, SWOSIO_STEPPER ){

  // ftPwrDrive has a bitmap motor representation, so precalc the Mx values
  pwrDriveMotor = 1 << port;

  // initialize local HW
  if (ctrl->isLocal()) {
    setupLocal();
  }

}

SwOSStepper::~SwOSStepper() {

  if (ctrl->isLocal() ) { setSpeed(0); apply(); }

}

void SwOSStepper::setRemote() {
  
  SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_SETACTORSPEED  );
  cmd.data.actorSpeedCmd.index      = ctrl->getIndex( this );
  cmd.data.actorSpeedCmd.motionType = motionType;
  cmd.data.actorSpeedCmd.speed      = speed;
  cmd.data.actorSpeedCmd.rampUpT    = 0;
  cmd.data.actorSpeedCmd.rampUpY    = 0;
  cmd.send( );

}

void SwOSStepper::setLocal() {

  if ( ( ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && ( ftPwrDrive ) ) {
    ftPwrDrive->setMaxSpeed( pwrDriveMotor, speed );
  }

}

void SwOSStepper::setDistance( int32_t distance, bool relative ) {

  if   (!ctrl->isLocal()) {

    // send remote
    SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_SETSTEPPERDISTANCE  );
    cmd.data.actorStepperCmd.index  = ctrl->getIndex( this );
    cmd.data.actorStepperCmd.paraml = distance;
    cmd.data.actorStepperCmd.paramb = relative;
    cmd.send( );

  } else if ( ( ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (ftPwrDrive ) ) {
    // set local
    if (relative) ftPwrDrive->setRelDistance( pwrDriveMotor, distance );
    else          ftPwrDrive->setAbsDistance( pwrDriveMotor, distance );

    distance = ftPwrDrive->getStepsToGo( pwrDriveMotor );

  }

}

int32_t SwOSStepper::getDistance( void ) {
  return distance;
}

void SwOSStepper::startStop( bool start ) {

  if (!ctrl->isLocal() )  {
    // send remote
    SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_STEPPERSTARTSTOP  );
    cmd.data.actorStepperCmd.index  = ctrl->getIndex(this);
    cmd.data.actorStepperCmd.paramb = start;
    cmd.send( );

  } else if ( ( ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && ( ftPwrDrive ) ) {

    if (start) ftPwrDrive->startMoving( pwrDriveMotor );
    else       ftPwrDrive->stopMoving( pwrDriveMotor );
    
  }

}

void SwOSStepper::setPosition( int32_t position ) {

  this->position = position;

  if   (!ctrl->isLocal()) {

    // send remote
    SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_SETSTEPPERPOSITION  );
    cmd.data.actorStepperCmd.index  = ctrl->getIndex(this);
    cmd.data.actorStepperCmd.paraml = position;
    cmd.send( );

  } else if ( ( ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (ftPwrDrive ) ) {
    // set local
    ftPwrDrive->setPosition( pwrDriveMotor, position );
  }

}

int32_t SwOSStepper::getPosition( void ) {
  return position;
}

void SwOSStepper::homing( int32_t maxDistance ) {

  if   (!ctrl->isLocal()) {
    // send remote
    SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_STEPPERHOMING );
    cmd.data.actorStepperCmd.index  = ctrl->getIndex(this);
    cmd.data.actorStepperCmd.paraml = maxDistance;
    cmd.send( );

  } else if ( ( ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (ftPwrDrive ) ) {
    // set local
    ftPwrDrive->homing(pwrDriveMotor, maxDistance );
  }

}

void SwOSStepper::setHomingOffset( int32_t offset ) {

  if   (!ctrl->isLocal()) {
    // send remote
    SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_SETSTEPPERHOMINGOFFSET );
    cmd.data.actorStepperCmd.index  = ctrl->getIndex(this);
    cmd.data.actorStepperCmd.paraml = offset;
    cmd.send( );

  } else if ( ( ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (ftPwrDrive ) ) {
    // set local
    ftPwrDrive->homingOffset( pwrDriveMotor, offset );
  }

}

void SwOSStepper::setIsHoming( bool isHoming ) {
  this->motorIsHoming = isHoming;
}

bool SwOSStepper::isHoming( void ) {
  return motorIsHoming;
}

void SwOSStepper::setIsRunning( bool isRunning ) {
  this->motorIsRunning = isRunning;
}

bool SwOSStepper::isRunning( void ) {
  return motorIsRunning;
}

void SwOSStepper::setValue( int32_t distance, int32_t position, bool isHoming, bool isRunning ) {

  this->distance = distance;
  this->position = position;
  this->motorIsHoming = isHoming;
  this->motorIsRunning = isRunning;
}

void SwOSStepper::read() {
  
  // no work on remote sensors
  if (!ctrl->isLocal()) return;

  // ftPwrDrive
  if ( ( ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && ( ftPwrDrive ) ) { 
  
    distance = ftPwrDrive->lastDistance[port];
    position = ftPwrDrive->lastPosition[port];
    motorIsRunning = ( ftPwrDrive->lastState[port] & ISMOVING ) > 0;
    motorIsHoming  = ( ftPwrDrive->lastState[port] & HOMING ) > 0;

  }

}

void SwOSStepper::serialize( Serialize *serialize ) {

  serialize->startObject( );
  SwOSIO::serialize( serialize );
  serialize->item( SERIALIZE_LITERAL_SPEED, getSpeed() );
  serialize->item( SERIALIZE_LITERAL_POSITION, position );
  serialize->item( SERIALIZE_LITERAL_DISTANCE, distance );
  serialize->item( SERIALIZE_LITERAL_HOMING, isHoming() );
  serialize->item( SERIALIZE_LITERAL_RUNNING, isRunning() );
  serialize->endObject();
}

uint8_t SwOSStepper::pushState( uint8_t *buffer ) { 
  
  uint8_t *buf = buffer;

  memcpy( buf, &position,       sizeof( position ) );       buf += sizeof( position );
  memcpy( buf, &distance,       sizeof( distance ) );       buf += sizeof( distance );
  memcpy( buf, &motorIsHoming,  sizeof( motorIsHoming ) );  buf += sizeof( motorIsHoming );
  memcpy( buf, &motorIsRunning, sizeof( motorIsRunning ) ); buf += sizeof( motorIsRunning ); 

  return sizeof( position ) + sizeof( distance ) + sizeof( motorIsHoming ) + sizeof( motorIsRunning );

};

uint8_t SwOSStepper::popState( uint8_t *buffer ) { 

  uint8_t *buf = buffer;

  memcpy( &position,       buf, sizeof( position ) );       buf += sizeof( position );
  memcpy( &distance,       buf, sizeof( distance ) );       buf += sizeof( distance );
  memcpy( &motorIsHoming,  buf, sizeof( motorIsHoming ) );  buf += sizeof( motorIsHoming );
  memcpy( &motorIsRunning, buf, sizeof( motorIsRunning ) ); buf += sizeof( motorIsRunning );

  return sizeof( position ) + sizeof( distance ) + sizeof( motorIsHoming ) + sizeof( motorIsRunning );
  
};

/***************************************************
 *
 *   SwOSServo
 *
 ***************************************************/

void SwOSServo::serialize( Serialize *serialize ) {

  serialize->startObject( );
  SwOSIO::serialize( serialize );
  serialize->item( SERIALIZE_LITERAL_OFFSET,   offset);
  serialize->item( SERIALIZE_LITERAL_POSITION, position);
  serialize->endObject();

}

void SwOSServo::setPosition( int16_t position ) {
  
  this->position = position;

  // apply local or remote
  if (ctrl->isLocal()) setLocal();
  else                 setRemote();

}

void SwOSServo::setOffset( int16_t offset ) {
  
  this->offset = offset;
 
  // apply local or remote
  if (ctrl->isLocal()) setLocal();
  else                 setRemote();

}

void SwOSServo::onTrigger( int32_t value ) {

  setPosition( (int16_t) value );

}

/***************************************************
 *
 *   SwOSDigitalServo
 *
 ***************************************************/

 SwOSDigitalServo::SwOSDigitalServo(const char *name, uint8_t port, SwOSCtrl *ctrl) : SwOSServo( name, port, ctrl ) {

  // initialize local HW
  if (ctrl->isLocal()) setupLocal();
}

void SwOSDigitalServo::setupLocal() {
  // initialize local HW

  SERVO = GPIO_SERVO[ctrl->getCPU()][port];

  // set digital port  to output
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << SERVO);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);

  // ledc channels
  channelSERVO = (ledc_channel_t) (4 + port);

  // use Timer 1
    ledc_timer_config_t ledc_timer = {
      .speed_mode       = LEDC_LOW_SPEED_MODE,
      .duty_resolution  = LEDC_TIMER_10_BIT,
      .timer_num        = LEDC_TIMER_1,
      .freq_hz          = 50,  // Set output frequency to 40Hz
      .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // register channel
    ledc_channel_config_t ledc_channel = {
      .gpio_num       = SERVO,
      .speed_mode     = LEDC_LOW_SPEED_MODE,
      .channel        = channelSERVO,
      .intr_type      = LEDC_INTR_DISABLE,
      .timer_sel      = LEDC_TIMER_1,
      .duty           = 0, // Set duty to 0%
      .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    // set coast
    setLocal();

}

void SwOSDigitalServo::setLocal() {

  // calc duty
  float p = offset + position;
  if (p <   0 ) p =   0;
  if (p > 255 ) p = 255;

  // 1 .. 2 ms pulse
  float ticks = 0.2*p+51;

  // set duty
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, channelSERVO, (uint16_t)ticks));

  // update duty
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, channelSERVO));

}

void SwOSDigitalServo::setRemote( ) {
  
  SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_SETSERVO );
  cmd.data.servoCmd.index    = ctrl->getIndex(this);
  cmd.data.servoCmd.position = position;
  cmd.data.servoCmd.offset   = offset;
  cmd.send( );
}


/***************************************************
 *
 *   SwOSRCServo
 *
 ***************************************************/

 // min/max positions

#define RCSERVO_LOW  1700.0
#define RCSERVO_HIGH 3750.0
#define RCMAXDELTA   20
#define RCMINSPEED   65

SwOSRCServo::SwOSRCServo(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSAnalogInput *poti, SwOSMotor *motor): SwOSServo( name, port, ctrl ) {

  this->poti  = poti;
  this->motor = motor;
  
}

SwOSRCServo::~SwOSRCServo() {
  
  if (poti)  delete poti;
  if (motor) delete motor;
  if (pid)   delete pid;

}

void SwOSRCServo::adjust(void) {

  // remote: no work
  if (!ctrl->isLocal()) return;

  // read poti value to fill up the filters
  poti->read();

  // no target set - noting to do 
  if ( target == FILTER_INVALID ) return;

  int16_t speed;
  int16_t sensor = poti->getValueI32();

  // target reached?
  if ( abs( sensor - target ) < RCMAXDELTA ) { 
    speed = 0; 
    target = FILTER_INVALID;

  } else {

    // calc next speed
    speed = pid->solve( target, sensor );
    
    // keep minimum speed
    if ( abs( speed ) < RCMINSPEED ) { 
      if ( speed < 0 ) speed = -RCMINSPEED;
      else             speed =  RCMINSPEED;
    }

  }

  motor->setSpeed( speed );
  motor->apply();

}

void SwOSRCServo::setLocal( void ) {

  // calc poti's new target value
  target = ( position + offset ) / 256.0 * ( RCSERVO_HIGH - RCSERVO_LOW ) + RCSERVO_LOW;
  if ( target > RCSERVO_HIGH ) target = RCSERVO_HIGH;
  if ( target < RCSERVO_LOW )  target = RCSERVO_LOW;

}