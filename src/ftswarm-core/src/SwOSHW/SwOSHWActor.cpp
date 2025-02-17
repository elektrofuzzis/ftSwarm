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

const char ACTORICON[FTSWARM_MAXACTOR][20] = {
  "16_xmotor.svg",
  "20_xmmotor.svg",
  "17_tractor.svg",
  "18_encoder.svg",
  "19_lamp.svg",
  "23_valve.svg",
  "22_compressor.svg",
  "24_buzzer.svg"
};

const char ACTORTYPE[FTSWARM_MAXACTOR][20] = { "MOTOR", "XMMOTOR", "TRACTORMOTOR", "ENCODERMOTOR", "LAMP", "VALVE", "COMPRESSOR", "BUZZER", "STEPPER" };
 
/***************************************************
 *
 *   SwOSActor
 *
 ***************************************************/

 SwOSActor::SwOSActor(const char *name, uint8_t port, SwOSCtrl *ctrl):SwOSIO(name, port, ctrl ){

  // ftPwrDrive has a bitmap motor representation, so precalc the Mx values
  _pwrDriveMotor = 1 << _port;
  if ( _ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) _highResolution = true;

  // initialize local HW
  if (_ctrl->isLocal()) {
    if ( _ctrl->isI2CSwarmCtrl() ) {
      _setupI2C();
    } else {
      _setupLocal();
    }
  }

}

SwOSActor::~SwOSActor() {
  if (_ctrl->isLocal() ) { setSpeed(0); apply(); }
  if ( ledc_channel ) free( ledc_channel );
}

char * SwOSActor::getIcon() { 
  return (char *) ACTORICON[ _actorType ]; 
}; 

void SwOSActor::_setupI2C() {
  
}

void SwOSActor::_setupLocal() {
  // initialize local HW

  // set HW Pins
  _IN1 = GPIO_ACTOR[_ctrl->getCPU()][_port][0];
  _IN2 = GPIO_ACTOR[_ctrl->getCPU()][_port][1];
  
  // set digital ports _in1 & in2 to output
  gpio_config_t io_conf = {
    .pin_bit_mask = 0,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };
  if ( _IN1 != GPIO_NUM_NC ) io_conf.pin_bit_mask = io_conf.pin_bit_mask | (1ULL << _IN1);
  if ( _IN2 != GPIO_NUM_NC ) io_conf.pin_bit_mask = io_conf.pin_bit_mask | (1ULL << _IN2);
  gpio_config(&io_conf);
  
  // set motor driver off
  if ( _IN1 != GPIO_NUM_NC ) gpio_set_level( _IN1, 0 );
  if ( _IN2 != GPIO_NUM_NC ) gpio_set_level( _IN2, 0 );

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
  ledc_channel->gpio_num       = _IN1;
  ledc_channel->speed_mode     = LEDC_LOW_SPEED_MODE;
  ledc_channel->channel        = (ledc_channel_t) (_port);
  ledc_channel->intr_type      = LEDC_INTR_DISABLE;
  ledc_channel->timer_sel      = LEDC_TIMER_0;
  ledc_channel->duty           = 0; 
  ledc_channel->hpoint         = 0;
  ledc_channel->flags.output_invert = 1;

}

void SwOSActor::setMotionType( FtSwarmMotion_t motionType ) {

  _motionType = motionType;
  
}

void SwOSActor::setActorType( FtSwarmActor_t actorType, bool highResolution, bool dontSendToRemote ) { 

  _actorType = actorType;
  _highResolution = highResolution;

  if (_ctrl->isLocal()) { 
    
  } else if (!dontSendToRemote) {    
    // send SN, SETSENSORTYPE, _port, sensorType
    SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_SETACTORTYPE );
    cmd.data.actorTypeCmd.index        = _port;
    cmd.data.actorTypeCmd.actorType    = _actorType;
    cmd.data.actorTypeCmd.highResolution = highResolution;
    cmd.send( );

  }
  
};   

