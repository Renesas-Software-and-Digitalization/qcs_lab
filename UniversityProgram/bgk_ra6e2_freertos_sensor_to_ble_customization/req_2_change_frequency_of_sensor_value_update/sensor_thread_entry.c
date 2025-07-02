/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier:  BSD-3-Clause
*/
#include "sensor_thread.h"
#include "sm.h"

static void sensor_configure_interval(uint32_t interval)
{
    sm_handle handle;
    uint16_t index;
 
    sm_get_sensor_handle(SENSOR_ANY_TYPE, &handle, &index);
    sm_set_sensor_attribute(handle, SM_ACQUISITION_INTERVAL, interval);
}

/* Sensor entry function */
/* pvParameters contains TaskHandle_t */
void sensor_thread_entry(void *pvParameters) {
    FSP_PARAMETER_NOT_USED (pvParameters);

    sensor_configure_interval(6000);

    /* TODO: add your own code here */
    sm_init();
    while (1) {
        sm_run();
    }
}