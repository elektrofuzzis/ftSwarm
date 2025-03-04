/*
 * SwOSHWBaseCtrl.h
 *
 * Basic classes for controller hardware impelmentation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include <nvs.h>

#include "SwOS.h"
#include "SwOSNVS.h"
#include "SwOSCom.h"

#include "SwOSHWBaseIO.h"

// only to feed that silly compiler
class SwOSActor;
class SwOSPixel;

const uint32_t LEDCOLOR0[MAXSTATE] = { CRGB::Blue, CRGB::Yellow, CRGB::Green, CRGB::Red, CRGB::Cyan, CRGB::Aquamarine };
const uint32_t LEDCOLOR1[MAXSTATE] = { CRGB::Blue, CRGB::Yellow, CRGB::Green, CRGB::Red, CRGB::Cyan, CRGB::Aquamarine };

const char     OLEDMSG[MAXSTATE][20] = { "booting", "connecting wifi", "online", "ERROR - check logs", "waiting on HW", "It's me!" };

/***************************************************
 *
 *   SwOSCtrl - Base class for controllers.
 *
 ***************************************************/

 struct SwOSAckState_t {
  SwOSCommand_t cmd;
  SwOSError_t   error;
  uint16_t      secret;
};

class SwOSCtrl : public SwOSObj {
protected:
  SemaphoreHandle_t _xAccessLock = xSemaphoreCreateMutex();
	FtSwarmVersion_t _CPU;
  bool             _local;
  
  unsigned long    _lastContact = 0;
  SwOSComState_t   _comState = COMSTATE_UNDEFINED;
  
  bool             _isSubscribed = false;
  char             *_subscribedCtrlName = NULL;

	const char *     version( FtSwarmVersion_t v);
  virtual void     _sendAlias( SwOSCom *alias );

public:
	FtSwarmSerialNumber_t serialNumber;
  MacAddr               macAddr;
  bool                  IAmKelda;
  
  // common hardware
	SwOSInput    *input[MAXINPUTS];
	SwOSActor    *actor[MAXACTORS]; 
	SwOSPixel    *led[MAXLEDS];
  uint8_t      inputs, actors, leds;
	
  // constructor
  SwOSCtrl( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, FtSwarmVersion_t CPU, bool IAmKelda, FtSwarmExtMode_t extentionPort );
  
  // destructor
  ~SwOSCtrl();

  // administrative stuff
  virtual void lock( void );
  virtual void unlock( void );
  virtual bool isInUse( void );
  virtual bool cmdAlias( const char *obj, const char *alias);            // set an alias for this board or IO device
	virtual bool cmdAlias( char *device, uint8_t port, const char *alias); // set an alias for a IO device
  virtual SwOSIO *getIO( FtSwarmIOType_t ioType, FtSwarmPort_t port);    // get a pointer to an IO port via address
	virtual SwOSIO *getIO( const char *name);                              // get a pointer to an IO port via name or alias
  virtual FtSwarmController_t getType();                                  // what I am?
	virtual char*              myType();                                   // what I am?
  virtual FtSwarmVersion_t   getCPU() { return _CPU; };                  // my CPU type
	virtual const char *       getVersionCPU();                            // my CPU type as string
  virtual bool               isLocal() { return _local; };               // local or remote?
	virtual char *             getHostname( );                             // hostname
	virtual void               jsonize( JSONize *json, uint8_t id);        // send board & IO device information as a json string
  virtual void               jsonizeIO( JSONize *json, uint8_t id);      // send IO device information as a json string
  virtual void loadAliasFromNVS(  nvs_handle_t my_handle );              // write my alias to NVS
  virtual void saveAliasToNVS(  nvs_handle_t my_handle );                // load my alias from NVS
  virtual void setState( SwOSState_t state, uint8_t members = 0, char *SSID = NULL ); // visualizes controller's state like booting, error,...
  virtual void factorySettings( void );                                  // reset factory settings
  virtual void halt( void );                                             // stop all actors
  virtual void unsubscribe( bool cascade );                              // unsubscribe userevents and if cascade = true all IOs
  virtual bool isI2CSwarmCtrl( void );                                   // is a ftSwarmI2C-Board 
  virtual void identify( void );                                         // set LEDs to aquamarine / OLED to "it's me" to identify HW 
  virtual char *subscribe( char *ctrlName );                             // listen on user event data
  virtual bool changeIOType( uint8_t port, FtSwarmIOType_t oldIOType, FtSwarmIOType_t newIOType ); // change port's IO Type if possible
  virtual bool hasGyro( void ) { return false; };                        // test if HW has a gyro

  virtual void read(); // run measurements

  // API commands
	virtual bool apiActorCmd( char *id, int cmd );                 // send an actor's command (from api)
  virtual bool apiActorSpeed( char *id, int speed );             // send an actors's speed (from api)
	virtual bool apiLEDBrightness( char *id, int brightness );     // send a LED command (from api)
	virtual bool apiLEDColor( char *id, int color );               // send a LED command (from api)
  virtual bool apiServoOffset( char *id, int offset );           // send a Servo command (from api)
  virtual bool apiServoPosition( char *id, int position );       // send a Servo command (from api)
  virtual bool apiCAMStreaming( char *id, bool onOff );            // start/stops CAM streaming
  virtual bool apiCAMFramesize( char *id, int framesize );         // set CAM framzesize / resolution
  virtual bool apiCAMQuality( char *id, int quality );             // set CAM quality
  virtual bool apiCAMBrightness( char *id, int brightness );       // set CAM brightness
  virtual bool apiCAMContrast( char *id, int contrast );           // set CAM contrast
  virtual bool apiCAMSaturation( char *id, int saturation );       // set CAM saturation
  virtual bool apiCAMSpecialEffect( char *id, int specialEffect ); // set CAM special effect
  virtual bool apiCAMWbMode( char *id, int wbMode );               // set CAM wbMode    
  virtual bool apiCAMHMirror( char *id, bool hMirror );            // set CAM H-Mirror
  virtual bool apiCAMVFlip( char *id, bool vFlip );                // set CAM V-Flip 
  virtual bool maintenanceMode();

  // Communications
  virtual bool OnDataRecv( SwOSCom *com );                         // data via espnow revceived
  virtual bool recvState( SwOSCom *com );                          // receive state from another ftSwarmXX
  virtual SwOSCom *state2Com( MacAddr destination );               // copy my state in a com struct
  virtual void registerMe( SwOSCom *com );                         // fill in my own data in registerCmd datagram
  virtual void sendAlias( MacAddr destination );                   // send my alias names

  // set comState
  virtual void setComState( SwOSComState_t comState ) { _comState = comState; };

  // get comState
  virtual SwOSComState_t getComState( void ) { return _comState; };

  // ms since last received package
  virtual unsigned long networkAge( void );
    
};