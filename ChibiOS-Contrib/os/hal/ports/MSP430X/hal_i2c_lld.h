/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

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
 * @brief   MSP430X I2C subsystem low level driver header.
 *
 * @addtogroup I2C
 * @{
 */

#ifndef HAL_I2C_LLD_H
#define HAL_I2C_LLD_H

#if (HAL_USE_I2C == TRUE) || defined(__DOXYGEN__)

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
 * @brief   I2CB0 driver enable switch.
 * @details If set to @p TRUE the support for I2CB0 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_I2C_USE_I2CB0) || defined(__DOXYGEN__)
#define MSP430X_I2C_USE_I2CB0                 FALSE
#endif

/**
 * @brief   I2CB1 driver enable switch.
 * @details If set to @p TRUE the support for I2CB1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_I2C_USE_I2CB1) || defined(__DOXYGEN__)
#define MSP430X_I2C_USE_I2CB1                 FALSE
#endif

/**
 * @brief   I2CB2 driver enable switch.
 * @details If set to @p TRUE the support for I2CB2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_I2C_USE_I2CB2) || defined(__DOXYGEN__)
#define MSP430X_I2C_USE_I2CB2                 FALSE
#endif

/**
 * @brief   I2CB3 driver enable switch.
 * @details If set to @p TRUE the support for I2CB3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_I2C_USE_I2CB3) || defined(__DOXYGEN__)
#define MSP430X_I2C_USE_I2CB3                 FALSE
#endif

/**
 * @brief   I2CB0 clock source switch.
 * @details Sets the clock source for I2CB0.
 * @note    Legal values are undefined, @p MSP430X_SMCLK_SRC or 
 * @p MSP430X_ACLK_SRC.
 * @note    Undefined defaults to UCLKI which requires MSP430X_I2CB0_CLK_FREQ
 * to be set.
 */
#if MSP430X_I2C_USE_I2CB0 && !defined(MSP430X_I2CB0_CLK_SRC) 
  #if !defined(MSP430X_I2CB0_CLK_FREQ)
    #error "Requested external clock for I2C on USCI_B0 but no frequency given"
  #else
    #define MSP430X_I2CB0_UCSSEL UCSSEL__UCLK
  #endif
#endif

/**
 * @brief   I2CB1 clock source switch.
 * @details Sets the clock source for I2CB1.
 * @note    Legal values are undefined, @p MSP430X_SMCLK_SRC or 
 * @p MSP430X_ACLK_SRC.
 * @note    Undefined defaults to UCLKI which requires MSP430X_I2CB1_CLK_FREQ
 * to be set.
 */
#if MSP430X_I2C_USE_I2CB1 && !defined(MSP430X_I2CB1_CLK_SRC) 
  #if !defined(MSP430X_I2CB1_CLK_FREQ)
    #error "Requested external clock for I2C on USCI_B1 but no frequency given"
  #else
    #define MSP430X_I2CB1_UCSSEL UCSSEL__UCLK
  #endif
#endif

/**
 * @brief   I2CB2 clock source switch.
 * @details Sets the clock source for I2CB2.
 * @note    Legal values are undefined, @p MSP430X_SMCLK_SRC or 
 * @p MSP430X_ACLK_SRC.
 * @note    Undefined defaults to UCLKI which requires MSP430X_I2CB2_CLK_FREQ
 * to be set.
 */
#if MSP430X_I2C_USE_I2CB2 && !defined(MSP430X_I2CB2_CLK_SRC) 
  #if !defined(MSP430X_I2CB2_CLK_FREQ)
    #error "Requested external clock for I2C on USCI_B2 but no frequency given"
  #else
    #define MSP430X_I2CB2_UCSSEL UCSSEL__UCLK
  #endif
#endif

/**
 * @brief   I2CB3 clock source switch.
 * @details Sets the clock source for I2CB3.
 * @note    Legal values are undefined, @p MSP430X_SMCLK_SRC or 
 * @p MSP430X_ACLK_SRC.
 * @note    Undefined defaults to UCLKI which requires MSP430X_I2CB3_CLK_FREQ
 * to be set.
 */
#if MSP430X_I2C_USE_I2CB3 && !defined(MSP430X_I2CB3_CLK_SRC) 
  #if !defined(MSP430X_I2CB3_CLK_FREQ)
    #error "Requested external clock for I2C on USCI_B3 but no frequency given"
  #else
    #define MSP430X_I2CB3_UCSSEL UCSSEL__UCLK
  #endif
#endif

/**
 * @brief   10-bit address support switch
 * @details Determines whether or not to include support for 10-bit addressing
 * @note The default is @p FALSE
 */
#if !defined(MSP430X_I2C_10BIT)
  #define MSP430X_I2C_10BIT FALSE
#endif

/**
 * @brief   Exclusive DMA support switch
 * @details Determines whether or not to include support for exclusive DMA
 * @note The default is @p FALSE
 */
