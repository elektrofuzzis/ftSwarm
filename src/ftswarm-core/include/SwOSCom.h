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
  CMD_REVOKEFROMSWARM,        // Kelda to member: Please leave my swarm

  CMD_SAVEALIAS2NVS,          // Kelda to Member: save alias setting to NVS

  CMD_GOTYOU,                 // anybody's reply on ANYBODYOUTTHERE
  CMD_STATE,                  // send my input's readings
  CMD_IOCONFIG,               // send my io config to kelda
  CMD_SETIOTYPE,              // change a port's IO Type
  CMD_SETPARAMETER,           // send a parameter to the IO, e.g. normallyOpen, highResolution
  CMD_HARTBEAT,               // Kelda to Member: I'm still out there
  
  CMD_IDENTIFY,               // show myself
  
  CMD_SETPIXEL,               // set LED color & brightness
  CMD_SETACTORSPEED,          // set actors motionType & speed
  CMD_SETSERVO,               // set servo position
  CMD_I2CREGISTER,            // set an I2C register
  CMD_SETSTEPPERDISTANCE,     // set distance to go
  CMD_STEPPERSTARTSTOP,       // start/stop
  CMD_SETSTEPPERPOSITION,     // set stepper position
  CMD_STEPPERHOMING,          // start stepper homing procedure
  CMD_SETSTEPPERHOMINGOFFSET, // set homing offset
  CMD_SETMICROSTEPMODE,       // set Microstepmode
  CMD_USEREVENT,              // send data from user exit back to Kelda
  CMD_RESETCOUNTER,           // Reset counter
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
struct IOConfig_t { SwOSIOType_t ioType; uint8_t port; char name[10]; char alias[MAXIDENTIFIER]; } __attribute__((packed));

struct SwOSCtrlConfig_t { 
  FtSwarmController_t   ctrlType; 
  FtSwarmVersion_t      CPU; 
  bool                  IAmKelda;
  FtSwarmExtMode_t      extensionPort;
  uint8_t               IOs;
  uint8_t               pixels;
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

#define MAXSTATECMDPAYLOAD 128

struct stateCmd_t {
  uint8_t items;
  uint8_t payload[MAXSTATECMDPAYLOAD];

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
  SwOSIOType_t ioType; 
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
  SwOSIOType_t actorType; 
  bool highResolution;
} __attribute__((packed));

struct pixelCmd_t { 
  uint8_t index; 
  uint8_t brightness; 
  uint32_t color;
} __attribute__((packed));

struct ioConfigCmd_t { 
  uint8_t payload[MAXCONFIGPAYLOAD];
} __attribute__((packed));

struct I2CRegisterCmd_t { 
  uint8_t index;
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

struct parameterCmd_t { 
  uint8_t index; 
  int32_t parameter;
} __attribute__((packed));

  struct setIOTypeCmd_t{ 
  uint8_t      index; 
  SwOSIOType_t newIOType;
  int32_t      payload;
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
    pixelCmd_t pixelCmd;
    ioConfigCmd_t ioConfigCmd;
    I2CRegisterCmd_t I2CRegisterCmd;
    ctrlCmd_t ctrlCmd;
    userEventCmd_t userEventCmd;
    setIOTypeCmd_t setIOTypeCmd;
    counterCmd_t counterCmd;
    parameterCmd_t parameterCmd;
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

public:
  uint8_t bufferIndex = 0;

public:
  MacAddr        macAddr;
  SwOSDatagram_t data;

  SwOSCom();
  SwOSCom( MacAddr macAddr, const uint8_t *buffer, int length); 
  SwOSCom( MacAddr macAddr, FtSwarmSerialNumber_t affectedSN, SwOSCommand_t cmd );
  size_t size( void );

  // send my alias names buffered
  void pushHostname( char *name, char *alias ) { pushIO( 254, SWOSIO_MAXIOTYPE, SWOS_NOPORT, name, alias); };
  void pushIO( uint8_t index, SwOSIOType_t ioType,  uint8_t port,  char *name,  char *alias );
  bool popIO( uint8_t *index, SwOSIOType_t *ioType, uint8_t *port, char **name, char **alias );
  void flushBuffer( void );
  
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
 