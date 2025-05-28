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

#include "SwOSHW/SwOSHWBaseIO.h"
#include "SwOSHW/SwOSHWActor.h"
#include "SwOSHW/SwOSHWI2CSensor.h"
#include "SwOSHW/SwOSHWCAM.h"
#include "SwOSHW/SwOSHWCounter.h"
#include "SwOSHW/SwOSHWDisplay.h"

// only to feed that silly compiler
class SwOSMotor;
class SwOSPixel;
class SwOSServo;
class SwOSGyro;
class SwOSI2C;
class SwOSCAM;
class SwOSCounter;
class SwOSStepper;

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
  SemaphoreHandle_t xAccessLock = xSemaphoreCreateMutex();
	FtSwarmVersion_t  CPU;
  bool              local;
  
  unsigned long     lastContact = 0;
  SwOSComState_t    comState = COMSTATE_UNDEFINED;
  
  bool              isSubscribed = false;
  char             *subscribedCtrlName = NULL;

	const char *     version( FtSwarmVersion_t v);

  uint8_t          microstepMode = 0;  // FtSwarmPwrDrive only

  // communications
  bool saveAlias2NVS( SwOSCom *com );
  bool setPixel( SwOSCom *com );
  bool resetCounter( SwOSCom *com );
  bool setActorType( SwOSCom *com );
  bool setActorSpeed( SwOSCom *com );
  bool userEvent( SwOSCom *com );
  SwOSIO* createIO( SwOSIOType_t ioType, uint8_t port, char *name, char *alias ); // create an IO by type
  bool ioConfig( SwOSCom *com );
  bool setServo( SwOSCom *com );
  bool setStepperDistance( SwOSCom *com );
  bool setStepperPosition( SwOSCom *com );
  bool stepperHoming( SwOSCom *com );
  bool setStepperHomingOffset( SwOSCom *com );
  bool stepperStartStop( SwOSCom *com );
  bool I2CRegister( SwOSCom *com );
  bool setParameter( SwOSCom *com );

  // initialize Hardware
  uint8_t setupLocalInputs( uint8_t maxIO );
  uint8_t setupLocalMotors( uint8_t maxIO, uint8_t actors );
  uint8_t setupLocalServos( uint8_t maxIO, uint8_t servos );
  uint8_t setupLocalPixels( uint8_t maxIO );
  uint8_t setupLocalButtons( uint8_t maxIO );
  uint8_t setupLocalJoysticks( uint8_t maxIO, SwOSCtrlConfig_t ctrlConfig  );
  uint8_t setupLocalI2C( uint8_t maxIO, FtSwarmExtMode_t extensionPort );
  uint8_t setupLocalGyro( uint8_t maxIO );
  uint8_t setupLocalOLED( uint8_t maxIO );

public:
	FtSwarmSerialNumber_t serialNumber;
  MacAddr               macAddr;
  bool                  IAmKelda;
  
  // dynamically allocated array SwOSIO *io[]
	SwOSIO **io = NULL;
  uint8_t IOs = 0;

  FtSwarmExtMode_t extensionPort;
	
  // constructor
  SwOSCtrl( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, SwOSCtrlConfig_t ctrlConfig );
  
  // destructor
  ~SwOSCtrl();

  // administrative stuff
  uint8_t getIndex( SwOSIO *io );                                        // get index to io pointer
  virtual void lock( void );
  virtual void unlock( void );
  virtual bool isInUse( void );
  virtual SwOSIO *getIO( SwOSIOType_t ioType, FtSwarmPort_t port );      // get a pointer to an IO port by address
	virtual SwOSIO *getIO( const char *name);                              // get a pointer to an IO port by name
  virtual void tick( void );                                             // mark communcation in lastContact

  SwOSMotor*    getMotor( char *name );                                  // get a pointer to a motor by name
  SwOSMotor*    getMotor( uint8_t index );                               // get a pointer to a motor by index
  SwOSCAM*      getCAM( char *name );                                    // get a pointer to a cam by name
  SwOSCounter*  getCounter( uint8_t index );                             // get a pointer to a counter by index
  SwOSI2C*      getI2C( uint8_t index );                                 // get a pointer to an i2c by index
  SwOSPixel*    getPixel( char *name );                                  // get a pointer to a pixel by name
  SwOSServo*    getServo( char *name );                                  // get a pointer to a servo by name
  SwOSServo*    getServo( uint8_t index );                               // get a pointer to a servo by index
  SwOSStepper*  getStepper( uint8_t index );                             // get a pointer to a stepper by index

  virtual FtSwarmController_t getType();                                  // what I am?
	virtual char*              myType();                                   // what I am?
  virtual FtSwarmVersion_t   getCPU() { return CPU; };                   // my CPU type
	virtual const char *       getVersionCPU();                            // my CPU type as string
  virtual bool               isLocal() { return local; };                // local or remote?
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
  virtual bool changeIOType( uint8_t index, SwOSIOType_t newIOType );    // change port's IO Type if possible
  virtual bool hasGyro( void );                                          // test if HW has a gyro
  virtual bool hasExtPort( void );                                       // test if HW has an ExtentionPort

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

  // Communications
  virtual bool OnDataRecv( SwOSCom *com );                         // data via espnow revceived
  virtual bool recvState( SwOSCom *com );                          // receive state from another ftSwarmXX
  virtual SwOSCom *state2Com( MacAddr destination );               // copy my state in a com struct
  virtual void registerMe( SwOSCom *com );                         // fill in my own data in registerCmd datagram
  virtual void sendIOConfig( MacAddr destination );                // send my IO config
  virtual void setMicrostepMode( uint8_t mode );                   // set microstep mode
  virtual uint8_t getMicrostepMode( void );                        // get microstep mode

  // set comState
  virtual void setComState( SwOSComState_t comState ) { this->comState = comState; };

  // get comState
  virtual SwOSComState_t getComState( void ) { return comState; };

  // ms since last received package
  virtual unsigned long networkAge( void );
    
};