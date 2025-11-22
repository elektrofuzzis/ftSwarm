/*
 * SwOSCLIParameter.h
 *
 * describe parameters from CLI for execute-Commands! 
 * 
 * (C) 2023 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include <SwOSHW.h>

typedef enum {
  CLICMD_login,
  CLICMD_triggerUserEvent,
  CLICMD_show,
  CLICMD_getSwarm,
  CLICMD_getEvents,
  CLICMD_save,
  CLICMD_useConfig,
  CLICMD_setAlias,
  CLICMD_setWifi,
  CLICMD_reboot,
  CLICMD_setMicrostepMode,
  CLICMD_getMicrostepMode,
  CLICMD_subscribe,
  CLICMD_unsubscribe,
  CLICMD_setIOType,
  CLICMD_getIOType,
  CLICMD_getValue,
  CLICMD_getVoltage,
  CLICMD_getResistance,
  CLICMD_getKelvin,
  CLICMD_getCelcius,
  CLICMD_getFahrenheit,
  CLICMD_getToggle,
  CLICMD_setSpeed,
  CLICMD_getSpeed,
  CLICMD_setMotionType,
  CLICMD_getMotionType,
  CLICMD_onTrigger,
  CLICMD_onTriggerLR,
  CLICMD_onTriggerFB,
  CLICMD_setPosition,
  CLICMD_getPosition,
  CLICMD_setOffset,
  CLICMD_getOffset,
  CLICMD_setColor,
  CLICMD_getColor,
  CLICMD_setBrightness,
  CLICMD_getBrightness,
  CLICMD_setRegister,
  CLICMD_getRegister,
  CLICMD_setDistance,
  CLICMD_getDistance,
  CLICMD_run,
  CLICMD_isRunning,
  CLICMD_stop,
  CLICMD_homing,
  CLICMD_isHoming,
  CLICMD_setHomingOffset,
  CLICMD_testPixels,
  CLICMD_MAX
} CLICmd_t;

class SwOSCLIParameter {

  protected:
    char   *str = NULL;
    char   *num = NULL;
    SwOSIO *io  = NULL;

  public:
    ~SwOSCLIParameter();
    void setNumber( char *value );
    long getNumber( void );
    
    void setString( char *value );
    char *getString( void ) { return str; };
    
    void setIO( SwOSIO *io );
    SwOSIO *getIO( void ) { return io; };

    bool isNumber( void ) { return (num); };
    bool isIO( void ) { return (io); };
    bool isString( void ) { return ( (str) && (*str) ); }  // ptr not null and not an empty string

    // Test, if an "execute"-parameter is in Range or not
    bool inRange( const char *name, int minValue, int maxValue, char *error );

};