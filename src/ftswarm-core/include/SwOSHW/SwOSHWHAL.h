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
  #define xGPIO_NUM_40    GPIO_NUM_40
  #define xGPIO_NUM_47    GPIO_NUM_47
  #define xGPIO_NUM_48    GPIO_NUM_48
#else
  #define xGPIO_NUM_40    GPIO_NUM_NC
  #define xGPIO_NUM_47    GPIO_NUM_NC
  #define xGPIO_NUM_48    GPIO_NUM_NC
#endif

typedef struct {
  char         name[7];
  SwOSIOType_t ioType;
  gpio_num_t   io;
  adc_unit_t   adc_unit;
  int8_t       adc_channel;
  adc_atten_t  attenuation;
} SwOSHALInput_t;

typedef struct {
  char         name[7];
  SwOSIOType_t ioType;
  uint8_t      port;
  gpio_num_t   io1, io2;
} SwOSHALActor_t;

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

#ifdef BOARD_FTSWARM_JST

  // FTSWARMJST_1V0 only, no support for FTSWARMJST_1V0

                            //  inputs motors rcservos servos pixels, buttons, joysticks, pwrctl,   firstJPoti, OLED, HC165
  const SwOSMaxIO_t MAXIOS = {  4,     2,     0,       1,     2,      0,       0,         NOPWRCTL, 0,          false, false };

  const SwOSHALInput_t HAL_INPUT[MAXINPUTS] = 
                               { { "A1",     SWOSIO_DIGITAL, GPIO_NUM_39,  ADC_UNIT_1, ADC1_CHANNEL_3,   ADC_ATTEN_DB_12 },
                                 { "A2",     SWOSIO_DIGITAL, GPIO_NUM_32,  ADC_UNIT_1, ADC1_CHANNEL_4,   ADC_ATTEN_DB_12 },
                                 { "A3",     SWOSIO_DIGITAL, GPIO_NUM_33,  ADC_UNIT_1, ADC1_CHANNEL_5,   ADC_ATTEN_DB_12 },
                                 { "A4",     SWOSIO_DIGITAL, GPIO_NUM_34,  ADC_UNIT_1, ADC1_CHANNEL_6,   ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               };

  const gpio_num_t USTCPUA[2] = { GPIO_NUM_15,  GPIO_NUM_14}; 

  const SwOSHALActor_t HAL_ACTOR[MAXACTORS] =
                               { { "M1",    SWOSIO_MOTOR,   0,           GPIO_NUM_13, GPIO_NUM_4}, 
                                 { "M2",    SWOSIO_MOTOR,   1,           GPIO_NUM_2,  GPIO_NUM_0},   
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},   
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},  
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}
                               };

  const gpio_num_t GPIO_SERVO[4] = { GPIO_NUM_25,  GPIO_NUM_13, GPIO_NUM_12, GPIO_NUM_NC};

  const bool HASEXTPORT = true;

  const gpio_num_t GPIO_I2C[2][2] = { { GPIO_NUM_21, GPIO_NUM_22 }, // External
                                      { GPIO_NUM_NC, GPIO_NUM_NC }  // Internal
                                    };
 
#elif  defined( BOARD_FTSWARM_USBMICRO )

                            //  inputs motors rcservos servos pixels, buttons, joysticks, pwrctl,   firstJPoti, OLED, HC165
  const SwOSMaxIO_t MAXIOS = {  8,     2,     0,       0,     0,      8,       2,         NOPWRCTL, 4,          true,  true };
  
  const SwOSHALInput_t HAL_INPUT[MAXINPUTS] = 
                               { { "A1",     SWOSIO_DIGITAL, GPIO_NUM_39,  ADC_UNIT_1, ADC1_CHANNEL_3,   ADC_ATTEN_DB_12 },   
                                 { "A2",     SWOSIO_DIGITAL, GPIO_NUM_25, ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "A3",     SWOSIO_DIGITAL, GPIO_NUM_26,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "A4",     SWOSIO_DIGITAL, GPIO_NUM_27,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "JOY1LR", SWOSIO_ANALOG,  GPIO_NUM_33,  ADC_UNIT_1, ADC1_CHANNEL_5,   ADC_ATTEN_DB_12 }, // JOY1.LR
                                 { "JOY1FB", SWOSIO_ANALOG,  GPIO_NUM_36,  ADC_UNIT_1, ADC1_CHANNEL_0,   ADC_ATTEN_DB_12 }, // JOY1.FB
                                 { "JOY2LR", SWOSIO_ANALOG,  GPIO_NUM_32,  ADC_UNIT_1, ADC1_CHANNEL_4,   ADC_ATTEN_DB_12 }, // JOY2.LR
                                 { "JOY2FB", SWOSIO_UNDEF,   GPIO_NUM_34,  ADC_UNIT_1, ADC1_CHANNEL_6,   ADC_ATTEN_DB_12 }, // JOY2.FB
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                                };

  const gpio_num_t USTCPUA[2] = { GPIO_NUM_15,  GPIO_NUM_14};

  const SwOSHALActor_t HAL_ACTOR[MAXACTORS] = 
                               { { "M1",    SWOSIO_MOTOR,   0,           GPIO_NUM_13, GPIO_NUM_4}, 
                                 { "M2",    SWOSIO_MOTOR,   1,           GPIO_NUM_2,  GPIO_NUM_0},   
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},   
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},  
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}
                                };

  const gpio_num_t GPIO_SERVO[4] = { GPIO_NUM_NC,   GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC};

  const bool HASEXTPORT = false;

  const gpio_num_t GPIO_I2C[2][2] = { { GPIO_NUM_21, GPIO_NUM_22 }, // External
                                      { GPIO_NUM_NC, GPIO_NUM_NC }  // Internal
                                    };

