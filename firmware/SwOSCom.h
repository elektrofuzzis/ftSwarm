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
#define DEFAULTSECRET 0x2506
#define VERSIONDATA   1

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
  CMD_MAX
} SwOSCommand_t;

// broadcast address
const uint8_t broadcast[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

extern QueueHandle_t sendNotification;
extern QueueHandle_t recvNotification;

struct Input_t { FtSwarmSensor_t inputType; uint32_t rawValue; };
struct Actor_t { FtSwarmActor_t actorType; FtSwarmMotion_t motionType; int16_t power; };
struct LED_t   { uint8_t brightness; uint32_t color; };
struct Servo_t { int16_t offset; int16_t position; };
struct Joystick_t { int16_t LR; int16_t FB; };

struct SwOSDatagram_t {
  uint16_t              secret;
  uint8_t               version;
  FtSwarmSerialNumber_t serialNumber;
  FtSwarmControler_t    ctrlType;
  SwOSCommand_t         cmd;
  union {
    struct { FtSwarmVersion_t versionCPU; FtSwarmVersion_t versionHAT; bool IAmAKelda; uint16_t pin; } registerCmd;
    struct { uint16_t pin; uint16_t swarmSecret; char swarmName[MAXIDENTIFIER]; } joinCmd;
    struct { Input_t input[4];
             Actor_t actor[2];
             LED_t led[2];
             Servo_t servo;
           } stateCmdJST;
    struct { Input_t input[4];
             Actor_t actor[2];
             Joystick_t joystick[2];
             uint8_t hc165;
           } stateCmdControl;
    struct { uint8_t index; FtSwarmSensor_t sensorType; bool normallyOpen; } sensorCmd;
    struct { uint8_t index; int16_t offset; int16_t position; } servoCmd;
    struct { uint8_t index; FtSwarmMotion_t motionType; uint16_t power; } actorPowerCmd;
    struct { uint8_t index; FtSwarmActor_t actorType; } actorTypeCmd;
    struct { uint8_t index; uint8_t brightness; uint32_t color; } ledCmd;
  };
} ;

class SwOSCom {
protected:
  bool    _isValid = true;

public:
  uint8_t        mac[ESP_NOW_ETH_ALEN];
  SwOSDatagram_t data;

  SwOSCom( const uint8_t *mac_addr, const uint8_t *buffer, int length);
  SwOSCom( const uint8_t *mac_addr, FtSwarmSerialNumber_t serialNumber, FtSwarmControler_t ctrlType, SwOSCommand_t cmd );

  void setMAC( const uint8_t *mac_addr );
  
  virtual esp_err_t send( void );

  bool isValid() { return _isValid; };  // true, if a ftSwarm sent this data
  void print(); // debugging

};

extern bool SwOSStartCommunication( uint16_t swarmSecret, uint16_t swarmPIN ) ;

extern void SwOSSetSecret( uint16_t swarmSecret, uint16_t swarmPIN ) ;
