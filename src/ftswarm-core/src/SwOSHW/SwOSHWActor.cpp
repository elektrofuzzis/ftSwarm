/*
 * SwOActor.cpp
 *
 * Actor hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOSHW/SwOSHWActor.h"
#include "SwOSHW/SwOSHWBaseCtrl.h"

#include "SwOSCom.h"

/***************************************************
 *
 *   SwOSMotor
 *
 ***************************************************/

SwOSMotor::SwOSMotor(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType, uint8_t flags ):SwOSIO(name, port, ctrl, ioType, flags ){
}

void SwOSMotor::setMotionType( FtSwarmMotion_t motionType ) {

  this->motionType = motionType;
  
}

int16_t SwOSMotor::getMaxSpeed( void ) {

  switch ( ioType ) {
    case SWOSIO_MOTOR:   return 4096;
    case SWOSIO_STEPPER: return 10240;
    default:             return 100;
  }

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
  if      (speed> getMaxSpeed()) this->speed =  getMaxSpeed();
  else if (speed<-getMaxSpeed()) this->speed = -getMaxSpeed();
  else                           this->speed =  speed;

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

void SwOSMotor::onTrigger( SwOSTriggerMath triggerMath, int32_t sensor, int32_t parameter ) {

  setSpeed( evalTriggerMath( triggerMath, sensor, getSpeed(), parameter, -getMaxSpeed(), getMaxSpeed() ) );
  apply();

}

void SwOSMotor::operate( void ) {

}

/***************************************************
 *
 *   SwOSDCMotor
 *
 ***************************************************/

SwOSDCMotor::SwOSDCMotor(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType, uint8_t flags ):SwOSMotor(name, port, ctrl, ioType, flags ){

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
  IN1 = MOTOR_GPIO[port][0];
  IN2 = MOTOR_GPIO[port][1];

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

  // just prepare led channel, don't register yet
  ledc_channel = (ledc_channel_config_t *) calloc( sizeof( ledc_channel_config_t ), 1 );
  ledc_channel->gpio_num       = GPIO_NUM_NC;
  ledc_channel->speed_mode     = LEDC_LOW_SPEED_MODE;
  ledc_channel->channel        = (ledc_channel_t) (port);
  ledc_channel->intr_type      = LEDC_INTR_DISABLE;
  ledc_channel->timer_sel      = LEDC_TIMER_0;
  ledc_channel->duty           = 0; 
  ledc_channel->hpoint         = 0;
  ledc_channel->flags.output_invert = 0;

}

int16_t SwOSDCMotor::duty( void ) {

  // calculate pwm duty based on actor + power supply
  
  // Motortype    4.5V   9.0V
  // Mini         2700   2200
  // XS           2700   2200   not measured, seems to be same like MiniMot
  // S            2700   1900
  // XM           3200   2300
  // Tractor      
  // Encoder      1600    500
  // WheelDrive   3173   2354   measured: 4.9V: 3100  6V: 2900  NOT WORKING BELOW 4.9V!
  // RCMotor      2700   2700   just run with max 6V!
  // Power
  // M            2700   2600
  // Lamp          400    100
  // Valve
  // Compressor
  // Buzzer

  // LED            50     10

  // speed zer0 is a 0 duty as well
  if ( speed == 0 ) return 0;

  // SWOSIO_MOTOR: Range 0..4095, no corrections
  if ( ioType == SWOSIO_MOTOR ) return abs(speed);
  
    // DC Motors 100 % = 4095
  if ( ( speed <= -100 ) || ( speed >= 100 ) ) return 4095;
  
  // set motor type specific parameters
  int32_t x45  = 2700;  // 4.5V power supply
  int32_t x90  = 1900;  // 9V power suppy
  int32_t xMax = 4096;  // max. power
  switch ( ioType ) {    
    case SWOSIO_POWERMOTOR:  
    case SWOSIO_BUZZER:     break;

    case SWOSIO_MINIMOTOR:
    case SWOSIO_XSMOTOR:    x45 = 2700; x90 = 2200;
                            break;

    case SWOSIO_XMMOTOR:    x45 = 3200; x90 = 2300;
                            break;

    case SWOSIO_ENCODER:    
    case SWOSIO_TRACTOR:    x45 = 1600; x90 = 500;
                            break;

    case SWOSIO_WHEELDRIVE: x45 = 3200; x90 = 2350;  // 3200
                            break;

    case SWOSIO_SMOTOR:     x45 = 2700; x90 = 1900;
                            break;

    case SWOSIO_MMOTOR:     x45 = 2700; x90 = 2600; 
                            break;
    
    case SWOSIO_VALVE:      
    case SWOSIO_COMPRESSOR: x45 = x90 = 4095;
                            break;

    case SWOSIO_LAMP:       x45 = x90 = 0;
                            break;

    case SWOSIO_RCSERVO:
    case SWOSIO_RCMOTOR:    x45 = 2800;  // beide 2700
                            x90 = 2800;
                            break;
  }

  float xMin = x90;
  if (ctrl->pwrctl) {

    // % power supply between 4.5 and 9.0 V
    float p = ( ctrl->pwrctl->getVoltage() - 4.5 ) / 4.5;

    // calc xMin
    if      ( p <= 0 ) xMin = x45;
    else if ( p >= 1 ) xMin = x90;
    else               xMin = ( x90 - x45 ) * p + x45;

  } else {
    // no PWRCTL available, take 9.0V value
    xMin = x90;
  }

  int16_t duty = xMin + int32_t( (xMax -xMin) ) * abs(speed) / 100;

  return duty;

}

void SwOSDCMotor::setPWM( int16_t xin1, int16_t xin2, gpio_num_t pwm, uint32_t duty ) {

  // check if it's needed to stop running pwm
  if ( ( ( duty == 0 ) || ( pwm != ledc_channel->gpio_num ) ) && ( ledc_channel->gpio_num != GPIO_NUM_NC ) ) {

    ESP_ERROR_CHECK( ledc_stop( LEDC_LOW_SPEED_MODE, ledc_channel->channel, 0 ) );
    ESP_ERROR_CHECK( gpio_reset_pin( (gpio_num_t) ledc_channel->gpio_num ) );

    // reconfigure pin
    ledc_channel->gpio_num = GPIO_NUM_NC;
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
  
    // set start levels
    if ( IN1 != GPIO_NUM_NC ) gpio_set_level( IN1, 0 );
    if ( IN2 != GPIO_NUM_NC ) gpio_set_level( IN2, 0 );

  }

  // off?
  if ( duty==0 ) {
    
    if ( IN1 != GPIO_NUM_NC ) gpio_set_level( IN1, xin1 );
    if ( IN2 != GPIO_NUM_NC ) gpio_set_level( IN2, xin2 );
    
    return;

  }

  // reconfigure ledc  due to a change of direction?
  if ( pwm != ledc_channel->gpio_num ) {

    // reconfigure to new pin
    ledc_channel->gpio_num = pwm;
    ledc_channel->duty     = 0;
    ESP_ERROR_CHECK( ledc_channel_config( ledc_channel ) );

  }

  // TODO rampUp

  // set fading & new duty
  if ( ioType == SWOSIO_WHEELDRIVE ) {

    if ( duty < 3500 ) {
      // use standard duty for "small" values
      ESP_ERROR_CHECK( ledc_set_duty( LEDC_LOW_SPEED_MODE, ledc_channel->channel, duty ) );
      ESP_ERROR_CHECK( ledc_update_duty( LEDC_LOW_SPEED_MODE, ledc_channel->channel ) );

    } else {
      // need to do a ramp for higher values
      ESP_ERROR_CHECK( ledc_set_duty( LEDC_LOW_SPEED_MODE, ledc_channel->channel, 3500 ) );
      ESP_ERROR_CHECK( ledc_update_duty( LEDC_LOW_SPEED_MODE, ledc_channel->channel ) );

      ESP_ERROR_CHECK( ledc_set_fade_with_step( LEDC_LOW_SPEED_MODE, ledc_channel->channel, duty, 1, 2 ) );
      ESP_ERROR_CHECK( ledc_fade_start( LEDC_LOW_SPEED_MODE, ledc_channel->channel, LEDC_FADE_NO_WAIT ) );
    }

  } else {
    ESP_ERROR_CHECK( ledc_set_duty( LEDC_LOW_SPEED_MODE, ledc_channel->channel, duty ) );
    ESP_ERROR_CHECK( ledc_update_duty( LEDC_LOW_SPEED_MODE, ledc_channel->channel ) );
  }

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
                          setPWM( 0, 1, IN1, duty( ) );
                        } else {
                          // SLEEP HIGH, IN1 HIGH, IN2 PWM
                          setPWM( 1, 0, IN2, duty( ) );
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

SwOSStepper::SwOSStepper(const char *name, uint8_t port, SwOSCtrl *ctrl, uint8_t flags ):SwOSMotor(name, port, ctrl, SWOSIO_STEPPER, flags ){

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

  this->motorIsRunning = start;
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

  this->motorIsHoming = true;
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
  if ( isSubscribed ) {
    if ( ( hysteresis & 0x01 ) && ( this->motorIsRunning != isRunning ) ) printf("S: %s running %d\n",  subscribedIOName, isRunning );
    if ( ( hysteresis & 0x02 ) && ( this->motorIsHoming  != isHoming  ) ) printf("S: %s homing %d\n",   subscribedIOName, isHoming );
    if ( ( hysteresis & 0x04 ) && ( this->distance != distance ) )        printf("S: %s distance %d\n", subscribedIOName, distance );
    if ( ( hysteresis & 0x08 ) && ( this->position != position ) )        printf("S: %s position %d\n", subscribedIOName, position );
  }

  this->distance = distance;
  this->position = position;
  this->motorIsHoming = isHoming;
  this->motorIsRunning = isRunning;
}

void SwOSStepper::operate() {
  
  // no work on remote sensors
  if (!ctrl->isLocal()) return;

  // ftPwrDrive
  if ( ( ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && ( ftPwrDrive ) ) { 
    bool newMotorIsRunning = ( ftPwrDrive->lastState[port] & ISMOVING ) > 0;
    bool newMotorIsHoming  = ( ftPwrDrive->lastState[port] & HOMING ) > 0;
    
    setValue( ftPwrDrive->lastDistance[port], ftPwrDrive->lastPosition[port], newMotorIsHoming, newMotorIsRunning );
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

  int32_t newPosition;
  int32_t newDistance;
  bool newMotorIsHoming;
  bool newMotorIsRunning;

  memcpy( &newPosition,       buf, sizeof( position ) );       buf += sizeof( position );
  memcpy( &newDistance,       buf, sizeof( distance ) );       buf += sizeof( distance );
  memcpy( &newMotorIsHoming,  buf, sizeof( motorIsHoming ) );  buf += sizeof( motorIsHoming );
  memcpy( &newMotorIsRunning, buf, sizeof( motorIsRunning ) ); buf += sizeof( motorIsRunning );

  setValue( newDistance, newPosition, newMotorIsHoming, newMotorIsRunning );

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

void SwOSServo::onTrigger( SwOSTriggerMath triggerMath, int32_t sensor, int32_t parameter ) {

  setPosition( evalTriggerMath( triggerMath, sensor, getPosition(), parameter, getMinPosition(), getMaxPosition() ) );

}

void SwOSServo::setRemote( ) {

  SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_SETSERVO );
  cmd.data.servoCmd.index    = ctrl->getIndex(this);
  cmd.data.servoCmd.position = position;
  cmd.data.servoCmd.offset   = offset;
  cmd.send( );
}

/***************************************************
 *
 *   SwOSDigitalServo
 *
 ***************************************************/

 SwOSDigitalServo::SwOSDigitalServo(const char *name, uint8_t port, SwOSCtrl *ctrl, uint8_t flags ) : SwOSServo( name, port, ctrl, SWOSIO_SERVO, flags ) {

  // initialize local HW
  if (ctrl->isLocal()) setupLocal();

}

void SwOSDigitalServo::setupLocal() {
  // initialize local HW

  #if FTSWARM_HAL_SERVOS > 0

  SERVO = SERVO_GPIO[port];

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

    #endif

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

/***************************************************
 *
 *   SwOSRCServo
 *
 ***************************************************/

 // min/max positions

#define RCSERVO_LOW  390 // 510 // 1600   // 1700.0
#define RCSERVO_HIGH 890 // 870 // 3300   // 3750.0
#define RCSERVO_RESOLUTION 90
#define RCMAXDELTA   2

int16_t SwOSRCServo::getMaxPosition( void ) { return RCSERVO_RESOLUTION - offset; };

SwOSRCServo::SwOSRCServo(const char *name, uint8_t port, SwOSCtrl *ctrl, uint8_t flags ): SwOSServo( name, port, ctrl, SWOSIO_RCSERVO, flags ) {
  
  // initialize local HW
  if (ctrl->isLocal()) setupLocal();

}

void SwOSRCServo::setupLocal( void ) {

  #if FTSWARM_HAL_RCSERVOS > 0

  // create a motor instance with my own port parameters but with a different name
  motor = new SwOSDCMotor( RCSERVO_MOTOR[port], port, ctrl, SWOSIO_RCMOTOR, FTSWARM_HAL_FLAG_HIDDEN );

  // get a pointer to my poti
  poti = new SwOSAnalogInput( INPUT_NAME[ RCSERVO_BASE_PORT + port ], RCSERVO_BASE_PORT + port, ctrl, SWOSIO_RCPOTI, INPUT_FLAGS[ RCSERVO_BASE_PORT + port ] );

  // adapt kp based on VM
  float kp = 0.55;
  if ( ( ctrl->pwrctl ) && ( ctrl->pwrctl->getVoltage() > 5.5 ) ) kp = 0.25;

  this->pid      = new SwOSPID( kp, 0.01, 0, -100, 100, -motor->getMaxSpeed(), motor->getMaxSpeed() );
// this->pid      = new SwOSPID( 0.25, 0.01, 0, -100, 100, -motor->getMaxSpeed(), motor->getMaxSpeed() );  // > 5.5V
// this->pid      = new SwOSPID( 0.35, 0.01, 0, -100, 100, -motor->getMaxSpeed(), motor->getMaxSpeed() );  // <= 5.5V
  this->target   = FILTER_INVALID;
  this->offset   = RCSERVO_RESOLUTION / 2;

  poti2position();

  #endif

}

SwOSRCServo::~SwOSRCServo() {
  
  if (poti)  delete poti;
  if (motor) delete motor;
  if (pid)   delete pid;

}

void SwOSRCServo::poti2position( ) {
  position = ( ( (float) poti->getValueI32() - RCSERVO_LOW ) / ( RCSERVO_HIGH - RCSERVO_LOW ) * RCSERVO_RESOLUTION ) - offset;
}

void SwOSRCServo::operate(void) {

  #if FTSWARM_HAL_RCSERVOS > 0

  // remote: no work
  if (!ctrl->isLocal()) return;

  // read poti value to fill up the filters
  poti->operate();

  // calc actual position
  poti2position();

  // no target set - noting to do 
  if ( target == FILTER_INVALID ) return;

  int16_t speed;
  int16_t sensor = poti->getValueI32();

  speed = -pid->solve( target, sensor );
    
  if ( motor->getSpeed() != speed ) {
    motor->setSpeed( speed );
    motor->apply();
  }

  if ( speed == 0 ) target = FILTER_INVALID;

  #endif

}

void SwOSRCServo::setLocal( void ) {

  // calc poti's new target value
  target = ( position + offset ) / (float) RCSERVO_RESOLUTION * ( RCSERVO_HIGH - RCSERVO_LOW ) + RCSERVO_LOW;
  if ( target > RCSERVO_HIGH ) target = RCSERVO_HIGH;
  if ( target < RCSERVO_LOW )  target = RCSERVO_LOW;

}
