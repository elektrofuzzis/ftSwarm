/*
 * SwOSLog.cpp
 *
 * ftSwarm logger
 * 
 * (C) 2025 Christian Bergschneider & Stefan Fuss
 * 
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <freertos/semphr.h>

#include "SwOSLog.h"
#include "SwOSSwarm.h"

static char ring_buffer[STDIO_BUFFER_SIZE];
static size_t ring_head = 0;
static size_t ring_len = 0;
static SemaphoreHandle_t ring_mutex;

static FILE *original_stdout = NULL;

// Schreibe Daten in Ringpuffer
void ringbuffer_write(const char *data, size_t len) {
    if (ring_mutex) xSemaphoreTake(ring_mutex, portMAX_DELAY);

    for (size_t i = 0; i < len; ++i) {
        ring_buffer[ring_head] = data[i];
        ring_head = (ring_head + 1) % STDIO_BUFFER_SIZE;
        if (ring_len < STDIO_BUFFER_SIZE) {
            ring_len++;
        }
    }

    if (ring_mutex) xSemaphoreGive(ring_mutex);
}

// Dump des Ringpuffers (zum Auslesen, z. B. Web)
void dumpStdIO(char *out, size_t max_len) {
    if (ring_mutex) xSemaphoreTake(ring_mutex, portMAX_DELAY);

    size_t start = (ring_head + STDIO_BUFFER_SIZE - ring_len) % STDIO_BUFFER_SIZE;
    for (size_t i = 0; i < ring_len && i < max_len - 1; ++i) {
        out[i] = ring_buffer[(start + i) % STDIO_BUFFER_SIZE];
    }
    out[(ring_len < max_len - 1) ? ring_len : max_len - 1] = '\0';

    if (ring_mutex) xSemaphoreGive(ring_mutex);
}

// Eigene stdout-Write-Funktion
int my_stdout_write(void *cookie, const char *buf, int len) {
    // 1. In Ringpuffer
    ringbuffer_write(buf, len);

    // 2. In originale stdout (UART-Ausgabe beibehalten)
    if (original_stdout) {
        return fwrite(buf, 1, len, original_stdout);
    }

    return len;
}

void flushStdIO(void) {

  if (original_stdout) fflush(original_stdout);

}

void redirectStdIO() {
    ring_mutex = xSemaphoreCreateMutex();

    // Backup von original stdout (UART)
    original_stdout = stdout;
    // if( original_stdout ) printf("stdout\n");

    // eigene FILE mit write-Funktion erzeugen
    FILE *custom_out = funopen(NULL, NULL, my_stdout_write, NULL, NULL);
    if (custom_out == NULL) {
        printf("funopen failed!\n");
        return;
    }

    setvbuf(custom_out, NULL, _IONBF, 0);

    // stdout ersetzen
    stdout = custom_out;
    stderr = custom_out;

}

void swarm_log(uint8_t loglevel, const char *file, int line, const char *fmt, ...) {
    
    char msg[256];

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    switch ( loglevel ) {

      case LOG_FATAL: printf("[FATAL][%s:%d]: %s\n", file, line, msg); 
                      myOSSwarm.halt();
                      myOSSwarm.setState( FATAL );
                      delay(1000);
                      myOSNetwork.stop();
                      while (1) delay(500); 
                      break;

      case LOG_ERROR: printf("[ERROR]: %s\n", msg);
                      myOSSwarm.setState( ERROR, msg ); 
                      break;

      case LOG_WAIT:  printf("[WAITING]: %s\n", msg); 
                      myOSSwarm.setState( WAITING ); 
                      break;

      case LOG_WARN:  printf("[WARNING]: %s\n", msg); 
                      break;

      default:        printf("[INFO]: %s\n", msg); 
                      break;
    }

    

}
