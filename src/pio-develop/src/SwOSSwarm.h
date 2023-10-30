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
  SemaphoreHandle_t _xAccessLock = xSemaphoreCreateMutex();
  uint16_t lastToken = rand();
  uint16_t readDelay = 25;
  bool     verbose = false;

  uint8_t getIndex( FtSwarmSerialNumber_t serialNumber );               // return index of controler with this s/n or are free slot if not found
	bool    splitID( char *id, uint8_t *index, char *io, size_t sizeIO);  // split identifier
  uint16_t nextToken( bool rotateToken);
  SwOSIO *waitFor( char *alias );
  bool    startEvents( void );
  void    startWifi( void );
  
public:
	int8_t   maxCtrl = -1;
  SwOSCtrl *Kelda = NULL;
	SwOSCtrl *Ctrl[MAXCTRL];
  bool     allowPairing = false;

  SwOSSwarm( ) { for ( uint8_t i=0; i<MAXCTRL; i++ ) { Ctrl[i] = NULL; } }; // constructor
	FtSwarmSerialNumber_t begin( bool verbose );                              // start my swarm

  void halt( void );        // stop all motors
  void unsubscribe( void ); // unsubscribe all io's

  // set/get time etween two reads
  void     setReadDelay( uint16_t readDelay ) { this->readDelay = readDelay; };
  uint16_t getReadDelay() { return readDelay; };

  // locking 
  void lock();   // request single access
  void unlock(); // free access

	void *getControler(char *name);

  // search an IO in my swarm via <serialNumber, ioType, port> or with my IO's name
  virtual SwOSIO* getIO( FtSwarmSerialNumber_t serialNumber, FtSwarmIOType_t ioType, FtSwarmPort_t port);
  virtual SwOSIO* getIO( const char *name );

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

  void setState( SwOSState_t state ); // visualizes controler's state

  // **** inter swarm communication ****
  void OnDataRecv( SwOSCom *buffer );                                     // receiving data from other controllers
  void registerMe( void );                                                // register myself in a swarm
  void leaveSwarm( void );                                                // send a leave swarm msg to all others and delete them
  void joinSwarm( bool createNewSwarm, char * newName, uint16_t newPIN ); // join a new swarm

  // **** some useful stuff ****
  void    reload( void ); // restore swarm from nvs
  uint8_t members( void ); // # of members in swarm

};

extern SwOSSwarm myOSSwarm;
