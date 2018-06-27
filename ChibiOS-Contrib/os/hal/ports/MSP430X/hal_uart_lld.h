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
 * @file    hal_uart_lld.h
 * @brief   MSP430X UART subsystem low level driver header.
 *
 * @addtogroup UART
 * @{
 */

#ifndef HAL_UART_LLD_H
#define HAL_UART_LLD_H

#if (HAL_USE_UART == TRUE) || defined(__DOXYGEN__)

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
 * @brief   UART driver enable switch.
 * @details If set to @p TRUE the support for UARTA0 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_UART_USE_UARTA0) || defined(__DOXYGEN__)
#define MSP430X_UART_USE_UARTA0             FALSE
#endif

/**
 * @brief   UART driver enable switch.
 * @details If set to @p TRUE the support for UARTA1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_UART_USE_UARTA1) || defined(__DOXYGEN__)
#define MSP430X_UART_USE_UARTA1             FALSE
#endif

/**
 * @brief   UART driver enable switch.
 * @details If set to @p TRUE the support for UARTA2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_UART_USE_UARTA2) || defined(__DOXYGEN__)
#define MSP430X_UART_USE_UARTA2             FALSE
#endif

/**
 * @brief   UART driver enable switch.
 * @details If set to @p TRUE the support for UARTA3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_UART_USE_UARTA3) || defined(__DOXYGEN__)
#define MSP430X_UART_USE_UARTA3             FALSE
#endif

/**
 * @brief   Exclusive DMA enable switch.
 * @details If set to @p TRUE the support for exclusive DMA is included.
 * @note    This increases the size of the compiled executable somewhat.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_UART_EXCLUSIVE_DMA) || defined(__DOXYGEN__)
#define MSP430X_UART_EXCLUSIVE_DMA              FALSE
#endif

/**
 * @brief   UARTA0 clock source switch.
 * @details Sets the clock source for UARTA0.
 * @note    Legal values are @p MSP430X_SMCLK_SRC, @p MSP430X_ACLK_SRC, or undefined.
 * @note    If undefined, the clock source on UCA0CLK is used. 
 * @note    For UCLK, MSP430X_UARTA0_CLK_FREQ must also be set.
 */
#if !defined(MSP430X_UARTA0_CLK_SRC)
  #define MSP430X_UARTA0_CLK_SRC 0xFF
#endif

/**
 * @brief   UARTA1 clock source switch.
 * @details Sets the clock source for UARTA1.
 * @note    Legal values are @p MSP430X_SMCLK_SRC, @p MSP430X_ACLK_SRC, or undefined.
 * @note    If undefined, the clock source on UCA0CLK is used. 
 * @note    For UCLK, MSP430X_UARTA1_CLK_FREQ must also be set.
 */
#if !defined(MSP430X_UARTA1_CLK_SRC)
  #define MSP430X_UARTA1_CLK_SRC  0xFF
#endif

/**
 * @brief   UARTA2 clock source switch.
 * @details Sets the clock source for UARTA2.
 * @note    Legal values are @p MSP430X_SMCLK_SRC, @p MSP430X_ACLK_SRC, or undefined.
 * @note    If undefined, the clock source on UCA0CLK is used. 
 * @note    For UCLK, MSP430X_UARTA2_CLK_FREQ must also be set.
 */
#if !defined(MSP430X_UARTA2_CLK_SRC)
  #define MSP430X_UARTA2_CLK_SRC 0xFF
#endif

/**
 * @brief   UARTA3 clock source switch.
 * @details Sets the clock source for UARTA3.
 * @note    Legal values are @p MSP430X_SMCLK_SRC, @p MSP430X_ACLK_SRC, or undefined.
 * @note    If undefined, the clock source on UCA0CLK is used. 
 * @note    For UCLK, MSP430X_UARTA3_CLK_FREQ must also be set.
 */
#if !defined(MSP430X_UARTA3_CLK_SRC)
  #define MSP430X_UARTA3_CLK_SRC 0xFF
#endif

