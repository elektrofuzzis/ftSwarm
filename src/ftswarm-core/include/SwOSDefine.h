/*
 * SwOSDefine.h
 *
 * set some defines which are not set in pins_arduino.h
 * 
 * (C) 2021-26 Christian Bergschneider & Stefan Fuss
 * 
 */

#pragma once

#include <Arduino.h>

// inter swarm communication deep debugging 
// #define DEBUG_COMMUNICATION
// #define DEBUG_COMMUNICATION_SWARM
// #define DEBUG_READTASK

#ifndef FTSWARM_HAL_OLEDS
  #define FTSWARM_HAL_OLEDS 0
#endif

#ifndef FTSWARM_HAL_HC165
  #define FTSWARM_HAL_HC165 0
#endif

#ifndef FTSWARM_HAL_EXT_PORT
  #define FTSWARM_HAL_EXT_PORT 0
#endif

#ifndef FTSWARM_HAL_CAMS
  #define FTSWARM_HAL_CAMS 0
#endif

#ifndef FTSWARM_HAL_RS485
  #define FTSWARM_HAL_RS485 0
#endif

#ifndef FTSWARM_HAL_DISCRETE_RGBS
  #define FTSWARM_HAL_DISCRETE_RGBS 0
#endif

#ifndef FTSWARM_HAL_INPUTS
  #define FTSWARM_HAL_INPUTS 0
#endif

#ifndef FTSWARM_HAL_AX_INPUTS
  #define FTSWARM_HAL_AX_INPUTS 0
#endif

#ifndef FTSWARM_HAL_MOTORS
  #define FTSWARM_HAL_MOTORS 0
#endif

#ifndef FTSWARM_HAL_RCSERVOS
  #define FTSWARM_HAL_RCSERVOS 0
#endif

#ifndef FTSWARM_HAL_SERVOS
  #define FTSWARM_HAL_SERVOS 0
#endif

#ifndef FTSWARM_HAL_PIXELS
  #define FTSWARM_HAL_PIXELS 0
#endif

#ifndef FTSWARM_HAL_BUTTONS
  #define FTSWARM_HAL_BUTTONS 0
#endif

#ifndef FTSWARM_HAL_JOYSTICKS
  #define FTSWARM_HAL_JOYSTICKS 0
#endif

#ifndef FTSWARM_HAL_FIRSTJPOTI
  #define FTSWARM_HAL_FIRSTJPOTI 0
#endif
