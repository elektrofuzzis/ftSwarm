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

const uint32_t LEDCOLOR0[MAXSTATE] = { CRGB::Black, CRGB::Blue, CRGB::Yellow, CRGB::Green, CRGB::Red, CRGB::Cyan, CRGB::Aquamarine, CRGB::DeepPink };
const uint32_t LEDCOLOR1[MAXSTATE] = { CRGB::Black, CRGB::Blue, CRGB::Yellow, CRGB::Green, CRGB::Red, CRGB::Cyan, CRGB::Aquamarine, CRGB::DeepPink };

const char     OLEDMSG[MAXSTATE][20] = { "offline", "booting", "connecting wifi", "online", "ERROR - check logs", "waiting on HW", "It's me!", "FATAL - check logs" };

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
  SwOSState_t       state = OFFLINE;
  
  bool              isSubscribed = false;
  char             *subscribedCtrlName = NULL;

	const char *     version( FtSwarmVersion_t v);

  uint8_t          microstepMode = 0;  // FtSwarmPwrDrive only

  uint8_t          pixels = 0;

  SwOSIO* createIO( SwOSIOType_t ioType, uint8_t port, char *name, char *alias ); // create an IO by type
  
  // communications
  bool setPixel( SwOSCom *com );
  bool resetCounter( SwOSCom *com );
  bool setActorSpeed( SwOSCom *com );
  bool userEvent( SwOSCom *com );
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
  uint8_t setupLocalMotors( uint8_t maxIO, uint8_t motors );
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
  // get index to io pointer
  uint8_t getIndex( SwOSIO *io );                                        

  // lock controller
  void    lock( void );

  // unlock controller
  void    unlock( void );

  // check, if the controller is actively used by someone
  bool    isInUse( void );

  // get a pointer to an IO port by address
  SwOSIO* getIO( SwOSIOType_t ioType, FtSwarmPort_t port );

  // get a pointer to an IO port by name
	SwOSIO* getIO( const char *name);

  // mark communcation in lastContact
  void    tick( void );

  // Online?
  bool    isOnline( void ) { return getComState() == COMSTATE_ONLINE; };

  // set wifi
  void setWifi( FtSwarmWifi_t mode, char *SSID, char*PSK );

  // reboot
  void reboot( void );

  SwOSMotor*    getMotor( char *name );                                  // get a pointer to a motor by name
  SwOSMotor*    getMotor( uint8_t index );                               // get a pointer to a motor by index
  SwOSCAM*      getCAM( char *name );                                    // get a pointer to a cam by name
  SwOSCounter*  getCounter( uint8_t index );                             // get a pointer to a counter by index
  SwOSI2C*      getI2C( uint8_t index );                                 // get a pointer to an i2c by index
  SwOSPixel*    getPixel( char *name );                                  // get a pointer to a pixel by name
  SwOSServo*    getServo( char *name );                                  // get a pointer to a servo by name
  SwOSServo*    getServo( uint8_t index );                               // get a pointer to a servo by index
  SwOSStepper*  getStepper( uint8_t index );                             // get a pointer to a stepper by index

  FtSwarmController_t getType();                                  // what I am?
  FtSwarmVersion_t   getCPU() { return CPU; };                    // my CPU type
	const char *       getVersionCPU();                             // my CPU type as string
  bool               isLocal() { return local; };                 // local or remote?
	char *             getHostname( );                              // hostname
	void               serialize( Serialize *serialize );   // send board & IO device information as a json string
  void               serializeIO( Serialize *serialize ); // send IO device information as a json string

  // load my port & alias settings from NVS
  void loadFromNVS( void );                                      
  
  // write my port & alias settings to NVS
  void saveToNVS( void );                  
  
  // save settings to nvs: scope 0 - all, 1 - controller, 2 - alias, 3 - events
  void save( uint8_t scope );

  void setState( SwOSState_t state, uint8_t members = 0, char *SSID = NULL ); // visualizes controller's state like booting, error,...
  SwOSState_t getState( void ) { return isOnline()?state:OFFLINE; };

  bool changeIOType( uint8_t index, SwOSIOType_t newIOType );    // change port's IO Type if possible

  void factorySettings( void );                                  // reset factory settings

  void halt( void );                                             // stop all actors
  bool isI2CSwarmCtrl( void );                                   // is a ftSwarmI2C-Board 
  void identify( void );                                         // set LEDs to aquamarine / OLED to "it's me" to identify HW 
  
  bool hasGyro( void );                                          // test if HW has a gyro
  bool hasExtPort( void );                                       // test if HW has an ExtentionPort

  void deleteEvents( void );                                     // delete all events

  char *subscribe( char *ctrlName );                             // listen on user event data
  void unsubscribe( bool cascade );                              // unsubscribe userevents and if cascade = true all IOs

  // run measurements
  void read();

  // API commands
	bool apiActorCmd( char *id, int cmd );                 // send an actor's command (from api)
  bool apiActorSpeed( char *id, int speed );             // send an actors's speed (from api)
	bool apiLEDBrightness( char *id, int brightness );     // send a LED command (from api)
	bool apiLEDColor( char *id, int color );               // send a LED command (from api)
  bool apiServoOffset( char *id, int offset );           // send a Servo command (from api)
  bool apiServoPosition( char *id, int position );       // send a Servo command (from api)
  bool apiCAMStreaming( char *id, bool onOff );            // start/stops CAM streaming
  bool apiCAMFramesize( char *id, int framesize );         // set CAM framzesize / resolution
  bool apiCAMQuality( char *id, int quality );             // set CAM quality
  bool apiCAMBrightness( char *id, int brightness );       // set CAM brightness
  bool apiCAMContrast( char *id, int contrast );           // set CAM contrast
  bool apiCAMSaturation( char *id, int saturation );       // set CAM saturation
  bool apiCAMSpecialEffect( char *id, int specialEffect ); // set CAM special effect
  bool apiCAMWbMode( char *id, int wbMode );               // set CAM wbMode    
  bool apiCAMHMirror( char *id, bool hMirror );            // set CAM H-Mirror
  bool apiCAMVFlip( char *id, bool vFlip );                // set CAM V-Flip 

  // Communications
  bool OnDataRecv( SwOSCom *com );                         // data via espnow revceived
  bool recvState( SwOSCom *com );                          // receive state from another ftSwarmXX
  SwOSCom *state2Com( MacAddr destination );               // copy my state in a com struct
  void registerMe( SwOSCom *com );                         // fill in my own data in registerCmd datagram
  void sendIOConfig( MacAddr destination );                // send my IO config
  void setMicrostepMode( uint8_t mode );                   // set microstep mode
  uint8_t getMicrostepMode( void );                        // get microstep mode
  bool ioConfig( SwOSCom *com );

  // set comState
  void setComState( SwOSComState_t comState ) { this->comState = comState; };

  // get comState
  SwOSComState_t getComState( void ) { return comState; };

  // ms since last received package
  unsigned long networkAge( void );
    
};