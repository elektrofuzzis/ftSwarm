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
#include "SwOSHW/SWOSHWCam.h"
#include "SwOSHW/SwOSHWCounter.h"
#include "SwOSHW/SwOSHWDisplay.h"

// only to feed that silly compiler
class SwOSMotor;
class SwOSLamp;
class SwOSPixel;
class SwOSServo;
class SwOSGyro;
class SwOSI2C;
class SwOSCAN;
class SwOSCAM;
class SwOSCounter;
class SwOSStepper;
class SwOSAnalogInput;

const CRGB LEDCOLOR0[MAXSTATE] = {CRGB::Black,CRGB::Blue,CRGB::Yellow,CRGB::Green,CRGB::Red,CRGB::Cyan,CRGB::Aquamarine,CRGB::DeepPink,CRGB::Purple,CRGB::Black };
const CRGB LEDCOLOR1[MAXSTATE] = {CRGB::Black,CRGB::Blue,CRGB::Yellow,CRGB::Green,CRGB::Red,CRGB::Cyan,CRGB::Aquamarine,CRGB::DeepPink,CRGB::Purple,CRGB::Black };

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

    uint8_t          microstepMode = 0;  // FtSwarmPwrDrive only

    uint8_t          pixels = 0;

    SwOSIO* createIO( SwOSIOType_t ioType, uint8_t port, const char *name, const char *alias, uint8_t flags ); // create an IO by type
  
    // communications
    bool setPixel( SwOSCom *com );
    bool setEffect( SwOSCom *com );
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
    bool CANSend( SwOSCom *com );
    bool CANRecv( SwOSCom *com );
    bool setParameter( SwOSCom *com );

    // initialize Hardware
    void    setupLocalCommonHardware( void ) ;  // setup general hardware like TIMER0
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
    SwOSAnalogInput       *pwrctl = NULL;
  
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
    void setWifi( FtSwarmWifi_t mode, char *SSID, char*PSK, bool reboot );

    // reboot
    void reboot( void );

    SwOSMotor*    getMotor( char *name );                                  // get a pointer to a motor by name
    SwOSMotor*    getMotor( uint8_t index );                               // get a pointer to a motor by index
    SwOSCAM*      getCAM( char *name );                                    // get a pointer to a cam by name
    SwOSCounter*  getCounter( uint8_t index );                             // get a pointer to a counter by index
    SwOSI2C*      getI2C( uint8_t index );                                 // get a pointer to an i2c by index
    SwOSCAN*      getCAN( uint8_t index );                                 // get a pointer to a can by index
    SwOSPixel*    getPixel( char *name );                                  // get a pointer to a pixel by name
    SwOSPixel*    getPixel( uint8_t index );                               // get a pointer to a pixel by index
    SwOSServo*    getServo( char *name );                                  // get a pointer to a servo by name
    SwOSServo*    getServo( uint8_t index );                               // get a pointer to a servo by index
    SwOSStepper*  getStepper( uint8_t index );                             // get a pointer to a stepper by index

    // FtSwarmController_t getType();                                  // what I am?
    FtSwarmVersion_t   getCPU() { return CPU; };                    // my CPU type
    bool               isLocal() { return local; };                 // local or remote?
	  char *             getHostname( );                              // hostname
	  void               serialize( Serialize *serialize );           // send board & IO device information as a json string
	  void               serializeEvents( Serialize *serialize );
    void               serializeIO( Serialize *serialize );         // send IO device information as a json string

    // load my port & alias settings from NVS
    void loadFromNVS( void );                                      
  
    // write my port & alias settings to NVS
    void saveToNVS( void );                  
  
    // print my local settings
    void printNVS( void ); 

    // save settings to nvs
    void save( FtSwarmNVSScope_t scope, uint8_t port );

    // visualizes controller's state like booting, error,...
    void setState( SwOSState_t state, const char *text = nullptr ); 

    // get local state or - if remote RUNNING/OFFLINE
    SwOSState_t getState( void );

    // change port's IO Type if possible
    bool changeIOType( uint8_t index, SwOSIOType_t newIOType, uint8_t flags );

    void halt( void );                                             // stop all actors
    bool isI2CSwarmCtrl( void );                                   // is a ftSwarmI2C-Board 
    void identify( void );                                         // set LEDs to aquamarine / OLED to "it's me" to identify HW 
  
    bool hasGyro( void );                                          // test if HW has a gyro
    bool hasOLED( void );                                          // test if HW has OLED

    void deleteEvents( void );                                     // delete all events

    char *subscribe( char *ctrlName );                             // listen on user event data
    void unsubscribe( bool cascade );                              // unsubscribe userevents and if cascade = true all IOs

    // run measurements
    void operate();

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

    // set Blink all internal ftPixels
    void setBlink( uint32_t periodMS, uint8_t signal, uint8_t duty, uint8_t pause, uint8_t p1, uint8_t p2, uint8_t p3 );

    // reset Blink all internal ftPixels
    void resetBlink( int32_t color );
    
};