#elif  defined( BOARD_FTSWARM_RS )

  // FTSWARMRS_2V1 only, no support for FTSWARMRS_2V0

                            //  inputs motors rcservos servos pixels, buttons, joysticks, pwrctl,   firstJPoti, OLED, HC165
  const SwOSMaxIO_t MAXIOS = {  7,     2,     0,       2,     2,      0,       0,         6,        0,          false, false };
  
  const SwOSHALInput_t HAL_INPUT[MAXINPUTS] = 
                               { { "A1",     SWOSIO_DIGITAL, GPIO_NUM_1,   ADC_UNIT_1, ADC1_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { "A2",     SWOSIO_DIGITAL, GPIO_NUM_2,   ADC_UNIT_1, ADC1_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { "A3",     SWOSIO_DIGITAL, GPIO_NUM_19,  ADC_UNIT_2, ADC2_CHANNEL_8,   ADC_ATTEN_DB_12 },
                                 { "A4",     SWOSIO_DIGITAL, GPIO_NUM_20,  ADC_UNIT_2, ADC2_CHANNEL_9,   ADC_ATTEN_DB_12 },
                                 { "A5",     SWOSIO_DIGITAL, GPIO_NUM_11,  ADC_UNIT_2, ADC2_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { "A6",     SWOSIO_DIGITAL, GPIO_NUM_13,  ADC_UNIT_2, ADC2_CHANNEL_2,   ADC_ATTEN_DB_12 },
                                 { "PWRCTL", SWOSIO_POWER,   GPIO_NUM_12,  ADC_UNIT_2, ADC2_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               };

  const gpio_num_t USTCPUA[2] = { GPIO_NUM_42, GPIO_NUM_41};

  const SwOSHALActor_t HAL_ACTOR[MAXACTORS] = 
                               { { "M1",    SWOSIO_MOTOR,   0,           GPIO_NUM_14, GPIO_NUM_21 }, 
                                 { "M2",    SWOSIO_MOTOR,   1,           GPIO_NUM_45, GPIO_NUM_46}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},   
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},  
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}
                               };

  const gpio_num_t GPIO_SERVO[4] = { GPIO_NUM_47,  GPIO_NUM_15, GPIO_NUM_8,  GPIO_NUM_9};

  const bool HASEXTPORT = true;

  const gpio_num_t GPIO_I2C[2][2] = { { GPIO_NUM_8,  GPIO_NUM_9 }, // External
                                      { GPIO_NUM_NC, GPIO_NUM_NC }  // Internal
                                    };

#elif  defined( BOARD_FTSWARM_CAM )

                            //  inputs motors rcservos servos pixels, buttons, joysticks, pwrctl,   firstJPoti, OLED, HC165
  const SwOSMaxIO_t MAXIOS = {  5,     2,     0,       1,     2,      0,       0,         4,        0,          false, false };
  
  const SwOSHALInput_t HAL_INPUT[MAXINPUTS] =
                               { { "A1",     SWOSIO_DIGITAL, GPIO_NUM_8,   ADC_UNIT_1, ADC1_CHANNEL_7,   ADC_ATTEN_DB_12 },
                                 { "A2",     SWOSIO_DIGITAL, GPIO_NUM_9,   ADC_UNIT_1, ADC1_CHANNEL_8,   ADC_ATTEN_DB_12 },
                                 { "A3",     SWOSIO_DIGITAL, GPIO_NUM_10,  ADC_UNIT_2, ADC1_CHANNEL_9,   ADC_ATTEN_DB_12 },
                                 { "A4",     SWOSIO_DIGITAL, GPIO_NUM_11,  ADC_UNIT_2, ADC2_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { "A5",     SWOSIO_DIGITAL, GPIO_NUM_6,   ADC_UNIT_1, ADC1_CHANNEL_5,   ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               };

  const gpio_num_t USTCPUA[2] = { GPIO_NUM_3,   GPIO_NUM_NC};

  const SwOSHALActor_t HAL_ACTOR[MAXACTORS] = 
                               { { "M1",    SWOSIO_MOTOR,   0,           GPIO_NUM_40, GPIO_NUM_41}, 
                                 { "M2",    SWOSIO_MOTOR,   1,           GPIO_NUM_1,  GPIO_NUM_2},   
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},   
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},  
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}
                               };

  const gpio_num_t GPIO_SERVO[4] = { GPIO_NUM_47,  GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC};

  const bool HASEXTPORT = false;

  const gpio_num_t GPIO_I2C[2][2] = { { GPIO_NUM_NC, GPIO_NUM_NC }, // External
                                      { GPIO_NUM_NC, GPIO_NUM_NC }  // Internal
                                    };

#elif  defined( BOARD_FTSWARM_DUINO )

                            //  inputs motors rcservos servos pixels, buttons, joysticks, pwrctl,   firstJPoti, OLED, HC165
  const SwOSMaxIO_t MAXIOS = {  12,    0,     0,       0,     2,      0,       0,         NOPWRCTL, 0,          false, false };
  
  const SwOSHALInput_t HAL_INPUT[MAXINPUTS] =
                               { { "I1",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "I2",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "I3",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "I4",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "I5",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "I6",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "I7",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "I8",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "C1",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "C2",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "C3",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "C4",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               };

  const gpio_num_t USTCPUA[2] = { GPIO_NUM_NC,  GPIO_NUM_NC};

  const SwOSHALActor_t HAL_ACTOR[MAXACTORS] = 
                               { { "M1",    SWOSIO_MOTOR,   0,           GPIO_NUM_NC, GPIO_NUM_NC},  
                                 { "M2",    SWOSIO_MOTOR,   1,           GPIO_NUM_NC, GPIO_NUM_NC},
                                 { "M3",    SWOSIO_MOTOR,   2,           GPIO_NUM_NC, GPIO_NUM_NC},
                                 { "M4",    SWOSIO_MOTOR,   3,           GPIO_NUM_NC, GPIO_NUM_NC},
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},  
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}
                               };

  const gpio_num_t GPIO_SERVO[4] = { GPIO_NUM_NC,   GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC};

  const bool HASEXTPORT = false;

  const gpio_num_t GPIO_I2C[2][2] = { { GPIO_NUM_NC, GPIO_NUM_NC }, // External
                                      { GPIO_NUM_5,  GPIO_NUM_4  }  // Internal
                                    };