void SwOSActor::setAcceleration( uint32_t rampUpT,  uint32_t rampUpY ) {

  _rampUpT = rampUpT;
  _rampUpY = rampUpY;

}

void SwOSActor::getAcceleration( uint32_t *rampUpT,  uint32_t *rampUpY ) {
  *rampUpY = _rampUpY;
  *rampUpT = _rampUpT;
}

void SwOSActor::setSpeed( int16_t speed ) {

  // if no change is needed, return
  if ( speed == _speed ) return;

  // Motor, OnOff-Actors: set COAST or ON automatically
  if ( ( _actorType != FTSWARM_XMMOTOR ) && ( _actorType != FTSWARM_TRACTOR ) && ( _actorType != FTSWARM_ENCODER ) ) {
    if ( ( _speed != 0 ) && ( speed == 0 ) ) _motionType = FTSWARM_COAST;
    if ( ( _speed == 0 ) && ( speed != 0 ) ) _motionType = FTSWARM_ON;
  }
  
  // limit speed values
  int16_t maxSpeed = MAXSPEED256;
  if (_highResolution) maxSpeed = MAXSPEED4096;
  if      (speed> maxSpeed) _speed =  maxSpeed;
  else if (speed<-maxSpeed) _speed = -maxSpeed;
  else                      _speed =  speed;

}

void SwOSActor::apply(void) {

  // set speed values
  if      (!_ctrl->isLocal())        _setRemote();
  else if ( _ctrl->isI2CSwarmCtrl()) _setLocalI2C();
  else                               _setLocalLHW();

}

void SwOSActor::_setRemote() {
  
  SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_SETACTORSPEED  );
  cmd.data.actorSpeedCmd.index      = _port;
  cmd.data.actorSpeedCmd.motionType = _motionType;
  cmd.data.actorSpeedCmd.speed      = _speed;
  cmd.data.actorSpeedCmd.rampUpT    = _rampUpT;
  cmd.data.actorSpeedCmd.rampUpY    = _rampUpY;
  cmd.send( );

}

void SwOSActor::_setLocalI2C() {

  if ( ( _ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && ( pwrDrive ) ) {
    pwrDrive->setMaxSpeed( _pwrDriveMotor, _speed );
  }

  if ( ( _ctrl->getCPU() == FTSWARMDUINO_1V141 ) && ( ftDuino ) ) {
    ftDuino->setMotor( _port, _motionType, _speed);
  }

}

void SwOSActor::setPWM( int16_t in1, int16_t in2, gpio_num_t pwm, uint32_t duty ) {

  // calc duty based on _highResolution
  uint32_t duty1 = duty;
  if ( (!_highResolution) && (duty) ) duty1 = ( duty1 << 4 ) + 0xF;

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

    if ( _rampUpT | _rampUpY ) {
      // acceleration
      uint32_t oldDuty = ledc_get_duty( LEDC_LOW_SPEED_MODE, ledc_channel->channel );
      ESP_ERROR_CHECK( ledc_set_fade( LEDC_LOW_SPEED_MODE, ledc_channel->channel, oldDuty, ( oldDuty > duty1 ) ? LEDC_DUTY_DIR_DECREASE : LEDC_DUTY_DIR_INCREASE, _rampUpT, _rampUpY, duty1 ) );
      ESP_ERROR_CHECK( ledc_update_duty( LEDC_LOW_SPEED_MODE, ledc_channel->channel ) );
    
    } else {
      // pwm signal
      ESP_ERROR_CHECK( ledc_set_duty( LEDC_LOW_SPEED_MODE, ledc_channel->channel, duty1 ) );
      ESP_ERROR_CHECK( ledc_update_duty( LEDC_LOW_SPEED_MODE, ledc_channel->channel ) );
    }

  }

  // 3rd step: set static levels if applicable
  if ( _IN1 != GPIO_NUM_NC ) gpio_set_level( _IN1, in1 );
  if ( _IN2 != GPIO_NUM_NC ) gpio_set_level( _IN2, in2 );

}

