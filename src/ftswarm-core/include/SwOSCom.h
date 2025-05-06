/*
 * SwOSCom.h
 *
 * Communication between your controllers
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 *
 */
 
#pragma once

#include <stdint.h>

#include <esp_now.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "SwOS.h"

#define ESPNOW_MAXDELAY     128
#define DEFAULTSECRET       0x2506
#define VERSIONDATA         9
#define MAXIOCONFIG         5
#define MAXUSEREVENTPAYLOAD 128
#define MAXCONFIGPAYLOAD    200

typedef enum {
  
  CMD_JOINMYSWARM,            // Kelda to member: Please join my Swarm 
  CMD_JOINACK,                // Member to Kelda: yes, I want to join your Swarm
  CMD_JOINNACK,               // Member to Kelda: no, I don't want to join your swarm

  CMD_SAVEALIAS2NVS,          // Kelda to Member: save alias setting to NVS

  CMD_GOTYOU,                 // anybody's reply on ANYBODYOUTTHERE
  CMD_STATE,                  // send my input's readings
  CMD_IOCONFIG,               // send my io config to kelda
  CMD_CHANGEIOTYPE,           // change a port's IO Type
  
  CMD_IDENTIFY,               // show myself
  
  CMD_SETLED,                 // set LED color & brightness
  CMD_SETACTORSPEED,          // set actors motionType & speed
  CMD_SETSERVO,               // set servo position
  CMD_SETSENSORTYPE,          // set an input's sensor type
  CMD_SETACTORTYPE,           // set an actors's actor type
  CMD_I2CREGISTER,            // set an I2C register
  CMD_SETSTEPPERDISTANCE,     // set distance to go
  CMD_STEPPERSTARTSTOP,       // start/stop
  CMD_SETSTEPPERPOSITION,     // set stepper position
  CMD_STEPPERHOMING,          // start stepper homing procedure
  CMD_SETSTEPPERHOMINGOFFSET, // set homing offset
  CMD_SETMICROSTEPMODE,       // set Microstepmode
  CMD_USEREVENT,              // send data from user exit back to Kelda
  CMD_RESETCOUNTER,           // Reset counter
  CMD_STARTFREQUENCYMETER,    // start frequency meter
  CMD_MAX
} SwOSCommand_t;

// broadcastSN
const FtSwarmSerialNumber_t broadcastSN = 0xFFFF;

extern QueueHandle_t sendNotificationWifi;
extern QueueHandle_t sendNotificationRS485;
extern QueueHandle_t recvNotification;

// struct Input_t { FtSwarmSensor_t sensorType; int32_t rawValue; } __attribute__((packed));
// struct Actor_t { FtSwarmActor_t actorType; FtSwarmMotion_t motionType; int16_t speed; uint32_t rampUpT; uint32_t rampUpY; } __attribute__((packed));
struct LED_t   { uint8_t brightness; uint32_t color; } __attribute__((packed));
struct Servo_t { int16_t offset; int16_t position; } __attribute__((packed));
struct Joystick_t { int16_t LR; int16_t FB; } __attribute__((packed));
struct IOConfig_t { FtSwarmIOType_t ioType; FtSwarmSensor_t sensorType; uint8_t port; char name[10]; char alias[MAXIDENTIFIER]; } __attribute__((packed));

struct SwOSCtrlConfig_t { 
  FtSwarmController_t   ctrlType; 
  FtSwarmVersion_t      CPU; 
  bool                  IAmKelda;
  FtSwarmExtMode_t      extensionPort;
  uint8_t               inputs;
  uint8_t               actors;
  uint8_t               leds;
  uint8_t               servos;
  bool                  gyro;
  int16_t               zero[2][2];
} __attribute__((packed));

struct registerCmd_t { 
  char                swarmName[MAXIDENTIFIER];
  uint16_t            swarmPIN;
  SwOSCtrlConfig_t    ctrlConfig;
} __attribute__((packed));

struct joinCmd_t { 
  uint16_t pin; 
  uint16_t swarmSecret; 
  char swarmName[MAXIDENTIFIER]; 
  bool IAmKelda; 
} __attribute__((packed));

struct ackCmd_t { 
  SwOSCommand_t cmd; 
  SwOSError_t error; 
  uint16_t secret;
} __attribute__((packed));

struct stateCmd_t {
  uint32_t inputValue[MAXINPUTS]; 
  int16_t LR[2]; int16_t FB[2]; uint8_t hc165; 
  uint8_t i2cValue[MAXI2CREGISTERS]; 
  union{
    struct{ float qw, qx, qy, qz; int16_t ax, ay, az; } gyroMPU;
  };
} __attribute__((packed));

struct stepperStateCmd_t{ 
  bool isHoming[4]; 
  bool isRunning[4]; 
  uint32_t inputValue[5]; 
  long distance[4]; 
  long position[4];
} __attribute__((packed));

struct sensorCmd_t { 
  uint8_t index; 
  FtSwarmSensor_t sensorType; 
  bool normallyOpen;
} __attribute__((packed));

struct servoCmd_t{ 
  uint8_t index; 
  int16_t offset; 
  int16_t position;
} __attribute__((packed));

struct actorSpeedCmd_t{ 
  uint8_t index; 
  FtSwarmMotion_t motionType; 
  int16_t speed;  
  uint32_t rampUpT; 
  uint32_t rampUpY;
} __attribute__((packed));