/**
 * @brief   Exclusive DMA enable switch
 * @details If set to @p TRUE the support for exclusive DMA is included.
 * @note    This increases the size of the compiled executable somewhat.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_UART_EXCLUSIVE_DMA) || defined(__DOXYGEN__)
  #define MSP430X_UART_EXCLUSIVE_DMA FALSE
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if MSP430X_UART_USE_UARTA0 && !defined(__MSP430_HAS_EUSCI_A0__)
  #error "Cannot find MSP430X_USCI module to use for UARTA0"
#endif

#if MSP430X_UART_USE_UARTA1 && !defined(__MSP430_HAS_EUSCI_A1__)
  #error "Cannot find MSP430X_USCI module to use for UARTA1"
#endif

#if MSP430X_UART_USE_UARTA2 && !defined(__MSP430_HAS_EUSCI_A2__)
  #error "Cannot find MSP430X_USCI module to use for UARTA2"
#endif

#if MSP430X_UART_USE_UARTA3 && !defined(__MSP430_HAS_EUSCI_A3__)
  #error "Cannot find MSP430X_USCI module to use for UARTA3"
#endif

#if MSP430X_UART_USE_UARTA0
  #ifdef MSP430X_USCI_A0_USED
    #error "USCI module A0 already in use - UARTA0 not available"
  #else
    #define MSP430X_USCI_A0_USED
  #endif
  #if MSP430X_UARTA0_CLK_SRC == MSP430X_ACLK_SRC
    #define MSP430X_UARTA0_CLK_FREQ MSP430X_ACLK_FREQ
    #define MSP430X_UARTA0_UCSSEL UCSSEL__ACLK
  #elif MSP430X_UARTA0_CLK_SRC == MSP430X_SMCLK_SRC
    #define MSP430X_UARTA0_CLK_FREQ MSP430X_SMCLK_FREQ
    #define MSP430X_UARTA0_UCSSEL UCSSEL__SMCLK
  #else
    #if !defined(MSP430X_UARTA0_CLK_FREQ)
      #error "Requested external UARTA0 clock but MSP430X_UARTA0_CLK_FREQ not defined"
    #endif
    #define MSP430X_UARTA0_UCSSEL UCSSEL__UCLK
  #endif
#endif

#if MSP430X_UART_USE_UARTA1
  #ifdef MSP430X_USCI_A1_USED
    #error "USCI module A1 already in use - UARTA1 not available"
  #else
    #define MSP430X_USCI_A1_USED
  #endif
  #if MSP430X_UARTA1_CLK_SRC == MSP430X_ACLK_SRC
    #define MSP430X_UARTA1_CLK_FREQ MSP430X_ACLK_FREQ
    #define MSP430X_UARTA1_UCSSEL UCSSEL__ACLK
  #elif MSP430X_UARTA1_CLK_SRC == MSP430X_SMCLK_SRC
    #define MSP430X_UARTA1_CLK_FREQ MSP430X_SMCLK_FREQ
    #define MSP430X_UARTA1_UCSSEL UCSSEL__SMCLK
  #else
    #if !defined(MSP430X_UARTA1_CLK_FREQ)
      #error "Requested external UARTA1 clock but MSP430X_UARTA1_CLK_FREQ not defined"
    #endif
    #define MSP430X_UARTA1_UCSSEL UCSSEL__UCLK
  #endif
#endif

#if MSP430X_UART_USE_UARTA2
  #ifdef MSP430X_USCI_A2_USED
    #error "USCI module A2 already in use - UARTA2 not available"
  #else
    #define MSP430X_USCI_A2_USED
  #endif
  #if MSP430X_UARTA2_CLK_SRC == MSP430X_ACLK_SRC
    #define MSP430X_UARTA2_CLK_FREQ MSP430X_ACLK_FREQ
    #define MSP430X_UARTA2_UCSSEL UCSSEL__ACLK
  #elif MSP430X_UARTA2_CLK_SRC == MSP430X_SMCLK_SRC
    #define MSP430X_UARTA2_CLK_FREQ MSP430X_SMCLK_FREQ
    #define MSP430X_UARTA2_UCSSEL UCSSEL__SMCLK
  #else
    #if !defined(MSP430X_UARTA2_CLK_FREQ)
      #error "Requested external UARTA2 clock but MSP430X_UARTA2_CLK_FREQ not defined"
    #endif
    #define MSP430X_UARTA2_UCSSEL UCSSEL__UCLK
  #endif
#endif

#if MSP430X_UART_USE_UARTA3
  #ifdef MSP430X_USCI_A3_USED
    #error "USCI module A3 already in use - UARTA3 not available"
  #else
    #define MSP430X_USCI_A3_USED
  #endif
  #if MSP430X_UARTA3_CLK_SRC == MSP430X_ACLK_SRC
    #define MSP430X_UARTA3_CLK_FREQ MSP430X_ACLK_FREQ
    #define MSP430X_UARTA3_UCSSEL UCSSEL__ACLK
  #elif MSP430X_UARTA3_CLK_SRC == MSP430X_SMCLK_SRC
    #define MSP430X_UARTA3_CLK_FREQ MSP430X_SMCLK_FREQ
    #define MSP430X_UARTA3_UCSSEL UCSSEL__SMCLK
  #else
    #if !defined(MSP430X_UARTA3_CLK_FREQ)
      #error "Requested external UARTA3 clock but MSP430X_UARTA3_CLK_FREQ not defined"
    #endif
    #define MSP430X_UARTA3_UCSSEL UCSSEL__UCLK
  #endif
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Enumerated type for UART parity.
 */
