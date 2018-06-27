/*
    ChibiOS - Copyright (C) 2016 Andrew Wygle aka awygle

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    hal_gpt_lld.c
 * @brief   MSP430X GPT subsystem low level driver source.
 *
 * @addtogroup GPT
 * @{
 */

#include "hal.h"

#if (HAL_USE_GPT == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   GPTDA0 driver identifier.
 */
#if (MSP430X_GPT_USE_TA0 == TRUE) || defined(__DOXYGEN__)
GPTDriver GPTDA0;
#endif

/**
 * @brief   GPTDA1 driver identifier.
 */
#if (MSP430X_GPT_USE_TA1 == TRUE) || defined(__DOXYGEN__)
GPTDriver GPTDA1;
#endif

/**
 * @brief   GPTDA2 driver identifier.
 */
#if (MSP430X_GPT_USE_TA2 == TRUE) || defined(__DOXYGEN__)
GPTDriver GPTDA2;
#endif

/**
 * @brief   GPTDA3 driver identifier.
 */
#if (MSP430X_GPT_USE_TA3 == TRUE) || defined(__DOXYGEN__)
GPTDriver GPTDA3;
#endif

/**
 * @brief   GPTDB0 driver identifier.
 */
#if (MSP430X_GPT_USE_TB0 == TRUE) || defined(__DOXYGEN__)
GPTDriver GPTDB0;
#endif

/**
 * @brief   GPTDB1 driver identifier.
 */
#if (MSP430X_GPT_USE_TB1 == TRUE) || defined(__DOXYGEN__)
GPTDriver GPTDB1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if MSP430X_GPT_USE_TA0 == TRUE
PORT_IRQ_HANDLER(TIMER0_A1_VECTOR) {
  OSAL_IRQ_PROLOGUE();
  
  switch(__even_in_range(TA0IV, TA0IV_TAIFG)) {
    case TA0IV_TAIFG:
      /* Timer has expired */
      if ((GPTDA0.state == GPT_ONESHOT || 
            GPTDA0.state == GPT_CONTINUOUS) &&
          NULL != GPTDA0.config->callback) {
        GPTDA0.config->callback(&GPTDA0);
      }
      
      if (GPTDA0.state == GPT_ONESHOT) {
        GPTDA0.state = GPT_READY;
        gpt_lld_stop_timer(&GPTDA0);
      }
      
  }

  OSAL_IRQ_EPILOGUE();
}
#endif

#if MSP430X_GPT_USE_TA1 == TRUE
PORT_IRQ_HANDLER(TIMER1_A1_VECTOR) {
  OSAL_IRQ_PROLOGUE();
  
  switch(__even_in_range(TA1IV, TA1IV_TAIFG)) {
    case TA1IV_TAIFG:
      /* Timer has expired */
      if ((GPTDA1.state == GPT_ONESHOT || 
            GPTDA1.state == GPT_CONTINUOUS) &&
          NULL != GPTDA1.config->callback) {
        GPTDA1.config->callback(&GPTDA1);
      }
      
      if (GPTDA1.state == GPT_ONESHOT) {
        GPTDA1.state = GPT_READY;
        gpt_lld_stop_timer(&GPTDA1);
      }
      
  }

  OSAL_IRQ_EPILOGUE();
}
#endif

#if MSP430X_GPT_USE_TA2 == TRUE
PORT_IRQ_HANDLER(TIMER2_A1_VECTOR) {
  OSAL_IRQ_PROLOGUE();
  
  switch(__even_in_range(TA2IV, TA2IV_TAIFG)) {
    case TA2IV_TAIFG:
      /* Timer has expired */
      if ((GPTDA2.state == GPT_ONESHOT || 
            GPTDA2.state == GPT_CONTINUOUS) &&
          NULL != GPTDA2.config->callback) {
        GPTDA2.config->callback(&GPTDA2);
      }
      
      if (GPTDA2.state == GPT_ONESHOT) {
        GPTDA2.state = GPT_READY;
        gpt_lld_stop_timer(&GPTDA2);
      }
      
  }

  OSAL_IRQ_EPILOGUE();
}
#endif

