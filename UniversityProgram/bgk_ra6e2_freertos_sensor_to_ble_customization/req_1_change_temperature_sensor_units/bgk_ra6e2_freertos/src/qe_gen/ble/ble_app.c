/***********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
 * SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2019-2020 Renesas Electronics Corporation. All rights reserved.
 ***********************************************************************************************************************/

/******************************************************************************
 * File Name    : app_main.c
 * Device(s)    : RA4W1
 * Tool-Chain   : e2Studio
 * Description  : This is a application file for peripheral role.
 *******************************************************************************/

/******************************************************************************
 Includes   <System Includes> , "Project Includes"
 *******************************************************************************/
#include <string.h>
#include "r_ble_api.h"
#include "rm_ble_abs.h"
#include "rm_ble_abs_api.h"
#include "gatt_db.h"
#include "profile_cmn/r_ble_servs_if.h"
#include "profile_cmn/r_ble_servc_if.h"
#include "hal_data.h"
#include "user.h"


/* This code is needed for using FreeRTOS */
#if (BSP_CFG_RTOS == 2 || BSP_CFG_RTOS_USED == 1)
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "ble_thread.h"
#define BLE_EVENT_PATTERN   (0x0A0A)
EventGroupHandle_t  g_ble_event_group_handle;
#endif
#include "r_ble_qc_svcs.h"

/******************************************************************************
 User file includes
 *******************************************************************************/
/* Start user code for file includes. Do not edit comment generated here */
#include "qe_ble_profile.h"
#include "gui_cfg.h"
#include "qc_svc.h"
#include "common_utils.h"
#include "ble_app.h"
#include "sm.h"
// Uncomment the desired debug level
#include "log_disabled.h"
//#include "log_error.h"
//#include "log_warning.h"
//#include "log_info.h"
//#include "log_debug.h"
/* End user code. Do not edit comment generated here */

#define BLE_LOG_TAG "app_main"
#define BLE_GATTS_QUEUE_ELEMENTS_SIZE       (14)
#define BLE_GATTS_QUEUE_BUFFER_LEN          (245)
#define BLE_GATTS_QUEUE_NUM                 (1)

/******************************************************************************
 User macro definitions
 *******************************************************************************/
/* Start user code for macro definitions. Do not edit comment generated here */
#define BLE_OPTIMAL_MTU                     101
#define EVENT_BIT_NOTIFY_CFM                (1UL << 0)
/* End user code. Do not edit comment generated here */

/******************************************************************************
 Generated function prototype declarations
 *******************************************************************************/
/* Internal functions */
void gap_cb(uint16_t type, ble_status_t result, st_ble_evt_data_t *p_data);
void gatts_cb(uint16_t type, ble_status_t result, st_ble_gatts_evt_data_t *p_data);
void gattc_cb(uint16_t type, ble_status_t result, st_ble_gattc_evt_data_t *p_data);
void vs_cb(uint16_t type, ble_status_t result, st_ble_vs_evt_data_t *p_data);
ble_status_t ble_init(void);
void app_main(void);

/******************************************************************************
 User function prototype declarations
 *******************************************************************************/
/* Start user code for function prototype declarations. Do not edit comment generated here */
static void handle_read_gui_req(uint16_t id, uint8_t const * const data);
static void handle_write_led(uint16_t id, uint8_t const * const data);
static void handle_read_version(uint16_t id, uint8_t const * const data);
static void send_qc_svc_response(uint8_t const * const p_data, uint16_t len);
static void led_timer_cb(TimerHandle_t xTimer);
static void handle_read_data(uint16_t id, uint8_t const * const data);
static const char pre_adv_data[] = "US000-";

/* End user code. Do not edit comment generated here */
#define MAX_ADV_DATA_LENGTH 		(20)
#define ADV_DATA_LEN 				(sizeof(adv_data) - 1)
#define PRE_ADV_DATA_LEN 			(sizeof(pre_adv_data) - 1)

/* Advertising Data */
static uint8_t gs_advertising_data[5 + PRE_ADV_DATA_LEN + ADV_DATA_LEN] =
{
    /* Flags */
    0x02, /**< Data Size */
    0x01, /**< Data Type */
    0x06, /**< Data Value */

    /* Complete Local Name */
	(PRE_ADV_DATA_LEN + ADV_DATA_LEN + 1), /**< Data Size */
    0x09, /**< Data Type */
};

