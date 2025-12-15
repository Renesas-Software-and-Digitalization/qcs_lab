/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier:  BSD-3-Clause
*/

#ifndef __MAIN_APPLICATION_H
#define __MAIN_APPLICATION_H

#include "common_utils.h"
 #define SET_PERIOD_1_SECS       0x10000  // 1 second period
 #define SET_PERIOD_500_MSECS    0x8000   // 500 ms period
 #define SET_PERIOD_250_MSECS    0x4000   // 250 ms period
 #define SET_PERIOD_125_MSECS    0x2000   // 125 ms period
 #define SET_PERIOD_75_MSECS     0x1000   // 75 ms period
 #define SET_PERIOD_37_MSECS     0x800    // 37.5 ms period
 #define SET_PERIOD_16_MSECS     0x400   // 16 ms period
 #define SET_PERIOD_8_MSECS      0x200  // 8 ms period
 #define SET_PERIOD_4_MSECS      0x100  // 4 ms period
 #define SET_PERIOD_2_MSECS      0x80   // 2 ms period
 #define SET_PERIOD_1_MSECS      0x40   // 1 ms period

 #define ENABLE_LED1 // Enable or disable LED1
 #define ENABLE_LED2 // Enable or disable LED2

 #ifdef ENABLE_LED1
 #define ENABLE_PWM_LED1
 #endif

 #ifdef ENABLE_LED2
 #define ENABLE_PWM_LED2
 #endif

 #ifdef ENABLE_PWM_LED1
 #define LED1_DEFAULT_DUTY_CYCLE 50    // 50 %
 #define LED1_DEFAULT_PERIOD  SET_PERIOD_1_SECS
 #endif
 #ifdef ENABLE_PWM_LED2  // 50 %
 #define LED2_DEFAULT_PERIOD        SET_PERIOD_1_SECS
 #define LED2_DEFAULT_DUTY_CYCLE 50
 #endif
/* Function declaration */
void main_application(void);

#endif /* __MAIN_APPLICATION_H */