#if MSP430X_GPT_USE_TA3 == TRUE
PORT_IRQ_HANDLER(TIMER3_A1_VECTOR) {
  OSAL_IRQ_PROLOGUE();
  
  switch(__even_in_range(TA3IV, TA3IV_TAIFG)) {
    case TA3IV_TAIFG:
      /* Timer has expired */
      if ((GPTDA3.state == GPT_ONESHOT || 
            GPTDA3.state == GPT_CONTINUOUS) &&
          NULL != GPTDA3.config->callback) {
        GPTDA3.config->callback(&GPTDA3);
      }
      
      if (GPTDA3.state == GPT_ONESHOT) {
        GPTDA3.state = GPT_READY;
        gpt_lld_stop_timer(&GPTDA3);
      }
      
  }

  OSAL_IRQ_EPILOGUE();
}
#endif

#if MSP430X_GPT_USE_TB0 == TRUE
PORT_IRQ_HANDLER(TIMER0_B1_VECTOR) {
  OSAL_IRQ_PROLOGUE();
  
  switch(__even_in_range(TB0IV, TB0IV_TAIFG)) {
    case TB0IV_TAIFG:
      /* Timer has expired */
      if ((GPTDB0.state == GPT_ONESHOT || 
            GPTDB0.state == GPT_CONTINUOUS) &&
          NULL != GPTDB0.config->callback) {
        GPTDB0.config->callback(&GPTDB0);
      }
      
      if (GPTDB0.state == GPT_ONESHOT) {
        GPTDB0.state = GPT_READY;
        gpt_lld_stop_timer(&GPTDB0);
      }
      
  }

  OSAL_IRQ_EPILOGUE();
}
#endif

