/*
 * SwOSCalibration.h
 *
 * Joystick & RCServo calibration used by serial and OLED firmware
 * 
 * (C) 2021-26 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include "SwOS.h"
#include "SwOSNVS.h"

extern bool calibrateJoysticks( bool useOLED, SwOSJoyCalibration_t calibration[4] );