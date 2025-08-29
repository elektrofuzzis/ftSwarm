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

#include "serialize.h"
#include "SwOSCom.h"
#include "SwOSHW.h"
#include "SwOSNVS.h"

class SwOSSwarm {
protected:
  uint16_t readDelay = 25;
  bool     verbose = false;
  bool     initialized = false;

  uint8_t  getIndex( FtSwarmSerialNumber_t serialNumber );               // return index of controller with this s/n or are free slot if not found
	bool     splitID( char *id, uint8_t *index, char *io, size_t sizeIO);  // split identifier
  SwOSIO  *waitFor( char *alias );
  void     startWifi( void );

  // replace controller in swarm list
  void replaceCtrl( SwOSCom *com, uint8_t source, uint8_t affected );

  // process CMD_JOINMYSWARM
  void cmdJoinMySwarm( SwOSCom *com, uint8_t source, uint8_t affected );

  // Member to Kelda: I don't want to join your Swarm
  void cmdJoinNAck( SwOSCom *com, uint8_t source, uint8_t affected );
  
  // Member to Kelda: I want to join your Swarm
  void cmdJoinAck( SwOSCom *com, uint8_t source, uint8_t affected );

  // Kelda to Member: get out of my Swarm
  void cmdRevokeFromSwarm( SwOSCom *com, uint8_t source, uint8_t affected );

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

  // Get an IO in the swarm using controllers serial number, port and ioType. Returns the IO's pointer or NULL if it doesn't exist.
  virtual SwOSIO* getIO( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType );

  // Get an IO in the swarm using io's uid. Returns the IO's pointer or NULL if it doesn't exist.
  virtual SwOSIO* getIO( SwOSIOUID_t uio ) { return getIO( uio.serialNumber, uio.port, uio.ioType ); };

  // Get an IO in the swarm using controllers serial number, port and ioType. Returns the IO's alias name
  virtual void getAlias( FtSwarmSerialNumber_t serialNumber, FtSwarmPort_t port, SwOSIOType_t ioType, char *alias );

  // Get an IO in the swarm using io's uid. Returns the IO's alias name
  virtual void getAlias( SwOSIOUID_t uio, char *alias ) { getAlias( uio.serialNumber, uio.port, uio.ioType, alias ); };

  // Get an IO in the swarm using his name/alias and ioType. Returns the IO#s pointer or NULL if it doesn't exist.
  virtual SwOSIO* getIO( const char *name, SwOSIOType_t ioType = SWOSIO_UNDEF );

  // search for offline or unconnected controllers and try to get them
  virtual void connect( void );

  // get swarm's read delay
  uint16_t getReadDelay( void ) { return readDelay; };

  // **** API ****
  size_t approxSerialize( SerialFormat_t format );
	void serialize( Serialize *serialize );                                                        // transfer my swarm to a JSON structure
  void serializeEvents( Serialize *serialize );

  void setState( SwOSState_t state ); 

  // receiving data from other controllers
  void OnDataRecv( SwOSCom *buffer );

  // As a Kelda send CMD_JOINMYSWARM to a potential member
  void joinMySwarm( MacAddr destinationMac, FtSwarmSerialNumber_t destinationSN ); 
  
  // **** some useful stuff ****

  // # of members in swarm
  uint8_t members( void ); 
  
  // create a new swam based on nvs settings
  void newSwarm( void );  
  
  // Test, if SN is part my my Swarm 
  bool isMember( FtSwarmSerialNumber_t serialNumber );  
  
  // Test, if SN is online
  bool isOnline( FtSwarmSerialNumber_t serialNumber );  
  
  // add Controller SN to the swarm
  bool addController( FtSwarmSerialNumber_t serialNumber );    

  // delete Controller SN
  bool deleteController( FtSwarmSerialNumber_t serialNumber ); 

  // delete an event
  bool deleteEvent( SwOSNVSEvent_t *event );

  // add an event
  bool addEvent( SwOSNVSEvent_t *event );

  // add all events
  void addEvents( uint8_t config, FtSwarmSerialNumber_t sn = 0 );

  // delete all events
  void deleteEvents( void );

  void save( uint8_t scope );

};

extern SwOSSwarm myOSSwarm;