ble_abs_legacy_advertising_parameter_t g_ble_advertising_parameter =
{
 .p_peer_address             = NULL,       ///< Peer address.
 .slow_advertising_interval  = 0x000000A0, ///< Slow advertising interval. 100.0(ms)
 .slow_advertising_period    = 0x0000,     ///< Slow advertising period.
 .p_advertising_data         = gs_advertising_data,             ///< Advertising data. If p_advertising_data is specified as NULL, advertising data is not set.
 .advertising_data_length    = ARRAY_SIZE(gs_advertising_data), ///< Advertising data length (in bytes).
 .advertising_filter_policy  = BLE_ABS_ADVERTISING_FILTER_ALLOW_ANY, ///< Advertising Filter Policy.
 .advertising_channel_map    = ( BLE_GAP_ADV_CH_37 | BLE_GAP_ADV_CH_38 | BLE_GAP_ADV_CH_39 ), ///< Channel Map.
 .own_bluetooth_address_type = BLE_GAP_ADDR_RAND, ///< Own Bluetooth address type.
 .own_bluetooth_address      = { 0 },
};

/* GATT server callback parameters */
ble_abs_gatt_server_callback_set_t gs_abs_gatts_cb_param[] =
{
 {
  .gatt_server_callback_function = gatts_cb,
  .gatt_server_callback_priority = 1,
 },
 {
  .gatt_server_callback_function = NULL,
 }
};

/* GATT client callback parameters */
ble_abs_gatt_client_callback_set_t gs_abs_gattc_cb_param[] =
{
 {
  .gatt_client_callback_function = gattc_cb,
  .gatt_client_callback_priority = 1,
 },
 {
  .gatt_client_callback_function = NULL,
 }
};

/* GATT server Prepare Write Queue parameters */
static st_ble_gatt_queue_elm_t  gs_queue_elms[BLE_GATTS_QUEUE_ELEMENTS_SIZE];
static uint8_t gs_buffer[BLE_GATTS_QUEUE_BUFFER_LEN];
static st_ble_gatt_pre_queue_t gs_queue[BLE_GATTS_QUEUE_NUM] = {
                                                                {
                                                                 .p_buf_start = gs_buffer,
                                                                 .buffer_len  = BLE_GATTS_QUEUE_BUFFER_LEN,
                                                                 .p_queue     = gs_queue_elms,
                                                                 .queue_size  = BLE_GATTS_QUEUE_ELEMENTS_SIZE,
                                                                }
};

/* Connection handle */
uint16_t g_conn_hdl;

/******************************************************************************
 User global variables
 *******************************************************************************/
/* Start user code for global variables. Do not edit comment generated here */
static TimerHandle_t      led_timer;
static EventGroupHandle_t event_group;
static char   * version_str           = "1.0.0";
static bool     led_on                = false;
extern QueueHandle_t g_sensor_queue;
// These are the handlers used by the BLE service to send/receive data
static const qc_svc_request_handlers_t qc_sv_req_handlers[] =
{
      /* Object ID   Read Handler                  Write Handler  SM handler*/
      { 0x0000,      handle_read_gui_req,          NULL,          {0}},
      { 0x0100,      NULL,                         handle_write_led,          {0}},
      { 0x0201,      handle_read_data,              NULL,{.channel=0,.address=0,.internal=1}                        },
      
      { 0x0202,      handle_read_data,              NULL,{.channel=1,.address=0,.internal=1}                        },
      
      { 0x0301,      handle_read_version,          NULL,         {0}},
      { 0xFFFF,      NULL,                         NULL,          {0}}
};
static float sensor_data_array[3];



/* End user code. Do not edit comment generated here */

/******************************************************************************
 Generated function definitions
 *******************************************************************************/
/******************************************************************************
 * Function Name: gap_cb
 * Description  : Callback function for GAP API.
 * Arguments    : uint16_t type -
 *                  Event type of GAP API.
 *              : ble_status_t result -
 *                  Event result of GAP API.
 *              : st_ble_vs_evt_data_t *p_data - 
 *                  Event parameters of GAP API.
 * Return Value : none
 ******************************************************************************/
