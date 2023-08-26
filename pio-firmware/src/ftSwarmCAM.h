/*
 * ftSwarmCAM.h
 *
 * framework to build a ftSwarm application
 * 
 * (C) 2021-23 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#if !defined(CONFIG_IDF_TARGET_ESP32)
    #error "Project configration error: please use an ESP32 board definition"
#endif

#include "SwOS.h"