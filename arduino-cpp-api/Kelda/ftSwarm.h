////////////////////////////////////////////////////
//
// ftSwarm.h
//
// common defintions for all classes
//
// (C) 2021 Christian Bergschneider, Stefan Fuss
//
///////////////////////////////////////////////////

#ifndef FTSWARM_H
#define FTSWARM_H

// CPU unit
#define AIN1         39
#define AIN2         25
#define AIN3         26
#define AIN4         27
#define MOSISD       23
#define MISOSD       19
#define CLKSD        18
#define CSSD         5
#define M1IN1        13
#define M1IN2        4
#define M2IN1        2
#define M2IN2        0

// ftSwarm
#define RGBLED       32
#define SERVO        33
#define ULTRASONICTX 15
#define ULTRASONICRX 29
#define PULLUPA2     14

//ftSwarmControl
#define MISO165      35
#define CS165        14
#define CLK165       13
#define LD165        15
#define JOY11        36
#define JOY12        33
#define JOY21        34
#define JOY22        32

// PWM
#define SERVOPWM 3
#define PWM_M1 0
#define PWM_M2 1

// RPC commands

#define SETPIXELCOLOR "setPixelColor"
#define SETBRIGHTNESS "setBrightness"
#define GETSTATE      "getState"
#define GETVALUE      "getValue"
#define SETPOSITION   "setPosition"
#define GETPOSITION   "getPosition"
#define MOTORCMD      "cmd"

#define UREF          4.7

// common errors
enum ftSwarmError { FTSWARM_OK, NO_PORT, UNKNOWN_CMD };

#endif
