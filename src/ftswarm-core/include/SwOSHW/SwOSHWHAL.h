/*
 * SwOSHWHAL.h
 *
 * some useful hardware abstraction
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include <driver/gpio.h>
#include <driver/adc.h>

#include "SwOS.h"

// handle different plattforms
#if CONFIG_IDF_TARGET_ESP32S3
  #define xGPIO_NUM_22    GPIO_NUM_NC
  #define xGPIO_NUM_25    GPIO_NUM_NC
  #define xADC1_CHANNEL_8 ADC1_CHANNEL_8
  #define xGPIO_NUM_40    GPIO_NUM_40
  #define xGPIO_NUM_41    GPIO_NUM_41
  #define xGPIO_NUM_42    GPIO_NUM_42
  #define xGPIO_NUM_45    GPIO_NUM_45
  #define xGPIO_NUM_46    GPIO_NUM_46
  #define xGPIO_NUM_47    GPIO_NUM_47
  #define xGPIO_NUM_48    GPIO_NUM_48
#else
  #define xGPIO_NUM_22    GPIO_NUM_22
  #define xGPIO_NUM_25    GPIO_NUM_25
  #define xADC1_CHANNEL_8 ADC1_CHANNEL_MAX
  #define xGPIO_NUM_40    GPIO_NUM_NC
  #define xGPIO_NUM_41    GPIO_NUM_NC
  #define xGPIO_NUM_42    GPIO_NUM_NC
  #define xGPIO_NUM_45    GPIO_NUM_NC
  #define xGPIO_NUM_46    GPIO_NUM_NC
  #define xGPIO_NUM_47    GPIO_NUM_NC
  #define xGPIO_NUM_48    GPIO_NUM_NC
  #define ADC1_CHANNEL_8  ADC1_CHANNEL_MAX
  #define ADC1_CHANNEL_9  ADC1_CHANNEL_MAX
#endif

typedef struct {
  gpio_num_t  io;
  adc_unit_t  adc_unit;
  int8_t      adc_channel;
  adc_atten_t attenuation;
} SwOSIODefinition_t;

typedef struct {
  uint8_t inputs;
  uint8_t motors;
  uint8_t rcservos;
  uint8_t servos;
  uint8_t pixels;
  uint8_t buttons;
  uint8_t joysticks;
  int8_t  pwrctl;
  uint8_t firstJPoti;
  bool    OLED;
  bool    HC165;
} SwOSMaxIO_t;

#define NOPWRCTL -1

const SwOSMaxIO_t MAXIOS[FTSWARMMAXVERSION] = {
                            //  inputs motors rcservos servos pixels, buttons, joysticks, pwrctl,   firstJPoti, OLED, HC165
  /* FTSWARMJST_1V0 */       {  4,     2,     0,       1,     2,      0,       0,         NOPWRCTL, 0,          false, false },
  /* FTSWARMCONTROL_1V3 */   {  4,     2,     0,       0,     0,      8,       2,         NOPWRCTL, 4,          true,  true },
  /* FTSWARMJST_1V15 */      {  4,     2,     0,       1,     2,      0,       0,         NOPWRCTL, 0,          false, false },
  /* FTSWARMRS_2V0 */        {  7,     2,     0,       2,     2,      0,       0,         6,        0,          false, false },
  /* FTSWARMRS_2V1 */        {  7,     2,     0,       2,     2,      0,       0,         6,        0,          false, false },
  /* FTSWARMCAM_3V12 */      {  5,     2,     0,       1,     2,      0,       0,         4,        0,          false, false }, 
  /* FTSWARMDUINO_1V14 */    {  0,     0,     0,       0,     2,      0,       0,         NOPWRCTL, 0,          false, false },
  /* FTSWARMPWRDRIVE_1V14 */ {  5,     4,     0,       0,     2,      0,       0,         NOPWRCTL, 0,          false, false },
  /* FTSWARMXL_1V00 */       {  8,     8,     0,       0,     2,      0,       0,         NOPWRCTL, 0,          false, false },
  /* FTSWARMRC_1V140 */      {  7,     4,     4,       0,     1,      0,       0,         6,        0,          false, false },
                          //  {  11,    4,     0,       0,     1,      0,       0,         6,        0,          false, false }, 
  /* FTSWARMCONTROL_1V3UC */ {  6,     2,     0,       0,     0,      8,       2,         11,       6,          true,  true },
};

