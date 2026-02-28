/*
 * SwOSHWDigital.h
 *
 * Digital inputs hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include "SwOSHWBaseIO.h"
 
/***************************************************
 *
 *   SwOSDigitalInput
 *
 ***************************************************/

 class SwOSDigitalInput : public SwOSInput {

  protected:
    FtSwarmToggle_t toggle = FTSWARM_NOTOGGLE;
    bool            normallyOpen = true;

    virtual void setupLocal();

  public:
 
	  SwOSDigitalInput(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType, uint8_t flags );
  
    // administrative stuff
	  virtual void serialize( Serialize *serialize );
    virtual bool isGPIOInput( void );
    virtual bool isDigitalInput( void ) { return true; };

    // read sensor
	  virtual void operate();
    virtual void setReading( int32_t newValue, FtSwarmTrigger_t secondTriggerEvent );

    // external commands
    virtual void            setParameter( int32_t parameter );                        
    virtual FtSwarmToggle_t getToggle( void );                                  

};
