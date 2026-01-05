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
#include "SwOSLog.h"

#include <MPU6050_6Axis_MotionApps20.h>

SwOSPID::SwOSPID( float kp, float ki, float kd, float min_integral, float max_integral, float min_output, float max_output ) {
  this->kp = kp;
  this->ki = ki;
  this->kd = kd;
  this->min_integral = min_integral;
  this->max_integral = max_integral;
  this->min_output   = min_output;
  this->max_output   = max_output;
}

float SwOSPID::solve( float target, float sensor ) {

  float error = target - sensor;
  integral += error;
  integral = max( integral, max_integral );
  integral = min( integral, min_integral );
  float derivative = ( error - last_error);
  float output = kp * error + ki * integral + kd * derivative;
  output = min( output, max_output );
  output = max( output, min_output );
  last_error = error;
  return output;

}

FtSwarm ftSwarm;

// **** FtSwarmIO ****

FtSwarmIO::FtSwarmIO( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType ) {
  // constructor: register at myOSSwarm and get a pointer to myself
  bool firstTry = true;

  while (!me) {

    // check, if IO is online
    me = (SwOSIOHandle_t) myOSSwarm.getIO( serialNumber, port, ioType);
    
    // no success, wait 25 ms
    if ( (!me) && ( firstTry ) ) {
      SWARM_LOG_WAIT("Waiting for device - SN controler: %d port: %d ioType: %d\n", serialNumber, port, ioType );
      firstTry = false;
    }
    
    if (!me) vTaskDelay( 25 / portTICK_PERIOD_MS );
  }

  // register myself
  static_cast<SwOSIO *>(me)->lock();
  static_cast<SwOSIO *>(me)->take();
  static_cast<SwOSIO *>(me)->unlock(); 
  
};

FtSwarmIO::FtSwarmIO( FtSwarmSerialNumber_t serialNumber, SwOSIOType_t ioType ):FtSwarmIO(serialNumber, SWOS_NOPORT, ioType ) {
  // helper for all devices, which don't have a port like CAM, OLED, ...
};

FtSwarmIO::FtSwarmIO( const char *name, SwOSIOType_t ioType ) {
  // constructor: register at myOSSwarm using a name/alias

  bool firstTry = true;

  while (!me) {

    me = myOSSwarm.getIO( name, ioType );

    // no success, wait 25 ms
    if ( (!me) && ( firstTry ) ) {
      SWARM_LOG_WAIT("Waiting for device %s ioType: %d\n", name, ioType );
      firstTry = false;
    }
    
    if (!me) vTaskDelay( 25 / portTICK_PERIOD_MS );
 
  }

  // register myself
  static_cast<SwOSIO *>(me)->lock(); 
  static_cast<SwOSIO *>(me)->take();
  static_cast<SwOSIO *>(me)->unlock(); 
  
}

FtSwarmIO::~FtSwarmIO() {

  // unregister myself
  if (me) {
    static_cast<SwOSIO *>(me)->lock(); 
    static_cast<SwOSIO *>(me)->give();
    static_cast<SwOSIO *>(me)->unlock(); 
  }
}

bool FtSwarmIO::isOnline() { 
  // check if I'm online
  return (me)?static_cast<SwOSIO *>(me)->isOnline():false;
};

// **** FtSwarmSensor

FtSwarmSensor::FtSwarmSensor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType):FtSwarmIO( serialNumber, port, ioType ) {
  // constructor: register at myOSSwarm and get a pointer to myself 

};

FtSwarmSensor::FtSwarmSensor( const char *name, SwOSIOType_t ioType ):FtSwarmIO( name, ioType ) {

};

void FtSwarmSensor::onTrigger( FtSwarmTrigger_t triggerEvent, FtSwarmOperator_t op, FtSwarmOperand_t v1, FtSwarmOperand_t v2, FtSwarmIO *actor, int32_t p1 ) {

  // set trigger using static values
  if ( (me) && (actor) ) {
    static_cast<SwOSInput *>(me)->lock();
    static_cast<SwOSInput *>(me)->addEvent( triggerEvent, op, v1, v2, static_cast<SwOSIO*>(actor->me), p1 );
    static_cast<SwOSInput *>(me)->unlock();
  }

};

// **** FtSwarmDigitalInput ****

FtSwarmDigitalInput::FtSwarmDigitalInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType, bool normallyOpen):FtSwarmSensor( serialNumber, port, SWOSIO_DIGITAL ) {
  
  if (me) static_cast<SwOSInput *>(me)->setParameter( normallyOpen );

}

FtSwarmDigitalInput::FtSwarmDigitalInput( const char *name, SwOSIOType_t ioType, bool normallyOpen ):FtSwarmSensor( name, ioType) {

  if (me) static_cast<SwOSInput *>(me)->setParameter( normallyOpen );

}

FtSwarmDigitalInput::FtSwarmDigitalInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen ) :FtSwarmSensor( serialNumber, port, SWOSIO_DIGITAL ) {

  if (me) static_cast<SwOSInput *>(me)->setParameter( normallyOpen );

}

FtSwarmDigitalInput::FtSwarmDigitalInput( const char *name, bool normallyOpen ):FtSwarmSensor( name, SWOSIO_DIGITAL) {

  if (me) static_cast<SwOSInput *>(me)->setParameter( normallyOpen );

}