#elif  defined( BOARD_FTSWARM_PWRDRIVE )

                            //  inputs motors rcservos servos pixels, buttons, joysticks, pwrctl,   firstJPoti, OLED, HC165
  const SwOSMaxIO_t MAXIOS = {  12,    0,     0,       0,     2,      0,       0,         NOPWRCTL, 0,          false, false };
  
  const SwOSHALInput_t HAL_INPUT[MAXINPUTS] =
                               { { "ES1",    SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }, 
                                 { "ES2",    SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "ES3",    SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "ES4",    SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "EM",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               };

  const gpio_num_t USTCPUA[2] = { GPIO_NUM_NC,  GPIO_NUM_NC};

  const SwOSHALActor_t HAL_ACTOR[MAXACTORS] =     
                               { { "STEP1", SWOSIO_STEPPER, 0,           GPIO_NUM_NC, GPIO_NUM_NC},  
                                 { "STEP2", SWOSIO_STEPPER, 1,           GPIO_NUM_NC, GPIO_NUM_NC},
                                 { "STEP3", SWOSIO_STEPPER, 2,           GPIO_NUM_NC, GPIO_NUM_NC},
                                 { "STEP4", SWOSIO_STEPPER, 3,           GPIO_NUM_NC, GPIO_NUM_NC},
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},  
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}
                               };

  const gpio_num_t GPIO_SERVO[4] = { GPIO_NUM_NC,   GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC};

  const bool HASEXTPORT = false;

  const gpio_num_t GPIO_I2C[2][2] = { { GPIO_NUM_NC, GPIO_NUM_NC }, // External
                                      { GPIO_NUM_5,  GPIO_NUM_4  }  // Internal
                                    };

