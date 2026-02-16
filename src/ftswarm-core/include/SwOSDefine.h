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

#ifndef FTSWARM_HAL_HAS_OLED
  #define FTSWARM_HAL_HAS_OLED 0
#endif

#ifndef FTSWARM_HAL_HAS_HC165
  #define FTSWARM_HAL_HAS_HC165 0
#endif

#ifndef FTSWARM_HAL_HAS_EXT_PORT
  #define FTSWARM_HAL_HAS_EXT_PORT 0
#endif

#ifndef FTSWARM_HAL_HAS_CAM
  #define FTSWARM_HAL_HAS_CAM 0
#endif

#ifndef FTSWARM_HAL_HAS_RS485
  #define FTSWARM_HAL_HAS_RS485 0
#endif

#ifndef FTSWARM_HAL_HAS_DISCRETE_RGB
  #define FTSWARM_HAL_HAS_DISCRETE_RGB 0
#endif

#ifndef FTSWARM_HAL_FIRSTJPOTI
  #define FTSWARM_HAL_FIRSTJPOTI 0
#endif
