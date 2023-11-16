/*
 * SwOSCom.h
 *
 * Communication between your controlers
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
#define VERSIONDATA         6
#define MAXALIAS            5
#define MAXUSEREVENTPAYLOAD 128

typedef enum {
  CMD_SWARMJOIN,              // I want to join a swarm
  CMD_SWARMJOINACK,           // Acknowledge on join swarm
  CMD_SWARMLEAVE,             // leave swarm
  CMD_ANYBODYOUTTHERE,        // Broadcast to get known by everybody 
  CMD_GOTYOU,                 // anybody's reply on ANYBODYOUTTHERE
  CMD_SETLED,                 // set LED color & brightness
  CMD_SETACTORSPEED,          // set actors motionType & speed
  CMD_SETSERVO,               // set servo position
  CMD_STATE,                  // send my input's readings
  CMD_SETSENSORTYPE,          // set an input's sensor type
  CMD_SETACTORTYPE,           // set an actors's actor type
  CMD_ALIAS,                  // send some alias names
  CMD_I2CREGISTER,            // set an I2C register
  CMD_SETSTEPPERDISTANCE,     // set distance to go
  CMD_STEPPERSTARTSTOP,       // start/stop
  CMD_SETSTEPPERPOSITION,     // set stepper position
  CMD_STEPPERHOMING,          // start stepper homing procedure
  CMD_SETSTEPPERHOMINGOFFSET, // set homing offset
  CMD_SETMICROSTEPMODE,       // set Microstepmode
  CMD_USEREVENT,              // send data from user exit back to Kelda
  CMD_IDENTIFY,               // show myself
  CMD_CHANGEIOTYPE,           // change a port's IO Type
  CMD_MAX
} SwOSCommand_t;

// broadcastSN
const FtSwarmSerialNumber_t broadcastSN = 0xFFFF;

extern QueueHandle_t sendNotificationWifi;
extern QueueHandle_t sendNotificationRS485;
extern QueueHandle_t recvNotification;

struct Input_t { FtSwarmSensor_t sensorType; uint32_t rawValue; } __attribute__((packed));
struct Actor_t { FtSwarmActor_t actorType; FtSwarmMotion_t motionType; int16_t speed; uint32_t rampUpT; uint32_t rampUpY; } __attribute__((packed));
struct LED_t   { uint8_t brightness; uint32_t color; } __attribute__((packed));
struct Servo_t { int16_t offset; int16_t position; } __attribute__((packed));
struct Joystick_t { int16_t LR; int16_t FB; } __attribute__((packed));
struct Alias_t { char name[10]; char alias[MAXIDENTIFIER]; } __attribute__((packed));

struct registerJST_t {
  uint8_t leds;
  LED_t   led[MAXLEDS];
  Servo_t servo[MAXSERVOS];
} __attribute__((packed));

struct registerControl_t {
  Joystick_t joystick[2];
  uint8_t    hc165;
} __attribute__((packed));

struct registerCmd_t { 
  FtSwarmControler_t ctrlType; 
  FtSwarmVersion_t versionCPU; 
  bool IAmAKelda; 
  FtSwarmExtMode_t extentionPort;
  uint8_t leds;
} __attribute__((packed));


struct SwOSDatagram_t {
  uint8_t               size;
  uint16_t              secret;
  uint8_t               version;
  FtSwarmSerialNumber_t sourceSN;      // who is sending this information?
  FtSwarmSerialNumber_t affectedSN;    // who will receive this information?
  SwOSCommand_t         cmd;
  union {
    registerCmd_t registerCmd;
    struct { uint16_t pin; uint16_t swarmSecret; char swarmName[MAXIDENTIFIER]; } joinCmd;
    struct { uint32_t inputValue[MAXINPUTS]; int16_t LR[2]; int16_t FB[2]; uint8_t hc165;} stateCmd;
    struct { bool isHoming[4]; bool isRunning[4]; uint32_t inputValue[5]; long distance[4]; long position[4];} stepperStateCmd;
    struct { uint8_t index; FtSwarmSensor_t sensorType; bool normallyOpen; } sensorCmd __attribute__((packed));
    struct { uint8_t index; int16_t offset; int16_t position; } servoCmd;
    struct { uint8_t index; FtSwarmMotion_t motionType; int16_t speed;  uint32_t rampUpT; uint32_t rampUpY; } actorSpeedCmd;
    struct { uint8_t index; long paraml; bool paramb; } actorStepperCmd;
    struct { uint8_t index; FtSwarmActor_t actorType; } actorTypeCmd;
    struct { uint8_t index; uint8_t brightness; uint32_t color; } ledCmd;
    struct { Alias_t alias[MAXALIAS]; } aliasCmd;
    struct { uint8_t reg; uint8_t value; } I2CRegisterCmd;
    struct { uint8_t microstepMode; } CtrlCmd;
    struct { bool trigger; uint8_t size; uint8_t payload[MAXUSEREVENTPAYLOAD]; } userEventCmd;
    struct { uint8_t index; FtSwarmIOType_t oldIOType; FtSwarmIOType_t newIOType; } changeIOTypeCmd;
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
  bool    _isValid;
  uint8_t bufferIndex;

public:
  MacAddr        macAddr;
  SwOSDatagram_t data;

  SwOSCom();
  SwOSCom( MacAddr macAddr, const uint8_t *buffer, int length); 
  SwOSCom( MacAddr macAddr, FtSwarmSerialNumber_t affectedSN, SwOSCommand_t cmd );
  size_t size( void );

  // send my alias names buffered
  void sendBuffered( char *name, char *alias );
  void flushBuffer( );
  
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
 