#elif  defined( BOARD_FTSWARM_XL )

                            //  inputs motors rcservos servos pixels, buttons, joysticks, pwrctl,   firstJPoti, OLED, HC165
  const SwOSMaxIO_t MAXIOS = {  8,     8,     0,       0,     2,      0,       0,         NOPWRCTL, 0,          false, false };
  
  const SwOSHALInput_t HAL_INPUT[MAXINPUTS] =
                               { { "A1",     SWOSIO_DIGITAL, GPIO_NUM_1,   ADC_UNIT_1, ADC1_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { "A2",     SWOSIO_DIGITAL, GPIO_NUM_2,   ADC_UNIT_1, ADC1_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { "A3",     SWOSIO_DIGITAL, GPIO_NUM_3,   ADC_UNIT_1, ADC1_CHANNEL_2,   ADC_ATTEN_DB_12 },
                                 { "A4",     SWOSIO_DIGITAL, GPIO_NUM_4,   ADC_UNIT_1, ADC1_CHANNEL_3,   ADC_ATTEN_DB_12 },
                                 { "A5",     SWOSIO_DIGITAL, GPIO_NUM_5,   ADC_UNIT_1, ADC1_CHANNEL_4,   ADC_ATTEN_DB_12 },
                                 { "A6",     SWOSIO_DIGITAL, GPIO_NUM_8,   ADC_UNIT_1, ADC1_CHANNEL_7,   ADC_ATTEN_DB_12 },
                                 { "A7",     SWOSIO_DIGITAL, GPIO_NUM_10,  ADC_UNIT_1, ADC1_CHANNEL_9,   ADC_ATTEN_DB_12 },
                                 { "A8",     SWOSIO_DIGITAL, GPIO_NUM_9,   ADC_UNIT_1, ADC1_CHANNEL_8,   ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               };

  const gpio_num_t USTCPUA[2] = { GPIO_NUM_NC,  GPIO_NUM_NC};

  const SwOSHALActor_t HAL_ACTOR[MAXACTORS] = 
                               { { "M1",    SWOSIO_MOTOR,   0,           GPIO_NUM_16, GPIO_NUM_15},  
                                 { "M2",    SWOSIO_MOTOR,   1,           GPIO_NUM_14, GPIO_NUM_13},
                                 { "M3",    SWOSIO_MOTOR,   2,           GPIO_NUM_11, GPIO_NUM_12}, 
                                 { "M4",    SWOSIO_MOTOR,   3,           GPIO_NUM_42, GPIO_NUM_41}, 
                                 { "M5",    SWOSIO_MOTOR,   4,           GPIO_NUM_40, GPIO_NUM_39}, 
                                 { "M6",    SWOSIO_MOTOR,   5,           GPIO_NUM_38, GPIO_NUM_37}, 
                                 { "M7",    SWOSIO_MOTOR,   6,           GPIO_NUM_19, GPIO_NUM_20}, 
                                 { "M8",    SWOSIO_MOTOR,   7,           GPIO_NUM_35, GPIO_NUM_36}
                               };

  const gpio_num_t GPIO_SERVO[4] = { GPIO_NUM_33,   GPIO_NUM_21, GPIO_NUM_NC, GPIO_NUM_NC};

  const bool HASEXTPORT = true;

  const gpio_num_t GPIO_I2C[2][2] = { { GPIO_NUM_33, GPIO_NUM_21 }, // External
                                      { GPIO_NUM_NC,  GPIO_NUM_NC }  // Internal
                                    };

#elif  defined( BOARD_FTSWARM_RC )

                            //  inputs motors rcservos servos pixels, buttons, joysticks, pwrctl,   firstJPoti, OLED, HC165
  const SwOSMaxIO_t MAXIOS = {  12,    4,     4,       0,     1,      0,       0,         6,        0,          false, false };

  // RC-Servo Debugging
  // const SwOSMaxIO_t MAXIOS = {  12,    4,     0,       0,     1,      0,       0,         6,        0,          false, false }

  const SwOSHALInput_t HAL_INPUT[MAXINPUTS] =
                               { { "A1",     SWOSIO_DIGITAL, GPIO_NUM_1,   ADC_UNIT_1, ADC1_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { "A2",     SWOSIO_DIGITAL, GPIO_NUM_2,   ADC_UNIT_1, ADC1_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { "A3",     SWOSIO_DIGITAL, GPIO_NUM_19,  ADC_UNIT_2, ADC2_CHANNEL_8,   ADC_ATTEN_DB_12 },
                                 { "A4",     SWOSIO_DIGITAL, GPIO_NUM_20,  ADC_UNIT_2, ADC2_CHANNEL_9,   ADC_ATTEN_DB_12 },
                                 { "A5",     SWOSIO_DIGITAL, GPIO_NUM_11,  ADC_UNIT_2, ADC2_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { "A6",     SWOSIO_DIGITAL, GPIO_NUM_13,  ADC_UNIT_2, ADC2_CHANNEL_2,   ADC_ATTEN_DB_12 },
                                 { "S1",     SWOSIO_DIGITAL, GPIO_NUM_16,  ADC_UNIT_2, ADC2_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { "PWRCTL", SWOSIO_POWER,   GPIO_NUM_12,  ADC_UNIT_2, ADC2_CHANNEL_1,   ADC_ATTEN_DB_12 },  // PwrControl
                                 { "RCP1",   SWOSIO_ANALOG,  GPIO_NUM_6,   ADC_UNIT_1, ADC1_CHANNEL_5,   ADC_ATTEN_DB_2_5},  // RC-Servo1 
                                 { "RCP2",   SWOSIO_ANALOG,  GPIO_NUM_7,   ADC_UNIT_1, ADC1_CHANNEL_6,   ADC_ATTEN_DB_2_5},  // RC-Servo2
                                 { "RCP3",   SWOSIO_ANALOG,  GPIO_NUM_8,   ADC_UNIT_1, ADC1_CHANNEL_7,   ADC_ATTEN_DB_2_5},  // RC-Servo3
                                 { "RCP4",   SWOSIO_ANALOG,  GPIO_NUM_9,   ADC_UNIT_1, ADC1_CHANNEL_8,   ADC_ATTEN_DB_2_5}   // RC-Servo4
                               };

  const gpio_num_t USTCPUA[2] = { GPIO_NUM_42, GPIO_NUM_41};

  const SwOSHALActor_t HAL_ACTOR[MAXACTORS] = 
                               { { "M1",    SWOSIO_MOTOR,   0,           GPIO_NUM_14,  GPIO_NUM_21 }, 
                                 { "M1",    SWOSIO_MOTOR,   1,           GPIO_NUM_45, GPIO_NUM_46}, 
                                 { "M1",    SWOSIO_MOTOR,   2,           GPIO_NUM_15, GPIO_NUM_17}, 
                                 { "M1",    SWOSIO_MOTOR,   3,           GPIO_NUM_18, GPIO_NUM_47},
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},  
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC} 
                               };

  const gpio_num_t GPIO_SERVO[4] = { GPIO_NUM_NC,   GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC};

  const bool HASEXTPORT = false;

  const gpio_num_t GPIO_I2C[2][2] = { { GPIO_NUM_NC, GPIO_NUM_NC }, // External
                                      { GPIO_NUM_NC, GPIO_NUM_NC }  // Internal
                                    };

#elif  defined( BOARD_FTSWARM_USBC )

                            //  inputs motors rcservos servos pixels, buttons, joysticks, pwrctl,   firstJPoti, OLED, HC165
  const SwOSMaxIO_t MAXIOS = {  10,    2,     0,       0,     0,      8,       2,         11,       6,          true,  true };
  
  const SwOSHALInput_t HAL_INPUT[MAXINPUTS] =
                               { { "A1",     SWOSIO_DIGITAL, GPIO_NUM_1,   ADC_UNIT_1, ADC1_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { "A2",     SWOSIO_DIGITAL, GPIO_NUM_2,   ADC_UNIT_1, ADC1_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { "A3",     SWOSIO_DIGITAL, GPIO_NUM_19,  ADC_UNIT_2, ADC2_CHANNEL_8,   ADC_ATTEN_DB_12 },
                                 { "A4",     SWOSIO_DIGITAL, GPIO_NUM_20,  ADC_UNIT_2, ADC2_CHANNEL_9,   ADC_ATTEN_DB_12 },
                                 { "A5",     SWOSIO_DIGITAL, GPIO_NUM_11,  ADC_UNIT_2, ADC2_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { "A6",     SWOSIO_DIGITAL, GPIO_NUM_13,  ADC_UNIT_2, ADC2_CHANNEL_2,   ADC_ATTEN_DB_12 },
                                 { "JOY1LR", SWOSIO_ANALOG,  GPIO_NUM_6,   ADC_UNIT_1, ADC1_CHANNEL_5,   ADC_ATTEN_DB_12 }, // JOY1.LR
                                 { "JOY1FB", SWOSIO_ANALOG,  GPIO_NUM_7,   ADC_UNIT_1, ADC1_CHANNEL_6,   ADC_ATTEN_DB_12 }, // JOY1.FB
                                 { "JOY2LR", SWOSIO_ANALOG,  GPIO_NUM_17,  ADC_UNIT_2, ADC2_CHANNEL_6,   ADC_ATTEN_DB_12 }, // JOY2.LR
                                 { "JOY2FB", SWOSIO_ANALOG,  GPIO_NUM_18,  ADC_UNIT_2, ADC2_CHANNEL_7,   ADC_ATTEN_DB_12 }, // JOY2.FB
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_12,  ADC_UNIT_2, ADC2_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_12,  ADC_UNIT_2, ADC2_CHANNEL_1,   ADC_ATTEN_DB_12 }
                               };

  const gpio_num_t USTCPUA[2] = { GPIO_NUM_42, GPIO_NUM_41};

  const SwOSHALActor_t HAL_ACTOR[MAXACTORS] = 
                               { { "M1",    SWOSIO_MOTOR,   0,           GPIO_NUM_45, GPIO_NUM_46}, 
                                 { "M1",    SWOSIO_MOTOR,   1,           GPIO_NUM_14, GPIO_NUM_21 }, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},   
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},  
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC} 
                               };

  const gpio_num_t GPIO_SERVO[4] = { GPIO_NUM_NC,   GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC};

  const bool HASEXTPORT = false;

  const gpio_num_t GPIO_I2C[2][2] = { { GPIO_NUM_4,  GPIO_NUM_5  }, // External
                                      { GPIO_NUM_NC, GPIO_NUM_NC }  // Internal
                                    };

#endif

#ifdef DONTUSEITSOLDSTUIFF

                            //  inputs motors rcservos servos pixels, buttons, joysticks, pwrctl,   firstJPoti, OLED, HC165
const SwOSMaxIO_t MAXIOS[FTSWARMMAXVERSION] = {
  /* FTSWARMJST_1V0 */       {  4,     2,     0,       1,     2,      0,       0,         NOPWRCTL, 0,          false, false },
  /* FTSWARMCONTROL_1V3 */   {  8,     2,     0,       0,     0,      8,       2,         NOPWRCTL, 4,          true,  true },
  /* FTSWARMJST_1V15 */      {  4,     2,     0,       1,     2,      0,       0,         NOPWRCTL, 0,          false, false },
  /* FTSWARMRS_2V0 */        {  7,     2,     0,       2,     2,      0,       0,         6,        0,          false, false },
  /* FTSWARMRS_2V1 */        {  7,     2,     0,       2,     2,      0,       0,         6,        0,          false, false },
  /* FTSWARMCAM_3V12 */      {  5,     2,     0,       1,     2,      0,       0,         4,        0,          false, false }, 
  /* FTSWARMDUINO_1V14 */    {  12,    0,     0,       0,     2,      0,       0,         NOPWRCTL, 0,          false, false },
  /* FTSWARMPWRDRIVE_1V14 */ {  5,     4,     0,       0,     2,      0,       0,         NOPWRCTL, 0,          false, false },
  /* FTSWARMXL_1V00 */       {  8,     8,     0,       0,     2,      0,       0,         NOPWRCTL, 0,          false, false },
  /* FTSWARMRC_1V141 */      {  12,    4,     4,       0,     1,      0,       0,         6,        0,          false, false },
                         //  {  12,    4,     0,       0,     1,      0,       0,         6,        0,          false, false }, 
  /* FTSWARMCONTROL_1V3UC */ {  10,    2,     0,       0,     0,      8,       2,         11,       6,          true,  true }
};

const SwOSHALInput_t HAL_INPUT[FTSWARMMAXVERSION][MAXINPUTS] = {
    /* FTSWARMJST_1V0 */       { { "A1",     SWOSIO_DIGITAL, GPIO_NUM_33,  ADC_UNIT_1, ADC1_CHANNEL_5,   ADC_ATTEN_DB_12 },
                                 { "A2",     SWOSIO_DIGITAL, GPIO_NUM_25, ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "A3",     SWOSIO_DIGITAL, GPIO_NUM_26,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }, 
                                 { "A4",     SWOSIO_DIGITAL, GPIO_NUM_27,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }, 
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }, 
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }, 
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }, 
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               },
    /* FTSWARMCONTROL_1V3 */   { { "A1",     SWOSIO_DIGITAL, GPIO_NUM_39,  ADC_UNIT_1, ADC1_CHANNEL_3,   ADC_ATTEN_DB_12 },   
                                 { "A2",     SWOSIO_DIGITAL, GPIO_NUM_25, ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "A3",     SWOSIO_DIGITAL, GPIO_NUM_26,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "A4",     SWOSIO_DIGITAL, GPIO_NUM_27,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "JOY1LR", SWOSIO_ANALOG,  GPIO_NUM_33,  ADC_UNIT_1, ADC1_CHANNEL_5,   ADC_ATTEN_DB_12 }, // JOY1.LR
                                 { "JOY1FB", SWOSIO_ANALOG,  GPIO_NUM_36,  ADC_UNIT_1, ADC1_CHANNEL_0,   ADC_ATTEN_DB_12 }, // JOY1.FB
                                 { "JOY2LR", SWOSIO_ANALOG,  GPIO_NUM_32,  ADC_UNIT_1, ADC1_CHANNEL_4,   ADC_ATTEN_DB_12 }, // JOY2.LR
                                 { "JOY2FB", SWOSIO_UNDEF,   GPIO_NUM_34,  ADC_UNIT_1, ADC1_CHANNEL_6,   ADC_ATTEN_DB_12 }, // JOY2.FB
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                                },
    /* FTSWARMJST_1V15 */      { { "A1",     SWOSIO_DIGITAL, GPIO_NUM_39,  ADC_UNIT_1, ADC1_CHANNEL_3,   ADC_ATTEN_DB_12 },
                                 { "A2",     SWOSIO_DIGITAL, GPIO_NUM_32,  ADC_UNIT_1, ADC1_CHANNEL_4,   ADC_ATTEN_DB_12 },
                                 { "A3",     SWOSIO_DIGITAL, GPIO_NUM_33,  ADC_UNIT_1, ADC1_CHANNEL_5,   ADC_ATTEN_DB_12 },
                                 { "A4",     SWOSIO_DIGITAL, GPIO_NUM_34,  ADC_UNIT_1, ADC1_CHANNEL_6,   ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               },
    
    /* FTSWARMRS_2V0 */        { { "A1",     SWOSIO_DIGITAL, GPIO_NUM_1,   ADC_UNIT_1, ADC1_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { "A2",     SWOSIO_DIGITAL, GPIO_NUM_2,   ADC_UNIT_1, ADC1_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { "A3",     SWOSIO_DIGITAL, GPIO_NUM_8,   ADC_UNIT_1, ADC1_CHANNEL_7,   ADC_ATTEN_DB_12 },
                                 { "A4",     SWOSIO_DIGITAL, GPIO_NUM_9,   ADC_UNIT_1, ADC1_CHANNEL_8,   ADC_ATTEN_DB_12 },
                                 { "A5",     SWOSIO_DIGITAL, GPIO_NUM_11,  ADC_UNIT_2, ADC2_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { "A6",     SWOSIO_DIGITAL, GPIO_NUM_13,  ADC_UNIT_2, ADC2_CHANNEL_2,   ADC_ATTEN_DB_12 },
                                 { "PWRCTL", SWOSIO_POWER,   GPIO_NUM_12,  ADC_UNIT_2, ADC2_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               },

    /* FTSWARMRS_2V1 */        { { "A1",     SWOSIO_DIGITAL, GPIO_NUM_1,   ADC_UNIT_1, ADC1_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { "A2",     SWOSIO_DIGITAL, GPIO_NUM_2,   ADC_UNIT_1, ADC1_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { "A3",     SWOSIO_DIGITAL, GPIO_NUM_19,  ADC_UNIT_2, ADC2_CHANNEL_8,   ADC_ATTEN_DB_12 },
                                 { "A4",     SWOSIO_DIGITAL, GPIO_NUM_20,  ADC_UNIT_2, ADC2_CHANNEL_9,   ADC_ATTEN_DB_12 },
                                 { "A5",     SWOSIO_DIGITAL, GPIO_NUM_11,  ADC_UNIT_2, ADC2_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { "A6",     SWOSIO_DIGITAL, GPIO_NUM_13,  ADC_UNIT_2, ADC2_CHANNEL_2,   ADC_ATTEN_DB_12 },
                                 { "PWRCTL", SWOSIO_POWER,   GPIO_NUM_12,  ADC_UNIT_2, ADC2_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               },
    
    /* FTSWARMCAM_3V12 */      { { "A1",     SWOSIO_DIGITAL, GPIO_NUM_8,   ADC_UNIT_1, ADC1_CHANNEL_7,   ADC_ATTEN_DB_12 },
                                 { "A2",     SWOSIO_DIGITAL, GPIO_NUM_9,   ADC_UNIT_1, ADC1_CHANNEL_8,   ADC_ATTEN_DB_12 },
                                 { "A3",     SWOSIO_DIGITAL, GPIO_NUM_10,  ADC_UNIT_2, ADC1_CHANNEL_9,   ADC_ATTEN_DB_12 },
                                 { "A4",     SWOSIO_DIGITAL, GPIO_NUM_11,  ADC_UNIT_2, ADC2_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { "A5",     SWOSIO_DIGITAL, GPIO_NUM_6,   ADC_UNIT_1, ADC1_CHANNEL_5,   ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               },

    /* FTSWARMDUINO_1V14 */    { { "I1",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "I2",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "I3",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "I4",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "I5",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "I6",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "I7",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "I8",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "C1",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "C2",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "C3",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "C4",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               },

    /* FTSWARMPWRDRIVE_1V14 */ { { "ES1",    SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }, 
                                 { "ES2",    SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "ES3",    SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "ES4",    SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "EM",     SWOSIO_DIGITAL, GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               },
 
    /* FTSWARMXL_1V00 */       { { "A1",     SWOSIO_DIGITAL, GPIO_NUM_1,   ADC_UNIT_1, ADC1_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { "A2",     SWOSIO_DIGITAL, GPIO_NUM_2,   ADC_UNIT_1, ADC1_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { "A3",     SWOSIO_DIGITAL, GPIO_NUM_3,   ADC_UNIT_1, ADC1_CHANNEL_2,   ADC_ATTEN_DB_12 },
                                 { "A4",     SWOSIO_DIGITAL, GPIO_NUM_4,   ADC_UNIT_1, ADC1_CHANNEL_3,   ADC_ATTEN_DB_12 },
                                 { "A5",     SWOSIO_DIGITAL, GPIO_NUM_5,   ADC_UNIT_1, ADC1_CHANNEL_4,   ADC_ATTEN_DB_12 },
                                 { "A6",     SWOSIO_DIGITAL, GPIO_NUM_8,   ADC_UNIT_1, ADC1_CHANNEL_7,   ADC_ATTEN_DB_12 },
                                 { "A7",     SWOSIO_DIGITAL, GPIO_NUM_10,  ADC_UNIT_1, ADC1_CHANNEL_9,   ADC_ATTEN_DB_12 },
                                 { "A8",     SWOSIO_DIGITAL, GPIO_NUM_9,   ADC_UNIT_1, ADC1_CHANNEL_8,   ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_NC,  ADC_UNIT_1, ADC1_CHANNEL_MAX, ADC_ATTEN_DB_12 }
                               },

    /* FTSWARMRC_1V141 */      { { "A1",     SWOSIO_DIGITAL, GPIO_NUM_1,   ADC_UNIT_1, ADC1_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { "A2",     SWOSIO_DIGITAL, GPIO_NUM_2,   ADC_UNIT_1, ADC1_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { "A3",     SWOSIO_DIGITAL, GPIO_NUM_19,  ADC_UNIT_2, ADC2_CHANNEL_8,   ADC_ATTEN_DB_12 },
                                 { "A4",     SWOSIO_DIGITAL, GPIO_NUM_20,  ADC_UNIT_2, ADC2_CHANNEL_9,   ADC_ATTEN_DB_12 },
                                 { "A5",     SWOSIO_DIGITAL, GPIO_NUM_11,  ADC_UNIT_2, ADC2_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { "A6",     SWOSIO_DIGITAL, GPIO_NUM_13,  ADC_UNIT_2, ADC2_CHANNEL_2,   ADC_ATTEN_DB_12 },
                                 { "S1",     SWOSIO_DIGITAL, GPIO_NUM_16,  ADC_UNIT_2, ADC2_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { "PWRCTL", SWOSIO_POWER,   GPIO_NUM_12,  ADC_UNIT_2, ADC2_CHANNEL_1,   ADC_ATTEN_DB_12 },  // PwrControl
                                 { "RCP1",   SWOSIO_ANALOG,  GPIO_NUM_6,   ADC_UNIT_1, ADC1_CHANNEL_5,   ADC_ATTEN_DB_2_5},  // RC-Servo1 
                                 { "RCP2",   SWOSIO_ANALOG,  GPIO_NUM_7,   ADC_UNIT_1, ADC1_CHANNEL_6,   ADC_ATTEN_DB_2_5},  // RC-Servo2
                                 { "RCP3",   SWOSIO_ANALOG,  GPIO_NUM_8,   ADC_UNIT_1, ADC1_CHANNEL_7,   ADC_ATTEN_DB_2_5},  // RC-Servo3
                                 { "RCP4",   SWOSIO_ANALOG,  GPIO_NUM_9,   ADC_UNIT_1, ADC1_CHANNEL_8,   ADC_ATTEN_DB_2_5}   // RC-Servo4
                               },

    /* FTSWARMCONTROL_1V3UC */ { { "A1",     SWOSIO_DIGITAL, GPIO_NUM_1,   ADC_UNIT_1, ADC1_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { "A2",     SWOSIO_DIGITAL, GPIO_NUM_2,   ADC_UNIT_1, ADC1_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { "A3",     SWOSIO_DIGITAL, GPIO_NUM_19,  ADC_UNIT_2, ADC2_CHANNEL_8,   ADC_ATTEN_DB_12 },
                                 { "A4",     SWOSIO_DIGITAL, GPIO_NUM_20,  ADC_UNIT_2, ADC2_CHANNEL_9,   ADC_ATTEN_DB_12 },
                                 { "A5",     SWOSIO_DIGITAL, GPIO_NUM_11,  ADC_UNIT_2, ADC2_CHANNEL_0,   ADC_ATTEN_DB_12 },
                                 { "A6",     SWOSIO_DIGITAL, GPIO_NUM_13,  ADC_UNIT_2, ADC2_CHANNEL_2,   ADC_ATTEN_DB_12 },
                                 { "JOY1LR", SWOSIO_ANALOG,  GPIO_NUM_6,   ADC_UNIT_1, ADC1_CHANNEL_5,   ADC_ATTEN_DB_12 }, // JOY1.LR
                                 { "JOY1FB", SWOSIO_ANALOG,  GPIO_NUM_7,   ADC_UNIT_1, ADC1_CHANNEL_6,   ADC_ATTEN_DB_12 }, // JOY1.FB
                                 { "JOY2LR", SWOSIO_ANALOG,  GPIO_NUM_17,  ADC_UNIT_2, ADC2_CHANNEL_6,   ADC_ATTEN_DB_12 }, // JOY2.LR
                                 { "JOY2FB", SWOSIO_ANALOG,  GPIO_NUM_18,  ADC_UNIT_2, ADC2_CHANNEL_7,   ADC_ATTEN_DB_12 }, // JOY2.FB
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_12,  ADC_UNIT_2, ADC2_CHANNEL_1,   ADC_ATTEN_DB_12 },
                                 { "",       SWOSIO_UNDEF,   GPIO_NUM_12,  ADC_UNIT_2, ADC2_CHANNEL_1,   ADC_ATTEN_DB_12 }
                               }

                              }; 

  /* USTX, PUA */
const gpio_num_t USTCPUA[FTSWARMMAXVERSION][2] = 
  { /* FTSWARMJST_1V0 */       { GPIO_NUM_NC,  GPIO_NUM_NC},
    /* FTSWARMCONTROL_1V3 */   { GPIO_NUM_15,  GPIO_NUM_14},
    /* FTSWARMJST_1V15 */      { GPIO_NUM_15,  GPIO_NUM_14},
    /* FTSWARMRS_2V0 */        { GPIO_NUM_42, GPIO_NUM_41},
    /* FTSWARMRS_2V1 */        { GPIO_NUM_42, GPIO_NUM_41},
    /* FTSWARMCAM_3V12 */      { GPIO_NUM_3,   GPIO_NUM_NC},
    /* FTSWARMDUINO_1V14 */    { GPIO_NUM_NC,  GPIO_NUM_NC},
    /* FTSWARMPWRDRIVE_1V14 */ { GPIO_NUM_NC,  GPIO_NUM_NC},
    /* FTSWARMXL_1V00 */       { GPIO_NUM_NC,  GPIO_NUM_NC},
    /* FTSWARMRC_1V141 */      { GPIO_NUM_42, GPIO_NUM_41},
    /* FTSWARMCONTROL_1V3UC */ { GPIO_NUM_42, GPIO_NUM_41}
  };   

const SwOSHALActor_t HAL_ACTOR[FTSWARMMAXVERSION][8] = 
  { /* FTSWARMJST_1V0 */       { { "M1",    SWOSIO_MOTOR,   0,           GPIO_NUM_13,  GPIO_NUM_4}, 
                                 { "M2",    SWOSIO_MOTOR,   1,           GPIO_NUM_2,   GPIO_NUM_0},   
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},   
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},  
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC} },

    /* FTSWARMCONTROL_1V3 */   { { "M1",    SWOSIO_MOTOR,   0,           GPIO_NUM_13,  GPIO_NUM_4}, 
                                 { "M2",    SWOSIO_MOTOR,   1,           GPIO_NUM_2,   GPIO_NUM_0},   
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},   
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},  
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC} },

    /* FTSWARMJST_1V15 */      { { "M1",    SWOSIO_MOTOR,   0,           GPIO_NUM_13,  GPIO_NUM_4}, 
                                 { "M2",    SWOSIO_MOTOR,   1,           GPIO_NUM_2,   GPIO_NUM_0},   
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},   
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},  
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC} },

    /* FTSWARMRS_2V0 */        { { "M1",    SWOSIO_MOTOR,   0,           GPIO_NUM_14,  GPIO_NUM_21 }, 
                                 { "M2",    SWOSIO_MOTOR,   1,           GPIO_NUM_45, GPIO_NUM_46}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},   
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},  
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC} },

    /* FTSWARMRS_2V1 */        { { "M1",    SWOSIO_MOTOR,   0,           GPIO_NUM_14,  GPIO_NUM_21 }, 
                                 { "M2",    SWOSIO_MOTOR,   1,           GPIO_NUM_45, GPIO_NUM_46}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},   
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},  
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC} },

    /* FTSWARMCAM_3V12 */      { { "M1",    SWOSIO_MOTOR,   0,           GPIO_NUM_40, GPIO_NUM_41}, 
                                 { "M2",    SWOSIO_MOTOR,   1,           GPIO_NUM_1,   GPIO_NUM_2},   
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},   
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},  
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC} },

    /* FTSWARMDUINO_1V14*/     { { "M1",    SWOSIO_MOTOR,   0,           GPIO_NUM_NC,  GPIO_NUM_NC},  
                                 { "M2",    SWOSIO_MOTOR,   1,           GPIO_NUM_NC,  GPIO_NUM_NC},
                                 { "M3",    SWOSIO_MOTOR,   2,           GPIO_NUM_NC,  GPIO_NUM_NC},
                                 { "M4",    SWOSIO_MOTOR,   3,           GPIO_NUM_NC,  GPIO_NUM_NC},
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},  
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC} },
                                 
    /* FTSWARMPWRDRIVE_1V14*/  { { "STEP1", SWOSIO_STEPPER, 0,           GPIO_NUM_NC,  GPIO_NUM_NC},  
                                 { "STEP2", SWOSIO_STEPPER, 1,           GPIO_NUM_NC,  GPIO_NUM_NC},
                                 { "STEP3", SWOSIO_STEPPER, 2,           GPIO_NUM_NC,  GPIO_NUM_NC},
                                 { "STEP4", SWOSIO_STEPPER, 3,           GPIO_NUM_NC,  GPIO_NUM_NC},
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},  
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC} },

    /* FTSWARMXL_1V00*/        { { "M1",    SWOSIO_MOTOR,   0,           GPIO_NUM_16,  GPIO_NUM_15},  
                                 { "M2",    SWOSIO_MOTOR,   1,           GPIO_NUM_14,  GPIO_NUM_13},
                                 { "M3",    SWOSIO_MOTOR,   2,           GPIO_NUM_11, GPIO_NUM_12}, 
                                 { "M4",    SWOSIO_MOTOR,   3,           GPIO_NUM_42, GPIO_NUM_41}, 
                                 { "M5",    SWOSIO_MOTOR,   4,           GPIO_NUM_40, GPIO_NUM_39}, 
                                 { "M6",    SWOSIO_MOTOR,   5,           GPIO_NUM_38, GPIO_NUM_37}, 
                                 { "M7",    SWOSIO_MOTOR,   6,           GPIO_NUM_19, GPIO_NUM_20}, 
                                 { "M8",    SWOSIO_MOTOR,   7,           GPIO_NUM_35, GPIO_NUM_36} },

    /* FTSWARMRC_1V141 */      { { "M1",    SWOSIO_MOTOR,   0,           GPIO_NUM_14,  GPIO_NUM_21 }, 
                                 { "M1",    SWOSIO_MOTOR,   1,           GPIO_NUM_45, GPIO_NUM_46}, 
                                 { "M1",    SWOSIO_MOTOR,   2,           GPIO_NUM_15, GPIO_NUM_17}, 
                                 { "M1",    SWOSIO_MOTOR,   3,           GPIO_NUM_18, GPIO_NUM_47},
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},  
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC} },
                                 
    /* FTSWARMCONTROL_1V3UC */ { { "M1",    SWOSIO_MOTOR,   0,           GPIO_NUM_45, GPIO_NUM_46}, 
                                 { "M1",    SWOSIO_MOTOR,   1,           GPIO_NUM_14,  GPIO_NUM_21 }, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},   
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC},  
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC}, 
                                 { "",      SWOSIO_UNDEF,   SWOS_NOPORT, GPIO_NUM_NC, GPIO_NUM_NC} }
  };   

