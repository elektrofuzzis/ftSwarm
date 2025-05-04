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

const FtSwarmIcon_t ACTORICON[FTSWARM_MAXACTOR] = { FTSWARM_16_XMOTOR, FTSWARM_20_XMMOTOR, FTSWARM_17_TRACTOR, FTSWARM_18_ENCODER, FTSWARM_19_LAMP, FTSWARM_23_VALVE, FTSWARM_22_COMPRESSOR, FTSWARM_24_BUZZER };
 
/***************************************************
 *
 *   SwOSActor
 *
 ***************************************************/

 SwOSActor::SwOSActor(const char *name, uint8_t port, SwOSCtrl *ctrl):SwOSIO(name, port, ctrl ){

  // ftPwrDrive has a bitmap motor representation, so precalc the Mx values
  pwrDriveMotor = 1 << port;
  if ( ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) highResolution = true;

  // initialize local HW
  if (ctrl->isLocal()) {
    if ( ctrl->isI2CSwarmCtrl() ) {
      setupI2C();
    } else {
      setupLocal();
    }
  }

}

SwOSActor::~SwOSActor() {
  if (ctrl->isLocal() ) { setSpeed(0); apply(); }
  if ( ledc_channel ) free( ledc_channel );
}

FtSwarmIcon_t SwOSActor::getIcon() { 
  return ACTORICON[ actorType ]; 
}; 

void SwOSActor::setupI2C() {
  
}

void SwOSActor::setupLocal() {
  // initialize local HW

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

void SwOSActor::setMotionType( FtSwarmMotion_t motionType ) {

  this->motionType = motionType;
  
}

void SwOSActor::setActorType( FtSwarmActor_t actorType, bool highResolution, bool dontSendToRemote ) { 

  this->actorType = actorType;
  this->highResolution = highResolution;

  if (ctrl->isLocal()) { 
    
  } else if (!dontSendToRemote) {    
    // send SN, SETSENSORTYPE, port, sensorType
    SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_SETACTORTYPE );
    cmd.data.actorTypeCmd.index          = port;
    cmd.data.actorTypeCmd.actorType      = actorType;
    cmd.data.actorTypeCmd.highResolution = highResolution;
    cmd.send( );

  }
  
};   

void SwOSActor::setAcceleration( uint32_t rampUpT,  uint32_t rampUpY ) {

  this->rampUpT = rampUpT;
  this->rampUpY = rampUpY;

}

void SwOSActor::getAcceleration( uint32_t *rampUpT,  uint32_t *rampUpY ) {
  *rampUpY = this->rampUpY;
  *rampUpT = this->rampUpT;
}

void SwOSActor::setSpeed( int16_t speed ) {

  // if no change is needed, return
  if ( speed == this->speed ) return;

  // Motor, OnOff-Actors: set COAST or ON automatically
  if ( ( actorType != FTSWARM_XMMOTOR ) && ( actorType != FTSWARM_TRACTOR ) && ( actorType != FTSWARM_ENCODER ) ) {
    if ( ( this->speed != 0 ) && ( speed == 0 ) ) motionType = FTSWARM_COAST;
    if ( ( this->speed == 0 ) && ( speed != 0 ) ) motionType = FTSWARM_ON;
  }
  
  // limit speed values
  int16_t maxSpeed = MAXSPEED256;
  if (highResolution) maxSpeed = MAXSPEED4096;
  if      (speed> maxSpeed) this->speed =  maxSpeed;
  else if (speed<-maxSpeed) this->speed = -maxSpeed;
  else                      this->speed =  speed;

}

void SwOSActor::apply(void) {

  // set speed values
  if      (!ctrl->isLocal())        setRemote();
  else if ( ctrl->isI2CSwarmCtrl()) setLocalI2C();
  else                              setLocalLHW();

}

void SwOSActor::setRemote() {
  
  SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_SETACTORSPEED  );
  cmd.data.actorSpeedCmd.index      = port;
  cmd.data.actorSpeedCmd.motionType = motionType;
  cmd.data.actorSpeedCmd.speed      = speed;
  cmd.data.actorSpeedCmd.rampUpT    = rampUpT;
  cmd.data.actorSpeedCmd.rampUpY    = rampUpY;
  cmd.send( );

}

