/*
 * SwOSLog.cpp
 *
 * ftSwarm logger
 * 
 * (C) 2025 Christian Bergschneider & Stefan Fuss
 * 
 */

#pragma once

#include <stdlib.h>
#include "SwOS.h"

#define STDIO_BUFFER_SIZE 2048

extern "C" void dumpStdIO(char *out, size_t max_len);

extern "C" void redirectStdIO();

extern "C" void flushStdIO(void);

#define LOG_INFO  0
#define LOG_WAIT  1
#define LOG_ERROR 2
#define LOG_FATAL 3
#define LOG_WARN  4

extern "C" void swarm_log(uint8_t loglevel, const char *file, int line, const char *fmt, ...);

// log INFO
#define SWARM_LOG_INFO(fmt, ...) swarm_log(LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

// log WAIT, set swarm state to WAITING
#define SWARM_LOG_WAIT(fmt, ...) swarm_log(LOG_WAIT, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

// log WARNING
#define SWARM_LOG_WARN(fmt, ...) swarm_log(LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

// log ERROR, set swarm state to error and continue
#define SWARM_LOG_ERROR(fmt, ...) swarm_log(LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

// log FATAL, set swarm state to error and stop processing
#define SWARM_LOG_FATAL(fmt, ...) swarm_log(LOG_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)