void gap_cb(uint16_t type, ble_status_t result, st_ble_evt_data_t *p_data)
{
    /* Hint: Input common process of callback function such as variable definitions */
    /* Start user code for GAP callback function common process. Do not edit comment generated here */
    switch(type)
    {
        case BLE_GAP_EVENT_CONN_IND:
        {
            if (BLE_SUCCESS == result)
            {
                uint16_t mtu;
                st_ble_gap_conn_evt_t *p_gap_conn_evt_param = (st_ble_gap_conn_evt_t *)p_data->p_param;

                /* Turn green LED on to show that someone has connected to us */
                xTimerStop(led_timer, 0);
                utils_set_LED(GREEN_LED, BSP_IO_LEVEL_HIGH);

                if (BLE_SUCCESS == R_BLE_GATT_GetMtu(p_gap_conn_evt_param->conn_hdl, &mtu))
                {
                    if (mtu > BLE_OPTIMAL_MTU)
                    {
                        mtu = BLE_OPTIMAL_MTU;
                    }
                    /* Notifications used to send data have an overhead of 3 bytes */
                    (void)qc_svc_set_transport_mtu(mtu - 3);
                }
            }
        }
        break;

        case BLE_GAP_EVENT_DISCONN_IND:
        {
            utils_set_LED(GREEN_LED, BSP_IO_LEVEL_LOW);
            led_on = true;
            xTimerStart(led_timer, 0);
        }
        break;

        default:
        {
            ; /* Do nothing */
        }
        break;
    }

    /* End user code. Do not edit comment generated here */

    switch(type)
    {
        case BLE_GAP_EVENT_STACK_ON:
        {
            /* Get BD address for Advertising */
            R_BLE_VS_GetBdAddr(BLE_VS_ADDR_AREA_REG, BLE_GAP_ADDR_RAND);
        } break;

        case BLE_GAP_EVENT_CONN_IND:
        {
            if (BLE_SUCCESS == result)
            {
                /* Store connection handle */
                st_ble_gap_conn_evt_t *p_gap_conn_evt_param = (st_ble_gap_conn_evt_t *)p_data->p_param;
                g_conn_hdl = p_gap_conn_evt_param->conn_hdl;
            }
            else
            {
                /* Restart advertising when connection failed */
                RM_BLE_ABS_StartLegacyAdvertising(&g_ble_abs0_ctrl, &g_ble_advertising_parameter);
            }
        } break;

        case BLE_GAP_EVENT_DISCONN_IND:
        {
            /* Restart advertising when disconnected */
            g_conn_hdl = BLE_GAP_INVALID_CONN_HDL;
            RM_BLE_ABS_StartLegacyAdvertising(&g_ble_abs0_ctrl, &g_ble_advertising_parameter);
        } break;

        case BLE_GAP_EVENT_CONN_PARAM_UPD_REQ:
        {
            /* Send connection update response with value received on connection update request */
            st_ble_gap_conn_upd_req_evt_t *p_conn_upd_req_evt_param = (st_ble_gap_conn_upd_req_evt_t *)p_data->p_param;

            st_ble_gap_conn_param_t conn_updt_param = {
                                                       .conn_intv_min = p_conn_upd_req_evt_param->conn_intv_min,
                                                       .conn_intv_max = p_conn_upd_req_evt_param->conn_intv_max,
                                                       .conn_latency  = p_conn_upd_req_evt_param->conn_latency,
                                                       .sup_to        = p_conn_upd_req_evt_param->sup_to,
            };

            R_BLE_GAP_UpdConn(p_conn_upd_req_evt_param->conn_hdl,
                    BLE_GAP_CONN_UPD_MODE_RSP,
                    BLE_GAP_CONN_UPD_ACCEPT,
                    &conn_updt_param);
        } break;

        /* Hint: Add cases of GAP event macros defined as BLE_GAP_XXX */
        /* Start user code for GAP callback function event process. Do not edit comment generated here */
        /* End user code. Do not edit comment generated here */
    }
}

/******************************************************************************
 * Function Name: gatts_cb
 * Description  : Callback function for GATT Server API.
 * Arguments    : uint16_t type -
 *                  Event type of GATT Server API.
 *              : ble_status_t result -
 *                  Event result of GATT Server API.
 *              : st_ble_gatts_evt_data_t *p_data - 
 *                  Event parameters of GATT Server API.
 * Return Value : none
 ******************************************************************************/