#if !defined(MSP430X_I2C_EXCLUSIVE_DMA)
  #define MSP430X_I2C_EXCLUSIVE_DMA FALSE
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if MSP430X_I2C_USE_I2CB0 && !defined(__MSP430_HAS_EUSCI_B0__)
  #error "Cannot find MSP430X_USCI module to use for I2CB0"
#endif

#if MSP430X_I2C_USE_I2CB1 && !defined(__MSP430_HAS_EUSCI_B1__)
  #error "Cannot find MSP430X_USCI module to use for I2CB1"
#endif

#if MSP430X_I2C_USE_I2CB2 && !defined(__MSP430_HAS_EUSCI_B2__)
  #error "Cannot find MSP430X_USCI module to use for I2CB2"
#endif

#if MSP430X_I2C_USE_I2CB3 && !defined(__MSP430_HAS_EUSCI_B3__)
  #error "Cannot find MSP430X_USCI module to use for I2CB3"
#endif

#if MSP430X_I2C_USE_I2CB0
  #if defined(MSP430X_USCI_B0_USED)
      #error "USCI module B0 already in use - I2CB0 not available"
  #else
    #define MSP430X_USCI_B0_USED
  #endif
  #if MSP430X_I2CB0_CLK_SRC == MSP430X_ACLK_SRC
    #define MSP430X_I2CB0_CLK_FREQ MSP430X_ACLK_FREQ
    #define MSP430X_I2CB0_UCSSEL UCSSEL__ACLK
  #elif MSP430X_I2CB0_CLK_SRC == MSP430X_SMCLK_SRC
    #define MSP430X_I2CB0_CLK_FREQ MSP430X_SMCLK_FREQ
    #define MSP430X_I2CB0_UCSSEL UCSSEL__SMCLK
  #endif
#endif

#if MSP430X_I2C_USE_I2CB1
  #if defined(MSP430X_USCI_B1_USED)
      #error "USCI module B1 already in use - I2CB1 not available"
  #else
    #define MSP430X_USCI_B1_USED
  #endif
  #if MSP430X_I2CB1_CLK_SRC == MSP430X_ACLK_SRC
    #define MSP430X_I2CB1_CLK_FREQ MSP430X_ACLK_FREQ
    #define MSP430X_I2CB1_UCSSEL UCSSEL__ACLK
  #elif MSP430X_I2CB1_CLK_SRC == MSP430X_SMCLK_SRC
    #define MSP430X_I2CB1_CLK_FREQ MSP430X_SMCLK_FREQ
    #define MSP430X_I2CB1_UCSSEL UCSSEL__SMCLK
  #endif
#endif

#if MSP430X_I2C_USE_I2CB2
  #if defined(MSP430X_USCI_B2_USED)
      #error "USCI module B2 already in use - I2CB2 not available"
  #else
    #define MSP430X_USCI_B2_USED
  #endif
  #if MSP430X_I2CB2_CLK_SRC == MSP430X_ACLK_SRC
    #define MSP430X_I2CB2_CLK_FREQ MSP430X_ACLK_FREQ
    #define MSP430X_I2CB2_UCSSEL UCSSEL__ACLK
  #elif MSP430X_I2CB2_CLK_SRC == MSP430X_SMCLK_SRC
    #define MSP430X_I2CB2_CLK_FREQ MSP430X_SMCLK_FREQ
    #define MSP430X_I2CB2_UCSSEL UCSSEL__SMCLK
  #endif
#endif

#if MSP430X_I2C_USE_I2CB3
  #if defined(MSP430X_USCI_B3_USED)
      #error "USCI module B3 already in use - I2CB3 not available"
  #else
    #define MSP430X_USCI_B3_USED
  #endif
  #if MSP430X_I2CB3_CLK_SRC == MSP430X_ACLK_SRC
    #define MSP430X_I2CB3_CLK_FREQ MSP430X_ACLK_FREQ
    #define MSP430X_I2CB3_UCSSEL UCSSEL__ACLK
  #elif MSP430X_I2CB3_CLK_SRC == MSP430X_SMCLK_SRC
    #define MSP430X_I2CB3_CLK_FREQ MSP430X_SMCLK_FREQ
    #define MSP430X_I2CB3_UCSSEL UCSSEL__SMCLK
  #endif
#endif
    
/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type representing an I2C address.
 */
#if MSP430X_I2C_10BIT
  typedef uint16_t i2caddr_t;
#else
  typedef uint8_t i2caddr_t;
#endif

/**
 * @brief   Type of I2C Driver condition flags.
 */
typedef uint16_t i2cflags_t;

/**
 * @brief   MSP I2C peripheral registers
 */
