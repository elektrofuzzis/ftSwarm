/*
 * ftSwarm.cpp
 *
 * framework to build a ftSwarm application
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#include "ftSwarm.h"
#include "SwOSSwarm.h"
#include "easykey.h"

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
    if (!me) {
      if ( firstTry ) ESP_LOGD( LOGFTSWARM, "waiting on device" );
      firstTry = false;
      vTaskDelay( 25 / portTICK_PERIOD_MS );
    }

  }
  
};

FtSwarmIO::FtSwarmIO( const char *name ) {
  // constructor: register at myOSSwarm using a name/alias

  bool firstTry = true;

  while (!me) {

    myOSSwarm.lock();
    me = myOSSwarm.getIO( name );
    myOSSwarm.unlock();

    // no success, wait 25 ms
    if (!me) {
      if ( firstTry ) ESP_LOGD( LOGFTSWARM, "waiting on device" );
      firstTry = false;
      vTaskDelay( 25 / portTICK_PERIOD_MS );
    }

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

// **** FtSwarmReedSwitch ****

FtSwarmReedSwitch::FtSwarmReedSwitch( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, bool normallyOpen):FtSwarmDigitalInput( serialNumber, port, FTSWARM_REEDSWITCH, normallyOpen ) {};
FtSwarmReedSwitch::FtSwarmReedSwitch( const char * name, bool normallyOpen):FtSwarmDigitalInput( name, FTSWARM_REEDSWITCH, normallyOpen ) {};

// **** FtSwarmButton ****

FtSwarmButton::FtSwarmButton( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port):FtSwarmIO( serialNumber, port, FTSWARM_BUTTON ) {};
FtSwarmButton::FtSwarmButton( const char *name):FtSwarmIO( name ) {};

// real stuff
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

// **** FtSwarmEncodeMotor

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

void FtSwarmOLED::clearDisplay(void) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->clearDisplay();
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

void FtSwarmOLED::drawPixel(int16_t x, int16_t y, uint16_t color) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->drawPixel( x, y, color );
  myOSSwarm.unlock();
}

void FtSwarmOLED::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->drawFastHLine( x, y, w, color );
  myOSSwarm.unlock();
}

void FtSwarmOLED::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->drawFastVLine( x, y, h, color );
  myOSSwarm.unlock();
}

void FtSwarmOLED::startscrollright(uint8_t start, uint8_t stop) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->startscrollright( start, stop );
  myOSSwarm.unlock();
}

void FtSwarmOLED::startscrollleft(uint8_t start, uint8_t stop) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->startscrollleft( start, stop );
  myOSSwarm.unlock();
}

void FtSwarmOLED::startscrolldiagright(uint8_t start, uint8_t stop) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->startscrolldiagright( start, stop );
  myOSSwarm.unlock();
}

void FtSwarmOLED::startscrolldiagleft(uint8_t start, uint8_t stop) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->startscrolldiagleft( start, stop );
  myOSSwarm.unlock();
}

void FtSwarmOLED::stopscroll(void) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->stopscroll( );
  myOSSwarm.unlock();
}

void FtSwarmOLED::setRotation(uint8_t r) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->setRotation( r );
  myOSSwarm.unlock();
}

void FtSwarmOLED::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->fillRect( x, y,w, h, color );
  myOSSwarm.unlock();
}

void FtSwarmOLED::fillScreen(uint16_t color) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->fillScreen( color );
  myOSSwarm.unlock();
}

void FtSwarmOLED::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->drawLine( x0, y0, x1, y1, color );
  myOSSwarm.unlock();
}

void FtSwarmOLED::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->drawRect( x, y, w, h, color );
  myOSSwarm.unlock();
}

void FtSwarmOLED::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->drawCircle( x0, y0, r, color );
  myOSSwarm.unlock();
}

void FtSwarmOLED::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->fillCircle( x0, y0, r, color );
  myOSSwarm.unlock();
}

void FtSwarmOLED::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->drawTriangle( x0, y0, x1, y1, x2, y2, color );
  myOSSwarm.unlock();
}

void FtSwarmOLED::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->fillTriangle( x0, y0, x1, y1, x2, y2, color );
  myOSSwarm.unlock();
}

void FtSwarmOLED::drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->drawRoundRect( x0, y0, w, h, radius, color );
  myOSSwarm.unlock();
}

void FtSwarmOLED::fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->fillRoundRect( x0, y0, w, h, radius, color );
  myOSSwarm.unlock();
}

void FtSwarmOLED::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->drawChar(x, y, c, color, bg, size );
  myOSSwarm.unlock();
}

void FtSwarmOLED::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y) {

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

void FtSwarmOLED::setTextSize(uint8_t s) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->setTextSize( s );
  myOSSwarm.unlock();
}

void FtSwarmOLED::setTextSize(uint8_t sx, uint8_t sy) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->setTextSize( sx, sy );
  myOSSwarm.unlock();
}

void FtSwarmOLED::setFont(const GFXfont *f ) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->setFont( f );
  myOSSwarm.unlock();
}
   
void FtSwarmOLED::setCursor(int16_t x, int16_t y) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->setCursor( x, y );
  myOSSwarm.unlock();
}
 
void FtSwarmOLED::setTextColor(uint16_t c) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->setTextColor( c );
  myOSSwarm.unlock();
}
 
void FtSwarmOLED::setTextColor(uint16_t c, uint16_t bg) {

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

void FtSwarmOLED::cp437(bool x) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->cp437( x );
  myOSSwarm.unlock();
}

void FtSwarmOLED::write(uint8_t ch)  {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->write( ch );
  myOSSwarm.unlock();
}

void FtSwarmOLED::write(const char *str) {

  if (!me) return;
  
  myOSSwarm.lock();
  static_cast<SwOSOLED*>(me)->write( str );
  myOSSwarm.unlock();
}

int16_t FtSwarmOLED::width(void) {

  if (!me) return 0;
  
  myOSSwarm.lock();
  int16_t result = static_cast<SwOSOLED*>(me)->width( );
  myOSSwarm.unlock();

  return result;
}

int16_t FtSwarmOLED::height(void) {

  if (!me) return 0;
  
  myOSSwarm.lock();
  int16_t result = static_cast<SwOSOLED*>(me)->height( );
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

int16_t FtSwarmOLED::getCursorX(void) {

  if (!me) return 0;
  
  myOSSwarm.lock();
  int16_t result = static_cast<SwOSOLED*>(me)->getCursorX( );
  myOSSwarm.unlock();

  return result;
}

int16_t FtSwarmOLED::getCursorY(void) {

  if (!me) return 0;
  
  myOSSwarm.lock();
  int16_t result = static_cast<SwOSOLED*>(me)->getCursorY( );
  myOSSwarm.unlock();

  return result;
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

void wifiMenu( void ) {

  char     prompt[250];
  uint16_t maxItem;
  bool     anythingChanged = false;

  while(1) {

    printf("\nWifi menu\n\n");

    if ( myOSSwarm.nvs.APMode ) {
      sprintf( prompt, "(1) AP-Mode:  AP-MODE\n(2) SSID:     %s\n(X) Password: NOPASSWORD\n(4) Channel:  %d\n\n(0) main\nwifi>", myOSSwarm.nvs.wifiSSID, myOSSwarm.nvs.channel );
      maxItem = 4;
      
    } else {
      sprintf( prompt, "(1) AP-Mode:  CLIENT-MODE\n(2) SSID:     %s\n(3) Password: ******\n\n(0) exit\nwifi>", myOSSwarm.nvs.wifiSSID );
      maxItem = 3;
    }
    
    switch( enterNumber( prompt, 0, 0, maxItem ) ) {
      
      case 0: // exit
        if ( ( anythingChanged) && ( yesNo( "To apply your changes, the device needs to be restarted.\nSave settings and restart now (Y/N)?") ) ) {
          // save config
          myOSSwarm.nvs.saveAndRestart();
        } else {
          return;
        }
        
      case 1: // AP-Mode/Client-Mode
        anythingChanged = true;
        myOSSwarm.nvs.APMode = !myOSSwarm.nvs.APMode;
        if ( ( myOSSwarm.nvs.APMode ) && ( ( myOSSwarm.nvs.channel < 1 ) || ( myOSSwarm.nvs.channel > 13 ) ) ) myOSSwarm.nvs.channel = 1; // to avoid invalid channel settings
        break;
        
      case 2: // SSID
        anythingChanged = true;
        enterString("Please enter new SSID: ", myOSSwarm.nvs.wifiSSID, 64);
        break;
        
      case 3: // Password
        if (!myOSSwarm.nvs.APMode) {
          anythingChanged = true;
          enterString("Please enter new Password: ", myOSSwarm.nvs.wifiPwd, 64, true);
        }
        break;

      case 4: // Channel
        anythingChanged = true;
        myOSSwarm.nvs.channel = enterNumber( "enter channel [1..13] - use 1,6 or 11 if possible: ", myOSSwarm.nvs.channel, 1, 13 );
        break;
    }
    
  }
  
}


void joinSwarm( bool createNewSwarm ) {

  // create a new swarm
  char name[64];
  char prompt[256];

  // get new swarm's name
  while (1) {
    
    enterString( "Please enter new swarm's name [minimm 5 chars]: ", name, MAXIDENTIFIER);

    // different names, at least 5 chars
    if ( ( strcmp( myOSSwarm.nvs.swarmName, name ) != 0 ) && ( strlen( name ) > 4 ) ) {
      break;
    }

  }

  // get new swarm's pin
  uint16_t pin = enterNumber("Please enter new swarm's PIN [1..9999]: ", 0, 1, 9999 );

  // build swarm
  if ( createNewSwarm ) {
    sprintf( prompt, "Do you really want to quit swarm \"%s\" and create new swarm \"%s\" with pin %d (Y/N) ?", myOSSwarm.nvs.swarmName, name, pin );
  } else {
    sprintf( prompt, "Do you really want to quit swarm \"%s\" and join swarm \"%s\" with pin %d (Y/N) ?", myOSSwarm.nvs.swarmName, name, pin );
  }

  // stop, if NO
  if ( !yesNo( prompt ) ) return;

  // now build/join swarm
  myOSSwarm.lock();
  myOSSwarm.leaveSwarm( );
  myOSSwarm.allowPairing = true;
  myOSSwarm.joinSwarm( createNewSwarm, name, pin );
  myOSSwarm.unlock();

  // wait some ticks to catch a response
  vTaskDelay( 250 / portTICK_PERIOD_MS );

  // stop pairing
  myOSSwarm.allowPairing = false;

  // now I need to register again
  myOSSwarm.lock();
  myOSSwarm.registerMe();
  myOSSwarm.unlock();

  // wait some ticks to catch a response
  vTaskDelay( 250 / portTICK_PERIOD_MS );

  // in case of a new swarm save nvs & quit.
  if ( ( createNewSwarm ) && ( myOSSwarm.members() <= 1 ) ) {
    myOSSwarm.nvs.save();
    printf("Swarm \"%s\" created sucessfully.\n", name );
    return;
  }

  // in case of joining a swarm, check on other devices and get new swarm's secret
  if ( myOSSwarm.members() <= 1 ) { // no other device out there
    // restore old settings
    myOSSwarm.lock();
    myOSSwarm.reload();
    myOSSwarm.unlock();
    printf("ERROR: swarm \"%s\" not found. Rejoined old swam %s\n", name, myOSSwarm.nvs.swarmName );
    return;
  }

  // the CMD_GOTYOU-Message catched the new SECRET
  // so just store all changes
  myOSSwarm.nvs.save();
  
  printf("Swarm \"%s\" joined sucessfully.\n", name );

}

void swarmMenu( void ) {
  
  while (1) {

    printf( "\nSwarm menu\n\nThis device is connected to swarm \"%s\" with %d member(s) online.\nSwarm PIN is %d.\n", myOSSwarm.nvs.swarmName, myOSSwarm.members(), myOSSwarm.nvs.swarmPIN );
    
    switch( enterNumber("(1) create a new swarm\n(2) join another swarm\n(3) list swarm members\n\n(0) main\nswarm>", 0, 0, 3) ) {
      case 0: // main
        return;
        
      case 1: // create new swarm
        joinSwarm( true ); 
        break;
        
      case 2: // join a swarm
        joinSwarm( false ); 
        break;
        
      case 3: // list swarm members
        printf("\nSwarm members:\n" );
        for (uint8_t i=0; i<=myOSSwarm.maxCtrl; i++ ) {
          if ( myOSSwarm.Ctrl[i] )
            printf("#%d %s\n", i, myOSSwarm.Ctrl[i]->getHostname() );
        }
        break;
    }
    
  }

}

void localMenu( void ) {

  SwOSObj *OSObj[20];
  bool    anythingChanged = false;

  while (1) {

    uint8_t item = 0;
    printf("local controler menu:\n\n");
    
    OSObj[item++] = myOSSwarm.Ctrl[0]; 
    printf("(%2d) hostname %s - %s\n", item, myOSSwarm.Ctrl[0]->getName(), myOSSwarm.Ctrl[0]->getAlias() );

    // list inputs
    for (uint8_t i=0; i<4; i++ ) { 
      OSObj[item++] = myOSSwarm.Ctrl[0]->input[i];
      printf("(%2d) %-4s - %-32s\n", 
              item, myOSSwarm.Ctrl[0]->input[i]->getName(), myOSSwarm.Ctrl[0]->input[i]->getAlias());
    }

    // list actors
    for (uint8_t i=0; i<2; i++ ) {
      OSObj[item++] = myOSSwarm.Ctrl[0]->actor[i];
      printf("(%2d) %-4s - %-32s\n", 
             item, myOSSwarm.Ctrl[0]->actor[i]->getName(), myOSSwarm.Ctrl[0]->actor[i]->getAlias());
    }

    // FTSWARM special HW
    if ( myOSSwarm.Ctrl[0]->getType() == FTSWARM ) {
      
      SwOSSwarmJST *ftSwarm = static_cast<SwOSSwarmJST *>(myOSSwarm.Ctrl[0]);

      // list LEDs
      for (uint8_t i=0; i<2; i++ ) {
        OSObj[item++] = ftSwarm->led[i];
        printf("(%2d) %-4s - %-32s\n", 
               item, ftSwarm->led[i]->getName(), ftSwarm->led[i]->getAlias());
      }
      
      // list gyro
      if (ftSwarm->gyro) {
        OSObj[item++] = ftSwarm->gyro;
        printf("(%2d) %-4s - %-32s\n", item, ftSwarm->gyro->getName(), ftSwarm->gyro->getAlias() );
      }
    }

    // FTSWARMCONTROL special HW
    if ( myOSSwarm.Ctrl[0]->getType() == FTSWARMCONTROL ) {
      
      SwOSSwarmControl *ftSwarmControl = static_cast<SwOSSwarmControl *>(myOSSwarm.Ctrl[0]);

      // buttons
      for (uint8_t i=0; i<8; i++ ) {
        OSObj[item++] = ftSwarmControl->button[i];
        printf("(%2d) %-4s - %-32s\n", 
                item, ftSwarmControl->button[i]->getName(),   ftSwarmControl->button[i]->getAlias() );
      }

      // joysticks
      for (uint8_t i=0; i<2; i++ ) {
        OSObj[item++] = ftSwarmControl->joystick[i];
        printf("(%2d) %-4s - %-32s\n", 
               item, ftSwarmControl->joystick[i]->getName(), ftSwarmControl->joystick[i]->getAlias());
      }
      
      if (ftSwarmControl->oled) {
        OSObj[item++] = ftSwarmControl->oled;
        printf("(%2d) %-4s - %-32s\n", item, ftSwarmControl->oled->getName(), ftSwarmControl->oled->getAlias() );
      }
      
      printf("(%2d) calibrate joysticks\n", ++item );
      
    }

    // User's choise
    uint8_t choise = enterNumber("\n(0) exit\nlocal controler>", 0, 0, item );

    // exit?
    if ( choise == 0 ) {
      if ( ( anythingChanged )  &&  yesNo( "Save changes? (Y/N)?" ) ) {

        // Open
        nvs_handle_t my_handle;
        ESP_ERROR_CHECK( nvs_open("ftSwarm", NVS_READWRITE, &my_handle) );

        // save
        myOSSwarm.Ctrl[0]->saveAliasToNVS( my_handle );

        // commit
        ESP_ERROR_CHECK( nvs_commit( my_handle ) );
          
      }
      return;

    // calibrate joystick?
    } else if ( ( choise == item ) && ( myOSSwarm.Ctrl[0]->getType() == FTSWARMCONTROL ) ) {
      if ( yesNo( "Start calibration (Y/N)?" ) ) {
        static_cast<SwOSSwarmControl *>(myOSSwarm.Ctrl[0])->joystick[0]->calibrate( &myOSSwarm.nvs.joyZero[0][0], &myOSSwarm.nvs.joyZero[0][1] );
        static_cast<SwOSSwarmControl *>(myOSSwarm.Ctrl[0])->joystick[1]->calibrate( &myOSSwarm.nvs.joyZero[1][0], &myOSSwarm.nvs.joyZero[1][1] );
        myOSSwarm.nvs.save();
        printf("Saving joystick calibration data...done!\n");
      }
    
    // set name
    } else {
      char alias[MAXIDENTIFIER];
      char prompt[250];
      sprintf( prompt, "%s - please enter new alias: ", OSObj[choise-1]->getName() );
      enterIdentifier( prompt, alias, MAXIDENTIFIER );
      OSObj[choise-1]->setAlias( alias );
      anythingChanged = true;
    }
    
  }
  
}

void FtSwarm::setup( void ) {

  printf("\n\nftSwarmOS %s\n\n(C) Christian Bergschneider & Stefan Fuss\n", SWOSVERSION );

  while (1) {
    
    switch( enterNumber("\nMain menu\n\n(1) wifi settings\n(2) swarm settings\n(3) local controller\n\n(0) exit\nmain>", 0, 0, 3) ) {
      case 0: return;
      case 1: wifiMenu(); break;
      case 2: swarmMenu(); break;
      case 3: localMenu(); break;
    }
    
  }

}