void gatts_cb(uint16_t type, ble_status_t result, st_ble_gatts_evt_data_t *p_data)
{
    /* Hint: Input common process of callback function such as variable definitions */
    /* Start user code for GATT Server callback function common process. Do not edit comment generated here */
    switch(type)
    {
        case BLE_GATTS_EVENT_DB_ACCESS_IND:
        {
            st_ble_gatts_db_access_evt_t * p_gatts_db_access_evt = p_data->p_param;
            st_ble_gatt_value_t * p_gatt_value = &p_gatts_db_access_evt->p_params->value;

            if (BLE_GATTS_OP_CHAR_PEER_WRITE_REQ == p_gatts_db_access_evt->p_params->db_op)
            {
                if (QE_ATTRIBUTE_HANDLE_CHARACTERISTIC_VALUE_QC_SVC_QC_REQ == p_gatts_db_access_evt->p_params->attr_hdl)
                {
                    qc_svc_handle_request((uint8_t *)p_gatt_value->p_value, p_gatt_value->value_len);
                }
            }
        }
        break;

        case BLE_GATTS_EVENT_HDL_VAL_CNF:
        {
            st_ble_gatts_cfm_evt_t * p_gatts_cfm_evt = p_data->p_param;

            if (QE_ATTRIBUTE_HANDLE_CHARACTERISTIC_VALUE_QC_SVC_QC_RSP == p_gatts_cfm_evt->attr_hdl)
            {
                xEventGroupSetBits(event_group, EVENT_BIT_NOTIFY_CFM);
            }
        }
        break;

        default:
        {
            /* Unhandled event type - do nothing */
        }
        break;
    }

    /* Not using the QE generated GATTS code */
    return;
    /* End user code. Do not edit comment generated here */

    R_BLE_SERVS_GattsCb(type, result, p_data);
    switch(type)
    {
        /* Hint: Add cases of GATT Server event macros defined as BLE_GATTS_XXX */
        /* Start user code for GATT Server callback function event process. Do not edit comment generated here */
        /* End user code. Do not edit comment generated here */
    }
}

/******************************************************************************
 * Function Name: gattc_cb
 * Description  : Callback function for GATT Client API.
 * Arguments    : uint16_t type -
 *                  Event type of GATT Client API.
 *              : ble_status_t result -
 *                  Event result of GATT Client API.
 *              : st_ble_gattc_evt_data_t *p_data - 
 *                  Event parameters of GATT Client API.
 * Return Value : none
 ******************************************************************************/
void gattc_cb(uint16_t type, ble_status_t result, st_ble_gattc_evt_data_t *p_data)
{
    /* Hint: Input common process of callback function such as variable definitions */
    /* Start user code for GATT Client callback function common process. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */

    R_BLE_SERVC_GattcCb(type, result, p_data);
    switch(type)
    {

        /* Hint: Add cases of GATT Client event macros defined as BLE_GATTC_XXX */
        /* Start user code for GATT Client callback function event process. Do not edit comment generated here */
        /* End user code. Do not edit comment generated here */
    }
}

/******************************************************************************
 * Function Name: vs_cb
 * Description  : Callback function for Vendor Specific API.
 * Arguments    : uint16_t type -
 *                  Event type of Vendor Specific API.
 *              : ble_status_t result -
 *                  Event result of Vendor Specific API.
 *              : st_ble_vs_evt_data_t *p_data - 
 *                  Event parameters of Vendor Specific API.
 * Return Value : none
 ******************************************************************************/
void vs_cb(uint16_t type, ble_status_t result, st_ble_vs_evt_data_t *p_data)
{
    /* Hint: Input common process of callback function such as variable definitions */
    /* Start user code for vender specific callback function common process. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */

    R_BLE_SERVS_VsCb(type, result, p_data);
    switch(type)
    {
        case BLE_VS_EVENT_GET_ADDR_COMP:
        {
            /* Start advertising when BD address is ready */
            st_ble_vs_get_bd_addr_comp_evt_t * get_address = (st_ble_vs_get_bd_addr_comp_evt_t *)p_data->p_param;
            memcpy(g_ble_advertising_parameter.own_bluetooth_address, get_address->addr.addr, BLE_BD_ADDR_LEN);
            RM_BLE_ABS_StartLegacyAdvertising(&g_ble_abs0_ctrl, &g_ble_advertising_parameter);
        } break;

        /* Hint: Add cases of vender specific event macros defined as BLE_VS_XXX */
        /* Start user code for vender specific callback function event process. Do not edit comment generated here */
        /* End user code. Do not edit comment generated here */
    }
}

/******************************************************************************
 * Function Name: qc_svcs_cb
 * Description  : Callback function for Quick Connect Service server feature.
 * Arguments    : uint16_t type -
 *                  Event type of Quick Connect Service server feature.
 *              : ble_status_t result -
 *                  Event result of Quick Connect Service server feature.
 *              : st_ble_servs_evt_data_t *p_data - 
 *                  Event parameters of Quick Connect Service server feature.
 * Return Value : none
 ******************************************************************************/
