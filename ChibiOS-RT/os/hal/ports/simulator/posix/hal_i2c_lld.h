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
 * @file    hal_i2c_lld.h
 * @brief   PLATFORM I2C subsystem low level driver header.
 *
 * @addtogroup I2C
 * @{
 */

#ifndef HAL_I2C_LLD_H
#define HAL_I2C_LLD_H

#if defined(__FUZZ__)
#include "fuzz_core.h"
#endif

#if (HAL_USE_I2C == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    PLATFORM configuration options
 * @{
 */
/**
 * @brief   I2C1 driver enable switch.
 * @details If set to @p TRUE the support for I2C1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(POSIX_I2C_USE_I2C1) || defined(__DOXYGEN__)
#define POSIX_I2C_USE_I2C1                  FALSE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type representing an I2C address.
 */
typedef uint16_t i2caddr_t;

/**
 * @brief   Type of I2C Driver condition flags.
 */
typedef uint32_t i2cflags_t;

/**
 * @brief   Type of I2C driver configuration structure.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
typedef struct {
  /* End of the mandatory fields.*/
  uint32_t                  dummy;
} I2CConfig;

/**
 * @brief   Type of a structure representing an I2C driver.
 */
typedef struct I2CDriver I2CDriver;

/**
 * @brief   Structure representing an I2C driver.
 */
struct I2CDriver {
  /**
   * @brief   Driver state.
   */
  i2cstate_t                state;
  /**
   * @brief   Current configuration data.
   */
  const I2CConfig           *config;
  /**
   * @brief   Error flags.
   */
  i2cflags_t                errors;
#if (I2C_USE_MUTUAL_EXCLUSION == TRUE) || defined(__DOXYGEN__)
  mutex_t                   mutex;
#endif
#if defined(I2C_DRIVER_EXT_FIELDS)
  I2C_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
#if defined(__FUZZ__)
  /**
   * @brief Maintains fuzz buffer input state
   */
  fuzz_buffer_state_t       buffer;
#endif
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Get errors from I2C driver.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
#define i2c_lld_get_errors(i2cp) ((i2cp)->errors)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/
#ifdef POSIX_EMU_MSP430X

/**
 * @brief   Callback type for MSP430X-specific I2C functions
 */
typedef void (*i2ccallback_t)(I2CDriver *i2cp, uint8_t * buffer, uint16_t n);

void i2cMSP430XStartTransmitMSBI(I2CDriver *i2cp, i2caddr_t addr, size_t n,
                                 uint8_t * txbuf, i2ccallback_t callback);
void i2cMSP430XStartReceiveToRegI(I2CDriver *i2cp, i2caddr_t addr, size_t n,
                                  uint8_t * regp, i2ccallback_t callback);
void i2cMSP430XStartReceiveI(I2CDriver *i2cp, i2caddr_t addr, size_t n,
                             uint8_t * rxbuf, i2ccallback_t callback);
void i2cMSP430XContinueTransmitI(I2CDriver *i2cp, i2caddr_t addr, size_t n,
                                 uint8_t * txbuf, i2ccallback_t callback);
void i2cMSP430XContinueTransmitMemsetI(I2CDriver *i2cp, i2caddr_t addr, size_t n,
                                       uint8_t * value, i2ccallback_t callback);
void i2cMSP430XEndTransferI(I2CDriver *i2cp);
#endif

#if (POSIX_I2C_USE_I2C1 == TRUE) && !defined(__DOXYGEN__)
extern I2CDriver I2CD1;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void i2c_lld_init(void);
  void i2c_lld_start(I2CDriver *i2cp);
  void i2c_lld_stop(I2CDriver *i2cp);
  msg_t i2c_lld_master_transmit_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                        const uint8_t *txbuf, size_t txbytes,
                                        uint8_t *rxbuf, size_t rxbytes,
                                        systime_t timeout);
  msg_t i2c_lld_master_receive_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                       uint8_t *rxbuf, size_t rxbytes,
                                       systime_t timeout);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_I2C == TRUE */

#endif /* HAL_I2C_LLD_H */

/** @} */
