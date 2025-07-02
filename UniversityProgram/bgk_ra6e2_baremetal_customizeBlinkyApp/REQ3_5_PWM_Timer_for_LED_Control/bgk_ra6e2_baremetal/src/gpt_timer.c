/***********************************************************************************************************************
 * File Name    : gpt_timer.c
 * Description  : Contains function definition.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "gpt_timer.h"
#include "log_disabled.h"
//#include "log_error.h"
//#include "log_warning.h"
//#include "log_info.h"
//#include "log_debug.h"
/*******************************************************************************************************************//**
 * @addtogroup r_gpt_ep
 * @{
 **********************************************************************************************************************/

/* Boolean flag to determine one-shot mode timer is expired or not.*/

/* Store Timer open state*/

/*****************************************************************************************************************
 * @brief       Initialize GPT timer.
 * @param[in]   p_timer_ctl     Timer instance control structure
 * @param[in]   p_timer_cfg     Timer instance Configuration structure
 * @param[in]   timer_mode      Mode of GPT Timer
 * @retval      FSP_SUCCESS     Upon successful open of timer.
 * @retval      Any Other Error code apart from FSP_SUCCES on Unsuccessful open .
 ****************************************************************************************************************/
fsp_err_t init_gpt_timer(timer_ctrl_t * const p_timer_ctl, timer_cfg_t const * const p_timer_cfg)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Initialize GPT Timer */
    err = R_GPT_Open(p_timer_ctl, p_timer_cfg);
    if (FSP_SUCCESS != err)
    {
    	log_error ("\r\n ** R_GPT_TimerOpen FAILED ** \r\n");
        return err;
    }

    return err;
}

/*****************************************************************************************************************
 * @brief       Start GPT timers in periodic, one shot, PWM mode.
 * @param[in]   p_timer_ctl     Timer instance control structure
 * @retval      FSP_SUCCESS     Upon successful start of timer.
 * @retval      Any Other Error code apart from FSP_SUCCES on Unsuccessful start .
 ****************************************************************************************************************/
fsp_err_t start_gpt_timer (timer_ctrl_t * const p_timer_ctl)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Starts GPT timer */
    err = R_GPT_Start(p_timer_ctl);
    if (FSP_SUCCESS != err)
    {
        /* In case of GPT_open is successful and start fails, requires a immediate cleanup.
         * Since, cleanup for GPT open is done in start_gpt_timer,Hence cleanup is not required */
    	log_error ("\r\n ** R_GPT_Start API failed ** \r\n");
    }
    return err;
}



/*****************************************************************************************************************
 *  @brief       set  duty cycle of PWM timer.
 *  @param[in]   duty_cycle_percent.
 *  @retval      FSP_SUCCESS on correct duty cycle set.
 *  @retval      FSP_INVALID_ARGUMENT on invalid info.
 ****************************************************************************************************************/
fsp_err_t set_timer_duty_cycle(uint8_t duty_cycle_percent ,timer_ctrl_t * const p_timer_ctl)
{
    fsp_err_t err                           = FSP_SUCCESS;
    uint32_t duty_cycle_counts              = RESET_VALUE;
    uint32_t current_period_counts          = RESET_VALUE;
    timer_info_t info                       = {(timer_direction_t)RESET_VALUE, RESET_VALUE, RESET_VALUE};

    /* Get the current period setting. */
    err = R_GPT_InfoGet(p_timer_ctl, &info);
    if (FSP_SUCCESS != err)
    {
        /* GPT Timer InfoGet Failure message */
    	log_error ("\r\n ** R_GPT_InfoGet API failed ** \r\n");
    }
    else
    {
        /* update period counts locally. */
        current_period_counts = info.period_counts;

        /* Calculate the desired duty cycle based on the current period. Note that if the period could be larger than
         * UINT32_MAX / 100, this calculation could overflow. A cast to uint64_t is used to prevent this. The cast is
         * not required for 16-bit timers. */
        duty_cycle_counts =(uint32_t) ((uint64_t) (current_period_counts * duty_cycle_percent) /
                GPT_MAX_PERCENT);
#if defined(BOARD_RA4W1_EK) || defined (BOARD_RA6T1_RSSK) ||defined (BOARD_RA6T3_MCK) || defined (BOARD_RA4T1_MCK)
        duty_cycle_counts = (current_period_counts - duty_cycle_counts);
#endif

        /* Duty Cycle Set API set the desired intensity on the on-board LED */
        err = R_GPT_DutyCycleSet(p_timer_ctl, duty_cycle_counts, TIMER_PIN);
        if(FSP_SUCCESS != err)
        {
            /* GPT Timer DutyCycleSet Failure message */
            /* In case of GPT_open is successful and DutyCycleSet fails, requires a immediate cleanup.
             * Since, cleanup for GPT open is done in timer_duty_cycle_set,Hence cleanup is not required */
        	log_error ("\r\n ** R_GPT_DutyCycleSet API failed ** \r\n");
        }
    }
    return err;
}