static void qc_svcs_cb(uint16_t type, ble_status_t result, st_ble_servs_evt_data_t *p_data)
{
    /* Hint: Input common process of callback function such as variable definitions */
    /* Start user code for Quick Connect Service Server callback function common process. Do not edit comment generated here */
    FSP_PARAMETER_NOT_USED(result);
    FSP_PARAMETER_NOT_USED(p_data);
    /* End user code. Do not edit comment generated here */

    switch(type)
    {
        /* Hint: Add cases of Quick Connect Service server events defined in e_ble_qc_svcs_event_t */
        /* Start user code for Quick Connect Service Server callback function event process. Do not edit comment generated here */
        /* End user code. Do not edit comment generated here */
    }    
}
/******************************************************************************
 * Function Name: ble_init
 * Description  : Initialize BLE and profiles.
 * Arguments    : none
 * Return Value : BLE_SUCCESS - SUCCESS
 *                BLE_ERR_INVALID_OPERATION -
 *                    Failed to initialize BLE or profiles.
 ******************************************************************************/
ble_status_t ble_init(void)
{
    ble_status_t status;
    fsp_err_t err;
	if (ADV_DATA_LEN > MAX_ADV_DATA_LENGTH) {
        return BLE_ERR_INVALID_DATA;
	}

    memcpy(&gs_advertising_data[5], pre_adv_data, PRE_ADV_DATA_LEN);
    memcpy(&gs_advertising_data[5+PRE_ADV_DATA_LEN], adv_data, ADV_DATA_LEN);

    /* Initialize BLE */
    err = RM_BLE_ABS_Open(&g_ble_abs0_ctrl, &g_ble_abs0_cfg);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    /* Initialize GATT Database */
    status = R_BLE_GATTS_SetDbInst(NULL);
    /* There is known issue causing BLE_ERR_GATT_WRITE_NOT_PERMITTED on the FSP5.9.0 */
    if (BLE_SUCCESS != status && BLE_ERR_GATT_WRITE_NOT_PERMITTED != status)
    {
        return BLE_ERR_INVALID_OPERATION;
    }

    /* Initialize GATT server */
    status = R_BLE_SERVS_Init();
    if (BLE_SUCCESS != status)
    {
        return BLE_ERR_INVALID_OPERATION;
    }

    /*Initialize GATT client */
    status = R_BLE_SERVC_Init();
    if (BLE_SUCCESS != status)
    {
        return BLE_ERR_INVALID_OPERATION;
    }

    /* Set Prepare Write Queue */
    R_BLE_GATTS_SetPrepareQueue(gs_queue, BLE_GATTS_QUEUE_NUM);

    /* Initialize Quick Connect Service server API */
    status = R_BLE_QC_SVCS_Init(qc_svcs_cb);
    if (BLE_SUCCESS != status)
    {
        return BLE_ERR_INVALID_OPERATION;
    }

    return status;
}

/******************************************************************************
 * Function Name: app_main
 * Description  : Application main function with main loop
 * Arguments    : none
 * Return Value : none
 ******************************************************************************/
void ble_app_init(void) {
#if (BSP_CFG_RTOS == 2 || BSP_CFG_RTOS_USED == 1)
    /* Create Event Group */
    g_ble_event_group_handle = xEventGroupCreate();
    assert(g_ble_event_group_handle);
#endif

    /* Initialize BLE and profiles */
    ble_init();

    /* Hint: Input process that should be done before main loop such as calling initial function or variable definitions */
    /* Start user code for process before main loop. Do not edit comment generated here */
    log_info("BLE Custom App init\n");

    /* Start timer used to blink LED, indicating connection status */
    led_timer = xTimerCreate( "led_timer", pdMS_TO_TICKS(500), pdTRUE, NULL, led_timer_cb);
    xTimerStart(led_timer, 0);

    event_group = xEventGroupCreate();

    (void)qc_svc_register_handlers(&qc_sv_req_handlers[0]);
    (void)qc_svc_register_transmit_cb(send_qc_svc_response);
}