struct actorStepperCmd_t{ 
  uint8_t index; 
  long paraml; 
  bool paramb;
} __attribute__((packed));

struct actorTypeCmd_t{ 
  uint8_t index; 
  FtSwarmActor_t actorType; 
  bool highResolution;
} __attribute__((packed));

struct ledCmd_t { 
  uint8_t index; 
  uint8_t brightness; 
  uint32_t color;
} __attribute__((packed));

struct ioConfigCmd_t { 
  uint8_t payload[MAXCONFIGPAYLOAD];
} __attribute__((packed));

struct I2CRegisterCmd_t { 
  uint8_t reg; 
  uint8_t value;
} __attribute__((packed));

struct ctrlCmd_t{ 
  uint8_t microstepMode; 
} __attribute__((packed));

struct userEventCmd_t { 
  bool trigger; 
  uint8_t size; 
  uint8_t payload[MAXUSEREVENTPAYLOAD];
} __attribute__((packed));

struct changeIOTypeCmd_t{ 
  uint8_t index; 
  FtSwarmIOType_t oldIOType; 
  FtSwarmIOType_t newIOType;
} __attribute__((packed));

struct counterCmd_t{ 
  uint8_t index;
} __attribute__((packed));

struct SwOSDatagram_t {
  uint8_t               size;
  uint8_t               version;
  FtSwarmSerialNumber_t sourceSN;      // who is sending this information?
  FtSwarmSerialNumber_t affectedSN;    // who will receive this information?
  SwOSCommand_t         cmd;
  union {
    registerCmd_t registerCmd;
    joinCmd_t joinCmd;
    ackCmd_t ackCmd;
    stateCmd_t stateCmd;
    stepperStateCmd_t stepperStateCmd;
    sensorCmd_t sensorCmd;
    servoCmd_t servoCmd;
    actorSpeedCmd_t actorSpeedCmd;
    actorStepperCmd_t actorStepperCmd;
    actorTypeCmd_t actorTypeCmd;
    ledCmd_t ledCmd;
    ioConfigCmd_t ioConfigCmd;
    I2CRegisterCmd_t I2CRegisterCmd;
    ctrlCmd_t ctrlCmd;
    userEventCmd_t userEventCmd;
    changeIOTypeCmd_t changeIOTypeCmd;
    counterCmd_t counterCmd;
  };
} __attribute__((packed));

const uint8_t broadcast[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
const uint8_t noMac[ESP_NOW_ETH_ALEN]     = {0, 0, 0, 0, 0, 0 };    

class MacAddr {
  public:
    
    uint8_t addr[ESP_NOW_ETH_ALEN];
    
    MacAddr();
    MacAddr( uint8_t addr[ESP_NOW_ETH_ALEN] );
    MacAddr( const uint8_t *addr );
    
    void set( MacAddr macAddr );
    bool isEqual( uint8_t addr[ESP_NOW_ETH_ALEN]);
    bool isBroadcast(void);
    bool isNull(void);
    void print(void);
};

class SwOSCom {
protected:
  bool    _isValid    = false;
  uint8_t bufferIndex = 0;

public:
  MacAddr        macAddr;
  SwOSDatagram_t data;

  SwOSCom();
  SwOSCom( MacAddr macAddr, const uint8_t *buffer, int length); 
  SwOSCom( MacAddr macAddr, FtSwarmSerialNumber_t affectedSN, SwOSCommand_t cmd );
  size_t size( void );

  // send my alias names buffered
  void sendHostname( char *name, char *alias ) { sendIO( FTSWARM_MAXIOTYPE, FTSWARM_MAXSENSOR, SWOS_NOPORT, name, alias); };
  void sendIO( FtSwarmIOType_t ioType, char *name, char *alias ) { sendIO( ioType, FTSWARM_MAXSENSOR, SWOS_NOPORT, name, alias); };
  void sendIO( FtSwarmIOType_t ioType, uint8_t port, char *name, char *alias ) { sendIO( ioType, FTSWARM_MAXSENSOR, port, name, alias); };
  void sendIO( FtSwarmIOType_t ioType, FtSwarmSensor_t sensorType, uint8_t port, char *name, char *alias );
  void flushBuffer( void );
  bool getNextIO( FtSwarmIOType_t *ioType, FtSwarmSensor_t *sensorType, uint8_t *port, char **name, char **alias );
  
  void send( void );

  bool isValid() { return _isValid; };  // true, if a ftSwarm sent this data
  void print(); // debugging

};

class SwOSNetwork {

  private:
    bool _StartWifi( void );
    bool _StartRS485( void );

  public:
    QueueHandle_t tx_queue = NULL;
    QueueHandle_t RS485_rx_queue = NULL;
    QueueHandle_t sendNotificationWifi = NULL;
    QueueHandle_t recvNotification = NULL;
    QueueHandle_t userEvent = NULL;

    uint16_t      secret = DEFAULTSECRET;
    uint16_t      pin    = 0;
    uint8_t       delayTime;
    FtSwarmCommunication_t communication;

    bool begin( uint16_t swarmSecret, uint16_t swarmPIN, FtSwarmCommunication_t swarmCommunication );
    void setSecret( uint16_t swarmSecret, uint16_t swarmPIN );
    bool hasJoinedASwarm( void );
    void AddPeer( MacAddr macAddr );
};

extern SwOSNetwork myOSNetwork;
 