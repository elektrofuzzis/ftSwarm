//////////////////////////////////////////////////////////////////
//
// ftSwarmInput.h
//
// ftSwarm Input Control
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
/////////////////////////////////////////////////////////////////

#ifndef FTSWARMINPUT_H
#define FTSWARMINPUT_H

#include "FtSwarmObj.h"
#include <arduino.h>

#define FTSWARM_A1 0
#define FTSWARM_A2 1
#define FTSWARM_A3 2
#define FTSWARM_A4 3

enum ftSwarmState       { FTSWARM_LOW = 0, FTSWARM_HIGH = 1, UNKNOWN = 255};
enum ftSwarmContactType { FTSWARM_NO = 0, FTSWARM_NC = 1 };
enum FtSwarmNTCType     { FTSWARM_NTC1k5 = 0, FTSWARM_NTC68k = 1 };
enum ftSwarmTrailState  { FTSWARM_NOTRAIL = 0, FTSWARM_LEFT = 1, FTSWARM_RIGHT = 2, FTSWARM_BOTH = 3 };

// general digital input class
class FtSwarmDigitalIn : public FtSwarmObj {
  protected:
    uint8_t _io;
  public:
    FtSwarmDigitalIn( char* UUID, uint8_t inputPort );
      // constructor
    virtual char* classname();
      // my class name as string
    ftSwarmState getState( );
      // FTSWARM_LOW or FTSWARM_HIGH
    boolean isHigh( );
      // check on low
    boolean isLow( );
      // check on high
};

// general analog input class
class FtSwarmAnalogIn : public FtSwarmDigitalIn {
  protected:
    ftSwarmState _lastState;
    uint16_t     _lowTrigger;
    uint16_t     _highTrigger;
    double       _refVoltage;
    uint32_t     _R1x, _R2x, _R3x;
  public:
    FtSwarmAnalogIn( char* UUID, uint8_t inputPort, 
                     uint16_t lowTrigger = 1365, uint16_t highTrigger = 2730, 
                     double refVoltage = UREF, uint32_t R1x = 2000, uint32_t R2x = 47000, uint32_t R3x = 82000);
      // constructor, trigger values are used in getState() to interpret an anlog value
    virtual char* classname();
      // my class name as string
    virtual uint16_t getValue( );
      // get analog reading
    virtual ftSwarmState getState( );
      // return FTSWARM_LOW or FTSWARM_HIGH based on constructor's trigger trasholds
    virtual double getVoltage( );
};

// fischertechnik switches (part no. 37780, 37783, 31332, 36591, 36707, 36708 )
class FtSwarmSwitch : public FtSwarmDigitalIn {
  protected:
    ftSwarmContactType _contactType;
  public:
    FtSwarmSwitch( char* UUID, uint8_t inputPort, ftSwarmContactType contactType = FTSWARM_NO );
      // constructor, if you use a normaly closed switch (FTSWARM_NC) getState will return HIGH, if the switch is opened
    virtual char* classname();
      // my class name as string
    virtual ftSwarmState getState( );
      // state
};

// fischertechnik read switch (part no. 36120)
class FtSwarmReadSwitch : public FtSwarmSwitch {
  public :
    FtSwarmReadSwitch( char* UUID, uint8_t inputPort, ftSwarmContactType contactType = FTSWARM_NO );
      // constructor
    virtual char* classname();
      // my class name as string
};

// general resistor
class FtSwarmResistor : protected FtSwarmAnalogIn {
  public:
    FtSwarmResistor( char* UUID, uint8_t inputPort, uint32_t R1x = 2000, uint32_t R2x = 47000, uint32_t R3x = 82000 );
      // constructor
    virtual char* classname();
      // my class name as string
    double getR();
      // get resistance
};

class FtSwarmNTC : protected FtSwarmResistor {
  protected:
    double _refResistance, _refTemperature, _temperatureCoefficient;
  public:
    FtSwarmNTC( char* UUID, uint8_t inputPort, double refResistance=1500, double refTemperature=25, double temperatureCoefficient=3000, uint32_t R1x = 2000, uint32_t R2x = 47000, uint32_t R3x = 82000 );
      // constructor
      // R restistance at T degree C, B thermal ntc constant
    virtual char* classname();
      // my class name as string
    double getTemperature();
      // reads the temperature 
};

class FtSwarmPhotoTransistor : public FtSwarmAnalogIn {
  public:
    FtSwarmPhotoTransistor( char* UUID, uint8_t inputPort, uint16_t lowTrigger = 1365, uint16_t highTrigger = 2730 );
    virtual char* classname();
      // my class name as string
};

class FtSwarmTrailSensor : protected FtSwarmObj {
  protected:
    FtSwarmDigitalIn *_leftSensor;
    FtSwarmDigitalIn *_rightSensor;
  public:
    FtSwarmTrailSensor( char* UUID, uint8_t inputPortLeft, uint8_t inputPortRight );
      // constructor
    virtual char* classname();
      // my class name as string
    ftSwarmTrailState getState( );
      // get position of trail sensor
};

class FtSwarmColorSensor : protected FtSwarmAnalogIn {
  public:
    FtSwarmColorSensor( char* UUID, uint8_t inputPort );
    virtual char* classname();
      // my class name as string
    uint16_t getColor();
};

class FtSwarmUltrasonic : protected FtSwarmObj {
  protected:
    double _lastReading = -1;
  public:
    FtSwarmUltrasonic( char* UUID );
      // constructor
    virtual char* classname();
      // my class name as string
    double getDistance();
};

#endif
