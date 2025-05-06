/*
 * SwOSHWXX.h
 *
 * Standard controller (ftSwarmJST, ftSwarmControl, ftSwarmCAM) hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include "SwOSHWBaseCtrl.h"
#include "SwOSHWI2CSensor.h"
#include "SwOSHWDigital.h"
#include "SwOSHWDisplay.h"
#include "SwOSHWAnalog.h"
#include "SwOSHWCounter.h"
#include "SWOSHWCam.h"
 
/***************************************************
 *
 *   SwOSSwarmXX - Base class for ftSwarmXX
 *
 ***************************************************/

 class SwOSSwarmXX : public SwOSCtrl {

  public:
    
    // constructor, destructor
    SwOSSwarmXX( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, SwOSCtrlConfig_t ctrlConfig );
    
};

/***************************************************
 *
 *   SwOSSwarmJST
 *
 ***************************************************/

class SwOSSwarmJST : public SwOSSwarmXX {
  
  public:

    // constructor, destructor
	  SwOSSwarmJST( FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, SwOSCtrlConfig_t ctrlConfig );
    SwOSSwarmJST( SwOSCom *com ); // constructor
  
    // administrative stuff
	  virtual FtSwarmController_t getType();                                  // what I am?
	  virtual char* myType();                                                // what I am?

};

/***************************************************
 *
 *   SwOSSwarmControl
 *
 ***************************************************/

class SwOSSwarmControl : public SwOSSwarmXX {

  protected:
    boolean _remoteControl = false;
    boolean _firstRead     = true;

    virtual void _sendAlias( SwOSCom *alias );

  public:
    // specific hardware
	  SwOSButton   *button[8];
	  SwOSHC165    *hc165;
	  SwOSJoystick *joystick[2];

    SwOSOLED     *oled;
 
	  SwOSSwarmControl(FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, SwOSCtrlConfig_t ctrlConfig ); // constructor
    SwOSSwarmControl( SwOSCom *com ); // constructor
    ~SwOSSwarmControl(); // destructor
  
    // administrative stuff
    virtual bool isInUse( void );
	  virtual bool cmdAlias( char *device, uint8_t port, const char *alias); // set an alias for a IO device
    virtual SwOSIO *getIO( FtSwarmIOType_t ioType,  FtSwarmPort_t port);   // get a pointer to the requested IO Device via type & port
	  virtual SwOSIO *getIO( const char *name);                              // get a pointer to the requested IO Device via name or alias
	  virtual char* myType();                                                // what I am?
	  virtual FtSwarmController_t getType();                                  // what I am?
	  virtual void jsonizeIO( JSONize *json, uint8_t id);                    // send IO device information as a json string
    virtual void loadAliasFromNVS(  nvs_handle_t my_handle );              // write my alias to NVS
    virtual void saveAliasToNVS(  nvs_handle_t my_handle );                // load my alias from NVS
    virtual void setState( SwOSState_t state, uint8_t members = 0, char *SSID = NULL ); // visualizes controller's state like booting, error,...
    virtual void factorySettings( void );                                  // reset factory settings
    virtual boolean getRemoteControl( void );                              // get remote control setting
    virtual void setRemoteControl( boolean remoteControl );                // set remote control setting
    virtual void unsubscribe(void );                                       // unsubscribe all IOs

	  virtual void read();                       // run measurements

    // **** Communications *****
    virtual bool recvState( SwOSCom *com );    // receive state from another FtSwarmControl
    virtual SwOSCom *state2Com( MacAddr destination );        // copy my state in a com struct
  
};

/***************************************************
 *
 *   SwOSSwarmCAM
 *
 ***************************************************/

class SwOSSwarmCAM : public SwOSSwarmXX {

  protected:

  public:
    SwOSCAM *cam = NULL;

    SwOSSwarmCAM(FtSwarmSerialNumber_t SN, MacAddr macAddr, bool local, SwOSCtrlConfig_t ctrlConfig ); // constructor
    SwOSSwarmCAM( SwOSCom *com ); // constructor
    ~SwOSSwarmCAM(); // destructor
  
    // administrative stuff
    virtual bool cmdAlias( char *device, uint8_t port, const char *alias); // set an alias for a IO device
    virtual SwOSIO *getIO( FtSwarmIOType_t ioType,  FtSwarmPort_t port);   // get a pointer to the requested IO Device via type & port
    virtual SwOSIO *getIO( const char *name);                              // get a pointer to the requested IO Device via name or alias
    virtual char* myType();                                                // what I am?
    virtual FtSwarmController_t getType();                                  // what I am?
    virtual void jsonizeIO( JSONize *json, uint8_t id);                    // send IO device information as a json string
    virtual void loadAliasFromNVS(  nvs_handle_t my_handle );              // write my alias to NVS
    virtual void saveAliasToNVS(  nvs_handle_t my_handle );                // load my alias from NVS
    virtual void setState( SwOSState_t state, uint8_t members = 0, char *SSID = NULL ); // visualizes controller's state like booting, error,...

    // api commands
    virtual bool apiCAMStreaming(  char *id, bool onOff );           // start/stops CAM streaming
    virtual bool apiCAMFramesize( char *id, int framesize );         // set CAM framzesize / resolution
    virtual bool apiCAMQuality( char *id, int quality );             // set CAM quality
    virtual bool apiCAMBrightness( char *id, int brightness );       // set CAM brightness
    virtual bool apiCAMContrast( char *id, int contrast );           // set CAM contrast
    virtual bool apiCAMSaturation( char *id, int saturation );       // set CAM saturation
    virtual bool apiCAMSpecialEffect( char *id, int specialEffect ); // set CAM special effect
    virtual bool apiCAMWbMode( char *id, int wbMode );               // set CAM wbMode
    virtual bool apiCAMHMirror( char *id, bool hMirror );            // set CAM H-Mirror
    virtual bool apiCAMVFlip( char *id, bool vFlip );                // set CAM V-Flip 

    // **** Communications *****
    virtual bool OnDataRecv( SwOSCom *com );   // data via espnow revceived


};