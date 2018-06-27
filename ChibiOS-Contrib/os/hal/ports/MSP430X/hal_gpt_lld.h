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
 * @file    hal_gpt_lld.h
 * @brief   MSP430X GPT subsystem low level driver header.
 *
 * @addtogroup GPT
 * @{
 */

#ifndef HAL_GPT_LLD_H
#define HAL_GPT_LLD_H

#if (HAL_USE_GPT == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    MSP430X configuration options
 * @{
 */
/**
 * @brief   GPTDA0 driver enable switch.
 * @details If set to @p TRUE the support for GPTD1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_GPT_USE_TA0) || defined(__DOXYGEN__)
#define MSP430X_GPT_USE_TA0               FALSE
#endif

/**
 * @brief   GPTDA1 driver enable switch.
 * @details If set to @p TRUE the support for GPTD1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_GPT_USE_TA1) || defined(__DOXYGEN__)
#define MSP430X_GPT_USE_TA1               FALSE
#endif

/**
 * @brief   GPTDA2 driver enable switch.
 * @details If set to @p TRUE the support for GPTD1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_GPT_USE_TA2) || defined(__DOXYGEN__)
#define MSP430X_GPT_USE_TA2               FALSE
#endif

/**
 * @brief   GPTDA3 driver enable switch.
 * @details If set to @p TRUE the support for GPTD1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_GPT_USE_TA3) || defined(__DOXYGEN__)
#define MSP430X_GPT_USE_TA3               FALSE
#endif

/**
 * @brief   GPTDB0 driver enable switch.
 * @details If set to @p TRUE the support for GPTD1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_GPT_USE_TB0) || defined(__DOXYGEN__)
#define MSP430X_GPT_USE_TB0               FALSE
#endif

/**
 * @brief   GPTDB1 driver enable switch.
 * @details If set to @p TRUE the support for GPTD1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_GPT_USE_TB1) || defined(__DOXYGEN__)
#define MSP430X_GPT_USE_TB1               FALSE
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if MSP430X_GPT_USE_TA0 && !defined(TIMER_A0_BASE)
  #error "Cannot find MSP430X Timer A0 module to use for GPTDA0"
#endif

#if MSP430X_GPT_USE_TA1 && !defined(TIMER_A1_BASE)
  #error "Cannot find MSP430X Timer A1 module to use for GPTDA1"
#endif

#if MSP430X_GPT_USE_TA2 && !defined(TIMER_A2_BASE)
  #error "Cannot find MSP430X Timer A2 module to use for GPTDA2"
#endif

#if MSP430X_GPT_USE_TA3 && !defined(TIMER_A3_BASE)
  #error "Cannot find MSP430X Timer A3 module to use for GPTDA3"
#endif

#if MSP430X_GPT_USE_TB0 && !defined(TIMER_B0_BASE)
  #error "Cannot find MSP430X Timer B0 module to use for GPTDB0"
#endif

#if MSP430X_GPT_USE_TB1 && !defined(TIMER_B1_BASE)
  #error "Cannot find MSP430X Timer B1 module to use for GPTDB1"
#endif

#if MSP430X_GPT_USE_TA0
  #if defined(MSP430X_TA0_USED)
    #error "Timer module A0 already in use - GPTDA0 not available"
  #else
    #define MSP430X_TA0_USED
  #endif
  #if MSP430X_TA0_CLK_SRC == MSP430X_ACLK_SRC
    #define MSP430X_TA0_CLK_FREQ MSP430X_ACLK_FREQ
    #define MSP430X_TA0_TASSEL TASSEL__ACLK
  #elif MSP430X_TA0_CLK_SRC == MSP430X_SMCLK_SRC
    #define MSP430X_TA0_CLK_FREQ MSP430X_SMCLK_FREQ
    #define MSP430X_TA0_TASSEL TASSEL__SMCLK
  #elif MSP430X_TA0_CLK_SRC == MSP430X_INCLK_SRC
    #if !defined(MSP430X_TA0_CLK_FREQ)
      #error "Requested INCLK as source for GPTDA0 but MSP430X_TA0_CLK_FREQ not defined"
    #endif
    #define MSP430X_TA0_TASSEL TASSEL__INCLK
  #else
    #if !defined(MSP430X_TA0_CLK_FREQ)
      #error "Requested TA0CLK as source for GPTDA0 but MSP430X_TA0_CLK_FREQ not defined"
    #endif
    #define MSP430X_TA0_TASSEL TASSEL__TACLK
  #endif
#endif

#if MSP430X_GPT_USE_TA1
  #if defined(MSP430X_TA1_USED)
    #error "Timer module A1 already in use - GPTDA1 not available"
  #else
    #define MSP430X_TA1_USED
  #endif
  #if MSP430X_TA1_CLK_SRC == MSP430X_ACLK_SRC
    #define MSP430X_TA1_CLK_FREQ MSP430X_ACLK_FREQ
    #define MSP430X_TA1_TASSEL TASSEL__ACLK
  #elif MSP430X_TA1_CLK_SRC == MSP430X_SMCLK_SRC
    #define MSP430X_TA1_CLK_FREQ MSP430X_SMCLK_FREQ
    #define MSP430X_TA1_TASSEL TASSEL__SMCLK
  #elif MSP430X_TA1_CLK_SRC == MSP430X_INCLK_SRC
    #if !defined(MSP430X_TA1_CLK_FREQ)
      #error "Requested INCLK as source for GPTDA1 but MSP430X_TA1_CLK_FREQ not defined"
    #endif
    #define MSP430X_TA1_TASSEL TASSEL__INCLK
  #else
    #if !defined(MSP430X_TA1_CLK_FREQ)
      #error "Requested TA1CLK as source for GPTDA1 but MSP430X_TA1_CLK_FREQ not defined"
    #endif
    #define MSP430X_TA1_TASSEL TASSEL__TACLK
  #endif
#endif

#if MSP430X_GPT_USE_TA2
  #if defined(MSP430X_TA2_USED)
    #error "Timer module A2 already in use - GPTDA2 not available"
  #else
    #define MSP430X_TA2_USED
  #endif
  #if MSP430X_TA2_CLK_SRC == MSP430X_ACLK_SRC
    #define MSP430X_TA2_CLK_FREQ MSP430X_ACLK_FREQ
    #define MSP430X_TA2_TASSEL TASSEL__ACLK
  #elif MSP430X_TA2_CLK_SRC == MSP430X_SMCLK_SRC
    #define MSP430X_TA2_CLK_FREQ MSP430X_SMCLK_FREQ
    #define MSP430X_TA2_TASSEL TASSEL__SMCLK
  #elif MSP430X_TA2_CLK_SRC == MSP430X_INCLK_SRC
    #if !defined(MSP430X_TA2_CLK_FREQ)
      #error "Requested INCLK as source for GPTDA2 but MSP430X_TA2_CLK_FREQ not defined"
    #endif
    #define MSP430X_TA2_TASSEL TASSEL__INCLK
  #else
    #if !defined(MSP430X_TA2_CLK_FREQ)
      #error "Requested TA2CLK as source for GPTDA2 but MSP430X_TA2_CLK_FREQ not defined"
    #endif
    #define MSP430X_TA2_TASSEL TASSEL__TACLK
  #endif
#endif

#if MSP430X_GPT_USE_TA3
  #if defined(MSP430X_TA3_USED)
    #error "Timer module A3 already in use - GPTDA3 not available"
  #else
    #define MSP430X_TA3_USED
  #endif
  #if MSP430X_TA3_CLK_SRC == MSP430X_ACLK_SRC
    #define MSP430X_TA3_CLK_FREQ MSP430X_ACLK_FREQ
    #define MSP430X_TA3_TASSEL TASSEL__ACLK
  #elif MSP430X_TA3_CLK_SRC == MSP430X_SMCLK_SRC
    #define MSP430X_TA3_CLK_FREQ MSP430X_SMCLK_FREQ
    #define MSP430X_TA3_TASSEL TASSEL__SMCLK
  #elif MSP430X_TA3_CLK_SRC == MSP430X_INCLK_SRC
    #if !defined(MSP430X_TA3_CLK_FREQ)
      #error "Requested INCLK as source for GPTDA3 but MSP430X_TA3_CLK_FREQ not defined"
    #endif
    #define MSP430X_TA3_TASSEL TASSEL__INCLK
  #else
    #if !defined(MSP430X_TA3_CLK_FREQ)
      #error "Requested TA3CLK as source for GPTDA3 but MSP430X_TA3_CLK_FREQ not defined"
    #endif
    #define MSP430X_TA3_TASSEL TASSEL__TACLK
  #endif
#endif

#if MSP430X_GPT_USE_TB0
  #if defined(MSP430X_TB0_USED)
    #error "Timer module B0 already in use - GPTDB0 not available"
  #else
    #define MSP430X_TB0_USED
  #endif
  #if MSP430X_TB0_CLK_SRC == MSP430X_ACLK_SRC
    #define MSP430X_TB0_CLK_FREQ MSP430X_ACLK_FREQ
    #define MSP430X_TB0_TASSEL TASSEL__ACLK
  #elif MSP430X_TB0_CLK_SRC == MSP430X_SMCLK_SRC
    #define MSP430X_TB0_CLK_FREQ MSP430X_SMCLK_FREQ
    #define MSP430X_TB0_TASSEL TASSEL__SMCLK
  #elif MSP430X_TB0_CLK_SRC == MSP430X_INCLK_SRC
    #if !defined(MSP430X_TB0_CLK_FREQ)
      #error "Requested INCLK as source for GPTDB0 but MSP430X_TB0_CLK_FREQ not defined"
    #endif
    #define MSP430X_TB0_TASSEL TASSEL__INCLK
  #else
    #if !defined(MSP430X_TB0_CLK_FREQ)
      #error "Requested TB0CLK as source for GPTDB0 but MSP430X_TB0_CLK_FREQ not defined"
    #endif
    #define MSP430X_TB0_TASSEL TASSEL__TACLK
  #endif
#endif

#if MSP430X_GPT_USE_TB1
  #if defined(MSP430X_TB1_USED)
    #error "Timer module B1 already in use - GPTDB1 not available"
  #else
    #define MSP430X_TB1_USED
  #endif
  #if MSP430X_TB1_CLK_SRC == MSP430X_ACLK_SRC
    #define MSP430X_TB1_CLK_FREQ MSP430X_ACLK_FREQ
    #define MSP430X_TB1_TASSEL TASSEL__ACLK
  #elif MSP430X_TB1_CLK_SRC == MSP430X_SMCLK_SRC
    #define MSP430X_TB1_CLK_FREQ MSP430X_SMCLK_FREQ
    #define MSP430X_TB1_TASSEL TASSEL__SMCLK
  #elif MSP430X_TB1_CLK_SRC == MSP430X_INCLK_SRC
    #if !defined(MSP430X_TB1_CLK_FREQ)
      #error "Requested INCLK as source for GPTDB1 but MSP430X_TB1_CLK_FREQ not defined"
    #endif
    #define MSP430X_TB1_TASSEL TASSEL__INCLK
  #else
    #if !defined(MSP430X_TB1_CLK_FREQ)
      #error "Requested TB1CLK as source for GPTDB1 but MSP430X_TB1_CLK_FREQ not defined"
    #endif
    #define MSP430X_TB1_TASSEL TASSEL__TACLK
  #endif
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   GPT frequency type.
 */
typedef uint32_t gptfreq_t;

/**
 * @brief   GPT counter type.
 */
typedef uint16_t gptcnt_t;

/**
 * @brief   MSP430X Timer registers struct
 */
typedef struct {
  uint16_t ctl;
  uint16_t cctl0;
  uint16_t cctl1;
  uint16_t cctl2;
  uint16_t cctl3;
  uint16_t cctl4;
  uint16_t cctl5;
  uint16_t cctl6;
  uint16_t r;
  uint16_t ccr0;
  uint16_t ccr1;
  uint16_t ccr2;
  uint16_t ccr3;
  uint16_t ccr4;
  uint16_t ccr5;
  uint16_t ccr6;
  uint16_t ex0;
  uint16_t padding[6];
  uint16_t iv;
} msp430x_timer_reg_t;

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
  /**
   * @brief   Timer clock in Hz.
   * @note    The low level can use assertions in order to catch invalid
   *          frequency specifications.
   */
  gptfreq_t                 frequency;
  /**
   * @brief   Timer callback pointer.
   * @note    This callback is invoked on GPT counter events.
   */
  gptcallback_t             callback;
  /* End of the mandatory fields.*/
} GPTConfig;

/**
 * @brief   Structure representing a GPT driver.
 */
struct GPTDriver {
  /**
   * @brief Driver state.
   */
  gptstate_t                state;
  /**
   * @brief Current configuration data.
   */
  const GPTConfig           *config;
#if defined(GPT_DRIVER_EXT_FIELDS)
  GPT_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  msp430x_timer_reg_t * regs;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Changes the interval of GPT peripheral.
 * @details This function changes the interval of a running GPT unit.
 * @pre     The GPT unit must have been activated using @p gptStart().
 * @pre     The GPT unit must have been running in continuous mode using
 *          @p gptStartContinuous().
 * @post    The GPT unit interval is changed to the new value.
 * @note    The function has effect at the next cycle start.
 *
 * @param[in] gptp      pointer to a @p GPTDriver object
 * @param[in] interval  new cycle time in timer ticks
 * @notapi
 */
#define gpt_lld_change_interval(gptp, interval) {                           \
  (void)gptp;                                                               \
  (void)interval;                                                           \
}

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (MSP430X_GPT_USE_TA0 == TRUE) && !defined(__DOXYGEN__)
extern GPTDriver GPTDA0;
#endif

#if (MSP430X_GPT_USE_TA1 == TRUE) && !defined(__DOXYGEN__)
extern GPTDriver GPTDA1;
#endif

#if (MSP430X_GPT_USE_TA2 == TRUE) && !defined(__DOXYGEN__)
extern GPTDriver GPTDA2;
#endif

#if (MSP430X_GPT_USE_TA3 == TRUE) && !defined(__DOXYGEN__)
extern GPTDriver GPTDA3;
#endif

#if (MSP430X_GPT_USE_TB0 == TRUE) && !defined(__DOXYGEN__)
extern GPTDriver GPTDB0;
#endif

#if (MSP430X_GPT_USE_TB1 == TRUE) && !defined(__DOXYGEN__)
extern GPTDriver GPTDB1;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void gpt_lld_init(void);
  void gpt_lld_start(GPTDriver *gptp);
  void gpt_lld_stop(GPTDriver *gptp);
  void gpt_lld_start_timer(GPTDriver *gptp, gptcnt_t interval);
  void gpt_lld_stop_timer(GPTDriver *gptp);
  void gpt_lld_polled_delay(GPTDriver *gptp, gptcnt_t interval);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_GPT == TRUE */

#endif /* HAL_GPT_LLD_H */

/** @} */