/*****************************************************************************************************************
 *  @brief       set  duty cycle of PWM timer.
 *  @param[in]   duty_cycle_percent.
 *  @retval      FSP_SUCCESS on correct duty cycle set.
 *  @retval      FSP_INVALID_ARGUMENT on invalid info.
 ****************************************************************************************************************/
fsp_err_t set_timer_Period_and_Dutycycle(uint32_t period_counts ,uint8_t duty_cycle_percent, timer_ctrl_t * const p_timer_ctl)
{
    fsp_err_t err                           = FSP_SUCCESS;
    uint32_t duty_cycle_counts              = RESET_VALUE;
    uint32_t current_period_counts          = RESET_VALUE;
    timer_info_t info                       = {(timer_direction_t)RESET_VALUE, RESET_VALUE, RESET_VALUE};
    gpt_instance_ctrl_t * p_instance_ctrl = (gpt_instance_ctrl_t *) p_timer_ctl;
	/* PeriodSet Set API set the desired intensity on the on-board LED */
	err = R_GPT_PeriodSet(p_timer_ctl, period_counts);
	if(FSP_SUCCESS != err)
	{
		/* GPT Timer DutyCycleSet Failure message */
		/* In case of GPT_open is successful and DutyCycleSet fails, requires a immediate cleanup.
		 * Since, cleanup for GPT open is done in timer_duty_cycle_set,Hence cleanup is not required */
		log_error ("\r\n ** R_GPT_PeriodSet API failed ** \r\n");
	}


	current_period_counts = period_counts;

	/* Calculate the desired duty cycle based on the current period. Note that if the period could be larger than
	 * UINT32_MAX / 100, this calculation could overflow. A cast to uint64_t is used to prevent this. The cast is
	 * not required for 16-bit timers. */
	duty_cycle_counts =(uint32_t) ((uint64_t) (current_period_counts * duty_cycle_percent) /
			GPT_MAX_PERCENT);
#if defined(BOARD_RA4W1_EK) || defined (BOARD_RA6T1_RSSK) ||defined (BOARD_RA6T3_MCK) || defined (BOARD_RA4T1_MCK)
	duty_cycle_counts = (current_period_counts - duty_cycle_counts);
#endif

	/* Duty Cycle Set API set the desired intensity on the on-board LED */
	err = R_GPT_DutyCycleSet(p_timer_ctl, duty_cycle_counts, TIMER_PIN);
	if(FSP_SUCCESS != err)
	{
		/* GPT Timer DutyCycleSet Failure message */
		/* In case of GPT_open is successful and DutyCycleSet fails, requires a immediate cleanup.
		 * Since, cleanup for GPT open is done in timer_duty_cycle_set,Hence cleanup is not required */
		log_error ("\r\n ** R_GPT_DutyCycleSet API failed ** \r\n");
	}
    return err;
}


/*****************************************************************************************************************
 * @brief      Close the GPT HAL driver.
 * @param[in]  p_timer_ctl     Timer instance control structure
 * @retval     None
 ****************************************************************************************************************/
void deinit_gpt_timer(timer_ctrl_t * const p_timer_ctl)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Timer Close API call*/
    err = R_GPT_Close(p_timer_ctl);
    if (FSP_SUCCESS != err)
    {
        /* GPT Close failure message */
    	log_error("\r\n ** R_GPT_Close FAILED ** \r\n");
    }

}


/*******************************************************************************************************************//**
 * @} (end addtogroup r_gpt_ep)
 **********************************************************************************************************************/
