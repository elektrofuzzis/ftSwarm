#pragma once

#include <stdint.h>
#include <esp_now.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "jsonize.h"
#include "SwOSCom.h"
#include "SwOSHW.h"
#include "SwOSNVS.h"

#define MAXCTRL 10

class SwOSSwarm {
protected:
  SemaphoreHandle_t _xAccessLock = xSemaphoreCreateMutex();
  uint16_t _readDelay = 25;

  uint8_t _getIndex( FtSwarmSerialNumber_t serialNumber );               // return index of controler with this s/n or are free slot if not found
	bool    _splitId( char *id, uint8_t *index, char *io, size_t sizeIO);  // split identifier
  
public:
  SwOSNVS  nvs;
	int8_t   maxCtrl = -1;
	SwOSCtrl *Ctrl[MAXCTRL];
  bool     allowPairing = false;

  SwOSSwarm( ) { for ( uint8_t i=0; i<MAXCTRL; i++ ) { Ctrl[i] = NULL; } }; // constructor
	void begin( bool IAmAKelda, bool verbose );                               // start my swarm

  // set/get time etween two reads
  void setReadDelay( uint16_t readDelay ) { _readDelay = readDelay; };
  uint16_t getReadDelay() { return _readDelay; };

  // locking 
  void lock();   // request single access
  void unlock(); // free access

	virtual void *getControler(char *name);

  virtual SwOSIO* getIO( FtSwarmSerialNumber_t serialNumber, FtSwarmIOType_t ioType, FtSwarmPort_t port);

  // **** REST API ****
	virtual void jsonize( JSONize *json);                         // transfer my swarm to a JSON structure
  virtual bool apiActorCmd( char *id, int cmd );                // send an actor's command (from api)
  virtual bool apiActorPower( char *id, int power );            // send an actor's power (from api)
	virtual bool apiLED( char *id, int brightness, int color );   // send a LED command (from api)
	virtual bool apiServo( char *id, int offset, int position );  // send a Servo command (from api)

  virtual void setState( int state );
    // visualizes controler's state

  // **** inter swarm communication ****
  void OnDataRecv( SwOSCom *buffer ); // receiving data from other controllers
  void send(SwOSCom *com);            // send a datagram to all Keldas
  void registerMe( void );                                                // register myself in a swarm
  void leaveSwarm( void );                                                // send a leave swarm msg to all others and delete them
  void joinSwarm( bool createNewSwarm, char * newName, uint16_t newPIN ); // join a new swarm

  // **** some useful stuff ****
  void    reload( void ); // restore swarm from nvs
  uint8_t members( void ); // # of members in swarm

};

extern SwOSSwarm myOSSwarm;