const SwOSIODefinition_t GPIO_INPUT[FTSWARMMAXVERSION][MAXINPUTS] = {
    /* FTSWARMJST_1V0 */       { { GPIO_NUM_33,  ADC_UNIT_1, ADC1_CHANNEL_5,   ADC_ATTEN_DB_12 },
                                 { xGPIO_NUM_25, ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_26,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }, 
                                 { GPIO_NUM_27,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }, 
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }, 
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }, 
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }, 
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               },
    /* FTSWARMCONTROL_1V3 */   { { GPIO_NUM_39,  ADC_UNIT_1, ADC1_CHANNEL_3,   ADC_ATTEN_DB_12 },   
                                 { xGPIO_NUM_25, ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_26,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_27,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_33,  ADC_UNIT_1, ADC1_CHANNEL_5,   ADC_ATTEN_DB_12 }, // JOY1.LR
                                 { GPIO_NUM_36,  ADC_UNIT_1, ADC1_CHANNEL_0,   ADC_ATTEN_DB_12 }, // JOY1.FB
                                 { GPIO_NUM_32,  ADC_UNIT_1, ADC1_CHANNEL_4,   ADC_ATTEN_DB_12 }, // JOY2.LR
                                 { GPIO_NUM_34,  ADC_UNIT_1, ADC1_CHANNEL_6,   ADC_ATTEN_DB_12 }, // JOY2.FB
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                                },
    /* FTSWARMJST_1V15 */      { { GPIO_NUM_39,  ADC_UNIT_1, ADC1_CHANNEL_3,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_32,  ADC_UNIT_1, ADC1_CHANNEL_4,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_33,  ADC_UNIT_1, ADC1_CHANNEL_5,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_34,  ADC_UNIT_1, ADC1_CHANNEL_6,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               },
    
    /* FTSWARMRS_2V0 */        { { GPIO_NUM_1,   ADC_UNIT_1, ADC1_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_2,   ADC_UNIT_1, ADC1_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_8,   ADC_UNIT_1, ADC1_CHANNEL_7,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_9,   ADC_UNIT_1, ADC1_CHANNEL_8,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_11,  ADC_UNIT_2, ADC2_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_13,  ADC_UNIT_2, ADC2_CHANNEL_2,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_12,  ADC_UNIT_2, ADC2_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               },

    /* FTSWARMRS_2V1 */        { { GPIO_NUM_1,   ADC_UNIT_1, ADC1_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_2,   ADC_UNIT_1, ADC1_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_19,  ADC_UNIT_2, ADC2_CHANNEL_8,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_20,  ADC_UNIT_2, ADC2_CHANNEL_9,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_11,  ADC_UNIT_2, ADC2_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_13,  ADC_UNIT_2, ADC2_CHANNEL_2,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_12,  ADC_UNIT_2, ADC2_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               },
    
    /* FTSWARMCAM_3V12 */      { { GPIO_NUM_8,   ADC_UNIT_1, ADC1_CHANNEL_7,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_9,   ADC_UNIT_1, ADC1_CHANNEL_8,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_10,  ADC_UNIT_2, ADC1_CHANNEL_9,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_11,  ADC_UNIT_2, ADC2_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_6,   ADC_UNIT_1, ADC1_CHANNEL_5,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               },

    /* FTSWARMDUINO_1V14 */    { { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               },

    /* FTSWARMPWRDRIVE_1V14 */ { { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }, 
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               },
 
    /* FTSWARMXL_1V00 */       { { GPIO_NUM_1,   ADC_UNIT_1, ADC1_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_2,   ADC_UNIT_1, ADC1_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_3,   ADC_UNIT_1, ADC1_CHANNEL_2,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_4,   ADC_UNIT_1, ADC1_CHANNEL_3,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_5,   ADC_UNIT_1, ADC1_CHANNEL_4,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_8,   ADC_UNIT_1, ADC1_CHANNEL_7,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_10,  ADC_UNIT_1, ADC1_CHANNEL_9,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_9,   ADC_UNIT_1, ADC1_CHANNEL_8,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               },

    /* FTSWARMRC_1V140 */      { { GPIO_NUM_1,   ADC_UNIT_1, ADC1_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_2,   ADC_UNIT_1, ADC1_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_19,  ADC_UNIT_2, ADC2_CHANNEL_8,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_20,  ADC_UNIT_2, ADC2_CHANNEL_9,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_11,  ADC_UNIT_2, ADC2_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_13,  ADC_UNIT_2, ADC2_CHANNEL_2,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_12,  ADC_UNIT_2, ADC2_CHANNEL_1,   ADC_ATTEN_DB_12 },  // PwrControl
                                 { GPIO_NUM_6,   ADC_UNIT_1, ADC1_CHANNEL_5,   ADC_ATTEN_DB_2_5},  // RC-Servo1 
                                 { GPIO_NUM_7,   ADC_UNIT_1, ADC1_CHANNEL_6,   ADC_ATTEN_DB_2_5},  // RC-Servo2
                                 { GPIO_NUM_8,   ADC_UNIT_1, ADC1_CHANNEL_7,   ADC_ATTEN_DB_2_5},  // RC-Servo3
                                 { GPIO_NUM_9,   ADC_UNIT_1, ADC1_CHANNEL_8,   ADC_ATTEN_DB_2_5}   // RC-Servo4
                               },

    /* FTSWARMCONTROL_1V3UC */ { { GPIO_NUM_1,   ADC_UNIT_1, ADC1_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_2,   ADC_UNIT_1, ADC1_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_19,  ADC_UNIT_2, ADC2_CHANNEL_8,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_20,  ADC_UNIT_2, ADC2_CHANNEL_9,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_11,  ADC_UNIT_2, ADC2_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_13,  ADC_UNIT_2, ADC2_CHANNEL_2,   ADC_ATTEN_DB_12 },
                                 { GPIO_NUM_6,   ADC_UNIT_1, ADC1_CHANNEL_5,   ADC_ATTEN_DB_12 }, // JOY1.LR
                                 { GPIO_NUM_7,   ADC_UNIT_1, ADC1_CHANNEL_6,   ADC_ATTEN_DB_12 }, // JOY1.FB
                                 { GPIO_NUM_17,  ADC_UNIT_2, ADC2_CHANNEL_6,   ADC_ATTEN_DB_12 }, // JOY2.LR
                                 { GPIO_NUM_18,  ADC_UNIT_2, ADC2_CHANNEL_7,   ADC_ATTEN_DB_12 }, // JOY2.FB
                                 { GPIO_NUM_12,  ADC_UNIT_2, ADC2_CHANNEL_1,   ADC_ATTEN_DB_12 }
                               }


                              }; 

  /* USTX, PUA */
const gpio_num_t USTCPUA[FTSWARMMAXVERSION][2] = 
  { /* FTSWARMJST_1V0 */       { GPIO_NUM_NC,  GPIO_NUM_NC},
    /* FTSWARMCONTROL_1V3 */   { GPIO_NUM_15,  GPIO_NUM_14},
    /* FTSWARMJST_1V15 */      { GPIO_NUM_15,  GPIO_NUM_14},
    /* FTSWARMRS_2V0 */        { xGPIO_NUM_42, xGPIO_NUM_41},
    /* FTSWARMRS_2V1 */        { xGPIO_NUM_42, xGPIO_NUM_41},
    /* FTSWARMCAM_3V12 */      { GPIO_NUM_3,   GPIO_NUM_NC},
    /* FTSWARMDUINO_1V14 */    { GPIO_NUM_NC,  GPIO_NUM_NC},
    /* FTSWARMPWRDRIVE_1V14 */ { GPIO_NUM_NC,  GPIO_NUM_NC},
    /* FTSWARMXL_1V00 */       { GPIO_NUM_NC,  GPIO_NUM_NC},
    /* FTSWARMRC_1V140 */      { xGPIO_NUM_42, xGPIO_NUM_41},
    /* FTSWARMCONTROL_1V3UC */ { xGPIO_NUM_42, xGPIO_NUM_41}
  };   

const gpio_num_t GPIO_ACTOR[FTSWARMMAXVERSION][8][2] = 
  { /* FTSWARMJST_1V0 */       { { GPIO_NUM_13,  GPIO_NUM_4  }, { GPIO_NUM_2,   GPIO_NUM_0},   { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC},   { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC},  { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC} },
    /* FTSWARMCONTROL_1V3 */   { { GPIO_NUM_13,  GPIO_NUM_4  }, { GPIO_NUM_2,   GPIO_NUM_0},   { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC},   { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC},  { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC} },
    /* FTSWARMJST_1V15 */      { { GPIO_NUM_13,  GPIO_NUM_4  }, { GPIO_NUM_2,   GPIO_NUM_0},   { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC},   { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC},  { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC} },
    /* FTSWARMRS_2V0 */        { { GPIO_NUM_14,  GPIO_NUM_21 }, { xGPIO_NUM_45, xGPIO_NUM_46}, { GPIO_NUM_NC, GPIO_NUM_8},  { GPIO_NUM_NC, GPIO_NUM_9},    { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC},  { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC} },
    /* FTSWARMRS_2V1 */        { { GPIO_NUM_14,  GPIO_NUM_21 }, { xGPIO_NUM_45, xGPIO_NUM_46}, { GPIO_NUM_NC, GPIO_NUM_8},  { GPIO_NUM_NC, GPIO_NUM_9},    { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC},  { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC} },
    /* FTSWARMCAM_3V12 */      { { xGPIO_NUM_40, xGPIO_NUM_41}, { GPIO_NUM_1,   GPIO_NUM_2},   { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC},   { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC},  { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC} },
    /* FTSWARMDUINO_1V14*/     { { GPIO_NUM_NC,  GPIO_NUM_NC},  { GPIO_NUM_NC,  GPIO_NUM_NC},  { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC},   { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC},  { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC} },
    /* FTSWARMPWRDRIVE_1V14*/  { { GPIO_NUM_NC,  GPIO_NUM_NC},  { GPIO_NUM_NC,  GPIO_NUM_NC},  { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC},   { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC},  { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC} },
    /* FTSWARMXL_1V00*/        { { GPIO_NUM_16,  GPIO_NUM_15},  { GPIO_NUM_14,  GPIO_NUM_13},  { GPIO_NUM_11, GPIO_NUM_12}, { xGPIO_NUM_42, xGPIO_NUM_41}, { xGPIO_NUM_40, GPIO_NUM_39}, { GPIO_NUM_38, GPIO_NUM_37}, { GPIO_NUM_19, GPIO_NUM_20}, { GPIO_NUM_35, GPIO_NUM_36} },
    /* FTSWARMRC_1V140 */      { { GPIO_NUM_14,  GPIO_NUM_21 }, { xGPIO_NUM_45, xGPIO_NUM_46}, { GPIO_NUM_15, GPIO_NUM_17}, { GPIO_NUM_18, xGPIO_NUM_47},  { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC},  { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC} },
    /* FTSWARMCONTROL_1V3UC */ { { xGPIO_NUM_45, xGPIO_NUM_46}, { GPIO_NUM_14,  GPIO_NUM_21 }, { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC},   { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC},  { GPIO_NUM_NC, GPIO_NUM_NC}, { GPIO_NUM_NC, GPIO_NUM_NC} }
  };   

const gpio_num_t GPIO_SERVO[FTSWARMMAXVERSION][4] = 
  { /* FTSWARMJST_1V0 */       { xGPIO_NUM_25,  GPIO_NUM_13, GPIO_NUM_12, GPIO_NUM_NC},
    /* FTSWARMCONTROL_1V3 */   { GPIO_NUM_NC,   GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC},
    /* FTSWARMJST_1V15 */      { xGPIO_NUM_25,  GPIO_NUM_13, GPIO_NUM_12, GPIO_NUM_NC},
    /* FTSWARMRS_2V0 */        { xGPIO_NUM_47,  GPIO_NUM_8,  GPIO_NUM_9, GPIO_NUM_NC},
    /* FTSWARMRS_2V1 */        { xGPIO_NUM_47,  GPIO_NUM_15, GPIO_NUM_8,  GPIO_NUM_9},
    /* FTSWARMCAM_3V12 */      { xGPIO_NUM_47,  GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC},
    /* FTSWARMDUINO_1V14 */    { GPIO_NUM_NC,   GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC},
    /* FTSWARMPWRDRIVE_1V14 */ { GPIO_NUM_NC,   GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC},
    /* FTSWARMXL_1V00 */       { GPIO_NUM_33,   GPIO_NUM_21, GPIO_NUM_NC, GPIO_NUM_NC},
    /* FTSWARMRC_1V140 */      { GPIO_NUM_NC,   GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC},
    /* FTSWARMCONTROL_1V3UC */ { GPIO_NUM_NC,   GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC}
  };

const bool HASEXTPORT[FTSWARMMAXVERSION] = {
  /* FTSWARMJST_1V0 */       true,
  /* FTSWARMCONTROL_1V3 */   false,
  /* FTSWARMJST_1V15 */      true,
  /* FTSWARMRS_2V0 */        true,
  /* FTSWARMRS_2V1 */        true,
  /* FTSWARMCAM_3V12 */      false, 
  /* FTSWARMDUINO_1V14 */    false,
  /* FTSWARMPWRDRIVE_1V14 */ false,
  /* FTSWARMXL_1V00 */       true,
  /* FTSWARMRC_1V140 */      false,
  /* FTSWARMRC_1V140 */      false
};

const gpio_num_t GPIO_I2C[FTSWARMMAXVERSION][2][2] = {
  /* FTSWARMJST_1V0 */       { { GPIO_NUM_13, GPIO_NUM_12 },    // External
                               { GPIO_NUM_NC, GPIO_NUM_NC } },  // Internal
  /* FTSWARMCONTROL_1V3 */   { { GPIO_NUM_21, xGPIO_NUM_22 }, 
                               { GPIO_NUM_NC, GPIO_NUM_NC } },
  /* FTSWARMJST_1V15 */      { { GPIO_NUM_21, xGPIO_NUM_22 }, 
                               { GPIO_NUM_NC, GPIO_NUM_NC } },
  /* FTSWARMRS_2V0 */        { { GPIO_NUM_8,  GPIO_NUM_9  }, 
                               { GPIO_NUM_NC, GPIO_NUM_NC } },
  /* FTSWARMRS_2V1 */        { { GPIO_NUM_8,  GPIO_NUM_9  }, 
                               { GPIO_NUM_NC, GPIO_NUM_NC } },
  /* FTSWARMCAM_3V12 */      { { GPIO_NUM_NC, GPIO_NUM_NC }, 
                               { GPIO_NUM_NC, GPIO_NUM_NC } }, 
  /* FTSWARMDUINO_1V14 */    { { GPIO_NUM_NC, GPIO_NUM_NC }, 
                               { GPIO_NUM_5,  GPIO_NUM_4  } }, 
  /* FTSWARMPWRDRIVE_1V14 */ { { GPIO_NUM_NC, GPIO_NUM_NC },
                               { GPIO_NUM_5,  GPIO_NUM_4 } }, 
  /* FTSWARMXL_1V00 */       { { GPIO_NUM_33, GPIO_NUM_21 }, 
                               { GPIO_NUM_NC, GPIO_NUM_NC } },
  /* FTSWARMRC_1V140 */      { { GPIO_NUM_NC, GPIO_NUM_NC }, 
                               { GPIO_NUM_NC, GPIO_NUM_NC } },
  /* FTSWARMCONTROL_1V3UC */ { { GPIO_NUM_4,  GPIO_NUM_5  }, 
                               { GPIO_NUM_NC, GPIO_NUM_NC } },
};