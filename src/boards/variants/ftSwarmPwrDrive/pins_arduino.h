/*
 * Pins_Arduino.h
 *
 * ftSwarmPwrDrive hardware definitions
 * 
 * (C) 2021-26 Christian Bergschneider & Stefan Fuss
 * 
 */

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <driver/gpio.h> 
#include <driver/adc.h>

// my hardware features
#define FTSWARM_HAL_HAS_RS485  1

// to solve some hen & egg problems
static const int8_t DIGITALIO          = 1;
static const int8_t ANALOGIO           = 2;
static const int8_t PWRCTLIO           = 3;
static const int8_t MOTORIO            = 4;
static const int8_t STEPPERIO          = 5;
static const int8_t JOYSTICKPOTI       = 6;

static const int8_t GYRO_NONE          = 0;
static const int8_t GYRO_6050          = 1;
static const int8_t GYRO_LSM6          = 2;

static const int8_t GYRO_SPI           = 1;
static const int8_t GYRO_INTERNAL_I2C  = 2;
static const int8_t GYRO_EXTERNAL_I2C  = 3;

// I2C (Standard-Bus)
static const gpio_num_t SDA = GPIO_NUM_4;
static const gpio_num_t SCL = GPIO_NUM_5;

// I2C (internal)
static const gpio_num_t SDA_INTERNAL = GPIO_NUM_NC;
static const gpio_num_t SCL_INTERNAL = GPIO_NUM_NC;

// SPI
static const gpio_num_t SS    = GPIO_NUM_NC;
static const gpio_num_t MOSI  = GPIO_NUM_NC;
static const gpio_num_t MISO  = GPIO_NUM_NC;
static const gpio_num_t SCK   = GPIO_NUM_NC;

// HC165
static const gpio_num_t HC165_CS   = GPIO_NUM_NC;
static const gpio_num_t HC165_LD   = GPIO_NUM_NC;
static const gpio_num_t HC165_CLK  = GPIO_NUM_NC;
static const gpio_num_t HC165_MISO = GPIO_NUM_NC;

// Serial (UART0)
static const gpio_num_t TX = GPIO_NUM_43;
static const gpio_num_t RX = GPIO_NUM_44;

// RGB LED 
#define RGB_BUILTIN GPIO_NUM_48

// ftSwarm definitions
#define NOPWRCTL -1
static const uint8_t FTSWARM_HAL_INPUTS       = 5;
static const uint8_t FTSWARM_HAL_AX_INPUTS    = 5;
static const uint8_t FTSWARM_HAL_MOTORS       = 4;
static const uint8_t FTSWARM_HAL_RCSERVOS     = 0;
static const uint8_t FTSWARM_HAL_SERVOS       = 4;
static const uint8_t FTSWARM_HAL_PIXELS       = 2;
static const uint8_t FTSWARM_HAL_BUTTONS      = 0;
static const uint8_t FTSWARM_HAL_JOYSTICKS    = 0;
static const int8_t  FTSWARM_HAL_GYRO         = GYRO_NONE;
static const int8_t  FTSWARM_HAL_GYRO_PORT    = GYRO_NONE;

// RS485
static const gpio_num_t RS485_R   = GPIO_NUM_6;
static const gpio_num_t RS485_REB = GPIO_NUM_7;
static const gpio_num_t RS485_DE  = GPIO_NUM_17;
static const gpio_num_t RS485_D   = GPIO_NUM_18;

// Inputs
static const gpio_num_t ES1     = GPIO_NUM_NC;
static const gpio_num_t ES2     = GPIO_NUM_NC;
static const gpio_num_t ES3     = GPIO_NUM_NC;
static const gpio_num_t ES4     = GPIO_NUM_NC;
static const gpio_num_t EM      = GPIO_NUM_NC;

static const gpio_num_t USTX    = GPIO_NUM_NC;
static const gpio_num_t PUA2    = GPIO_NUM_NC;

// Named ports

// array based
static const char         INPUT_NAME[][7]     = { "ES1",           "ES2",           "ES3",           "ES4",           "EM" };
static const gpio_num_t   INPUT_GPIO[]        = { ES1,             ES2,             ES3,             ES4,             EM };
static const int8_t       INPUT_ADC_UNIT[]    = { ADC_UNIT_1,      ADC_UNIT_1,      ADC_UNIT_1,      ADC_UNIT_1,      ADC_UNIT_1 };
static const int8_t       INPUT_ADC_CHANNEL[] = { ADC1_CHANNEL_0,  ADC1_CHANNEL_0,  ADC1_CHANNEL_0,  ADC1_CHANNEL_0,  ADC1_CHANNEL_0 };
static const adc_atten_t  INPUT_ATTENUATION[] = { ADC_ATTEN_DB_12, ADC_ATTEN_DB_12, ADC_ATTEN_DB_12, ADC_ATTEN_DB_12, ADC_ATTEN_DB_12 };
static const int8_t       INPUT_IOTYPE[]      = { DIGITALIO,       DIGITALIO,       DIGITALIO,       DIGITALIO,       DIGITALIO };

// Motor
static const gpio_num_t M1A = GPIO_NUM_NC;
static const gpio_num_t M1B = GPIO_NUM_NC;
static const gpio_num_t M2A = GPIO_NUM_NC;
static const gpio_num_t M2B = GPIO_NUM_NC;
static const gpio_num_t M3A = GPIO_NUM_NC;
static const gpio_num_t M3B = GPIO_NUM_NC;
static const gpio_num_t M4A = GPIO_NUM_NC;
static const gpio_num_t M4B = GPIO_NUM_NC;

static const char         MOTOR_NAME[][6] = { "STEP1",   "STEP2",   "STEP3",   "STEP4" };
static const gpio_num_t   MOTOR_GPIO[][2] = { {M1A,M1B}, {M2A,M2B}, {M3A,M3B}, {M4A,M4B} };
static const int8_t       MOTOR_IOTYPE[]  = { STEPPERIO, STEPPERIO, STEPPERIO, STEPPERIO };

// Servo
static const gpio_num_t SERVO1 = GPIO_NUM_NC;
static const gpio_num_t SERVO2 = GPIO_NUM_NC;
static const gpio_num_t SERVO3 = GPIO_NUM_NC;
static const gpio_num_t SERVO4 = GPIO_NUM_NC;

static const char         SERVO_NAME[][7] = { "SERVO1",   "SERVO2",   "SEROV3",   "SERVO4" };
static const gpio_num_t   SERVO_GPIO[]    = { SERVO1,     SERVO2,     SERVO3,     SERVO4 };

#endif /* Pins_Arduino_h */