void ble_app_run(void) {
        /* Process BLE Event */
        R_BLE_Execute();
        /* When this BLE application works on the FreeRTOS */
#if (BSP_CFG_RTOS == 2 || BSP_CFG_RTOS_USED == 1)
    if(0 != R_BLE_IsTaskFree()) {
            /* If the BLE Task has no operation to be processed, it transits block state until the event from RF transciever occurs. */
            xEventGroupWaitBits(g_ble_event_group_handle,
                                (EventBits_t)BLE_EVENT_PATTERN,
                                pdTRUE,
                                pdFALSE,
                                portMAX_DELAY);
        }
#endif
        sm_sensor_data sensor_data;
        EventBits_t event_bits = xEventGroupGetBits(event_group);
        if (event_bits & EVENT_BIT_NOTIFY_CFM) {
            xEventGroupClearBits(event_group, EVENT_BIT_NOTIFY_CFM);
            qc_svc_handle_response_write_cfm();
        }
        if (pdTRUE == xQueueReceive(g_sensor_queue, &sensor_data, 0)) {
            uint32_t index = 2;
            while (index < (uint32_t)sm_get_total_sensor_count()+2) {
                if (qc_sv_req_handlers[index].sensor_handler.value == sensor_data.handle.value) {
                    sm_scaling scaling;
                    sm_get_sensor_scaling(sensor_data.handle, &scaling);
                    float floatData = ((float)sensor_data.data * (float)scaling.multiplier)/(float)scaling.divider + (float)scaling.offset;
                    
                    if (sm_get_sensor_type_by_handle(qc_sv_req_handlers[index].sensor_handler) == TEMPERATURE)
                    {
                        floatData = ((floatData * 18) / 10) + 32;
                    }

                    sensor_data_array[index-2] = floatData;
                    break;
                }
                index++;
            }
        }
    }

void ble_app_close(void) {
    /* Terminate BLE */
    RM_BLE_ABS_Close(&g_ble_abs0_ctrl);
}

/******************************************************************************
 User function definitions
 *******************************************************************************/
/* Start user code for function definitions. Do not edit comment generated here */
static void handle_read_gui_req(uint16_t id, uint8_t const * const data)
{
    FSP_PARAMETER_NOT_USED(id);
    FSP_PARAMETER_NOT_USED(data);

    uint16_t len = strlen((char *)gui_cfg);

    if (0 < len)
    {
        qc_svc_send_read_response(QC_SVC_SUCCESS, id, len, (uint8_t *)&gui_cfg[0]);
    }
}

static void handle_write_led(uint16_t id, uint8_t const * const data)
{
    FSP_PARAMETER_NOT_USED(id);
    log_info("sending id: 0x%04x\n", id);
    if (data[4] == 0x00)
    {
        utils_set_LED(BLUE_LED,BSP_IO_LEVEL_LOW);
    }
    else
    {
        utils_set_LED(BLUE_LED,BSP_IO_LEVEL_HIGH);
    }
}

static void handle_read_version(uint16_t id, uint8_t const * const data)
{
    FSP_PARAMETER_NOT_USED(data);

    qc_svc_send_read_response(QC_SVC_SUCCESS, id, (uint16_t)strlen(version_str), (uint8_t *)version_str);
}

static void send_qc_svc_response(uint8_t const * const p_data, uint16_t len)
{
    st_ble_gatt_hdl_value_pair_t hdl_value_pair;

    hdl_value_pair.attr_hdl        = QE_ATTRIBUTE_HANDLE_CHARACTERISTIC_VALUE_QC_SVC_QC_RSP;
    hdl_value_pair.value.p_value   = (uint8_t *)p_data;
    hdl_value_pair.value.value_len = len;

    (void)R_BLE_GATTS_Notification(g_conn_hdl, &hdl_value_pair);
}

static void led_timer_cb(TimerHandle_t xTimer)
{
    FSP_PARAMETER_NOT_USED(xTimer);

    if (true == led_on)
    {
        utils_set_LED(GREEN_LED, BSP_IO_LEVEL_LOW);
        led_on = false;
    }
    else
    {
        utils_set_LED(GREEN_LED,BSP_IO_LEVEL_HIGH);
        led_on = true;
    }
}

static void handle_read_data(uint16_t id, uint8_t const * const data)
{
    FSP_PARAMETER_NOT_USED(data);
    uint32_t index = 2;
    while (index < (uint32_t)sm_get_total_sensor_count()+2) {
        if (qc_sv_req_handlers[index].id == id) {
            log_debug("Sending id: 0x%04x");
            qc_svc_send_read_response(QC_SVC_SUCCESS, id, sizeof(sensor_data_array[0]), (uint8_t *)&sensor_data_array[index-2]);
            break;
        }
        index++;
    }
}

/* End user code. Do not edit comment generated here */
