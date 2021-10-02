//////////////////////////////////////////////////////////////////
//
// ftSwarmInput.h
//
// ftSwarm Input Control
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
/////////////////////////////////////////////////////////////////

#include "ftSwarmInput.h"
#include "math.h"

/////////////////////////////////////////////////////////////
//
//                     FtSwarmDigitalIn
//
/////////////////////////////////////////////////////////////

FtSwarmDigitalIn::FtSwarmDigitalIn( char* UUID, uint8_t inputPort ) : FtSwarmObj( UUID, inputPort ) {

  if (isLocal()) {
    
    switch ( _port ) {
      case 0:
        _io = AIN1;
        break;
      case 1:
        _io = AIN2;
        break;
      case 2:
        _io = AIN3;
        break;
      case 3:
        _io = AIN4;
        break;
      default:
        _lastError = NO_PORT;
        break;
    }
    pinMode( _io, INPUT );
        
  } else {
    
  }
  
}

char* FtSwarmDigitalIn::classname() {
  return "FtSwarmDigitalIn";
}

ftSwarmState FtSwarmDigitalIn::getState( ) {

  if (isLocal()) {

    if ( digitalRead( _io ) ) {
      return FTSWARM_HIGH;
    } else {
      return FTSWARM_LOW;
    }
        
  } else {

    return (ftSwarmState) rpc( GETSTATE );
    
  }
  
}

boolean FtSwarmDigitalIn::isHigh( ) {
  return (getState() == FTSWARM_HIGH );
}

boolean FtSwarmDigitalIn::isLow( ) {
  return (getState() == FTSWARM_LOW );
}

/////////////////////////////////////////////////////////////
//
//                     FtSwarmAnalogIn
//
/////////////////////////////////////////////////////////////

FtSwarmAnalogIn::FtSwarmAnalogIn( char* UUID, uint8_t inputPort, 
                                  uint16_t lowTrigger, uint16_t highTrigger, 
                                  double   refVoltage, uint32_t R1x, uint32_t R2x, uint32_t R3x) : FtSwarmDigitalIn( UUID, inputPort ) {

  // keep initialisation values
  _refVoltage  = refVoltage;
  _R1x = R1x;
  _R2x = R2x;
  _R3x = R3x;
  _lowTrigger  = lowTrigger;
  _highTrigger = highTrigger;

  // get a first reading
  if ( getValue() > _highTrigger ) {
    _lastState = FTSWARM_HIGH;
  } else {
    _lastState = FTSWARM_LOW;
  }
}

char* FtSwarmAnalogIn::classname() {
  return "FtSwarmAnalogIn";
}

uint16_t FtSwarmAnalogIn::getValue( ) {

  if ( isLocal() ) {

    // attention: this is a little confusing...
    // first read analog value, no problem
    // afterwards we need to interpret the trashhold based state as well, to identify changes on that state

    uint16_t newValue = analogRead( _io );

    // decide digital value
    if ( newValue > _highTrigger ) {
      // allways high
      _lastState = FTSWARM_HIGH;
    } else if ( newValue < _lowTrigger ) {
      // allways low
      _lastState = FTSWARM_LOW;
    } else {
      // between lowTrigger & highTrigger: keep last value
    }

    return newValue;
    
  } else {
    
    return (uint16_t) rpc( GETVALUE );
    
  }
}

ftSwarmState FtSwarmAnalogIn::getState( ) {

  // get reading, ignore result
  getValue( );

  // because the triggerd value is interpreted by getValue and stored in _lastState
  return _lastState;
  
}

double FtSwarmAnalogIn::getVoltage( ) {
   
   // read analog value
  double a_value = (double) FtSwarmAnalogIn::getValue();
  
  // calculate RL's voltage
  double U = (_R2x + _R3x) / _R3x * a_value;

  return U;
}

/////////////////////////////////////////////////////////////
//
//                     FtSwarmSwitch
//
/////////////////////////////////////////////////////////////

FtSwarmSwitch::FtSwarmSwitch( char* UUID, uint8_t inputPort, ftSwarmContactType contactType ) : FtSwarmDigitalIn( UUID, inputPort ) {

  // store cotactType to use in getState
  _contactType = contactType;
}

char* FtSwarmSwitch::classname(){
  return "FtSwarmSwitch";
}

ftSwarmState FtSwarmSwitch::getState( ) {

  ftSwarmState state = FtSwarmDigitalIn::getState();

  if ( _contactType == FTSWARM_NC ) {
    
    // in case of an normally closed contac NC, invert reading
    if ( state == FTSWARM_HIGH ) { 
      return FTSWARM_LOW;  
    } else { 
      return FTSWARM_HIGH;
    }

  } else {
    // normally open contact: nothing to change
    return state;

  }
  
}

/////////////////////////////////////////////////////////////
//
//                     FtSwarmReadSwitch
//
/////////////////////////////////////////////////////////////

FtSwarmReadSwitch::FtSwarmReadSwitch( char* UUID, uint8_t inputPort, ftSwarmContactType contactType ) : FtSwarmSwitch ( UUID, inputPort, contactType ) {
  
}

char* FtSwarmReadSwitch::classname() {
  return "FtSwarmReadSwitch";
}

/////////////////////////////////////////////////////////////
//
//                     FtSwarmResistor
//
/////////////////////////////////////////////////////////////

/*  
                                   UR3 = Reading
                                        |
                         |----[R2=47k]--|--[R3=82k]---GND
  +4.7V ------[R1=2k]----|
                         |------------[RL]------------GND

 
*/

