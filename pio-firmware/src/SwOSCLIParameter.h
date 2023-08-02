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
  IOCMD_subscribe,
  IOCMD_getIOType,
  IOCMD_getSensorType,
  IOCMD_setSensorType,
  IOCMD_getValue,
  IOCMD_getVoltage,
  IOCMD_getResistance,
  IOCMD_getKelvin,
  IOCMD_getCelcius,
  IOCMD_getFahrenheit,
  IOCMD_getToggle,
  IOCMD_setActorType,
  IOCMD_getActorType,
  IOCMD_setSpeed,
  IOCMD_getSpeed,
  IOCMD_setMotionType,
  IOCMD_getMotionType,
  IOCMD_onTrigger,
  IOCMD_onTriggerLR,
  IOCMD_onTriggerFB,
  IOCMD_setPosition,
  IOCMD_getPosition,
  IOCMD_setOffset,
  IOCMD_getOffset,
  IOCMD_setColor,
  IOCMD_getColor,
  IOCMD_setBrightness,
  IOCMD_getBrightness,
  IOCMD_getRegister,
  IOCMD_setRegister,
  IOCMD_MAX
} IOCmd_t;

class SwOSCLIParameter {
  protected:
    char *_value = NULL;
    SwOSIO *_io  = NULL;
  public:
    ~SwOSCLIParameter();
    void setValue( char *value );
    void setValue( SwOSIO *io, char *ioName );
    int  getValue( void );
    SwOSIO *getIO( void ) { return _io; };
    bool isConstant( void ) { return (_io == NULL); };
    bool isIO( void ) { return !isConstant(); };
    // Test, if an "execute"-parameter is in Range or not
    bool inRange( const char *name, int minValue, int maxValue );
} ;