/*
 * ftSwarmRS.h
 *
 * framework to build a ftSwarm application
 * 
 * (C) 2021-23 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#define __FTSWARMRS__

#if !defined(CONFIG_IDF_TARGET_ESP32S3)
    #error "Project configration error: please use an ESP32-S3 board definition"
#endif

#include "SwOS.h"
