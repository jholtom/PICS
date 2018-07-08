/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio
    ChibiOS - Copyright (C) 2018 Reed Koser

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
 * @brief   PLATFORM GPT subsystem low level driver source.
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
 * @brief   GPTD1 driver identifier.
 */
#if (POSIX_GPT_USE_GPT1 == TRUE) || defined(__DOXYGEN__)
GPTDriver GPTD1;
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

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level GPT driver initialization.
 *
 * @notapi
 */
void gpt_lld_init(void) {

#if POSIX_GPT_USE_GPT1 == TRUE
  /* Driver initialization.*/
  gptObjectInit(&GPTD1);
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

  if (gptp->state == GPT_STOP) {
    /* Enables the peripheral.*/
#if POSIX_GPT_USE_GPT1 == TRUE
    if (&GPTD1 == gptp) {
    }
#endif
  }
  /* Configures the peripheral.*/

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

    /* Disables the peripheral.*/
#if POSIX_GPT_USE_GPT1 == TRUE
    if (&GPTD1 == gptp) {
#if defined(__FUZZ__)
#else
#error "Only implemented for fuzz mode"
#endif
    }
#endif
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
#if defined(__FUZZ__)
  (void)gptp;
  (void)interval;
#else
#error "Only implemented for fuzz mode"
#endif
}

/**
 * @brief   Stops the timer.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 *
 * @notapi
 */
void gpt_lld_stop_timer(GPTDriver *gptp) {
#if defined(__FUZZ__)
  (void)gptp;
#else
#error "Only implemented for fuzz mode"
#endif
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
#if defined(__FUZZ__)
  (void)gptp;
  (void)interval;
  // Do no work in fuzzing mode
#else
#error "Only implemented for fuzz mode"
#endif
}

#endif /* HAL_USE_GPT == TRUE */

bool gpt_lld_next_bit(GPTDriver *gptp) {
  if (gptp->state == GPT_ONESHOT || gptp->state == GPT_CONTINUOUS) {
    gptp->state = GPT_READY;
    return (fuzz_next_bit(&gptp->random_state) & 1);
  }
  return false;
}

/** @} */
