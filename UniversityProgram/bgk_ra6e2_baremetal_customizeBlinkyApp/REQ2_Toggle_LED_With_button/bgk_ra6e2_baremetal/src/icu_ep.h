/***********************************************************************************************************************
 * File Name    : icu_ep.h
 * Description  : Contains Macros and function declarations.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/
#ifndef ICU_EP_H_
#define ICU_EP_H_
 
#define USER_SW1_IRQ_NUMBER    (9)          
 
  
 
 
/* Function declaration */
fsp_err_t icu_init(void);
fsp_err_t icu_enable(void);
void icu_deinit(void);
 
#endif /* ICU_EP_H_ */