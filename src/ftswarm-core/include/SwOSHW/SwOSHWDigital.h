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
    gpio_num_t      PUA2   = GPIO_NUM_NC;
    gpio_num_t      USTX   = GPIO_NUM_NC;
    FtSwarmToggle_t toggle = FTSWARM_NOTOGGLE;
    bool            normallyOpen = true;

    virtual void setupLocal();

  public:
 
	  SwOSDigitalInput(const char *name, uint8_t port, SwOSCtrl *ctrl, SwOSIOType_t ioType );
  
    // administrative stuff
	  virtual void serialize( Serialize *serialize );
    virtual bool isGPIOInput( void );
    virtual bool isDigitalInput( void ) { return true; };

    // read sensor
	  virtual void read();
    virtual void setReading( int32_t newValue );

    // external commands
    virtual void            setParameter( int32_t parameter );
    virtual void            setValue( int32_t value );                          
    virtual FtSwarmToggle_t getToggle( void );                                  

};

/***************************************************
 *
 *   SwOSHC165
 *
 ***************************************************/

 class SwOSHC165 : public SwOSIO {
  protected:
    gpio_num_t LD, CS, CLK, MISO;
    uint8_t    lastValue;
  
    // local HW procedures
    virtual void setupLocal();
  
  public:
  
    // constructor
    SwOSHC165(const char *name, SwOSCtrl *ctrl);
  
    // administrative stuff
    virtual void read();
  
    // commands
    virtual void    setValue( uint8_t value ) { this->lastValue = value; };
    virtual uint8_t getValue( uint8_t bit )   { return lastValue && 1<<bit; };
    virtual uint8_t getValue()                { return lastValue; };
  
  };

extern SwOSHC165 *hc165;
  