typedef enum {
  MSP430X_UART_PARITY_NONE = 0x00,
  MSP430X_UART_PARITY_ODD  = 0x02,
  MSP430X_UART_PARITY_EVEN = 0x03
} msp430x_uart_parity_t;

/**
 * @brief   Enumerated type for UART bit order.
 */
typedef enum {
  MSP430X_UART_BO_LSB = 0,
  MSP430X_UART_BO_MSB = 1
} msp430x_uart_bitorder_t;

/**
 * @brief   Enumerated type for UART char size.
 */
typedef enum {
  MSP430X_UART_DS_EIGHT = 0,
  MSP430X_UART_DS_SEVEN = 1
} msp430x_uart_char_size_t;

/**
 * @brief   Enumerated type for UART stop bits.
 */
typedef enum {
  MSP430X_UART_ONE_STOP = 0,
  MSP430X_UART_TWO_STOP = 1
} msp430x_uart_data_size_t;

/**
 * @brief   UART driver condition flags type.
 */
typedef uint32_t uartflags_t;

/**
 * @brief   Type of structure representing an UART driver.
 */
typedef struct UARTDriver UARTDriver;

/**
 * @brief   Generic UART notification callback type.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 */
typedef void (*uartcb_t)(UARTDriver *uartp);

/**
 * @brief   Character received UART notification callback type.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object triggering the
 *                      callback
 * @param[in] c         received character
 */
typedef void (*uartccb_t)(UARTDriver *uartp, uint16_t c);

/**
 * @brief   Receive error UART notification callback type.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object triggering the
 *                      callback
 * @param[in] e         receive error mask
 */
typedef void (*uartecb_t)(UARTDriver *uartp, uartflags_t e);

/**
 * @brief   Driver configuration structure.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
typedef struct {
  /**
   * @brief End of transmission buffer callback.
   */
  uartcb_t                  txend1_cb;
  /**
   * @brief Physical end of transmission callback.
   */
  uartcb_t                  txend2_cb;
  /**
   * @brief Receive buffer filled callback.
   */
  uartcb_t                  rxend_cb;
  /**
   * @brief Character received while out if the @p UART_RECEIVE state.
   */
  uartccb_t                 rxchar_cb;
  /**
   * @brief Receive error callback.
   * @note  NOTE that in this architecture this callback is called only from RX_IDLE
   */
  uartecb_t                 rxerr_cb;
  /* End of the mandatory fields.*/
  /**
   * @brief Baud rate
   */
  uint32_t baud;
  /**
   * @brief Parity
   */
  uint8_t parity:2;
  /**
   * @brief Bit order
   */
  uint8_t order:1;
  /**
   * @brief Char length
   */
  uint8_t char_size:1;
  /**
   * @brief Number of stop bits
   */
  uint8_t stop_bits:1;
  /**
   * @brief Enable autobaud
   */
  uint8_t autobaud:1;
