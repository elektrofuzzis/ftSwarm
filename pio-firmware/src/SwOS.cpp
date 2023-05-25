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

FtSwarm ftSwarm;

// **** FtSwarmIO ****

FtSwarmIO::FtSwarmIO( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmIOType_t ioType ) {
  // constructor: register at myOSSwarm and get a pointer to myself

  bool firstTry = true;

  while (!me) {

    // check, if IO is online
    myOSSwarm.lock(); 
    me = (SwOSIOHandle_t) myOSSwarm.getIO( serialNumber, ioType, port);
    myOSSwarm.unlock();

    // no success, wait 25 ms
    if ( (!me) && ( firstTry ) ) {
      ESP_LOGD( LOGFTSWARM, "waiting on device" );
      myOSSwarm.setState( WAITING );
      firstTry = false;
    }
    
    if (!me) vTaskDelay( 25 / portTICK_PERIOD_MS );
  }
  
};

FtSwarmIO::FtSwarmIO( const char *name ) {
  // constructor: register at myOSSwarm using a name/alias

  bool firstTry = true;

  while (!me) {

    myOSSwarm.lock();
    me = myOSSwarm.getIO( name );

    // no success, wait 25 ms
    if ( (!me) && ( firstTry ) ) {
      ESP_LOGD( LOGFTSWARM, "waiting on device" );
      myOSSwarm.setState( WAITING );
      firstTry = false;
    }
    
    myOSSwarm.unlock();

    if (!me) vTaskDelay( 25 / portTICK_PERIOD_MS );
  }
  
}

bool FtSwarmIO::isOnline() { 
  // check if I'm online
  // TODO!
  // return (me)?me->isOnline():false;
  return true;
};

// **** FtSwarmInput

FtSwarmInput::FtSwarmInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmSensor_t sensorType, bool normallyOpen):FtSwarmIO(serialNumber, port, FTSWARM_INPUT ) {
  // constructor: register at myOSSwarm and get a pointer to myself 

  // set sensor type
  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSInput *>(me)->setSensorType( sensorType, normallyOpen );
    myOSSwarm.unlock();
  }

};

FtSwarmInput::FtSwarmInput( const char *name, FtSwarmSensor_t sensorType, bool normallyOpen ):FtSwarmIO(name) {

  // set sensor type
  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSInput *>(me)->setSensorType( sensorType, normallyOpen );
    myOSSwarm.unlock();
  }

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

FtSwarmDigitalInput::FtSwarmDigitalInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmSensor_t sensorType, bool normallyOpen):FtSwarmInput( serialNumber, port, sensorType ) {};
FtSwarmDigitalInput::FtSwarmDigitalInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmInput( serialNumber, port, FTSWARM_DIGITAL ) {};
FtSwarmDigitalInput::FtSwarmDigitalInput( const char *name, FtSwarmSensor_t sensorType, bool normallyOpen ):FtSwarmInput( name, sensorType, normallyOpen) {};
FtSwarmDigitalInput::FtSwarmDigitalInput( const char *name  ):FtSwarmInput( name, FTSWARM_DIGITAL ) {};


// some facades
bool FtSwarmDigitalInput::isPressed()            { return getState(); };
bool FtSwarmDigitalInput::isReleased()           { return !getState(); };
bool FtSwarmDigitalInput::hasToggledUp()         { return ( getToggle() == FTSWARM_TOGGLEUP ); };
bool FtSwarmDigitalInput::hasToggledDown()       { return ( getToggle() == FTSWARM_TOGGLEDOWN ); };

// real stuff
FtSwarmToggle_t FtSwarmDigitalInput::getToggle() { 

  if (!me) return FTSWARM_NOTOGGLE;

  myOSSwarm.lock();
  FtSwarmToggle_t xReturn = static_cast<SwOSInput *>(me)->getToggle();
  myOSSwarm.unlock();

  return xReturn;
};

bool FtSwarmDigitalInput::getState() { 

  if (!me) return false;

  myOSSwarm.lock();
  bool xReturn = (static_cast<SwOSInput *>(me)->getValueUI32()>0);
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
FtSwarmButton::FtSwarmButton( const char *name):FtSwarmIO( name ) {};

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

FtSwarmAnalogInput::FtSwarmAnalogInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmSensor_t sensorType):FtSwarmInput( serialNumber, port, sensorType ) {};
FtSwarmAnalogInput::FtSwarmAnalogInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmInput( serialNumber, port, FTSWARM_ANALOG ) {};
FtSwarmAnalogInput::FtSwarmAnalogInput( const char *name ):FtSwarmInput( name, FTSWARM_ANALOG ) {};
FtSwarmAnalogInput::FtSwarmAnalogInput( const char *name, FtSwarmSensor_t sensorType ):FtSwarmInput( name, sensorType ) {};

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
  float xReturn = (static_cast<SwOSInput *>(me)->getVoltage());
  myOSSwarm.unlock();

  return xReturn;
};