#if MSP430X_GPT_USE_TB1 == TRUE
PORT_IRQ_HANDLER(TIMER1_B1_VECTOR) {
  OSAL_IRQ_PROLOGUE();
  
  switch(__even_in_range(TB1IV, TB1IV_TAIFG)) {
    case TB1IV_TAIFG:
      /* Timer has expired */
      if ((GPTDB1.state == GPT_ONESHOT || 
            GPTDB1.state == GPT_CONTINUOUS) &&
          NULL != GPTDB1.config->callback) {
        GPTDB1.config->callback(&GPTDB1);
      }
      
      if (GPTDB1.state == GPT_ONESHOT) {
        GPTDB1.state = GPT_READY;
        gpt_lld_stop_timer(&GPTDB1);
      }
      
  }

  OSAL_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level GPT driver initialization.
 *
 * @notapi
 */
void gpt_lld_init(void) {

#if MSP430X_GPT_USE_TA0 == TRUE
  /* Driver initialization.*/
  GPTDA0.regs = (msp430x_timer_reg_t *)(&TA0CTL);
  gptObjectInit(&GPTDA0);
#endif
#if MSP430X_GPT_USE_TA1 == TRUE
  /* Driver initialization.*/
  GPTDA1.regs = (msp430x_timer_reg_t *)(&TA1CTL);
  gptObjectInit(&GPTDA1);
#endif
#if MSP430X_GPT_USE_TA2 == TRUE
  /* Driver initialization.*/
  GPTDA2.regs = (msp430x_timer_reg_t *)(&TA2CTL);
  gptObjectInit(&GPTDA2);
#endif
#if MSP430X_GPT_USE_TA3 == TRUE
  /* Driver initialization.*/
  GPTDA3.regs = (msp430x_timer_reg_t *)(&TA3CTL);
  gptObjectInit(&GPTDA3);
#endif
#if MSP430X_GPT_USE_TB0 == TRUE
  /* Driver initialization.*/
  GPTDB0.regs = (msp430x_timer_reg_t *)(&TB0CTL);
  gptObjectInit(&GPTDB0);
#endif
#if MSP430X_GPT_USE_TB1 == TRUE
  /* Driver initialization.*/
  GPTDB1.regs = (msp430x_timer_reg_t *)(&TB1CTL);
  gptObjectInit(&GPTDB1);
#endif
}

/**
 * @brief   Configures and activates the GPT peripheral.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 *
 * @notapi
 */
void gpt_lld_start(GPTDriver *gptp) {
  uint32_t divider = 0;

  /* Configures the peripheral.*/
#if MSP430X_GPT_USE_TA0 == TRUE
  if (&GPTDA0 == gptp) {
    /* Make sure the timer is stopped */
    gptp->regs->ctl = MSP430X_TA0_TASSEL | TACLR;
    
    /* Calculate the divider */
    divider = MSP430X_TA0_CLK_FREQ / gptp->config->frequency;
  }
#endif
#if MSP430X_GPT_USE_TA1 == TRUE
  if (&GPTDA1 == gptp) {
    /* Make sure the timer is stopped */
    gptp->regs->ctl = MSP430X_TA1_TASSEL | TACLR;
    
    /* Calculate the divider */
    divider = MSP430X_TA1_CLK_FREQ / gptp->config->frequency;
  }
#endif
#if MSP430X_GPT_USE_TA2 == TRUE
  if (&GPTDA2 == gptp) {
    /* Make sure the timer is stopped */
    gptp->regs->ctl = MSP430X_TA2_TASSEL | TACLR;
    
    /* Calculate the divider */
    divider = MSP430X_TA2_CLK_FREQ / gptp->config->frequency;
  }
#endif
#if MSP430X_GPT_USE_TA3 == TRUE
  if (&GPTDA3 == gptp) {
    /* Make sure the timer is stopped */
    gptp->regs->ctl = MSP430X_TA3_TASSEL | TACLR;
    
    /* Calculate the divider */
    divider = MSP430X_TA3_CLK_FREQ / gptp->config->frequency;
  }
#endif
#if MSP430X_GPT_USE_TB0 == TRUE
  if (&GPTDB0 == gptp) {
    /* Make sure the timer is stopped */
    gptp->regs->ctl = MSP430X_TB0_TASSEL | TACLR;
    
    /* Calculate the divider */
    divider = MSP430X_TB0_CLK_FREQ / gptp->config->frequency;
  }
#endif
#if MSP430X_GPT_USE_TB0 == TRUE
  if (&GPTDB0 == gptp) {
    /* Make sure the timer is stopped */
    gptp->regs->ctl = MSP430X_TB0_TASSEL | TACLR;
    
    /* Calculate the divider */
    divider = MSP430X_TB0_CLK_FREQ / gptp->config->frequency;
  }
#endif
  osalDbgAssert(divider <= 64, "Invalid timer frequency w/ current config");
  if (!(divider & 0x00000007)) { /* divisible by 8 */
    gptp->regs->ctl |= ID__8; /* Input Divider == 8 */
    divider /= 8;
  }
  else if (!(divider & 0x00000003)) { /* divisible by 4 */
    gptp->regs->ctl |= ID__4; /* Input Divider == 4 */
    divider /= 4;
  }
  else if (!(divider & 0x00000001)) { /* divisible by 2 */
    gptp->regs->ctl |= ID__2; /* Input Divider == 2 */
    divider /= 2;
  }
  osalDbgAssert(divider <= 8, "Invalid timer frequency w/ current config");
  gptp->regs->ex0 = divider - 1; /* Extra Divider */

}

/**
 * @brief   Deactivates the GPT peripheral.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 *
 * @notapi
 */
void gpt_lld_stop(GPTDriver *gptp) {

  if (gptp->state == GPT_READY) {
    /* Resets the peripheral.*/
    gptp->regs->ctl = TACLR;
    gptp->regs->cctl0 = 0;

  }
}

/**
 * @brief   Starts the timer in continuous mode.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 * @param[in] interval  period in ticks
 *
 * @notapi
 */
void gpt_lld_start_timer(GPTDriver *gptp, gptcnt_t interval) {

  /* Enable interrupt if callback required */
  if (NULL != gptp->config->callback) {
    gptp->regs->ctl |= TAIE;
  }
  /* Clear timer */
  gptp->regs->ctl |= TACLR;
  /* CCR0 is the interval */
  gptp->regs->ccr0 = interval;
  /* Up mode */
  gptp->regs->ctl |= MC__UP;
}

/**
 * @brief   Stops the timer.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 *
 * @notapi
 */
void gpt_lld_stop_timer(GPTDriver *gptp) {

  /* Stop timer */
  gptp->regs->ctl &= ~MC_3;

}

/**
 * @brief   Starts the timer in one shot mode and waits for completion.
 * @details This function specifically polls the timer waiting for completion
 *          in order to not have extra delays caused by interrupt servicing,
 *          this function is only recommended for short delays.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 * @param[in] interval  time interval in ticks
 *
 * @notapi
 */
void gpt_lld_polled_delay(GPTDriver *gptp, gptcnt_t interval) {

  /* Clear timer */
  gptp->regs->ctl |= TACLR;
  /* CCR0 is the interval */
  gptp->regs->ccr0 = interval;
  /* Up mode */
  gptp->regs->ctl |= MC__UP;
  
  /* Poll for completion */
  while (!(gptp->regs->ctl & TAIFG));

}

#endif /* HAL_USE_GPT == TRUE */

/** @} */