const gpio_num_t GPIO_SERVO[FTSWARMMAXVERSION][4] = 
  { /* FTSWARMJST_1V0 */       { GPIO_NUM_25,  GPIO_NUM_13, GPIO_NUM_12, GPIO_NUM_NC},
    /* FTSWARMCONTROL_1V3 */   { GPIO_NUM_NC,   GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC},
    /* FTSWARMJST_1V15 */      { GPIO_NUM_25,  GPIO_NUM_13, GPIO_NUM_12, GPIO_NUM_NC},
    /* FTSWARMRS_2V0 */        { GPIO_NUM_47,  GPIO_NUM_8,  GPIO_NUM_9, GPIO_NUM_NC},
    /* FTSWARMRS_2V1 */        { GPIO_NUM_47,  GPIO_NUM_15, GPIO_NUM_8,  GPIO_NUM_9},
    /* FTSWARMCAM_3V12 */      { GPIO_NUM_47,  GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC},
    /* FTSWARMDUINO_1V14 */    { GPIO_NUM_NC,   GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC},
    /* FTSWARMPWRDRIVE_1V14 */ { GPIO_NUM_NC,   GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC},
    /* FTSWARMXL_1V00 */       { GPIO_NUM_33,   GPIO_NUM_21, GPIO_NUM_NC, GPIO_NUM_NC},
    /* FTSWARMRC_1V141 */      { GPIO_NUM_NC,   GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC},
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
  /* FTSWARMRC_1V141 */      false,
  /* FTSWARMRC_1V141 */      false
};

const gpio_num_t GPIO_I2C[FTSWARMMAXVERSION][2][2] = {
  /* FTSWARMJST_1V0 */       { { GPIO_NUM_13, GPIO_NUM_12 },    // External
                               { GPIO_NUM_NC, GPIO_NUM_NC } },  // Internal
  /* FTSWARMCONTROL_1V3 */   { { GPIO_NUM_21, GPIO_NUM_22 }, 
                               { GPIO_NUM_NC, GPIO_NUM_NC } },
  /* FTSWARMJST_1V15 */      { { GPIO_NUM_21, GPIO_NUM_22 }, 
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
  /* FTSWARMRC_1V141 */      { { GPIO_NUM_NC, GPIO_NUM_NC }, 
                               { GPIO_NUM_NC, GPIO_NUM_NC } },
  /* FTSWARMCONTROL_1V3UC */ { { GPIO_NUM_4,  GPIO_NUM_5  }, 
                               { GPIO_NUM_NC, GPIO_NUM_NC } },
};

#endif