FtSwarmResistor::FtSwarmResistor( char* UUID, uint8_t inputPort, uint32_t R1x, uint32_t R2x, uint32_t R3x ) : FtSwarmAnalogIn( UUID, inputPort, 0, 0, UREF, R1x, R2x, R3x ) {
  
}

char* FtSwarmResistor::classname() {
  return "FtSwarmResistor";
}

double FtSwarmResistor::getR() {

  // read voltage
  double URL = FtSwarmAnalogIn::getVoltage();

  // repeating constant
  double c = _R2x + _R3x;

  // calculate RL
  double RL = _R1x * c / ( c * 4.7/URL - _R1x - c );

  return RL;
}

/////////////////////////////////////////////////////////////
//
//                     FtSwarmNTC
//
/////////////////////////////////////////////////////////////

FtSwarmNTC::FtSwarmNTC( char* UUID, uint8_t inputPort, double refResistance, double refTemperature, double temperatureCoefficient, uint32_t R1x, uint32_t R2x, uint32_t R3x ) : FtSwarmResistor( UUID, inputPort, R1x, R2x, R3x ) {

  // store NTC parameters
  _refResistance          = refResistance;
  _refTemperature         = refTemperature+273.15; // Kelvin!
  _temperatureCoefficient = temperatureCoefficient;
  
}

char* FtSwarmNTC::classname() {
  return "FtSwarmNTC";
}

double FtSwarmNTC::getTemperature() {

  // read restistance
  double r = FtSwarmResistor::getR();

  // calculate T [Kelvin]
  double t = _refTemperature / ( 1 - _refTemperature/_temperatureCoefficient * log( r / _refResistance ) );

  // return °C
  return t - 273.15;
  
}

/////////////////////////////////////////////////////////////
//
//                FtSwarmPhotoTransistor
//
/////////////////////////////////////////////////////////////

FtSwarmPhotoTransistor::FtSwarmPhotoTransistor( char* UUID, uint8_t inputPort, uint16_t lowTrigger, uint16_t highTrigger ) : FtSwarmAnalogIn( UUID, inputPort, UREF, lowTrigger, highTrigger ) {
  
}

char* FtSwarmPhotoTransistor::classname() {
  return "FtSwarmPhotoTransistor";
}

/////////////////////////////////////////////////////////////
//
//                FtSwarmTrailSensor
//
/////////////////////////////////////////////////////////////

FtSwarmTrailSensor::FtSwarmTrailSensor( char* UUID, uint8_t inputPortLeft, uint8_t inputPortRight ) : FtSwarmObj( UUID, 0 ) {

  _leftSensor = new FtSwarmDigitalIn( UUID, inputPortLeft );
  _rightSensor = new FtSwarmDigitalIn( UUID, inputPortRight );
  
}
char* FtSwarmTrailSensor::classname() {
  return "FtSwarmTrailSensor";
}

ftSwarmTrailState FtSwarmTrailSensor::getState( ) {

  ftSwarmState stateLeft  = _leftSensor->getState();
  ftSwarmState stateRight = _leftSensor->getState();

  // 00 -> NOTRAIL
  if ( ( stateLeft == FTSWARM_LOW ) && ( stateRight == FTSWARM_LOW ) ) return FTSWARM_NOTRAIL;

  // 01 -> go RIGHT
  if ( ( stateLeft == FTSWARM_LOW ) && ( stateRight == FTSWARM_HIGH ) ) return FTSWARM_RIGHT;

  // 10 -> go left
  if ( ( stateLeft == FTSWARM_HIGH ) && ( stateRight == FTSWARM_LOW ) ) return FTSWARM_LEFT;

  // else BOTH
  return FTSWARM_BOTH;
  
}

/////////////////////////////////////////////////////////////
//
//                FtSwarmColorSensor
//
/////////////////////////////////////////////////////////////

FtSwarmColorSensor::FtSwarmColorSensor( char* UUID, uint8_t inputPort ) : FtSwarmAnalogIn( UUID, inputPort ) {
  
}

char* FtSwarmColorSensor::classname() {
  return "FtSwarmColorSensor";
}

uint16_t FtSwarmColorSensor::getColor() {
  return FtSwarmAnalogIn::getValue();
}

/////////////////////////////////////////////////////////////
//
//                FtSwarmUltrasonic
//
/////////////////////////////////////////////////////////////

#define ULTRASONIC_VALID 1<<6

FtSwarmUltrasonic::FtSwarmUltrasonic( char* UUID ) : FtSwarmObj( UUID, 0 ) {

  Serial1.begin(115200, SERIAL_8N1, ULTRASONICRX, ULTRASONICTX );
  digitalWrite( ULTRASONICTX, HIGH );
  
}

char* FtSwarmUltrasonic::classname() {
  return "FtSwarmUltrasonic";
}

double FtSwarmUltrasonic::getDistance() {

  uint16_t byte1 = 0;
  uint16_t byte2 = 0;

  // pulse
  digitalWrite( ULTRASONICTX, HIGH);
  ets_delay_us(78);
  digitalWrite( ULTRASONICTX, LOW);

  // wait until response
  delay(50);

  // read 2 bytes
  if ( Serial1.available() ) { byte1 = Serial1.read(); }
  if ( Serial1.available() ) { byte2 = Serial1.read(); }

  // only valid measurements, if no or unvalid data is read, byte 1 bit 6 is 0
  if ( byte1 & ULTRASONIC_VALID ) {
    _lastReading = (double) ( ( byte1 << 8 + byte2 ) & 0x3FF ) * 0.5;
  } 

  return _lastReading;
  
}
