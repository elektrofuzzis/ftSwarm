/*
 * SwOSSNVS.h
 *
 * internal represenation of nvs values.
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include "SwOS.h"

#include <stdint.h>

#define MAXNVSEVENTS    50
#define NVSVERSION      3

static const char* NVSNAMESPACE = "ftSwarm";

struct SwOSJoyCalibration_t {

  int16_t minValue;
  int16_t midValue;
  int16_t maxValue; 

} __attribute__((packed));

struct SwOSServoParameter_t {

  int16_t offset;
  int16_t minValue;
  int16_t maxValue;

} __attribute__((packed));

class SwOSIOUID {

  public:
    FtSwarmSerialNumber_t serialNumber;
    SwOSIOType_t ioType;
    uint8_t port;

    SwOSIOUID() { serialNumber = 0; ioType = SWOSIO_UNDEF; port = 0; };

    SwOSIOUID( FtSwarmSerialNumber_t serialNumber, SwOSIOType_t ioType, uint8_t port ) { this->serialNumber = serialNumber; this->ioType = ioType; this->port = port; };

    // a UID is null, if the serialNumber is 0
    bool isNull( void) { return ( serialNumber == 0 ); };

    // Equality Operator
    bool operator==(const SwOSIOUID& other) const { return ( serialNumber == other.serialNumber ) && ( ioType == other.ioType ) && ( port == other.port ); }

    // Inequality Operator
    bool operator!=(const SwOSIOUID& other) const { return !( *this == other ); }

} __attribute__((packed));

class SwOSTriggerMath {

  public:

    uint16_t raw;
    struct {
        FtSwarmTrigger_t  trigger : 4;  // trigger event like up, down, change_value
        FtSwarmOperator_t op      : 4;  // operators like +, -, =
        FtSwarmOperand_t  v1      : 4;  // 1st operand like constant, sensor's value, actor's value
        FtSwarmOperand_t  v2      : 4;  // 2nd operand like constant, sensor's value, actor's value
    } bits;

  SwOSTriggerMath() { raw = 0; };
  SwOSTriggerMath( FtSwarmTrigger_t trigger, FtSwarmOperator_t op, FtSwarmOperand_t v1, FtSwarmOperand_t v2 ) { bits.trigger = trigger; bits.op = op; bits.v1 = v1; bits.v2 = v2; };

} __attribute__((packed));

class SwOSNVSEvent {

  public: 
    SwOSIOUID sensor;
    SwOSIOUID actor;
    SwOSTriggerMath triggerMath;
    FtSwarmTriggerParameter parameter = { 0 };

    SwOSNVSEvent() { sensor.serialNumber = 0; };
    SwOSNVSEvent( SwOSIOUID sensor, SwOSIOUID actor, SwOSTriggerMath triggerMath, FtSwarmTriggerParameter parameter ) { this->sensor = sensor; this->actor = actor; this->triggerMath = triggerMath; this->parameter = parameter; };
    SwOSNVSEvent( SwOSIOUID sensor, SwOSIOUID actor, SwOSTriggerMath triggerMath, int32_t parameter ) { this->sensor = sensor; this->actor = actor; this->triggerMath = triggerMath; this->parameter.raw = parameter; };

    // checks, if this event is "NULL"
    bool isNull( void ) { return sensor.isNull( ); };

    // checks, if myself and otherEvent are the same
    bool cmp( SwOSNVSEvent *otherEvent );

    // set this event to "NULL"
    void clear( void ) { sensor.serialNumber = 0; };

} __attribute__((packed));

// wifi types
typedef enum { wifiOFF, wifiAP, wifiClient } FtSwarmWifi_t;

typedef enum {
  FTSWARM_NVSSCOPE_NONE         = 0,
  
  FTSWARM_NVSSCOPE_CORE         = 0b00000000001,
  FTSWARM_NVSSCOPE_FACTORYRESET = 0b00000000010,
  FTSWARM_NVSSCOPE_SWARM        = 0b00000000100,
  FTSWARM_NVSSCOPE_JOYSTICK     = 0b00000001000,
  FTSWARM_NVSSCOPE_SERVO        = 0b00000010000,
  FTSWARM_NVSSCOPE_PIXEL        = 0b00000100000,
  FTSWARM_NVSSCOPE_WIFI         = 0b00001000000,
  FTSWARM_NVSSCOPE_WEBUI        = 0b00010000000,
  FTSWARM_NVSSCOPE_EXTPORT      = 0b00100000000,
  FTSWARM_NVSSCOPE_EVENTS       = 0b01000000000,
  FTSWARM_NVSSCOPE_ALIAS        = 0b10000000000,

  FTSWARM_NVSSCOPE_INITAL       = 0b11111111111,
  FTSWARM_NVSSCOPE_ALL          = 0b11111111110,
  FTSWARM_NVSSCOPE_ALIASSERVO   = FTSWARM_NVSSCOPE_ALIAS | FTSWARM_NVSSCOPE_SERVO

} FtSwarmNVSScope_t;

class SwOSNVS {
    
  public:
    
	  int32_t version; // Version

	  FtSwarmVersion_t CPU;               // CPU Type & Version
	  FtSwarmSerialNumber_t serialNumber; // Serial Number

    bool factoryReset; // flag to reset the IOs settings via reboot

    // wifi
    struct wifi_t {
      uint8_t channel;       // Channel in AP Mode
	    char SSID[64];         // wifi SSID
      char Password[64];     // wifi password
      FtSwarmWifi_t mode;    // wifi mode
      bool webUI;            // webUI on/off
    } wifi;

    // swarm
    struct swarm_t {    
      bool IAmKelda;                         // who is Kelda?
      char name[MAXIDENTIFIER];              // name
      uint16_t secret;                       // protocol secret
      uint16_t pin;                          // pin
      FtSwarmCommunication_t communication;  // communication protocol
      FtSwarmSerialNumber_t member[MAXCTRL]; // members
      uint8_t speed;                         // communication speed for RS485
    } swarm;
    
    SwOSJoyCalibration_t joystick[4];        // Joystick calibration
    SwOSServoParameter_t servo[4];           // Servo offsets

    uint8_t pixels; // ftSwarmPixels

    // event
    struct event_t {
      uint8_t activeConfig;                               // used event config
      SwOSNVSEvent events[MAXEVENTCONFIGS][MAXNVSEVENTS]; // event configs
      char oledLabel[MAXEVENTCONFIGS][12][4];             // labels
      FtSwarmQuickConfig_t quickConfig[MAXEVENTCONFIGS];  // used quick configuration per config
    } events;

    bool spiGyro; // spi gyro?

    struct extensionPort_t {
      FtSwarmExtMode_t mode;      // use of extension port
      uint8_t I2CAddr;            // own I2C address
      uint8_t interruptLine;      // Interrupt line for I2C based communication - 0 off, 1 M1, 2 M2
      int16_t interruptOnOff[2];  // High/Low-Values for Interrupt lines
      uint8_t I2CRegisters;       // number of I2C-Registers
      bool gyro;                  // gyro enabled?
    } extensionPort;

    // ask user for HW details
    void initialSetup();   

    // constructor
	  SwOSNVS();

    // read data from nvs & run an _initialSetup if needed
	  void begin();

    // load data from nvs, return false if nvs contains invalid data
    bool load();

    // save config to flash
	  void save( FtSwarmNVSScope_t scope );

    // save config & restart
    void saveAndRestart( FtSwarmNVSScope_t scope );

    // save events
    void saveEvents();

    // load events
    void loadEvents();

    // delete all events
    void deleteAllEvents( uint8_t configuration );

    // delete all events
    void deleteAllEvents( void ) { for ( uint8_t i=0; i<MAXEVENTCONFIGS; i++ ) deleteAllEvents(i); };

    // delete event
    void deleteEvent( uint8_t configuration, uint8_t eventIndex );

    // add event
    bool addEvent( uint8_t configuration, SwOSNVSEvent *event );

    // add event
    bool addEvent( SwOSNVSEvent *event ) { return addEvent( events.activeConfig, event ); };

    // check on dublicates
    bool exists( uint8_t configuration, SwOSNVSEvent *event, SwOSNVSEvent *exclude );

    // create a new swarm
    void createSwarm( char *name, uint16_t pin );

    // add a controller
    bool addController( FtSwarmSerialNumber_t serialNumber );

    // delete a controller
    bool deleteController( FtSwarmSerialNumber_t serialNumber );

    // delete all controllers
    void deleteAllControllers( void );

    // number of swarm members
    uint8_t swarmMembers( void );

    // initialize
    void reset( bool factorySettings );

    // print settings for debugging only  
    void printNVS();

    // runs an nvs version upgrade, true if an upgrade took place
    bool upgrade( void );

};

extern SwOSNVS nvs;