#if MSP430X_UART_EXCLUSIVE_DMA == TRUE || defined(__DOXYGEN__)
  /**
   * @brief The index of the TX DMA channel.
   * @note This may be >MSP430X_DMA_CHANNELS to indicate that exclusive DMA is
   * not used.
   */
  uint8_t                   dmatx_index : 4;
  /**
   * @brief The index of the RX DMA channel.
   * @note This may be >MSP430X_DMA_CHANNELS to indicate that exclusive DMA is
   * not used.
   */
  uint8_t                   dmarx_index : 4;
#endif
} UARTConfig;

/**
 * @brief   MSP430X UART register structure.
 */
typedef volatile struct {
  uint16_t ctlw0;
  uint16_t ctlw1;
  uint16_t _padding0;
  uint16_t brw;
  uint16_t mctlw;
  uint16_t statw;
  uint16_t rxbuf;
  uint16_t txbuf;
  uint16_t abctl;
  uint16_t irctl;
  uint16_t _padding1;
  uint16_t _padding2;
  uint16_t _padding3;
  uint16_t ie;
  uint16_t ifg;
  uint16_t iv;
} msp430x_uart_reg_t;

/**
 * @brief   Structure representing an UART driver.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
struct UARTDriver {
  /**
   * @brief Driver state.
   */
  uartstate_t               state;
  /**
   * @brief Transmitter state.
   */
  uarttxstate_t             txstate;
  /**
   * @brief Receiver state.
   */
  uartrxstate_t             rxstate;
  /**
   * @brief Current configuration data.
   */
  const UARTConfig          *config;
#if (UART_USE_WAIT == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   Synchronization flag for transmit operations.
   */
  bool                      early;
  /**
   * @brief   Waiting thread on RX.
   */
  thread_reference_t        threadrx;
  /**
   * @brief   Waiting thread on TX.
   */
  thread_reference_t        threadtx;
#endif /* UART_USE_WAIT */
#if (UART_USE_MUTUAL_EXCLUSION == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   Mutex protecting the peripheral.
   */
  mutex_t                   mutex;
#endif /* UART_USE_MUTUAL_EXCLUSION */
#if defined(UART_DRIVER_EXT_FIELDS)
  UART_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief   Source clock frequency
   */
  uint32_t freq;
  /**
   * @brief   Registers
   */
  msp430x_uart_reg_t * regs;
  /** 
   * @brief   TX DMA request
   */
  msp430x_dma_req_t dmareq_tx;
  /** 
   * @brief   RX DMA request
   */
  msp430x_dma_req_t dmareq_rx;
  /**
   * @brief   TX DMA stream.
   */
  msp430x_dma_ch_t dma_tx;
  /**
   * @brief   RX DMA stream.
   */
  msp430x_dma_ch_t dma_rx;
  /**
   * @brief   Exclusive DMA mode (TX)
   */
  bool dma_acquired_tx;
  /**
   * @brief   Exclusive DMA mode (RX)
   */
  bool dma_acquired_rx;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (MSP430X_UART_USE_UARTA0 == TRUE) && !defined(__DOXYGEN__)
extern UARTDriver UARTDA0;
#endif

#if (MSP430X_UART_USE_UARTA1 == TRUE) && !defined(__DOXYGEN__)
extern UARTDriver UARTDA1;
#endif

#if (MSP430X_UART_USE_UARTA2 == TRUE) && !defined(__DOXYGEN__)
extern UARTDriver UARTDA2;
#endif

#if (MSP430X_UART_USE_UARTA3 == TRUE) && !defined(__DOXYGEN__)
extern UARTDriver UARTDA3;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void uart_lld_init(void);
  void uart_lld_start(UARTDriver *uartp);
  void uart_lld_stop(UARTDriver *uartp);
  void uart_lld_start_send(UARTDriver *uartp, size_t n, const void *txbuf);
  size_t uart_lld_stop_send(UARTDriver *uartp);
  void uart_lld_start_receive(UARTDriver *uartp, size_t n, void *rxbuf);
  size_t uart_lld_stop_receive(UARTDriver *uartp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_UART == TRUE */

#endif /* HAL_UART_LLD_H */

/** @} */
