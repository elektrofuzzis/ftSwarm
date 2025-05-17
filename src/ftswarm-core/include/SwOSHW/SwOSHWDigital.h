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
 
	  SwOSDigitalInput(const char *name, uint8_t port, SwOSCtrl *ctrl );
  
    // administrative stuff
	  virtual void jsonize( JSONize *json, uint8_t id);

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
  
/***************************************************
 *
 *   SwOSButton
 *
 ***************************************************/

 class SwOSButton : public SwOSIO, public SwOSEventInput {
  bool            firstRead = true;
	bool            lastState;
  FtSwarmToggle_t toggle;
public:
  // constructor
	SwOSButton(const char *name, uint8_t port, SwOSCtrl *ctrl);

  virtual void read();
  
  // administrative stuff
  virtual FtSwarmIcon_t getIcon()   { return FTSWARM_12_BUTTON; };
	virtual void jsonize( JSONize *json, uint8_t id);
  virtual uint8_t pushState( uint8_t *buffer );
  virtual uint8_t popState( uint8_t *buffer );

  // commands
	virtual FtSwarmToggle_t getToggle();
	virtual bool getState();
  virtual void setState( bool state, bool clearToggle = false );
};