void SwOSActor::setLocalI2C() {

  if ( ( ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && ( pwrDrive ) ) {
    pwrDrive->setMaxSpeed( pwrDriveMotor, speed );
  }

  if ( ( ctrl->getCPU() == FTSWARMDUINO_1V141 ) && ( ftDuino ) ) {
    ftDuino->setMotor( port, motionType, speed);
  }

}

void SwOSActor::setPWM( int16_t xin1, int16_t xin2, gpio_num_t pwm, uint32_t duty ) {

  // calc duty based on _highResolution
  uint32_t duty1 = duty;
  if ( (!highResolution) && (duty) ) duty1 = ( duty1 << 4 ) + 0xF;

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

void SwOSActor::setLocalLHW() {

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

void SwOSActor::setDistance( long distance, bool relative, bool dontSendToRemote ) {

  if   (!ctrl->isLocal()) {

    if (!dontSendToRemote) {
      // send remote
      SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_SETSTEPPERDISTANCE  );
      cmd.data.actorStepperCmd.index = port;
      cmd.data.actorStepperCmd.paraml = distance;
      cmd.data.actorStepperCmd.paramb = relative;
      cmd.send( );

    }

  } else if ( ( ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (pwrDrive ) ) {
    // set local
    if (relative) pwrDrive->setRelDistance( pwrDriveMotor, distance );
    else          pwrDrive->setAbsDistance( pwrDriveMotor, distance );

    distance = pwrDrive->getStepsToGo( pwrDriveMotor );

  }

}

long SwOSActor::getDistance( void ) {
  return distance;
}

void SwOSActor::startStop( bool start ) {

  if (!ctrl->isLocal() )  {
    // send remote
    SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_STEPPERSTARTSTOP  );
    cmd.data.actorStepperCmd.index  = port;
    cmd.data.actorStepperCmd.paramb = start;
    cmd.send( );

  } else if ( ( ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (pwrDrive ) ) {

    if (start) pwrDrive->startMoving( pwrDriveMotor );
    else       pwrDrive->stopMoving( pwrDriveMotor );
    
  }

}

void SwOSActor::setPosition( long position, bool dontSendToRemote ) {

  this->position = position;

  if   (!ctrl->isLocal()) {

    if (!dontSendToRemote) {
    
      // send remote
      SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_SETSTEPPERPOSITION  );
      cmd.data.actorStepperCmd.index = port;
      cmd.data.actorStepperCmd.paraml = position;
      cmd.send( );

    }

  } else if ( ( ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (pwrDrive ) ) {
    // set local
    pwrDrive->setPosition( pwrDriveMotor, position );
  }

}

long SwOSActor::getPosition( void ) {
  return position;
}

void SwOSActor::homing( long maxDistance ) {

  if   (!ctrl->isLocal()) {
    // send remote
    SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_STEPPERHOMING );
    cmd.data.actorStepperCmd.index = port;
    cmd.data.actorStepperCmd.paraml = maxDistance;
    cmd.send( );

  } else if ( ( ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (pwrDrive ) ) {
    // set local
    pwrDrive->homing(pwrDriveMotor, maxDistance );
  }

}

void SwOSActor::setHomingOffset( long offset ) {

  if   (!ctrl->isLocal()) {
    // send remote
    SwOSCom cmd( ctrl->macAddr, ctrl->serialNumber, CMD_SETSTEPPERHOMINGOFFSET );
    cmd.data.actorStepperCmd.index = port;
    cmd.data.actorStepperCmd.paraml = offset;
    cmd.send( );

  } else if ( ( ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (pwrDrive ) ) {
    // set local
    pwrDrive->homingOffset( pwrDriveMotor, offset );
  }

}

void SwOSActor::jsonize( JSONize *json, uint8_t id) {

  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variableUI32("subType",      actorType );
  json->variableUI32("motiontype",   getMotionType() );
  json->variableI16 ("speed",        getSpeed() );
  json->variableB( "highResolution", highResolution );
  json->endObject();
}

void SwOSActor::onTrigger( int32_t value ) {

  setSpeed( (int16_t) value );
  apply();

}

void SwOSActor::read( void ) {

}

void SwOSActor::setIsHoming( bool isHoming ) {
  this->motorIsHoming = isHoming;
}

bool SwOSActor::isHoming( void ) {
  return motorIsHoming;
}

void SwOSActor::setIsRunning( bool isRunning ) {
  this->motorIsRunning = isRunning;
}

bool SwOSActor::isRunning( void ) {
  return motorIsRunning;
}

void SwOSActor::setValue( long distance, long position, bool isHoming, bool isRunning ) {

  this->distance = distance;
  this->position = position;
  this->motorIsHoming = isHoming;
  this->motorIsRunning = isRunning;
}

/***************************************************
 *
 *   SwOSServo
 *
 ***************************************************/

 SwOSServo::SwOSServo(const char *name, uint8_t port, SwOSCtrl *ctrl) : SwOSIO( name, port, ctrl ) {

}

void SwOSServo::jsonize( JSONize *json, uint8_t id) {

  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variableI16("offset",   offset);
  json->variableI16("position", position);
  json->endObject();

}

void SwOSServo::setPosition( int16_t position, bool dontSendToRemote ) {
  
  this->position = position;

  // apply local or remote
  if (ctrl->isLocal())        setLocal();
  else if (!dontSendToRemote) setRemote();

}

void SwOSServo::setOffset( int16_t offset, bool dontSendToRemote ) {
  
  this->offset = offset;
 
  // apply local or remote
  if (ctrl->isLocal())        setLocal();
  else if (!dontSendToRemote) setRemote();

}

void SwOSServo::onTrigger( int32_t value ) {

  setPosition( (int16_t) value, false );

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
  cmd.data.servoCmd.index    = port;
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

SwOSRCServo::SwOSRCServo(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSAnalogInput *poti, SwOSActor *actor): SwOSServo( name, port, ctrl ) {

  this->poti  = poti;
  this->motor = actor;
  
}

SwOSRCServo::~SwOSRCServo() {
  
  if (poti)  delete poti;
  if (motor) delete motor;
  if (pid)   delete pid;

}

void SwOSRCServo::adjust(void) {

  // remote: nothing todo
  if (!ctrl->isLocal()) return;

  // read poti value to fille up the filters
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