typedef struct {
  uint16_t ctlw0;
  uint16_t ctlw1;
  uint16_t _padding0;
  uint16_t brw;
  uint16_t statw;
  uint16_t tbcnt;
  uint16_t rxbuf;
  uint16_t txbuf;
  uint16_t _padding1;
  uint16_t _padding2;
  uint16_t i2coa0;
  uint16_t i2coa1;
  uint16_t i2coa2;
  uint16_t i2coa3;
  uint16_t addrx;
  uint16_t addmask;
  uint16_t i2csa;
  uint16_t _padding3;
  uint16_t _padding4;
  uint16_t _padding5;
  uint16_t _padding6;
  uint16_t ie;
  uint16_t ifg;
  uint16_t iv;
} msp430x_i2c_reg_t;

/**
 * @brief   Type of I2C driver configuration structure.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
typedef struct {
  /* End of the mandatory fields.*/
  /**
   * @brief The bit rate of the I2C interface
   * @note Nearest available rate is used.
   */
  uint32_t                  bit_rate;
#if MSP430X_I2C_10BIT == TRUE || defined(__DOXYGEN__)
  /**
   * @brief The addressing mode of the I2C interface - @p 1 is 10 bit, @p 0 is
   * 7 bit.
   */
  uint8_t                   long_addr : 1;
#endif
#if MSP430X_I2C_EXCLUSIVE_DMA == TRUE || defined(__DOXYGEN__)
  /**
   * @brief The index of the DMA channel.
   * @note This may be >MSP430X_DMA_CHANNELS to indicate that exclusive DMA is
   * not used.
   */
  uint8_t                   dma_index : 4;
#endif
} I2CConfig;

/**
 * @brief   Type of a structure representing an I2C driver.
 */
typedef struct I2CDriver I2CDriver;

/**
 * @brief   Callback type for MSP430X-specific I2C functions
 */
typedef void (*i2ccallback_t)(I2CDriver *i2cp, uint8_t * buffer, uint16_t n);

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
  /**
   * @brief   The active buffer
   */
  uint8_t * buffer;
  /**
   * @brief   The active callback
   */
  i2ccallback_t callback;
  /**
   * @brief   Configuration registers.
   */
  volatile msp430x_i2c_reg_t * regs;
  /**
   * @brief   DMA request.
   */
  msp430x_dma_req_t req;
  /**
   * @brief   Thread reference for timeouts
   */
  thread_reference_t thread;
  /**
   * @brief   DMA trigger reference for transmit
   */
  uint8_t txtrig;
  /**
   * @brief   DMA trigger reference for receive
   */
  uint8_t rxtrig;
  /**
   * @brief   DMA stream.
   */
  msp430x_dma_ch_t dma;
#if (MSP430X_I2C_EXCLUSIVE_DMA == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   Exclusive DMA mode
   */
  bool dma_acquired;
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

#if (MSP430X_I2C_USE_I2CB0 == TRUE) && !defined(__DOXYGEN__)
extern I2CDriver I2CDB0;
#endif

#if (MSP430X_I2C_USE_I2CB1 == TRUE) && !defined(__DOXYGEN__)
extern I2CDriver I2CDB1;
#endif

#if (MSP430X_I2C_USE_I2CB2 == TRUE) && !defined(__DOXYGEN__)
extern I2CDriver I2CDB2;
#endif

#if (MSP430X_I2C_USE_I2CB3 == TRUE) && !defined(__DOXYGEN__)
extern I2CDriver I2CDB3;
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
  
  void i2cMSP430XEndTransferI(I2CDriver *i2cp);
  void i2cMSP430XStartReceiveI(I2CDriver *i2cp, i2caddr_t addr, size_t n, 
      uint8_t * rxbuf, i2ccallback_t callback);
  void i2cMSP430XStartTransmitI(I2CDriver *i2cp, i2caddr_t addr, size_t n, 
      uint8_t * txbuf, i2ccallback_t callback);
  void i2cMSP430XContinueTransmitMSBI(I2CDriver *i2cp, i2caddr_t addr, size_t n, 
    uint8_t * txbuf, i2ccallback_t callback);
  void i2cMSP430XContinueTransmitI(I2CDriver *i2cp, i2caddr_t addr, size_t n, 
    uint8_t * txbuf, i2ccallback_t callback);
  void i2cMSP430XStartTransmitMSBI(I2CDriver *i2cp, i2caddr_t addr, size_t n, 
    uint8_t * txbuf, i2ccallback_t callback);
  void i2cMSP430XStartReceiveToRegI(I2CDriver *i2cp, i2caddr_t addr, size_t n, 
    uint8_t * regp, i2ccallback_t callback);
  void i2cMSP430XContinueTransmitMemsetI(I2CDriver *i2cp, i2caddr_t addr, size_t n, 
    uint8_t * value, i2ccallback_t callback);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_I2C == TRUE */

#endif /* HAL_I2C_LLD_H */

/** @} */