// **** FtSwarmOhmmeter ****

FtSwarmOhmmeter::FtSwarmOhmmeter( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmAnalogInput( serialNumber, port, FTSWARM_OHMMETER ) {};
FtSwarmOhmmeter::FtSwarmOhmmeter( const char *name ):FtSwarmAnalogInput( name, FTSWARM_OHMMETER ) {};

float FtSwarmOhmmeter::getResistance() {

  if (!me) return 0;

  myOSSwarm.lock();
  float xReturn = (static_cast<SwOSInput *>(me)->getResistance());
  myOSSwarm.unlock();

  return xReturn;
};

// **** FtSwarmThermometer ****

FtSwarmThermometer::FtSwarmThermometer( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmAnalogInput( serialNumber, port, FTSWARM_THERMOMETER ) {};
FtSwarmThermometer::FtSwarmThermometer( const char *name ):FtSwarmAnalogInput( name, FTSWARM_THERMOMETER ) {};

float FtSwarmThermometer::getCelcius() {

  if (!me) return 0;

  myOSSwarm.lock();
  float xReturn = (static_cast<SwOSInput *>(me)->getCelcius());
  myOSSwarm.unlock();

  return xReturn;
};

float FtSwarmThermometer::getKelvin() {
  if (!me) return 0;

  myOSSwarm.lock();
  float xReturn = (static_cast<SwOSInput *>(me)->getKelvin());
  myOSSwarm.unlock();

  return xReturn;
}

float FtSwarmThermometer::getFahrenheit() {
  if (!me) return 0;

  myOSSwarm.lock();
  float xReturn = (static_cast<SwOSInput *>(me)->getFahrenheit());
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
    static_cast<SwOSActor *>(me)->setActorType( actorType );
    myOSSwarm.unlock();
  }

};

FtSwarmActor::FtSwarmActor( const char *name, FtSwarmActor_t actorType):FtSwarmIO( name ) {
  // constructor: register at myOSSwarm and get a pointer to myself 

  // set sensor type
  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSActor *>(me)->setActorType( actorType );
    myOSSwarm.unlock();
  }

};

// **** FtSwarmMotor ****

FtSwarmMotor::FtSwarmMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmActor_t actorType):FtSwarmActor( serialNumber, port, actorType) {};
FtSwarmMotor::FtSwarmMotor( const char *name, FtSwarmActor_t actorType):FtSwarmActor( name, actorType ) {};
    
void FtSwarmMotor::setSpeed( int16_t speed ) {
  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSActor *>(me)->setPower( speed );
    myOSSwarm.unlock();
  }
}

uint16_t FtSwarmMotor::getSpeed() {

  if (!me) return 0;

  myOSSwarm.lock();
  uint16_t xReturn = (static_cast<SwOSActor *>(me)->getPower());
  myOSSwarm.unlock();

  return xReturn;
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

// **** FtSwarmLamp ****

FtSwarmLamp::FtSwarmLamp( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmActor( serialNumber, port, FTSWARM_LAMP ) {};
FtSwarmLamp::FtSwarmLamp( const char *name ):FtSwarmActor( name, FTSWARM_LAMP ) {};

void FtSwarmLamp::on( uint8_t power ) {
  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSActor *>(me)->setPower( power);
    myOSSwarm.unlock();
  }
}

void FtSwarmLamp::off( void ) {
  on(0);
}

// **** FtSwarmValve ****

FtSwarmValve::FtSwarmValve( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmActor( serialNumber, port, FTSWARM_VALVE ) {};
FtSwarmValve::FtSwarmValve( const char *name ):FtSwarmActor( name, FTSWARM_LAMP ) {};

void FtSwarmValve::on( void ) {
  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSActor *>(me)->setPower( 255);
    myOSSwarm.unlock();
  }
}

void FtSwarmValve::off( void ) {
  on();
}

// **** FtSwarmCompressor ****

FtSwarmCompressor::FtSwarmCompressor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmActor( serialNumber, port, FTSWARM_VALVE ) {};
FtSwarmCompressor::FtSwarmCompressor( const char *name ):FtSwarmActor( name, FTSWARM_COMPRESSOR ) {};

void FtSwarmCompressor::on( void ) {
  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSActor *>(me)->setPower( 255);
    myOSSwarm.unlock();
  }
}

