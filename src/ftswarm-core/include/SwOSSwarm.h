/*
 * SwOSSwarm.h
 *
 * internal represenation of my swarm. Use FtSwarm-Classes in FtSwarm.h to access your swarm!
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include "SwOS.h"

#include <stdint.h>
#include <esp_now.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "jsonize.h"
#include "SwOSCom.h"
#include "SwOSHW.h"
#include "SwOSNVS.h"

class SwOSSwarm {
protected:
  uint16_t lastToken = rand();
  uint16_t readDelay = 25;
  bool     verbose = false;
  bool     initialized = false;

  uint8_t getIndex( FtSwarmSerialNumber_t serialNumber );               // return index of controller with this s/n or are free slot if not found
	bool    splitID( char *id, uint8_t *index, char *io, size_t sizeIO);  // split identifier
  uint16_t nextToken( bool rotateToken);
  SwOSIO *waitFor( char *alias, FtSwarmIOType_t ioType );
  bool    startEvents( void );
  void    startWifi( void );

  // internal function to handle CMD_SWARMJOIN
  void cmdJoin( SwOSCom *com, uint8_t source, uint8_t affected );

  // internal function to handle CMD_ACK
  void cmdAck( SwOSCom *com, uint8_t source, uint8_t affected );

  // internale function to handle CMD_SWARMLEAVE
  void cmdLeave( SwOSCom *com, uint8_t source, uint8_t affected );

  // send an ack package to destinationSN to inform about command cmd with error error 
  void sendAck( FtSwarmSerialNumber_t destinationSN, SwOSCommand_t cmd, SwOSError_t error, uint16_t secret );

  
public:
	int8_t   maxCtrl = -1;
  SwOSCtrl *Kelda = NULL;
	SwOSCtrl *Ctrl[MAXCTRL];

  // constructor
  SwOSSwarm( ) { for ( uint8_t i=0; i<MAXCTRL; i++ ) { Ctrl[i] = NULL; } }; 

  // Start the swarm. If verbose is set, do some inormational console output
  FtSwarmSerialNumber_t begin( bool verbose );

  // Stop all motors within the swarm
  void halt( void );

  // unsibscribe all subscribed IO's  
  void unsubscribe( void ); 

  // Get a controller in the swarm using his name/alias. Returns the controller's pointer or NULL if it doesn't exist.
	void *getController(char *name);

  // Get a controller in the swarm using his serial number. Returns the controller's pointer or NULL if it doesn't exist.
  void *getController( FtSwarmSerialNumber_t SN );

  // Get an IO in the swarm using controllers serial number, port and ioType. Returns the IO#s pointer or NULL if it doesn't exist.
  virtual SwOSIO* getIO( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, FtSwarmIOType_t ioType);

  // Get an IO in the swarm using his name/alias and ioType. Returns the IO#s pointer or NULL if it doesn't exist.
  virtual SwOSIO* getIO( const char *name, FtSwarmIOType_t ioType );

  // search for offline or unconnected controllers and try to get them
  virtual void connect( void );

  // get swarm's read delay
  uint16_t getReadDelay( void ) { return readDelay; };

  // **** REST API ****
	void jsonize( JSONize *json);                                                               // transfer my swarm to a JSON structure
  void getToken( JSONize *json);                                                              // get a new token
  uint16_t apiIsAuthorized( uint16_t token, bool rotateToken );                               // check, if it's a correct token
  bool apiPeekIsAuthorized( uint16_t token );
  uint16_t apiActorCmd( uint16_t token, char *id, int cmd, bool rotateToken );                // send an actor's command (from api)
  uint16_t apiActorSpeed( uint16_t token, char *id, int speed, bool rotateToken );            // send an actor's speed (from api)
	uint16_t apiLEDBrightness( uint16_t token, char *id, int brightness, bool rotateToken );    // send a LED command (from api)
	uint16_t apiLEDColor( uint16_t token, char *id, int color, bool rotateToken);               // send a LED command (from api)
  uint16_t apiServoOffset( uint16_t token, char *id, int offset, bool rotateToken );          // send a Servo command (from api)
  uint16_t apiServoPosition( uint16_t token, char *id, int position, bool rotateToken);       // send a Servo command (from api)
  uint16_t apiCAMStreaming( uint16_t token, char *id, int onOff, bool rotateToken );             // set CAM streaming on/off
  uint16_t apiCAMFramesize( uint16_t token, char *id, int framesize, bool rotateToken );         // set CAM framzesize / resolution
  uint16_t apiCAMQuality( uint16_t token, char *id, int quality, bool rotateToken );             // set CAM quality
  uint16_t apiCAMBrightness( uint16_t token, char *id, int brightness, bool rotateToken );       // set CAM brightness
  uint16_t apiCAMContrast( uint16_t token, char *id, int contrast, bool rotateToken );           // set CAM contrast
  uint16_t apiCAMSaturation( uint16_t token, char *id, int saturation, bool rotateToken );       // set CAM saturation
  uint16_t apiCAMSpecialEffect( uint16_t token, char *id, int specialEffect, bool rotateTokent ); // set CAM special effect
  uint16_t apiCAMWbMode( uint16_t token, char *id, int wbMode, bool rotateToken );               // set CAM wbMode
  uint16_t apiCAMHMirror( uint16_t token, char *id, int hMirror, bool rotateToken );             // set CAM H-Mirror
  uint16_t apiCAMVFlip( uint16_t token, char *id, int vFlip, bool rotateToken );                 // set CAM V-Flip 

  void setState( SwOSState_t state ); // visualizes controller's state

  // receiving data from other controllers
  void OnDataRecv( SwOSCom *buffer );

  // void registerMe( void );                                                        // register myself in a swarm
  
  // Introduce myself to a specific controller. 
  void registerMe( MacAddr destinationMac, FtSwarmSerialNumber_t destinationSN );  
  
  // The local controller leaves the swarm.
  SwOSError_t leaveSwarm( void );

  // Kelda rejects/deletes a controller
  SwOSError_t rejectController( FtSwarmSerialNumber_t serialNumber );

  // create a new swarm using defaults. Call leaveSwarm before using it.
  SwOSError_t createSwarm( void );

  // create a new swarm. Call leaveSwarm before using it.
  SwOSError_t createSwarm( char *name, uint16_t pin );

  // invite a controller to join the swarm
  SwOSError_t inviteToSwarm( FtSwarmSerialNumber_t serialNumber );

  // I want to join an existing swarm
  SwOSError_t joinSwarm( char *name, uint16_t pin );

  // **** some useful stuff ****
  uint8_t members( void ); // # of members in swarm

};

extern SwOSSwarm myOSSwarm;