void SwOSActor::_setLocalLHW() {

  // just in case of non-existent HW
  if ( ( _IN1 == GPIO_NUM_NC ) || ( _IN2 == GPIO_NUM_NC ) ) return;

  // calculate duty
  switch (_motionType) {
    case FTSWARM_COAST: // SLEEP HIGH, IN1 LOW, IN2 LOW
                        setPWM( 0, 0, _IN1, 0 );
                        break;
  
    case FTSWARM_BRAKE: // SLEEP HIGH, IN1 HIGH, IN2 HIGH
                        setPWM( 1, 1, _IN1, 0 );
                        break;

    case FTSWARM_ON:    if ( _speed <  0) {
                          // SLEEP HIGH, IN1 PWM, IN2 HIGH
                          setPWM( 0, 1, _IN1, abs(_speed) );
                        } else {
                          // SLEEP HIGH, IN1 HIGH, IN2 PWM
                          setPWM( 1, 0, _IN2, abs(_speed) );
                        }
                        break;

    default:            // SLEEP HIGH, IN1 LOW, IN2 LOW
                        setPWM( 0, 0, _IN1, 0 );
                        break;
  }  
  
}

void SwOSActor::setDistance( long distance, bool relative, bool dontSendToRemote ) {

  if   (!_ctrl->isLocal()) {

    if (!dontSendToRemote) {
      // send remote
      SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_SETSTEPPERDISTANCE  );
      cmd.data.actorStepperCmd.index = _port;
      cmd.data.actorStepperCmd.paraml = distance;
      cmd.data.actorStepperCmd.paramb = relative;
      cmd.send( );

    }

  } else if ( ( _ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (pwrDrive ) ) {
    // set local
    if (relative) pwrDrive->setRelDistance( _pwrDriveMotor, distance );
    else          pwrDrive->setAbsDistance( _pwrDriveMotor, distance );

    _distance = pwrDrive->getStepsToGo( _pwrDriveMotor );

  }

}

long SwOSActor::getDistance( void ) {
  return _distance;
}

void SwOSActor::startStop( bool start ) {

  if (!_ctrl->isLocal() )  {
    // send remote
    SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_STEPPERSTARTSTOP  );
    cmd.data.actorStepperCmd.index  = _port;
    cmd.data.actorStepperCmd.paramb = start;
    cmd.send( );

  } else if ( ( _ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (pwrDrive ) ) {

    if (start) pwrDrive->startMoving( _pwrDriveMotor );
    else       pwrDrive->stopMoving( _pwrDriveMotor );
    
  }

}

void SwOSActor::setPosition( long position, bool dontSendToRemote ) {

  _position = position;

  if   (!_ctrl->isLocal()) {

    if (!dontSendToRemote) {
    
      // send remote
      SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_SETSTEPPERPOSITION  );
      cmd.data.actorStepperCmd.index = _port;
      cmd.data.actorStepperCmd.paraml = position;
      cmd.send( );

    }

  } else if ( ( _ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (pwrDrive ) ) {
    // set local
    pwrDrive->setPosition( _pwrDriveMotor, position );
  }

}

long SwOSActor::getPosition( void ) {
  return _position;
}

void SwOSActor::homing( long maxDistance ) {

  if   (!_ctrl->isLocal()) {
    // send remote
    SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_STEPPERHOMING );
    cmd.data.actorStepperCmd.index = _port;
    cmd.data.actorStepperCmd.paraml = maxDistance;
    cmd.send( );

  } else if ( ( _ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (pwrDrive ) ) {
    // set local
    pwrDrive->homing( _pwrDriveMotor, maxDistance );
  }

}

