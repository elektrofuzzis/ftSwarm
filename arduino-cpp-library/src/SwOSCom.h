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

#include "ftSwarm.h"

#define ESPNOW_MAXDELAY 512
#define DEFAULTSECRET   0x2506
#define VERSIONDATA     4
#define MAXALIAS        5

typedef enum {
  CMD_SWARMJOIN,         // I want to join a swarm
  CMD_SWARMJOINACK,      // Acknowledge on join swarm
  CMD_SWARMLEAVE,        // leave swarm
  CMD_ANYBODYOUTTHERE,   // Broadcast to get known by everybody 
  CMD_GOTYOU,            // anybody's reply on ANYBODYOUTTHERE
  CMD_SETLED,            // set LED color & brightness
  CMD_SETACTORPOWER,     // set actors motionType & power
  CMD_SETSERVO,          // set servo position
  CMD_STATE,             // send my input's readings
  CMD_SETSENSORTYPE,     // set an input's sensor type
  CMD_SETACTORTYPE,      // set an actors's actor type
  CMD_ALIAS,             // send some alias names
  CMD_MAX
} SwOSCommand_t;

// broadcast address
const uint8_t broadcast[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const FtSwarmSerialNumber_t broadcastSN = 0xFFFF;

extern QueueHandle_t sendNotification;
extern QueueHandle_t recvNotification;

struct Input_t { FtSwarmSensor_t sensorType; uint32_t rawValue; };
struct Actor_t { FtSwarmActor_t actorType; FtSwarmMotion_t motionType; int16_t power; };
struct LED_t   { uint8_t brightness; uint32_t color; };
struct Servo_t { int16_t offset; int16_t position; };
struct Joystick_t { int16_t LR; int16_t FB; };
struct Alias_t { char name[10]; char alias[MAXIDENTIFIER]; };

struct registerJST_t {
  uint8_t RGBLeds;
  LED_t   led[MAXLED];
  Servo_t servo;
};

struct registerControl_t {
  Joystick_t joystick[2];
  uint8_t    hc165;
};

struct registerCmd_t { 
  FtSwarmControler_t ctrlType; 
  FtSwarmVersion_t versionCPU; 
  FtSwarmVersion_t versionHAT; 
  bool IAmAKelda; 
  uint16_t pin; 
  Input_t input[4];
  Actor_t actor[2];
  union {
    registerJST_t     jst;
    registerControl_t control;
  };
};

struct SwOSDatagram_t {
  uint16_t              secret;
  uint8_t               version;
  FtSwarmSerialNumber_t sourceSN, destinationSN;
  SwOSCommand_t         cmd;
  union {
    // struct { FtSwarmControler_t ctrlType; FtSwarmVersion_t versionCPU; FtSwarmVersion_t versionHAT; bool IAmAKelda; uint16_t pin; } registerCmd;
    registerCmd_t registerCmd;
    struct { uint16_t pin; uint16_t swarmSecret; char swarmName[MAXIDENTIFIER]; } joinCmd;
    struct { uint32_t inputValue[6]; int16_t LR[2]; int16_t FB[2]; uint8_t hc165;} stateCmd;
    struct { uint8_t index; FtSwarmSensor_t sensorType; bool normallyOpen; } sensorCmd;
    struct { uint8_t index; int16_t offset; int16_t position; } servoCmd;
    struct { uint8_t index; FtSwarmMotion_t motionType; int16_t power; } actorPowerCmd;
    struct { uint8_t index; FtSwarmActor_t actorType; } actorTypeCmd;
    struct { uint8_t index; uint8_t brightness; uint32_t color; } ledCmd;
    struct { Alias_t alias[MAXALIAS]; } aliasCmd;
  };
};

class SwOSCom {
protected:
  bool    _isValid;
  uint8_t bufferIndex;
  size_t  _size( void );
  esp_err_t _sendWiFi( void );
  esp_err_t _sendRS485( void );

public:
  uint8_t        mac[ESP_NOW_ETH_ALEN];
  SwOSDatagram_t data;

  SwOSCom( const uint8_t *mac_addr, const uint8_t *buffer, int length);
  SwOSCom( const uint8_t *mac_addr, FtSwarmSerialNumber_t destinationSN, SwOSCommand_t cmd );

  void setMAC( const uint8_t *mac_addr, FtSwarmSerialNumber_t destinationSN );

  // send my alias names buffered
  void sendBuffered( char *name, char *alias );
  void flushBuffer( );
  
  esp_err_t send( void );

  bool isValid() { return _isValid; };  // true, if a ftSwarm sent this data
  void print(); // debugging

};

extern bool SwOSStartCommunication( uint16_t swarmSecret, uint16_t swarmPIN ) ;

extern void SwOSSetSecret( uint16_t swarmSecret, uint16_t swarmPIN ) ;