// some facades
bool FtSwarmDigitalInput::isPressed()            { return getState(); };
bool FtSwarmDigitalInput::isReleased()           { return !getState(); };
bool FtSwarmDigitalInput::hasToggledUp()         { return ( getToggle() == FTSWARM_TOGGLEUP ); };
bool FtSwarmDigitalInput::hasToggledDown()       { return ( getToggle() == FTSWARM_TOGGLEDOWN ); };

// real stuff
FtSwarmToggle_t FtSwarmDigitalInput::getToggle() { 

  if (!me) return FTSWARM_NOTOGGLE;

  static_cast<SwOSDigitalInput *>(me)->lock();
  FtSwarmToggle_t xReturn = static_cast<SwOSDigitalInput *>(me)->getToggle();
  static_cast<SwOSDigitalInput *>(me)->unlock();

  return xReturn;
};

bool FtSwarmDigitalInput::getState() { 

  if (!me) return false;

  static_cast<SwOSDigitalInput *>(me)->lock();
  bool xReturn = (static_cast<SwOSDigitalInput *>(me)->getValueI32()>0);
  static_cast<SwOSDigitalInput *>(me)->unlock();

  return xReturn;
};

// **** FtSwarmSwitch ****

FtSwarmSwitch::FtSwarmSwitch( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen):FtSwarmDigitalInput( serialNumber, port, SWOSIO_SWITCH, normallyOpen ) {};
FtSwarmSwitch::FtSwarmSwitch( const char * name, bool normallyOpen):FtSwarmDigitalInput( name, SWOSIO_SWITCH, normallyOpen ) {};

// **** FtSwarmLightBarrier ****

FtSwarmLightBarrier::FtSwarmLightBarrier( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen):FtSwarmDigitalInput( serialNumber, port, SWOSIO_LIGHTBARRIER, normallyOpen ) {};
FtSwarmLightBarrier::FtSwarmLightBarrier( const char * name, bool normallyOpen):FtSwarmDigitalInput( name, SWOSIO_LIGHTBARRIER, normallyOpen ) {};

// **** FtSwarmReedSwitch ****

FtSwarmReedSwitch::FtSwarmReedSwitch( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen):FtSwarmDigitalInput( serialNumber, port, SWOSIO_REEDSWITCH, normallyOpen ) {};
FtSwarmReedSwitch::FtSwarmReedSwitch( const char * name, bool normallyOpen):FtSwarmDigitalInput( name, SWOSIO_REEDSWITCH, normallyOpen ) {};

// **** FtSwarmButton ****

