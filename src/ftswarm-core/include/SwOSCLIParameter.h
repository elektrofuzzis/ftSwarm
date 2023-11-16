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
  CLICMD_triggerUserEvent,
  CLICMD_show,
  CLICMD_setMicrostepMode,
  CLICMD_getMicrostepMode,
  CLICMD_subscribe,
  CLICMD_getIOType,
  CLICMD_getSensorType,
  CLICMD_setSensorType,
  CLICMD_getValue,
  CLICMD_getVoltage,
  CLICMD_getResistance,
  CLICMD_getKelvin,
  CLICMD_getCelcius,
  CLICMD_getFahrenheit,
  CLICMD_getToggle,
  CLICMD_setActorType,
  CLICMD_getActorType,
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
  CLICMD_getRegister,
  CLICMD_setRegister,
  CLICMD_setDistance,
  CLICMD_getDistance,
  CLICMD_run,
  CLICMD_isRunning,
  CLICMD_stop,
  CLICMD_homing,
  CLICMD_isHoming,
  CLICMD_setHomingOffset,
  CLICMD_MAX
} CLICmd_t;

class SwOSCLIParameter {
  protected:
    char *_value = NULL;
    SwOSIO *_io  = NULL;
  public:
    ~SwOSCLIParameter();
    void setValue( char *value );
    void setValue( SwOSIO *io, char *ioName );
    int  getValue( void );
    long  getLongValue( void );
    SwOSIO *getIO( void ) { return _io; };
    bool isConstant( void ) { return (_io == NULL); };
    bool isIO( void ) { return !isConstant(); };
    // Test, if an "execute"-parameter is in Range or not
    bool inRange( const char *name, int minValue, int maxValue );
} ;