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
#define MAXEVENTCONFIGS 4
#define NVSVERSION      3

static const char* NVSNAMESPACE = "ftSwarm";

struct SwOSJoyCalibration_t {

  int16_t minValue;
  int16_t midValue;
  int16_t maxValue; 

} __attribute__((packed));

class SwOSIOUID {

  public:
    FtSwarmSerialNumber_t serialNumber;
    SwOSIOType_t ioType;
    uint8_t port;

    SwOSIOUID() { serialNumber = 0; ioType = SWOSIO_UNDEF; port = 0; };

    SwOSIOUID( FtSwarmSerialNumber_t serialNumber, SwOSIOType_t ioType, uint8_t port ) { this->serialNumber = serialNumber; this->ioType = ioType; this->port = port; };

    bool isNull( void) { return ( serialNumber == 0 ); };

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
    int32_t parameter = 0;

    SwOSNVSEvent() { sensor.serialNumber = 0; };
    SwOSNVSEvent( SwOSIOUID sensor, SwOSIOUID actor, SwOSTriggerMath triggerMath, int32_t parameter = 0 ) { this->sensor = sensor; this->actor = actor; this->triggerMath = triggerMath; this->parameter = parameter; };

    // checks, if this event is "NULL"
    bool isNull( void ) { return sensor.isNull( ); };

    // checks, if myself and otherEvent are the same
    bool cmp( SwOSNVSEvent *otherEvent );

    // set this event to "NULL"
    void clear( void ) { sensor.serialNumber = 0; };

} __attribute__((packed));

// wifi types
typedef enum { wifiOFF, wifiAP, wifiClient } FtSwarmWifi_t;

class SwOSNVS {
  public:
    void initialSetup();   // ask user for HW details
	  int32_t                version = NVSVERSION;
    bool                   factoryReset = false; // flag to reset the IOs settings via reboot
	  FtSwarmVersion_t       CPU;
	  FtSwarmSerialNumber_t  serialNumber;
    uint8_t                channel;
	  char                   wifiSSID[64], wifiPwd[128];
    char                   swarmName[MAXIDENTIFIER];
    uint16_t               swarmSecret, swarmPIN;
    FtSwarmWifi_t          wifiMode;
    SwOSJoyCalibration_t   calibration[4];
    uint8_t                pixels;
    uint8_t                activeEventConfig;
    SwOSNVSEvent           events[MAXEVENTCONFIGS][MAXNVSEVENTS];
    char                   oledLabel[MAXEVENTCONFIGS][12][4];
    bool                   webUI;
    bool                   IAmKelda;
    FtSwarmCommunication_t swarmCommunication;
    FtSwarmSerialNumber_t  swarmMember[MAXCTRL];
    uint8_t                swarmSpeed = 4;
    FtSwarmExtMode_t       extensionPort;
    bool                   gyro;
    bool                   spiGyro;
    uint8_t                I2CAddr;
    uint8_t                interruptLine; // 0 off, 1 M1, 2 M2
    int16_t                interruptOnOff[2];
    uint8_t                I2CRegisters;

    // constructor
	  SwOSNVS();

    // read data from nvs & run an _initialSetup if needed
	  void begin();

    // load data from nvs, return false if nvs contains invalid data
    bool load();

    // save config to flash
	  void save( bool writeAll = false );

    // save config & restart
    void saveAndRestart();

    // save events
    void saveEvents();

    // load events
    void loadEvents();

    // delete all events
    void deleteAllEvents( uint8_t configuration );

    // add event
    bool addEvent( uint8_t configuration, SwOSNVSEvent *event );

    // add event
    bool addEvent( SwOSNVSEvent *event ) { return addEvent( activeEventConfig, event ); };

    // check on dublicates
    bool exists( uint8_t configuration, SwOSNVSEvent *event );

    // check on dublicates
    bool exists( SwOSNVSEvent *event ) { return exists ( activeEventConfig, event ); };

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

    // reset to factory settings
    void factorySettings( void );

    // print settings for debugging only  
    void printNVS();

    // runs an nvs version upgrade, true if an upgrade took place
    bool upgrade( void );

};

extern SwOSNVS nvs;