FtSwarmButton::FtSwarmButton( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmDigitalInput( serialNumber, port, SWOSIO_BUTTON ) {};
FtSwarmButton::FtSwarmButton( const char *name):FtSwarmDigitalInput( name, SWOSIO_BUTTON, true ) {};

// **** FtSwarmCounter

FtSwarmCounter::FtSwarmCounter( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType ):FtSwarmSensor( serialNumber, port, SWOSIO_COUNTER ) {

};

FtSwarmCounter::FtSwarmCounter( const char *name, SwOSIOType_t ioType ):FtSwarmSensor( name, SWOSIO_COUNTER ) {
  
};

FtSwarmCounter::FtSwarmCounter( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmCounter( serialNumber, port, SWOSIO_COUNTER ) {

};

FtSwarmCounter::FtSwarmCounter( const char *name):FtSwarmCounter( name, SWOSIO_COUNTER ) {

};

int16_t FtSwarmCounter::getCounter() {

  if (!me) return 0;

  static_cast<SwOSCounter *>(me)->lock();
  uint16_t xReturn = (static_cast<SwOSCounter *>(me)->getValueI32());
  static_cast<SwOSCounter *>(me)->unlock();

  return xReturn;
};

void FtSwarmCounter::resetCounter( void ) {

  static_cast<SwOSCounter *>(me)->lock();
  static_cast<SwOSCounter *>(me)->resetCounter();
  static_cast<SwOSCounter *>(me)->unlock();

};

// **** FtSwarmRotaryEncoder

FtSwarmRotaryEncoder::FtSwarmRotaryEncoder( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType ):FtSwarmSensor( serialNumber, port, SWOSIO_ROTARYENCODER ) {

};

FtSwarmRotaryEncoder::FtSwarmRotaryEncoder( const char *name, SwOSIOType_t ioType ):FtSwarmSensor( name, SWOSIO_ROTARYENCODER ) {
  
};

FtSwarmRotaryEncoder::FtSwarmRotaryEncoder( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmRotaryEncoder( serialNumber, port, SWOSIO_ROTARYENCODER ) {

};

FtSwarmRotaryEncoder::FtSwarmRotaryEncoder( const char *name):FtSwarmRotaryEncoder( name, SWOSIO_ROTARYENCODER ) {

};

int16_t FtSwarmRotaryEncoder::getCounter() {

  if (!me) return 0;

  static_cast<SwOSCounter *>(me)->lock();
  uint16_t xReturn = (static_cast<SwOSCounter *>(me)->getValueI32());
  static_cast<SwOSCounter *>(me)->unlock();

  return xReturn;
};

void FtSwarmRotaryEncoder::resetCounter( void ) {

  static_cast<SwOSCounter *>(me)->lock();
  static_cast<SwOSCounter *>(me)->resetCounter();
  static_cast<SwOSCounter *>(me)->unlock();

};

// **** FtSwarmFrequencymeter

FtSwarmFrequencymeter::FtSwarmFrequencymeter( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmSensor( serialNumber, port, SWOSIO_FREQUENCYMETER ) {

};

FtSwarmFrequencymeter::FtSwarmFrequencymeter( const char *name ):FtSwarmSensor( name, SWOSIO_FREQUENCYMETER ) {

};

int16_t FtSwarmFrequencymeter::getFrequency() {

  if (!me) return 0;

  static_cast<SwOSFrequencymeter *>(me)->lock();
  uint16_t xReturn = (static_cast<SwOSFrequencymeter *>(me)->getValueI32());
  static_cast<SwOSFrequencymeter *>(me)->unlock();

  return xReturn;
};

// **** FtSwarmAnalogInput ****

FtSwarmAnalogInput::FtSwarmAnalogInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType):FtSwarmSensor( serialNumber, port, SWOSIO_ANALOG ) {

};

FtSwarmAnalogInput::FtSwarmAnalogInput( const char *name, SwOSIOType_t ioType ):FtSwarmSensor( name, SWOSIO_ANALOG ) {

};

FtSwarmAnalogInput::FtSwarmAnalogInput( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port ):FtSwarmSensor( serialNumber, port, SWOSIO_ANALOG ) {

}
    
FtSwarmAnalogInput::FtSwarmAnalogInput( const char *name ):FtSwarmSensor( name, SWOSIO_ANALOG ) {

}


int32_t FtSwarmAnalogInput::getValue() {

  if (!me) return 0;

  static_cast<SwOSInput *>(me)->lock();
  uint16_t xReturn = (static_cast<SwOSInput *>(me)->getValueI32());
  static_cast<SwOSInput *>(me)->unlock();

  return xReturn;
};

// **** FtSwarmVoltmeter ****

FtSwarmVoltmeter::FtSwarmVoltmeter( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmAnalogInput( serialNumber, port, SWOSIO_VOLTMETER ) {};
FtSwarmVoltmeter::FtSwarmVoltmeter( const char *name ):FtSwarmAnalogInput( name, SWOSIO_VOLTMETER ) {};

float FtSwarmVoltmeter::getVoltage() {

  if (!me) return 0;

  static_cast<SwOSAnalogInput *>(me)->lock();
  float xReturn = (static_cast<SwOSAnalogInput *>(me)->getVoltage());
  static_cast<SwOSAnalogInput *>(me)->unlock();

  return xReturn;
};

// **** FtSwarmOhmmeter ****

FtSwarmOhmmeter::FtSwarmOhmmeter( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmAnalogInput( serialNumber, port, SWOSIO_OHMMETER ) {};
FtSwarmOhmmeter::FtSwarmOhmmeter( const char *name ):FtSwarmAnalogInput( name, SWOSIO_OHMMETER ) {};

float FtSwarmOhmmeter::getResistance() {

  if (!me) return 0;

  static_cast<SwOSAnalogInput *>(me)->lock();
  float xReturn = (static_cast<SwOSAnalogInput *>(me)->getResistance());
  static_cast<SwOSAnalogInput *>(me)->unlock();

  return xReturn;
};

// **** FtSwarmThermometer ****

FtSwarmThermometer::FtSwarmThermometer( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmAnalogInput( serialNumber, port, SWOSIO_THERMOMETER ) {};
FtSwarmThermometer::FtSwarmThermometer( const char *name ):FtSwarmAnalogInput( name, SWOSIO_THERMOMETER ) {};

float FtSwarmThermometer::getCelcius() {

  if (!me) return 0;

  static_cast<SwOSAnalogInput *>(me)->lock();
  float xReturn = (static_cast<SwOSAnalogInput *>(me)->getCelcius());
  static_cast<SwOSAnalogInput *>(me)->unlock();

  return xReturn;
};

float FtSwarmThermometer::getKelvin() {
  if (!me) return 0;

  static_cast<SwOSAnalogInput *>(me)->lock();
  float xReturn = (static_cast<SwOSAnalogInput *>(me)->getKelvin());
  static_cast<SwOSAnalogInput *>(me)->unlock();

  return xReturn;
}

float FtSwarmThermometer::getFahrenheit() {
  if (!me) return 0;

  static_cast<SwOSAnalogInput *>(me)->lock();
  float xReturn = (static_cast<SwOSAnalogInput *>(me)->getFahrenheit());
  static_cast<SwOSAnalogInput *>(me)->unlock();

  return xReturn;
}


// **** FtSwarmLDR ****

FtSwarmLDR::FtSwarmLDR( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmAnalogInput( serialNumber, port, SWOSIO_LDR ) {};
FtSwarmLDR::FtSwarmLDR( const char *name ):FtSwarmAnalogInput( name, SWOSIO_LDR ) {};

// **** FtSwarmMotor ****

FtSwarmMotor::FtSwarmMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType):FtSwarmIO( serialNumber, port, ioType) {

};

FtSwarmMotor::FtSwarmMotor( const char *name, SwOSIOType_t ioType):FtSwarmIO( name, ioType ) {
  
};
    
void FtSwarmMotor::setSpeed( int16_t speed ) {
  if (me) {
    static_cast<SwOSMotor *>(me)->lock();
    static_cast<SwOSMotor *>(me)->setSpeed( speed );
    static_cast<SwOSMotor *>(me)->apply();
    static_cast<SwOSMotor *>(me)->unlock();
  }
}

uint16_t FtSwarmMotor::getSpeed() {

  if (!me) return 0;

  static_cast<SwOSMotor *>(me)->lock();
  uint16_t xReturn = (static_cast<SwOSMotor *>(me)->getSpeed());
  static_cast<SwOSMotor *>(me)->unlock();

  return xReturn;
};

void FtSwarmMotor::setAcceleration( uint32_t rampUpT, uint32_t rampUpY ) {
  if (me) {
    static_cast<SwOSMotor *>(me)->lock();
    static_cast<SwOSMotor *>(me)->setAcceleration( rampUpT, rampUpY );
    static_cast<SwOSMotor *>(me)->unlock();
  }
}

void FtSwarmMotor::getAcceleration( uint32_t *rampUpT, uint32_t *rampUpY ) {

  if (!me) return;

  static_cast<SwOSMotor *>(me)->lock();
  static_cast<SwOSMotor *>(me)->getAcceleration( rampUpT, rampUpY );
  static_cast<SwOSMotor *>(me)->unlock();

};

// **** FtSwarmTractorMotor

FtSwarmTractorMotor::FtSwarmTractorMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType):FtSwarmMotor( serialNumber, port, ioType) {};
FtSwarmTractorMotor::FtSwarmTractorMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmMotor( serialNumber, port, SWOSIO_TRACTOR) {};
FtSwarmTractorMotor::FtSwarmTractorMotor( const char *name, SwOSIOType_t ioType ):FtSwarmMotor( name, ioType ) {};
FtSwarmTractorMotor::FtSwarmTractorMotor( const char *name ):FtSwarmMotor( name, SWOSIO_TRACTOR ) {};

void FtSwarmTractorMotor::setMotionType( FtSwarmMotion_t motionType ) {
  if (me) {
    static_cast<SwOSMotor *>(me)->lock();
    static_cast<SwOSMotor *>(me)->setMotionType( motionType );
    static_cast<SwOSMotor *>(me)->unlock();
  }
}

FtSwarmMotion_t FtSwarmTractorMotor::getMotionType( void ) {
  
  FtSwarmMotion_t motionType = FTSWARM_COAST;
  
  if (me) {
    static_cast<SwOSMotor *>(me)->lock();
    motionType = static_cast<SwOSMotor *>(me)->getMotionType( );
    static_cast<SwOSMotor *>(me)->unlock();
  }

  return motionType;
  
}

// **** FtSwarmStepperMotor

FtSwarmStepperMotor::FtSwarmStepperMotor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmTractorMotor( serialNumber, port, SWOSIO_STEPPER ) {};
FtSwarmStepperMotor::FtSwarmStepperMotor( const char * name ):FtSwarmTractorMotor( name, SWOSIO_STEPPER ) {};

void FtSwarmStepperMotor::setDistance( int32_t distance, bool relative ) {

  if (me) {
    static_cast<SwOSStepper *>(me)->lock();
    static_cast<SwOSStepper *>(me)->setDistance( distance, relative );
    static_cast<SwOSStepper *>(me)->unlock();
  }

}

int32_t FtSwarmStepperMotor::getDistance( void ) {

  int32_t distance = 0;

  if (me) {
    static_cast<SwOSStepper *>(me)->lock();
    distance = static_cast<SwOSStepper *>(me)->getDistance( );
    static_cast<SwOSStepper *>(me)->unlock();
  }

  return distance;

}

bool FtSwarmStepperMotor::isRunning( void ) {

  bool result = false;

  if (me) {
    static_cast<SwOSStepper *>(me)->lock();
    result = static_cast<SwOSStepper *>(me)->isRunning( );
    static_cast<SwOSStepper *>(me)->unlock();
  }

  return result;

}

void FtSwarmStepperMotor::run( void ) {

  if (me) {
    static_cast<SwOSStepper *>(me)->lock();
    static_cast<SwOSStepper *>(me)->startStop( true );
    static_cast<SwOSStepper *>(me)->unlock();
  }

}

void FtSwarmStepperMotor::stop( void ) {

  if (me) {
    static_cast<SwOSStepper *>(me)->lock();
    static_cast<SwOSStepper *>(me)->startStop( false );
    static_cast<SwOSStepper *>(me)->unlock();
  }

}

void FtSwarmStepperMotor::setPosition( int32_t position ) {

  if (me) {
    static_cast<SwOSStepper *>(me)->lock();
    static_cast<SwOSStepper *>(me)->setPosition( position );
    static_cast<SwOSStepper *>(me)->unlock();
  }

}

int32_t FtSwarmStepperMotor::getPosition( void ) {

  int32_t position = 0;

  if (me) {
    static_cast<SwOSStepper *>(me)->lock();
    position = static_cast<SwOSStepper *>(me)->getPosition( );
    static_cast<SwOSStepper *>(me)->unlock();
  }

  return position;

}

bool FtSwarmStepperMotor::isHoming( void ) {

  bool result = false;

  if (me) {
    static_cast<SwOSStepper *>(me)->lock();
    result = static_cast<SwOSStepper *>(me)->isHoming( );
    static_cast<SwOSStepper *>(me)->unlock();
  }

  return result;

}

void FtSwarmStepperMotor::homing( int32_t maxDistance ) {

  if (me) {
    static_cast<SwOSStepper *>(me)->lock();
    static_cast<SwOSStepper *>(me)->homing( maxDistance );
    static_cast<SwOSStepper *>(me)->unlock();
  }

}

void FtSwarmStepperMotor::setHomingOffset( int32_t offset ) {

  if (me) {
    static_cast<SwOSStepper *>(me)->lock();
    static_cast<SwOSStepper *>(me)->setHomingOffset( offset );
    static_cast<SwOSStepper *>(me)->unlock();
  }

}

// **** FtSwarmOnOffActor ****

FtSwarmOnOffActor::FtSwarmOnOffActor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType):FtSwarmMotor( serialNumber, port, ioType ) {};
FtSwarmOnOffActor::FtSwarmOnOffActor( const char *name, SwOSIOType_t ioType ):FtSwarmMotor( name, ioType ) {};

void FtSwarmOnOffActor::on( int16_t speed ) {
  if (me) {
    static_cast<SwOSMotor *>(me)->lock();
    static_cast<SwOSMotor *>(me)->setSpeed(speed);
    static_cast<SwOSMotor *>(me)->apply();
    static_cast<SwOSMotor *>(me)->unlock();
  }
}

void FtSwarmOnOffActor::off( void ) {
  on(0);
}

// **** FtSwarmLamp ****

FtSwarmLamp::FtSwarmLamp( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmOnOffActor( serialNumber, port, SWOSIO_LAMP ) {};
FtSwarmLamp::FtSwarmLamp( const char *name ):FtSwarmOnOffActor( name, SWOSIO_LAMP ) {};

// **** FtSwarmValve ****

FtSwarmValve::FtSwarmValve( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmOnOffActor( serialNumber, port, SWOSIO_VALVE ) {};
FtSwarmValve::FtSwarmValve( const char *name ):FtSwarmOnOffActor( name, SWOSIO_LAMP ) {};


// **** FtSwarmCompressor ****

FtSwarmCompressor::FtSwarmCompressor( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmOnOffActor( serialNumber, port, SWOSIO_VALVE ) {};
FtSwarmCompressor::FtSwarmCompressor( const char *name ):FtSwarmOnOffActor( name, SWOSIO_COMPRESSOR ) {};

// **** FtSwarmBuzzer ****

FtSwarmBuzzer::FtSwarmBuzzer( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmOnOffActor( serialNumber, port, SWOSIO_VALVE ) {};
FtSwarmBuzzer::FtSwarmBuzzer( const char *name ):FtSwarmOnOffActor( name, SWOSIO_BUZZER ) {};


// **** FtSwarmJoystick ****

FtSwarmJoystick::FtSwarmJoystick( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmIO(serialNumber, port, SWOSIO_JOYSTICK) {

  // add joystick button
  switch (port) {
    case FTSWARM_JOY1: button = myOSSwarm.getIO( serialNumber, FTSWARM_J1, SWOSIO_BUTTON); break;
    case FTSWARM_JOY2: button = myOSSwarm.getIO( serialNumber, FTSWARM_J2, SWOSIO_BUTTON); break;
    default:           button = NULL;
  }

};

FtSwarmJoystick::FtSwarmJoystick( const char *name ):FtSwarmIO( name, SWOSIO_JOYSTICK ) {

  // add joystick button
  SwOSJoystick *joystick = static_cast<SwOSJoystick *>(me);
  switch (joystick->getPort()) {
    case FTSWARM_JOY1: button = myOSSwarm.getIO( joystick->getCtrl()->serialNumber, FTSWARM_J1, SWOSIO_BUTTON); break;
    case FTSWARM_JOY2: button = myOSSwarm.getIO( joystick->getCtrl()->serialNumber, FTSWARM_J2, SWOSIO_BUTTON); break;
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

  static_cast<SwOSJoystick *>(me)->lock();
  static_cast<SwOSJoystick *>(me)->getValue(FB, LR);
  if (button) *buttonState = static_cast<SwOSDigitalInput *>(button)->getValueI32();
  static_cast<SwOSJoystick *>(me)->unlock();
}

void FtSwarmJoystick::onTriggerLR( FtSwarmTrigger_t triggerEvent, FtSwarmOperator_t op, FtSwarmOperand_t v1, FtSwarmOperand_t v2, FtSwarmIO *actor, int32_t p1 ) {

  // set trigger using static values
  if ( (me) && (actor) ) {
    static_cast<SwOSJoystick*>(me)->lock();
    static_cast<SwOSJoystick*>(me)->lr->addEvent( triggerEvent, op, v1, v2, static_cast<SwOSIO *>(actor->me), p1 );
    static_cast<SwOSJoystick*>(me)->unlock();
  }

};

void FtSwarmJoystick::onTriggerFB( FtSwarmTrigger_t triggerEvent, FtSwarmOperator_t op, FtSwarmOperand_t v1, FtSwarmOperand_t v2, FtSwarmIO *actor, int32_t p1 ) {

  // set trigger using static values
  if ( (me) && (actor) ) {
    static_cast<SwOSJoystick*>(me)->lock();
    static_cast<SwOSJoystick*>(me)->fb->addEvent( triggerEvent, op, v1, v2, static_cast<SwOSIO *>(actor->me), p1 );
    static_cast<SwOSJoystick*>(me)->unlock();
  }

};


// **** FtSwarmPixel ****

FtSwarmPixel::FtSwarmPixel( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmIO( serialNumber, port, SWOSIO_PIXEL) {};
FtSwarmPixel::FtSwarmPixel( const char *name ):FtSwarmIO( name, SWOSIO_PIXEL ) {};

uint8_t FtSwarmPixel::getBrightness() {

  if (!me) return 0;
  
  static_cast<SwOSPixel*>(me)->lock();
  uint8_t xReturn = (static_cast<SwOSPixel *>(me)->getBrightness());
  static_cast<SwOSPixel*>(me)->unlock();

  return xReturn;
};

void FtSwarmPixel::setBrightness(uint8_t brightness) {

  if (!me) return;

  static_cast<SwOSPixel*>(me)->lock();
  static_cast<SwOSPixel *>(me)->setBrightness(brightness);
  static_cast<SwOSPixel*>(me)->unlock();
}

uint32_t FtSwarmPixel::getColor() {

  if (!me) return 0;
  
  static_cast<SwOSPixel*>(me)->lock();
  uint32_t xReturn = (static_cast<SwOSPixel *>(me)->getColor());
  static_cast<SwOSPixel*>(me)->unlock();

  return xReturn;
};

void FtSwarmPixel::setColor(uint32_t color) {

  if (!me) return;

  static_cast<SwOSPixel*>(me)->lock();
  static_cast<SwOSPixel *>(me)->setColor(color);
  static_cast<SwOSPixel*>(me)->unlock();
}

// **** FtSwarmI2C   ****

FtSwarmI2C::FtSwarmI2C( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port ) : FtSwarmIO( serialNumber, port, SWOSIO_I2C ) {
}
    
FtSwarmI2C::FtSwarmI2C( const char *name ) : FtSwarmIO( name, SWOSIO_I2C ) {
}

uint8_t FtSwarmI2C::getRegister(uint8_t reg) {
    
  if (!me) return 0;
  
  static_cast<SwOSI2C*>(me)->lock();
  uint8_t xReturn = (static_cast<SwOSI2C *>(me)->getRegister( reg ));
  static_cast<SwOSI2C*>(me)->unlock();

  return xReturn;
}

void  FtSwarmI2C::setRegister(uint8_t reg, uint8_t value) {

  if (!me) return;

  static_cast<SwOSI2C*>(me)->lock();
  static_cast<SwOSI2C *>(me)->setRegister(reg, value);
  static_cast<SwOSI2C*>(me)->unlock();
}

void FtSwarmI2C::onTrigger( FtSwarmTrigger_t triggerEvent, FtSwarmOperator_t op, FtSwarmOperand_t v1, FtSwarmOperand_t v2, FtSwarmIO *actor, int32_t p1 ) {

  // set trigger using static values
  if ( (me) && (actor) ) {
    static_cast<SwOSI2C*>(me)->lock();
    static_cast<SwOSI2C *>(me)->addEvent( triggerEvent, op, v1, v2, static_cast<SwOSIO*>(actor->me), p1 );
    static_cast<SwOSI2C*>(me)->unlock();
  }

};

// **** FtSwarmGyro   ****

FtSwarmGyro::FtSwarmGyro( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port ) : FtSwarmIO( serialNumber, port, SWOSIO_GYRO ) {
}
    
FtSwarmGyro::FtSwarmGyro( const char *name ) : FtSwarmIO( name, SWOSIO_GYRO ) {
}

void FtSwarmGyro::getYawPitchRoll(float *yaw, float *pitch, float *roll, bool radiants ) {
  
  if (!me) return;
  
  static_cast<SwOSGyro*>(me)->lock();
  static_cast<SwOSGyro*>(me)->getYawPitchRoll( yaw, pitch, roll, radiants );
  static_cast<SwOSGyro*>(me)->unlock();

};

// **** FtSwarmServo ****

FtSwarmServo::FtSwarmServo( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmIO( serialNumber, port, SWOSIO_SERVO) {};
FtSwarmServo::FtSwarmServo( const char *name ):FtSwarmIO( name, SWOSIO_SERVO ) {};

int16_t FtSwarmServo::getPosition() {

  if (!me) return 0;
  
  static_cast<SwOSServo*>(me)->lock();
  int16_t xReturn = (static_cast<SwOSServo *>(me)->getPosition());
  static_cast<SwOSServo*>(me)->unlock();

  return xReturn;
};

void FtSwarmServo::setPosition(int16_t position) {

  if (!me) return;
  
  static_cast<SwOSServo*>(me)->lock();
  static_cast<SwOSServo *>(me)->setPosition( position );
  static_cast<SwOSServo*>(me)->unlock();
}

int16_t FtSwarmServo::getOffset() {

  if (!me) return 0;
  
  static_cast<SwOSServo*>(me)->lock();
  int16_t xReturn = (static_cast<SwOSServo *>(me)->getOffset());
  static_cast<SwOSServo*>(me)->unlock();

  return xReturn;
};

void FtSwarmServo::setOffset(int16_t offset) {

  if (!me) return;
  
  static_cast<SwOSServo*>(me)->lock();
  static_cast<SwOSServo*>(me)->setOffset( offset );
  static_cast<SwOSServo*>(me)->unlock();
}

// **** FtSwarmOLED ****

FtSwarmOLED::FtSwarmOLED(FtSwarmSerialNumber_t serialNumber):FtSwarmIO( serialNumber, SWOSIO_OLED) {};
FtSwarmOLED::FtSwarmOLED( const char *name ):FtSwarmIO( name, SWOSIO_OLED ) {};

void FtSwarmOLED::invertDisplay(bool i) {

  if (!me) return;
  
  static_cast<SwOSOLED*>(me)->lock();
  static_cast<SwOSOLED*>(me)->invertDisplay( i );
  static_cast<SwOSOLED*>(me)->unlock();
}

void FtSwarmOLED::dim(bool dim) {

  if (!me) return;
  
  static_cast<SwOSOLED*>(me)->lock();
  static_cast<SwOSOLED*>(me)->dim( dim );
  static_cast<SwOSOLED*>(me)->unlock();
}

void FtSwarmOLED::drawPixel(int16_t x, int16_t y, bool white ) {

  if (!me) return;
  
  static_cast<SwOSOLED*>(me)->lock();
  static_cast<SwOSOLED*>(me)->drawPixel( x, y, white );
  static_cast<SwOSOLED*>(me)->unlock();
}

void FtSwarmOLED::setRotation(uint8_t r) {

  if (!me) return;
  
  static_cast<SwOSOLED*>(me)->lock();
  static_cast<SwOSOLED*>(me)->setRotation( r );
  static_cast<SwOSOLED*>(me)->unlock();
}

void FtSwarmOLED::clearScreen() {

  fillScreen(0);

}


void FtSwarmOLED::fillScreen(bool white) {

  if (!me) return;
  
  static_cast<SwOSOLED*>(me)->lock();
  static_cast<SwOSOLED*>(me)->fillScreen( white );
  static_cast<SwOSOLED*>(me)->unlock();
}

void FtSwarmOLED::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool white ) {

  if (!me) return;
  
  static_cast<SwOSOLED*>(me)->lock();
  static_cast<SwOSOLED*>(me)->drawLine( x0, y0, x1, y1, white );
  static_cast<SwOSOLED*>(me)->unlock();
}

void FtSwarmOLED::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, bool fill, bool white ) {

  if (!me) return;
  
  static_cast<SwOSOLED*>(me)->lock();
  static_cast<SwOSOLED*>(me)->drawRect( x, y, w, h, fill, white );
  static_cast<SwOSOLED*>(me)->unlock();
}

void FtSwarmOLED::drawCircle(int16_t x0, int16_t y0, int16_t r, bool fill, bool white ) {

  if (!me) return;
  
  static_cast<SwOSOLED*>(me)->lock();
  static_cast<SwOSOLED*>(me)->drawCircle( x0, y0, r, fill, white );
  static_cast<SwOSOLED*>(me)->unlock();
}

void FtSwarmOLED::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool fill, bool white ) {

  if (!me) return;
  
  static_cast<SwOSOLED*>(me)->lock();
  static_cast<SwOSOLED*>(me)->drawTriangle( x0, y0, x1, y1, x2, y2, fill, white );
  static_cast<SwOSOLED*>(me)->unlock();
}

void FtSwarmOLED::drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, bool fill, bool white ) {

  if (!me) return;
  
  static_cast<SwOSOLED*>(me)->lock();
  static_cast<SwOSOLED*>(me)->drawRoundRect( x0, y0, w, h, radius, fill, white );
  static_cast<SwOSOLED*>(me)->unlock();
}

void FtSwarmOLED::drawChar(int16_t x, int16_t y, unsigned char c, bool color, bool bg, uint8_t size_x, uint8_t size_y) {

  if (!me) return;
  
  static_cast<SwOSOLED*>(me)->lock();
  static_cast<SwOSOLED*>(me)->drawChar(x, y, c, color, bg, size_x, size_y );
  static_cast<SwOSOLED*>(me)->unlock();
}

void FtSwarmOLED::getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {

  if (!me) return;
  
  static_cast<SwOSOLED*>(me)->lock();
  static_cast<SwOSOLED*>(me)->getTextBounds( string, x, y, x1, y1, w, h );
  static_cast<SwOSOLED*>(me)->unlock();
}

void FtSwarmOLED::setTextSize(uint8_t sx, uint8_t sy) {

  if (!me) return;
  
  static_cast<SwOSOLED*>(me)->lock();
  static_cast<SwOSOLED*>(me)->setTextSize( sx, sy );
  static_cast<SwOSOLED*>(me)->unlock();
}

void FtSwarmOLED::setCursor(int16_t x, int16_t y) {

  if (!me) return;
  
  static_cast<SwOSOLED*>(me)->lock();
  static_cast<SwOSOLED*>(me)->setCursor( x, y );
  static_cast<SwOSOLED*>(me)->unlock();
}
 
void FtSwarmOLED::setTextColor(bool c, bool bg) {

  if (!me) return;
  
  static_cast<SwOSOLED*>(me)->lock();
  static_cast<SwOSOLED*>(me)->setTextColor( c, bg );
  static_cast<SwOSOLED*>(me)->unlock();
}

void FtSwarmOLED::setTextWrap(bool w) {

  if (!me) return;
  
  static_cast<SwOSOLED*>(me)->lock();
  static_cast<SwOSOLED*>(me)->setTextWrap( w );
  static_cast<SwOSOLED*>(me)->unlock();
}

void FtSwarmOLED::write( const char *str, int16_t x, int16_t y, FtSwarmAlign_t align, bool fill, bool invert ) {

  if (!me) return;
  
  static_cast<SwOSOLED*>(me)->lock();
  static_cast<SwOSOLED*>(me)->write( str, x, y, align, fill, invert );
  static_cast<SwOSOLED*>(me)->unlock();
}

void FtSwarmOLED::write( const char *str ) {

  if (!me) return;
  
  static_cast<SwOSOLED*>(me)->lock();
  static_cast<SwOSOLED*>(me)->write( str );
  static_cast<SwOSOLED*>(me)->unlock();
}

int16_t FtSwarmOLED::getWidth(void) {

  if (!me) return 0;
  
  static_cast<SwOSOLED*>(me)->lock();
  int16_t result = static_cast<SwOSOLED*>(me)->getWidth( );
  static_cast<SwOSOLED*>(me)->unlock();

  return result;
}

int16_t FtSwarmOLED::getHeight(void) {

  if (!me) return 0;
  
  static_cast<SwOSOLED*>(me)->lock();
  int16_t result = static_cast<SwOSOLED*>(me)->getHeight( );
  static_cast<SwOSOLED*>(me)->unlock();

  return result;
}

uint8_t FtSwarmOLED::getRotation(void) {

  if (!me) return 0;
  
  static_cast<SwOSOLED*>(me)->lock();
  uint8_t result = static_cast<SwOSOLED*>(me)->getRotation( );
  static_cast<SwOSOLED*>(me)->unlock();

  return result;
}

void FtSwarmOLED::getCursor(int16_t *x, int16_t *y) {

  if (!me) return;
  
  static_cast<SwOSOLED*>(me)->lock();
  static_cast<SwOSOLED*>(me)->getCursor( x, y);
  static_cast<SwOSOLED*>(me)->unlock();

}

// **** FtSwarmCAM ****

FtSwarmCAM::FtSwarmCAM( FtSwarmSerialNumber_t serialNumber ):FtSwarmIO( serialNumber, SWOSIO_CAM) {};
FtSwarmCAM::FtSwarmCAM( const char *name ):FtSwarmIO( name, SWOSIO_CAM ) {};

void FtSwarmCAM::streaming( bool onOff ) {

  if (!me) return;

  static_cast<SwOSCAM*>(me)->lock();
  static_cast<SwOSCAM *>(me)->setStreaming( onOff, false);
  static_cast<SwOSCAM*>(me)->unlock();
}

void FtSwarmCAM::setFramesize( framesize_t framesize ) {

  if (!me) return;

  static_cast<SwOSCAM*>(me)->lock();
  static_cast<SwOSCAM *>(me)->setFramesize(framesize, false);
  static_cast<SwOSCAM*>(me)->unlock();
}

void FtSwarmCAM::setQuality( int quality ) {

  if (!me) return;

  static_cast<SwOSCAM*>(me)->lock();
  static_cast<SwOSCAM *>(me)->setQuality( quality, false );
  static_cast<SwOSCAM*>(me)->unlock();
}

void FtSwarmCAM::setBrightness( int brightness ) {

  if (!me) return;

  static_cast<SwOSCAM*>(me)->lock();
  static_cast<SwOSCAM *>(me)->setBrightness(brightness, false);
  static_cast<SwOSCAM*>(me)->unlock();
}

void FtSwarmCAM::setContrast( int contrast ) {

  if (!me) return;

  static_cast<SwOSCAM*>(me)->lock();
  static_cast<SwOSCAM *>(me)->setContrast(contrast, false);
  static_cast<SwOSCAM*>(me)->unlock();
}

void FtSwarmCAM::setSaturation( int saturation ) {

  if (!me) return;

  static_cast<SwOSCAM*>(me)->lock();
  static_cast<SwOSCAM *>(me)->setSaturation(saturation, false);
  static_cast<SwOSCAM*>(me)->unlock();
}

void FtSwarmCAM::setSpecialEffect( int specialEffect ) {

  if (!me) return;

  static_cast<SwOSCAM*>(me)->lock();
  static_cast<SwOSCAM *>(me)->setSpecialEffect(specialEffect, false);
  static_cast<SwOSCAM*>(me)->unlock();
}

void FtSwarmCAM::setWbMode( int wbMode ) {

  if (!me) return;

  static_cast<SwOSCAM*>(me)->lock();
  static_cast<SwOSCAM *>(me)->setWbMode(wbMode, false);
  static_cast<SwOSCAM*>(me)->unlock();
}

void FtSwarmCAM::setHMirror( bool hMirror ) {

  if (!me) return;

  static_cast<SwOSCAM*>(me)->lock();
  static_cast<SwOSCAM *>(me)->setHMirror(hMirror, false);
  static_cast<SwOSCAM*>(me)->unlock();
}

void FtSwarmCAM::setVFlip( bool vFlip ) {

  if (!me) return;

  static_cast<SwOSCAM*>(me)->lock();
  static_cast<SwOSCAM *>(me)->setVFlip(vFlip, false);
  static_cast<SwOSCAM*>(me)->unlock();
}


// **** FtSwarm ****

FtSwarmSerialNumber_t FtSwarm::begin( bool verbose, bool waitOnControllers ) {

  FtSwarmSerialNumber_t result = myOSSwarm.begin( verbose );

  if (!nvs.IAmKelda) {
    SWARM_LOG_ERROR("Please configure this controller as Kelda.");
    firmware();
    ESP.restart();
  }

  while (!myOSSwarm.isOnline()) {
    delay(250);
  }

  return result;

}

void FtSwarm::halt( void ) {

  myOSSwarm.halt( );

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

void forever( const char *prompt) { forever( (char*) prompt ); }