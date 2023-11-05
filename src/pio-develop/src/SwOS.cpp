/*
 * ftSwarm.cpp
 *
 * framework to build a ftSwarm application
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#include "SwOS.h" 
#include "SwOSSwarm.h"
#include "easyKey.h"

FtSwarmIOType_t sensorType2IOType( FtSwarmSensor_t sensor2IOType ) {

  switch ( sensor2IOType ) {
    case FTSWARM_DIGITAL:
    case FTSWARM_SWITCH:
    case FTSWARM_REEDSWITCH:
    case FTSWARM_LIGHTBARRIER:  return FTSWARM_DIGITALINPUT;
    default:                    return FTSWARM_ANALOGINPUT;
  }

}

FtSwarm ftSwarm;

// **** FtSwarmIO ****

FtSwarmIO::FtSwarmIO( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmIOType_t ioType ) {
  // constructor: register at myOSSwarm and get a pointer to myself

  bool firstTry = true;

  while (!me) {

    // check, if IO is online
    myOSSwarm.lock(); 
    me = (SwOSIOHandle_t) myOSSwarm.getIO( serialNumber, port, ioType);
    myOSSwarm.unlock();

    // no success, wait 25 ms
    if ( (!me) && ( firstTry ) ) {
      ESP_LOGD( LOGFTSWARM, "waiting on device" );
      myOSSwarm.setState( WAITING );
      firstTry = false;
    }
    
    if (!me) vTaskDelay( 25 / portTICK_PERIOD_MS );
  }

  // register myself
  myOSSwarm.lock(); 
  static_cast<SwOSIO *>(me)->take();
  myOSSwarm.unlock(); 
  
};

FtSwarmIO::FtSwarmIO( const char *name, FtSwarmIOType_t ioType ) {
  // constructor: register at myOSSwarm using a name/alias

  bool firstTry = true;

  while (!me) {

    myOSSwarm.lock();
    me = myOSSwarm.getIO( name, ioType );

    // no success, wait 25 ms
    if ( (!me) && ( firstTry ) ) {
      ESP_LOGD( LOGFTSWARM, "waiting on device" );
      myOSSwarm.setState( WAITING );
      firstTry = false;
    }
    
    myOSSwarm.unlock();

    if (!me) vTaskDelay( 25 / portTICK_PERIOD_MS );
 
  }

  // register myself
  myOSSwarm.lock(); 
  static_cast<SwOSIO *>(me)->take();
  myOSSwarm.unlock(); 
  
}

FtSwarmIO::~FtSwarmIO() {

  // unregister myself
  if (me) {
    myOSSwarm.lock(); 
    static_cast<SwOSIO *>(me)->give();
    myOSSwarm.unlock(); 
  }
}

bool FtSwarmIO::isOnline() { 
  // check if I'm online
  // TODO!
  // return (me)?me->isOnline():false;
  return true;
};

// **** FtSwarmInput

FtSwarmInput::FtSwarmInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmIOType_t ioType):FtSwarmIO( serialNumber, port, ioType ) {
  // constructor: register at myOSSwarm and get a pointer to myself 

};

FtSwarmInput::FtSwarmInput( const char *name, FtSwarmIOType_t ioType ):FtSwarmIO( name, ioType ) {

};

void FtSwarmInput::onTrigger( FtSwarmTrigger_t triggerEvent, FtSwarmIO *actor, int32_t p1 ) {

  // set trigger using static values
  if ( (me) && (actor) ) {
    myOSSwarm.lock();
    static_cast<SwOSInput *>(me)->registerEvent( triggerEvent, (SwOSIO *)actor->me, false, p1 );
    myOSSwarm.unlock();
  }

};

void FtSwarmInput::onTrigger( FtSwarmTrigger_t triggerEvent, FtSwarmIO *actor )  {

  // set trigger using port's value
  if ( (me) && (actor) ) {
    myOSSwarm.lock();
    static_cast<SwOSInput *>(me)->registerEvent( triggerEvent, (SwOSIO *)actor->me, true, 0 );
    myOSSwarm.unlock();
  }

};


// **** FtSwarmDigitalInput ****

void FtSwarmDigitalInput::setSensorType( FtSwarmSensor_t sensorType, bool normallyOpen ) {

  // set sensor type
  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSDigitalInput *>(me)->setSensorType( sensorType, normallyOpen, true );
    myOSSwarm.unlock();
  }

}

FtSwarmDigitalInput::FtSwarmDigitalInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmSensor_t sensorType, bool normallyOpen):FtSwarmInput( serialNumber, port, FTSWARM_DIGITALINPUT ) {
  
  setSensorType( sensorType, normallyOpen );

}

FtSwarmDigitalInput::FtSwarmDigitalInput( const char *name, FtSwarmSensor_t sensorType, bool normallyOpen ):FtSwarmInput( name, FTSWARM_DIGITALINPUT) {

  setSensorType( sensorType, normallyOpen );

}


// some facades
bool FtSwarmDigitalInput::isPressed()            { return getState(); };
bool FtSwarmDigitalInput::isReleased()           { return !getState(); };
bool FtSwarmDigitalInput::hasToggledUp()         { return ( getToggle() == FTSWARM_TOGGLEUP ); };
bool FtSwarmDigitalInput::hasToggledDown()       { return ( getToggle() == FTSWARM_TOGGLEDOWN ); };

// real stuff
FtSwarmToggle_t FtSwarmDigitalInput::getToggle() { 

  if (!me) return FTSWARM_NOTOGGLE;

  myOSSwarm.lock();
  FtSwarmToggle_t xReturn = static_cast<SwOSDigitalInput *>(me)->getToggle();
  myOSSwarm.unlock();

  return xReturn;
};

bool FtSwarmDigitalInput::getState() { 

  if (!me) return false;

  myOSSwarm.lock();
  bool xReturn = (static_cast<SwOSDigitalInput *>(me)->getValueUI32()>0);
  myOSSwarm.unlock();

  return xReturn;
};

// **** FtSwarmSwitch ****

FtSwarmSwitch::FtSwarmSwitch( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen):FtSwarmDigitalInput( serialNumber, port, FTSWARM_SWITCH, normallyOpen ) {};
FtSwarmSwitch::FtSwarmSwitch( const char * name, bool normallyOpen):FtSwarmDigitalInput( name, FTSWARM_SWITCH, normallyOpen ) {};

// **** FtSwarmLightBarrier ****

FtSwarmLightBarrier::FtSwarmLightBarrier( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen):FtSwarmDigitalInput( serialNumber, port, FTSWARM_LIGHTBARRIER, normallyOpen ) {};
FtSwarmLightBarrier::FtSwarmLightBarrier( const char * name, bool normallyOpen):FtSwarmDigitalInput( name, FTSWARM_LIGHTBARRIER, normallyOpen ) {};

// **** FtSwarmReedSwitch ****

FtSwarmReedSwitch::FtSwarmReedSwitch( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen):FtSwarmDigitalInput( serialNumber, port, FTSWARM_REEDSWITCH, normallyOpen ) {};
FtSwarmReedSwitch::FtSwarmReedSwitch( const char * name, bool normallyOpen):FtSwarmDigitalInput( name, FTSWARM_REEDSWITCH, normallyOpen ) {};

// **** FtSwarmButton ****

FtSwarmButton::FtSwarmButton( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmIO( serialNumber, port, FTSWARM_BUTTON ) {};
FtSwarmButton::FtSwarmButton( const char *name):FtSwarmIO( name,FTSWARM_BUTTON ) {};

// real stuff

void FtSwarmButton::onTrigger( FtSwarmTrigger_t triggerEvent, FtSwarmIO *actor, int32_t p1 ) {

  // set trigger using static values
  if ( (me) && (actor) ) {
    myOSSwarm.lock();
    static_cast<SwOSInput *>(me)->registerEvent( triggerEvent, (SwOSIO *)actor->me, false, p1 );
    myOSSwarm.unlock();
  }

};

void FtSwarmButton::onTrigger( FtSwarmTrigger_t triggerEvent, FtSwarmIO *actor )  {

  // set trigger using port's value
  if ( (me) && (actor) ) {
    myOSSwarm.lock();
    static_cast<SwOSInput *>(me)->registerEvent( triggerEvent, (SwOSIO *)actor->me, true, 0 );
    myOSSwarm.unlock();
  }

};

FtSwarmToggle_t FtSwarmButton::FtSwarmButton::getToggle() { 

  if (!me) return FTSWARM_NOTOGGLE;

  myOSSwarm.lock();
  FtSwarmToggle_t xReturn = static_cast<SwOSButton *>(me)->getToggle();
  myOSSwarm.unlock();

  return xReturn;
};

bool FtSwarmButton::getState() { 

  if (!me) return false;

  myOSSwarm.lock();
  bool xReturn = (static_cast<SwOSButton *>(me)->getState());
  myOSSwarm.unlock();

  return xReturn;
};

// some facades
bool FtSwarmButton::isPressed()            { return getState(); };
bool FtSwarmButton::isReleased()           { return !getState(); };
bool FtSwarmButton::hasToggledUp()         { return ( getToggle() == FTSWARM_TOGGLEUP ); };
bool FtSwarmButton::hasToggledDown()       { return ( getToggle() == FTSWARM_TOGGLEDOWN ); };

// **** FtSwarmAnalogInput ****

void FtSwarmAnalogInput::setSensorType( FtSwarmSensor_t sensorType ) {

  // set sensor type
  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSAnalogInput *>(me)->setSensorType( sensorType, true );
    myOSSwarm.unlock();
  }

}

FtSwarmAnalogInput::FtSwarmAnalogInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmSensor_t sensorType):FtSwarmInput( serialNumber, port, FTSWARM_ANALOGINPUT ) {

  setSensorType( sensorType );

};

FtSwarmAnalogInput::FtSwarmAnalogInput( const char *name, FtSwarmSensor_t sensorType ):FtSwarmInput( name, FTSWARM_ANALOGINPUT ) {

  setSensorType( sensorType );

};

int32_t FtSwarmAnalogInput::getValue() {

  if (!me) return 0;

  myOSSwarm.lock();
  uint16_t xReturn = (static_cast<SwOSInput *>(me)->getValueUI32());
  myOSSwarm.unlock();

  return xReturn;
};

// **** FtSwarmVoltmeter ****

FtSwarmVoltmeter::FtSwarmVoltmeter( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmAnalogInput( serialNumber, port, FTSWARM_VOLTMETER ) {};
FtSwarmVoltmeter::FtSwarmVoltmeter( const char *name ):FtSwarmAnalogInput( name, FTSWARM_VOLTMETER ) {};

float FtSwarmVoltmeter::getVoltage() {

  if (!me) return 0;

  myOSSwarm.lock();
  float xReturn = (static_cast<SwOSAnalogInput *>(me)->getVoltage());
  myOSSwarm.unlock();

  return xReturn;
};

// **** FtSwarmOhmmeter ****

FtSwarmOhmmeter::FtSwarmOhmmeter( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmAnalogInput( serialNumber, port, FTSWARM_OHMMETER ) {};
FtSwarmOhmmeter::FtSwarmOhmmeter( const char *name ):FtSwarmAnalogInput( name, FTSWARM_OHMMETER ) {};

float FtSwarmOhmmeter::getResistance() {

  if (!me) return 0;

  myOSSwarm.lock();
  float xReturn = (static_cast<SwOSAnalogInput *>(me)->getResistance());
  myOSSwarm.unlock();

  return xReturn;
};

// **** FtSwarmThermometer ****

FtSwarmThermometer::FtSwarmThermometer( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmAnalogInput( serialNumber, port, FTSWARM_THERMOMETER ) {};
FtSwarmThermometer::FtSwarmThermometer( const char *name ):FtSwarmAnalogInput( name, FTSWARM_THERMOMETER ) {};

float FtSwarmThermometer::getCelcius() {

  if (!me) return 0;

  myOSSwarm.lock();
  float xReturn = (static_cast<SwOSAnalogInput *>(me)->getCelcius());
  myOSSwarm.unlock();

  return xReturn;
};

float FtSwarmThermometer::getKelvin() {
  if (!me) return 0;

  myOSSwarm.lock();
  float xReturn = (static_cast<SwOSAnalogInput *>(me)->getKelvin());
  myOSSwarm.unlock();

  return xReturn;
}

float FtSwarmThermometer::getFahrenheit() {
  if (!me) return 0;

  myOSSwarm.lock();
  float xReturn = (static_cast<SwOSAnalogInput *>(me)->getFahrenheit());
  myOSSwarm.unlock();

  return xReturn;
}


// **** FtSwarmLDR ****

FtSwarmLDR::FtSwarmLDR( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmAnalogInput( serialNumber, port, FTSWARM_LDR ) {};
FtSwarmLDR::FtSwarmLDR( const char *name ):FtSwarmAnalogInput( name, FTSWARM_LDR ) {};

// **** FtSwarmActor

FtSwarmActor::FtSwarmActor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmActor_t actorType ):FtSwarmIO(serialNumber, port, FTSWARM_ACTOR ) {
  // constructor: register at myOSSwarm and get a pointer to myself 

  // set sensor type
  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSActor *>(me)->setActorType( actorType, false );
    myOSSwarm.unlock();
  }

};

FtSwarmActor::FtSwarmActor( const char *name, FtSwarmActor_t actorType):FtSwarmIO( name, FTSWARM_ACTOR ) {
  // constructor: register at myOSSwarm and get a pointer to myself 

  // set sensor type
  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSActor *>(me)->setActorType( actorType, false );
    myOSSwarm.unlock();
  }

};

// **** FtSwarmMotor ****

FtSwarmMotor::FtSwarmMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmActor_t actorType):FtSwarmActor( serialNumber, port, actorType) {};
FtSwarmMotor::FtSwarmMotor( const char *name, FtSwarmActor_t actorType):FtSwarmActor( name, actorType ) {};
    
void FtSwarmMotor::setSpeed( int16_t speed ) {
  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSActor *>(me)->setSpeed( speed );
    myOSSwarm.unlock();
  }
}

uint16_t FtSwarmMotor::getSpeed() {

  if (!me) return 0;

  myOSSwarm.lock();
  uint16_t xReturn = (static_cast<SwOSActor *>(me)->getSpeed());
  myOSSwarm.unlock();

  return xReturn;
};

void FtSwarmMotor::setAcceleration( uint32_t rampUpT, uint32_t rampUpY ) {
  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSActor *>(me)->setAcceleration( rampUpT, rampUpY );
    myOSSwarm.unlock();
  }
}

void FtSwarmMotor::getAcceleration( uint32_t *rampUpT, uint32_t *rampUpY ) {

  if (!me) return;

  myOSSwarm.lock();
  static_cast<SwOSActor *>(me)->getAcceleration( rampUpT, rampUpY );
  myOSSwarm.unlock();

};

// **** FtSwarmTractorMotor

FtSwarmTractorMotor::FtSwarmTractorMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmActor_t actorType):FtSwarmMotor( serialNumber, port, actorType) {};
FtSwarmTractorMotor::FtSwarmTractorMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmMotor( serialNumber, port, FTSWARM_TRACTOR) {};
FtSwarmTractorMotor::FtSwarmTractorMotor( const char *name, FtSwarmActor_t actorType ):FtSwarmMotor( name, actorType ) {};
FtSwarmTractorMotor::FtSwarmTractorMotor( const char *name ):FtSwarmMotor( name, FTSWARM_TRACTOR ) {};

void FtSwarmTractorMotor::setMotionType( FtSwarmMotion_t motionType ) {
  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSActor *>(me)->setMotionType( motionType );
    myOSSwarm.unlock();
  }
}

FtSwarmMotion_t FtSwarmTractorMotor::getMotionType( void ) {
  
  FtSwarmMotion_t motionType = FTSWARM_COAST;
  
  if (me) {
    myOSSwarm.lock();
    motionType = static_cast<SwOSActor *>(me)->getMotionType( );
    myOSSwarm.unlock();
  }

  return motionType;
  
}

// **** FtSwarmXMMotor

FtSwarmXMMotor::FtSwarmXMMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmTractorMotor( serialNumber, port, FTSWARM_XMMOTOR ) {};
FtSwarmXMMotor::FtSwarmXMMotor( const char *name ):FtSwarmTractorMotor( name, FTSWARM_XMMOTOR ) {};

// **** FtSwarmEncoderMotor

FtSwarmEncoderMotor::FtSwarmEncoderMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmTractorMotor( serialNumber, port, FTSWARM_ENCODER ) {};
FtSwarmEncoderMotor::FtSwarmEncoderMotor( const char *name ):FtSwarmTractorMotor( name, FTSWARM_ENCODER ) {};

// **** FtSwarmStepperMotor

FtSwarmStepperMotor::FtSwarmStepperMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmTractorMotor( serialNumber, port, FTSWARM_STEPPER ) {};
FtSwarmStepperMotor::FtSwarmStepperMotor( const char * name ):FtSwarmTractorMotor( name, FTSWARM_STEPPER ) {};

void FtSwarmStepperMotor::setDistance( long distance, bool relative ) {

  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSActor *>(me)->setDistance( distance, relative, false );
    myOSSwarm.unlock();
  }

}

long FtSwarmStepperMotor::getDistance( void ) {

  long distance = 0;

  if (me) {
    myOSSwarm.lock();
    distance = static_cast<SwOSActor *>(me)->getDistance( );
    myOSSwarm.unlock();
  }

  return distance;

}

bool FtSwarmStepperMotor::isRunning( void ) {

  bool result = false;

  if (me) {
    myOSSwarm.lock();
    result = static_cast<SwOSActor *>(me)->isRunning( );
    myOSSwarm.unlock();
  }

  return result;

}

void FtSwarmStepperMotor::run( void ) {

  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSActor *>(me)->startStop( true );
    myOSSwarm.unlock();
  }

}

void FtSwarmStepperMotor::stop( void ) {

  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSActor *>(me)->startStop( false );
    myOSSwarm.unlock();
  }

}

void FtSwarmStepperMotor::setPosition( long position ) {

  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSActor *>(me)->setPosition( position, false );
    myOSSwarm.unlock();
  }

}

long FtSwarmStepperMotor::getPosition( void ) {

  long position = 0;

  if (me) {
    myOSSwarm.lock();
    position = static_cast<SwOSActor *>(me)->getPosition( );
    myOSSwarm.unlock();
  }

  return position;

}

bool FtSwarmStepperMotor::isHoming( void ) {

  bool result = false;

  if (me) {
    myOSSwarm.lock();
    result = static_cast<SwOSActor *>(me)->isHoming( );
    myOSSwarm.unlock();
  }

  return result;

}

void FtSwarmStepperMotor::homing( long maxDistance ) {

  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSActor *>(me)->homing( maxDistance );
    myOSSwarm.unlock();
  }

}

void FtSwarmStepperMotor::setHomingOffset( long offset ) {

  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSActor *>(me)->setHomingOffset( offset );
    myOSSwarm.unlock();
  }

}

// **** FtSwarmOnOffActor ****

FtSwarmOnOffActor::FtSwarmOnOffActor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmActor_t actorType):FtSwarmActor( serialNumber, port, actorType ) {};
FtSwarmOnOffActor::FtSwarmOnOffActor( const char *name, FtSwarmActor_t actorType ):FtSwarmActor( name, actorType ) {};

void FtSwarmOnOffActor::on( int16_t speed ) {
  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSActor *>(me)->setSpeed( speed);
    myOSSwarm.unlock();
  }
}

void FtSwarmOnOffActor::off( void ) {
  on(0);
}

// **** FtSwarmLamp ****

FtSwarmLamp::FtSwarmLamp( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmOnOffActor( serialNumber, port, FTSWARM_LAMP ) {};
FtSwarmLamp::FtSwarmLamp( const char *name ):FtSwarmOnOffActor( name, FTSWARM_LAMP ) {};

// **** FtSwarmValve ****

FtSwarmValve::FtSwarmValve( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmOnOffActor( serialNumber, port, FTSWARM_VALVE ) {};
FtSwarmValve::FtSwarmValve( const char *name ):FtSwarmOnOffActor( name, FTSWARM_LAMP ) {};


// **** FtSwarmCompressor ****

FtSwarmCompressor::FtSwarmCompressor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmOnOffActor( serialNumber, port, FTSWARM_VALVE ) {};
FtSwarmCompressor::FtSwarmCompressor( const char *name ):FtSwarmOnOffActor( name, FTSWARM_COMPRESSOR ) {};

// **** FtSwarmBuzzer ****

FtSwarmBuzzer::FtSwarmBuzzer( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmOnOffActor( serialNumber, port, FTSWARM_VALVE ) {};
FtSwarmBuzzer::FtSwarmBuzzer( const char *name ):FtSwarmOnOffActor( name, FTSWARM_BUZZER ) {};


// **** FtSwarmJoystick ****

FtSwarmJoystick::FtSwarmJoystick( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmIO(serialNumber, port, FTSWARM_JOYSTICK) {

  // add joystick button
  switch (port) {
    case FTSWARM_JOY1: button = myOSSwarm.getIO( serialNumber, FTSWARM_J1, FTSWARM_BUTTON); break;
    case FTSWARM_JOY2: button = myOSSwarm.getIO( serialNumber, FTSWARM_J2, FTSWARM_BUTTON); break;
    default:           button = NULL;
  }

};

FtSwarmJoystick::FtSwarmJoystick( const char *name ):FtSwarmIO( name, FTSWARM_JOYSTICK ) {

  // add joystick button
  SwOSJoystick *joystick = static_cast<SwOSJoystick *>(me);
  switch (joystick->getPort()) {
    case FTSWARM_JOY1: button = myOSSwarm.getIO( joystick->getCtrl()->serialNumber, FTSWARM_J1, FTSWARM_BUTTON); break;
    case FTSWARM_JOY2: button = myOSSwarm.getIO( joystick->getCtrl()->serialNumber, FTSWARM_J2, FTSWARM_BUTTON); break;
    default:           button = NULL;
  }

};

int16_t FtSwarmJoystick::getFB() {
  int16_t FB, LR;
  boolean b;
  getValue( &FB, &LR, &b );
  return FB;
}

int16_t FtSwarmJoystick::getLR() {
  int16_t FB, LR;
  boolean b;
  getValue( &FB, &LR, &b );
  return LR;
}

boolean FtSwarmJoystick::getButtonState() {
  int16_t FB, LR;
  boolean b;
  getValue( &FB, &LR, &b );
  return b;
}

void FtSwarmJoystick::getValue( int16_t *FB, int16_t *LR, boolean *buttonState ) {

  if (!me) return;

  myOSSwarm.lock();
  static_cast<SwOSJoystick *>(me)->getValue(FB, LR);
  if (button) *buttonState = static_cast<SwOSButton *>(button)->getState();
  myOSSwarm.unlock();
}

void FtSwarmJoystick::onTriggerLR( FtSwarmTrigger_t triggerEvent, FtSwarmIO *actor, int32_t p1 ) {

  // set trigger using static values
  if ( (me) && (actor) ) {
    myOSSwarm.lock();
    static_cast<SwOSJoystick *>(me)->triggerLR.registerEvent( triggerEvent, (SwOSIO *)actor->me, false, p1 );
    myOSSwarm.unlock();
  }

};

void FtSwarmJoystick::onTriggerLR( FtSwarmTrigger_t triggerEvent, FtSwarmIO *actor )  {

  // set trigger using port's value
  if ( (me) && (actor) ) {
    myOSSwarm.lock();
    static_cast<SwOSJoystick *>(me)->triggerLR.registerEvent( triggerEvent, (SwOSIO *)actor->me, true, 0 );
    myOSSwarm.unlock();
  }

};

void FtSwarmJoystick::onTriggerFB( FtSwarmTrigger_t triggerEvent, FtSwarmIO *actor, int32_t p1 ) {

  // set trigger using static values
  if ( (me) && (actor) ) {
    myOSSwarm.lock();
    static_cast<SwOSJoystick *>(me)->triggerFB.registerEvent( triggerEvent, (SwOSIO *)actor->me, false, p1 );
    myOSSwarm.unlock();
  }

};

void FtSwarmJoystick::onTriggerFB( FtSwarmTrigger_t triggerEvent, FtSwarmIO *actor )  {

  // set trigger using port's value
  if ( (me) && (actor) ) {
    myOSSwarm.lock();
    static_cast<SwOSJoystick *>(me)->triggerFB.registerEvent( triggerEvent, (SwOSIO *)actor->me, true, 0 );
    myOSSwarm.unlock();
  }

};

// **** FtSwarmPixel ****

FtSwarmPixel::FtSwarmPixel( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmIO( serialNumber, port, FTSWARM_PIXEL) {};
FtSwarmPixel::FtSwarmPixel( const char *name ):FtSwarmIO( name, FTSWARM_PIXEL ) {};

uint8_t FtSwarmPixel::getBrightness() {

  if (!me) return 0;
  
  myOSSwarm.lock();
  uint8_t xReturn = (static_cast<SwOSPixel *>(me)->getBrightness());
  myOSSwarm.unlock();

  return xReturn;
};

void FtSwarmPixel::setBrightness(uint8_t brightness) {

  if (!me) return;

  myOSSwarm.lock();
  static_cast<SwOSPixel *>(me)->setBrightness(brightness);
  myOSSwarm.unlock();
}

uint32_t FtSwarmPixel::getColor() {

  if (!me) return 0;
  
  myOSSwarm.lock();
  uint32_t xReturn = (static_cast<SwOSPixel *>(me)->getColor());
  myOSSwarm.unlock();

  return xReturn;
};

void FtSwarmPixel::setColor(uint32_t color) {

  if (!me) return;

  myOSSwarm.lock();
  static_cast<SwOSPixel *>(me)->setColor(color);
  myOSSwarm.unlock();
}

// **** FtSwarmI2C   ****

FtSwarmI2C::FtSwarmI2C( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port ) : FtSwarmIO( serialNumber, port, FTSWARM_I2C ) {
}
    
FtSwarmI2C::FtSwarmI2C( const char *name ) : FtSwarmIO( name, FTSWARM_I2C ) {
}

uint8_t FtSwarmI2C::getRegister(uint8_t reg) {
    
  if (!me) return 0;
  
  myOSSwarm.lock();
  uint8_t xReturn = (static_cast<SwOSI2C *>(me)->getRegister( reg ));
  myOSSwarm.unlock();

  return xReturn;
}

void  FtSwarmI2C::setRegister(uint8_t reg, uint8_t value) {

  if (!me) return;

  myOSSwarm.lock();
  static_cast<SwOSI2C *>(me)->setRegister(reg, value);
  myOSSwarm.unlock();
}

void FtSwarmI2C::onTrigger( FtSwarmTrigger_t triggerEvent, FtSwarmIO *actor, int32_t p1 ) {

  // set trigger using static values
  if ( (me) && (actor) ) {
    myOSSwarm.lock();
    static_cast<SwOSI2C *>(me)->registerEvent( triggerEvent, (SwOSIO *)actor->me, false, p1 );
    myOSSwarm.unlock();
  }

};

// **** FtSwarmServo ****

FtSwarmServo::FtSwarmServo( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmIO( serialNumber, port, FTSWARM_SERVO) {};
FtSwarmServo::FtSwarmServo( const char *name ):FtSwarmIO( name, FTSWARM_SERVO ) {};

int16_t FtSwarmServo::getPosition() {

  if (!me) return 0;
  
  myOSSwarm.lock();
  int16_t xReturn = (static_cast<SwOSServo *>(me)->getPosition());
  myOSSwarm.unlock();

  return xReturn;
};

void FtSwarmServo::setPosition(int16_t position) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSServo *>(me)->setPosition(position, false);
  myOSSwarm.unlock();
}

int16_t FtSwarmServo::getOffset() {

  if (!me) return 0;
  
  myOSSwarm.lock();
  int16_t xReturn = (static_cast<SwOSServo *>(me)->getOffset());
  myOSSwarm.unlock();

  return xReturn;
};

void FtSwarmServo::setOffset(int16_t offset) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSServo *>(me)->setOffset(offset, false);
  myOSSwarm.unlock();
}

// **** FtSwarmOLED ****

void FtSwarmOLED::display(void) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->display();
  myOSSwarm.unlock();
}

void FtSwarmOLED::invertDisplay(bool i) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->invertDisplay( i );
  myOSSwarm.unlock();
}

void FtSwarmOLED::dim(bool dim) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->dim( dim );
  myOSSwarm.unlock();
}

void FtSwarmOLED::drawPixel(int16_t x, int16_t y, bool white ) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->drawPixel( x, y, white );
  myOSSwarm.unlock();
}

void FtSwarmOLED::setRotation(uint8_t r) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->setRotation( r );
  myOSSwarm.unlock();
}

void FtSwarmOLED::fillScreen(bool white) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->fillScreen( white );
  myOSSwarm.unlock();
}

void FtSwarmOLED::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool white ) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->drawLine( x0, y0, x1, y1, white );
  myOSSwarm.unlock();
}

void FtSwarmOLED::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, bool fill, bool white ) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->drawRect( x, y, w, h, fill, white );
  myOSSwarm.unlock();
}

void FtSwarmOLED::drawCircle(int16_t x0, int16_t y0, int16_t r, bool fill, bool white ) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->drawCircle( x0, y0, r, fill, white );
  myOSSwarm.unlock();
}

void FtSwarmOLED::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool fill, bool white ) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->drawTriangle( x0, y0, x1, y1, x2, y2, fill, white );
  myOSSwarm.unlock();
}

void FtSwarmOLED::drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, bool fill, bool white ) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->drawRoundRect( x0, y0, w, h, radius, fill, white );
  myOSSwarm.unlock();
}

void FtSwarmOLED::drawChar(int16_t x, int16_t y, unsigned char c, bool color, bool bg, uint8_t size_x, uint8_t size_y) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->drawChar(x, y, c, color, bg, size_x, size_y );
  myOSSwarm.unlock();
}

void FtSwarmOLED::getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->getTextBounds( string, x, y, x1, y1, w, h );
  myOSSwarm.unlock();
}

void FtSwarmOLED::setTextSize(uint8_t sx, uint8_t sy) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->setTextSize( sx, sy );
  myOSSwarm.unlock();
}

void FtSwarmOLED::setCursor(int16_t x, int16_t y) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->setCursor( x, y );
  myOSSwarm.unlock();
}
 
void FtSwarmOLED::setTextColor(bool c, bool bg) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->setTextColor( c, bg );
  myOSSwarm.unlock();
}

void FtSwarmOLED::setTextWrap(bool w) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->setTextWrap( w );
  myOSSwarm.unlock();
}

void FtSwarmOLED::write( char *str, int16_t x, int16_t y, FtSwarmAlign_t align, bool fill ) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->write( str, x, y, align, fill );
  myOSSwarm.unlock();
}

void FtSwarmOLED::write( char *str ) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->write( str );
  myOSSwarm.unlock();
}

int16_t FtSwarmOLED::getWidth(void) {

  if (!me) return 0;
  
  myOSSwarm.lock();
  int16_t result = static_cast<SwOSOLED*>(me)->getWidth( );
  myOSSwarm.unlock();

  return result;
}

int16_t FtSwarmOLED::getHeight(void) {

  if (!me) return 0;
  
  myOSSwarm.lock();
  int16_t result = static_cast<SwOSOLED*>(me)->getHeight( );
  myOSSwarm.unlock();

  return result;
}

uint8_t FtSwarmOLED::getRotation(void) {

  if (!me) return 0;
  
  myOSSwarm.lock();
  uint8_t result = static_cast<SwOSOLED*>(me)->getRotation( );
  myOSSwarm.unlock();

  return result;
}

void FtSwarmOLED::getCursor(int16_t *x, int16_t *y) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->getCursor( x, y);
  myOSSwarm.unlock();

}

// **** FtSwarmCAM ****

FtSwarmCAM::FtSwarmCAM( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmIO( serialNumber, port, FTSWARM_CAM) {};
FtSwarmCAM::FtSwarmCAM( const char *name ):FtSwarmIO( name, FTSWARM_CAM ) {};

void FtSwarmCAM::streaming( bool onOff ) {

  if (!me) return;

  myOSSwarm.lock();
  static_cast<SwOSCAM *>(me)->streaming( onOff, false);
  myOSSwarm.unlock();
}

void FtSwarmCAM::setFramesize( framesize_t framesize ) {

  if (!me) return;

  myOSSwarm.lock();
  static_cast<SwOSCAM *>(me)->setFramesize(framesize, false);
  myOSSwarm.unlock();
}

void FtSwarmCAM::setQuality( int quality ) {

  if (!me) return;

  myOSSwarm.lock();
  static_cast<SwOSCAM *>(me)->setQuality( quality, false );
  myOSSwarm.unlock();
}

void FtSwarmCAM::setBrightness( int brightness ) {

  if (!me) return;

  myOSSwarm.lock();
  static_cast<SwOSCAM *>(me)->setBrightness(brightness, false);
  myOSSwarm.unlock();
}

void FtSwarmCAM::setContrast( int contrast ) {

  if (!me) return;

  myOSSwarm.lock();
  static_cast<SwOSCAM *>(me)->setContrast(contrast, false);
  myOSSwarm.unlock();
}

void FtSwarmCAM::setSaturation( int saturation ) {

  if (!me) return;

  myOSSwarm.lock();
  static_cast<SwOSCAM *>(me)->setSaturation(saturation, false);
  myOSSwarm.unlock();
}

void FtSwarmCAM::setSpecialEffect( int specialEffect ) {

  if (!me) return;

  myOSSwarm.lock();
  static_cast<SwOSCAM *>(me)->setSpecialEffect(specialEffect, false);
  myOSSwarm.unlock();
}

void FtSwarmCAM::setWbMode( int wbMode ) {

  if (!me) return;

  myOSSwarm.lock();
  static_cast<SwOSCAM *>(me)->setWbMode(wbMode, false);
  myOSSwarm.unlock();
}

void FtSwarmCAM::setHMirror( bool hMirror ) {

  if (!me) return;

  myOSSwarm.lock();
  static_cast<SwOSCAM *>(me)->setHMirror(hMirror, false);
  myOSSwarm.unlock();
}

void FtSwarmCAM::setVFlip( bool vFlip ) {

  if (!me) return;

  myOSSwarm.lock();
  static_cast<SwOSCAM *>(me)->setVFlip(vFlip, false);
  myOSSwarm.unlock();
}


// **** FtSwarm ****

FtSwarmSerialNumber_t FtSwarm::begin( bool verbose ) {

  bool result = myOSSwarm.begin( verbose );

  if (!nvs.IAmKelda) {
    printf("ERROR: Please configure this controler as Kelda.\n");
    myOSSwarm.setState( ERROR );
    firmware();
    ESP.restart();
  }

  return result;

}

void FtSwarm::setReadDelay( uint16_t readDelay ) {

  myOSSwarm.lock();
  myOSSwarm.setReadDelay( readDelay);
  myOSSwarm.unlock();

}

void FtSwarm::halt( void ) {

  myOSSwarm.lock();
  myOSSwarm.halt( );
  myOSSwarm.unlock();

}

bool FtSwarm::waitOnUserEvent( int parameter[10], TickType_t xTicksToWait ) {

  SwOSCom userEvent;

  // queue exists?
  if ( !myOSNetwork.userEvent ) return false;
  
  // wait for new event
  if ( xQueueReceive( myOSNetwork.userEvent, &userEvent, xTicksToWait ) == pdTRUE ) {
    for ( uint8_t i=0; i<10; i++ ) {
      memcpy( &parameter[i], &userEvent.data.userEventCmd.payload[ i * sizeof (int ) ], sizeof( int ) );
    }
    return true;
  }

  return false;

}

bool FtSwarm::sendEventData( uint8_t *buffer, size_t size ) {

  // error, if size is to big
  if (size > MAXUSEREVENTPAYLOAD ) return false;

  // ignore, if there is no Kelda
  if (!myOSSwarm.Kelda) return true;

  // create packet
  SwOSCom eventData( myOSSwarm.Kelda->macAddr, myOSSwarm.Ctrl[0]->serialNumber, CMD_USEREVENT );

  // copy payload
  eventData.data.userEventCmd.trigger = false;
  eventData.data.userEventCmd.size = size;
  memcpy( eventData.data.userEventCmd.payload, buffer, size );

  // send
  return true;

}


void forever( char *prompt) {
  printf("%s\n", prompt); while(1) delay(500);
}