void SwOSActor::setHomingOffset( long offset ) {

  if   (!_ctrl->isLocal()) {
    // send remote
    SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_SETSTEPPERHOMINGOFFSET );
    cmd.data.actorStepperCmd.index = _port;
    cmd.data.actorStepperCmd.paraml = offset;
    cmd.send( );

  } else if ( ( _ctrl->getCPU() == FTSWARMPWRDRIVE_1V141 ) && (pwrDrive ) ) {
    // set local
    pwrDrive->homingOffset( _pwrDriveMotor, offset );
  }

}

void SwOSActor::jsonize( JSONize *json, uint8_t id) {

  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variable("subType",    (char *) ACTORTYPE[ _actorType ] );
  json->variableUI32("motiontype", getMotionType() );
  json->variableI16 ("speed",      getSpeed() );
  json->variableB( "highResolution", _highResolution );
  json->endObject();
}

void SwOSActor::onTrigger( int32_t value ) {

  setSpeed( (int16_t) value );
  apply();

}

void SwOSActor::read( void ) {

}

void SwOSActor::setIsHoming( bool isHoming ) {
  _isHoming = isHoming;
}

bool SwOSActor::isHoming( void ) {
  return _isHoming;
}

void SwOSActor::setIsRunning( bool isRunning ) {
  _isRunning = isRunning;
}

bool SwOSActor::isRunning( void ) {
  return _isRunning;
}

void SwOSActor::setValue( long distance, long position, bool isHoming, bool isRunning ) {

  //printf("setValue: %s %ld %d %d\n", getName(), distance, position, isHoming, isRunning );
  _distance = distance;
  _position = position;
  _isHoming = isHoming;
  _isRunning = isRunning;
}

/***************************************************
 *
 *   SwOSServo
 *
 ***************************************************/

SwOSServo::SwOSServo(const char *name, uint8_t port, SwOSCtrl *ctrl) : SwOSIO( name, port, ctrl ) {

  // initialize local HW
  if (ctrl->isLocal()) _setupLocal();
}

void SwOSServo::_setupLocal() {
  // initialize local HW

  _SERVO = SERVO[_ctrl->getCPU()][_port];

  // set digital port  to output
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << _SERVO);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);

  // ledc channels
  _channelSERVO = (ledc_channel_t) (4 + _port);

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
      .gpio_num       = _SERVO,
      .speed_mode     = LEDC_LOW_SPEED_MODE,
      .channel        = _channelSERVO,
      .intr_type      = LEDC_INTR_DISABLE,
      .timer_sel      = LEDC_TIMER_1,
      .duty           = 0, // Set duty to 0%
      .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    // set coast
    _setLocal();

}

void SwOSServo::_setLocal() {

  // calc duty
  float p = _offset + _position;
  if (p <   0 ) p =   0;
  if (p > 255 ) p = 255;

  // 1 .. 2 ms pulse
  float ticks = 0.2*p+51;

  // set duty
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, _channelSERVO, (uint16_t)ticks));

  // update duty
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, _channelSERVO));

}

void SwOSServo::_setRemote( ) {
  
  SwOSCom cmd( _ctrl->macAddr, _ctrl->serialNumber, CMD_SETSERVO );
  cmd.data.servoCmd.index    = _port;
  cmd.data.servoCmd.position = _position;
  cmd.data.servoCmd.offset   = _offset;
  cmd.send( );
}

void SwOSServo::jsonize( JSONize *json, uint8_t id) {
  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variableI16("offset",   _offset);
  json->variableI16("position", _position);
  json->endObject();
}

void SwOSServo::setPosition( int16_t position, bool dontSendToRemote ) {
  _position = position;

  // apply local or remote
  if (_ctrl->isLocal())       _setLocal();
  else if (!dontSendToRemote) _setRemote();

}

void SwOSServo::setOffset( int16_t offset, bool dontSendToRemote ) {
  _offset = offset;
 
  // apply local or remote
  if (_ctrl->isLocal())       _setLocal();
  else if (!dontSendToRemote) _setRemote();

}

void SwOSServo::onTrigger( int32_t value ) {

  setPosition( (int16_t) value, false );

}