void FtSwarmCompressor::off( void ) {
  on();
}

// **** FtSwarmBuzzer ****

FtSwarmBuzzer::FtSwarmBuzzer( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmActor( serialNumber, port, FTSWARM_VALVE ) {};
FtSwarmBuzzer::FtSwarmBuzzer( const char *name ):FtSwarmActor( name, FTSWARM_BUZZER ) {};

void FtSwarmBuzzer::on( void ) {
  if (me) {
    myOSSwarm.lock();
    static_cast<SwOSActor *>(me)->setPower( 255);
    myOSSwarm.unlock();
  }
}

void FtSwarmBuzzer::off( void ) {
  on();
}

// **** FtSwarmJoystick ****

FtSwarmJoystick::FtSwarmJoystick( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmIO(serialNumber, port, FTSWARM_JOYSTICK) {

  // add joystick button
  switch (port) {
    case FTSWARM_JOY1: button = myOSSwarm.getIO( serialNumber, FTSWARM_BUTTON, FTSWARM_J1); break;
    case FTSWARM_JOY2: button = myOSSwarm.getIO( serialNumber, FTSWARM_BUTTON, FTSWARM_J2); break;
    default:           button = NULL;
  }

};

FtSwarmJoystick::FtSwarmJoystick( const char *name ):FtSwarmIO( name ) {

  // add joystick button
  SwOSJoystick *joystick = static_cast<SwOSJoystick *>(me);
  switch (joystick->getPort()) {
    case FTSWARM_JOY1: button = myOSSwarm.getIO( joystick->getCtrl()->serialNumber, FTSWARM_BUTTON, FTSWARM_J1); break;
    case FTSWARM_JOY2: button = myOSSwarm.getIO( joystick->getCtrl()->serialNumber, FTSWARM_BUTTON, FTSWARM_J2); break;
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

// **** FtSwarmLED ****

FtSwarmLED::FtSwarmLED( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmIO( serialNumber, port, FTSWARM_LED) {};
FtSwarmLED::FtSwarmLED( const char *name ):FtSwarmIO( name ) {};

uint8_t FtSwarmLED::getBrightness() {

  if (!me) return 0;
  
  myOSSwarm.lock();
  uint8_t xReturn = (static_cast<SwOSLED *>(me)->getBrightness());
  myOSSwarm.unlock();

  return xReturn;
};

void FtSwarmLED::setBrightness(uint8_t brightness) {

  if (!me) return;

  myOSSwarm.lock();
  static_cast<SwOSLED *>(me)->setBrightness(brightness);
  myOSSwarm.unlock();
}

uint32_t FtSwarmLED::getColor() {

  if (!me) return 0;
  
  myOSSwarm.lock();
  uint32_t xReturn = (static_cast<SwOSLED *>(me)->getColor());
  myOSSwarm.unlock();

  return xReturn;
};

void FtSwarmLED::setColor(uint32_t color) {

  if (!me) return;

  myOSSwarm.lock();
  static_cast<SwOSLED *>(me)->setColor(color);
  myOSSwarm.unlock();
}

// **** FtSwarmI2C   ****

FtSwarmI2C::FtSwarmI2C( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port ) : FtSwarmIO( serialNumber, port, FTSWARM_I2C ) {
}
    
FtSwarmI2C::FtSwarmI2C( const char *name ) : FtSwarmIO( name ) {
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
FtSwarmServo::FtSwarmServo( const char *name ):FtSwarmIO( name ) {};

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
  static_cast<SwOSServo *>(me)->setPosition(position);
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
  static_cast<SwOSServo *>(me)->setOffset(offset);
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

// **** FtSwarm ****

void FtSwarm::verbose( bool on ) {
  _verbose = on;
}

FtSwarmSerialNumber_t FtSwarm::begin( bool IAmAKelda ) {

  _IAmAKelda = IAmAKelda;
  return myOSSwarm.begin( _IAmAKelda, _verbose );

}

void FtSwarm::setReadDelay( uint16_t readDelay ) {

  myOSSwarm.lock();
  myOSSwarm.setReadDelay( readDelay);
  myOSSwarm.unlock();

}

void FtSwarm::setup( void ) {
  firmware();
}
