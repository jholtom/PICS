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
 * @file    hal_uart_lld.c
 * @brief   PLATFORM UART subsystem low level driver source.
 *
 * @addtogroup UART
 * @{
 */

#include "hal.h"

#if (HAL_USE_UART == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   UART1 driver identifier.
 */
#if (POSIX_UART_USE_UART1 == TRUE) || defined(__DOXYGEN__)
UARTDriver UARTD1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */


/*===========================================================================*/
/* Driver local functions.                                                   */

static void uart_enter_rx_idle_loop(UARTDriver* uartp) {
#if defined(__FUZZ__)
  (void) uartp;
  // Nothing to do
#else
#error "Only fuzz mode is supported"
#endif
}

/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level UART driver initialization.
 *
 * @notapi
 */
void uart_lld_init(void) {

#if POSIX_UART_USE_UART1 == TRUE
  /* Driver initialization.*/
  uartObjectInit(&UARTD1);
  UARTD1.buffer.current = 0;
#endif
}

/**
 * @brief   Configures and activates the UART peripheral.
 *
 * @param[in] uartp      pointer to the @p UARTDriver object
 *
 * @notapi
 */
void uart_lld_start(UARTDriver *uartp) {

  if (uartp->state == UART_STOP) {
    /* Enables the peripheral.*/
#if POSIX_UART_USE_UART1 == TRUE
    if (&UARTD1 == uartp) {
      // nothing to do
    }
#endif
  }
  /* Configures the peripheral.*/

}

/**
 * @brief   Deactivates the UART peripheral.
 *
 * @param[in] uartp      pointer to the @p UARTDriver object
 *
 * @notapi
 */
void uart_lld_stop(UARTDriver *uartp) {

  if (uartp->state == UART_READY) {
    /* Resets the peripheral.*/

    /* Disables the peripheral.*/
#if POSIX_UART_USE_UART1 == TRUE
    if (&UARTD1 == uartp) {
      // Nothing to do
    }
#endif
  }
}

/**
 * @brief   Starts a transmission on the UART peripheral.
 * @note    The buffers are organized as uint8_t arrays for data sizes below
 *          or equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 * @param[in] n         number of data frames to send
 * @param[in] txbuf     the pointer to the transmit buffer
 *
 * @notapi
 */
void uart_lld_start_send(UARTDriver *uartp, size_t n, const void *txbuf) {
#if defined(__FUZZ__)
  // We don't actually simulate this, but we do run the callbacks
  (void) txbuf;
  uartp->tx_remaining = n;
  while(uartp->tx_remaining) {
    // We use tx_remaining here since the tx2 callback might call stop_send
    CH_IRQ_PROLOGUE();
    _uart_tx2_isr_code(uartp);
    CH_IRQ_EPILOGUE();
    /* TODO(Reed): Simulate timeouts and errors */
    uartp->tx_remaining--;
  }
  CH_IRQ_PROLOGUE();
  _uart_tx1_isr_code(uartp);
  CH_IRQ_EPILOGUE();
#else
#error "We only support fuzzing"
#endif
}

/**
 * @brief   Stops any ongoing transmission.
 * @note    Stopping a transmission also suppresses the transmission callbacks.
 *
 * @param[in] uartp      pointer to the @p UARTDriver object
 *
 * @return              The number of data frames not transmitted by the
 *                      stopped transmit operation.
 *
 * @notapi
 */
size_t uart_lld_stop_send(UARTDriver *uartp) {
  size_t tr = uartp->tx_remaining;
  uartp->tx_remaining = 0;
  return tr;
}

/**
 * @brief   Starts a receive operation on the UART peripheral.
 * @note    The buffers are organized as uint8_t arrays for data sizes below
 *          or equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 * @param[in] n         number of data frames to send
 * @param[out] rxbuf    the pointer to the receive buffer
 *
 * @notapi
 */
void uart_lld_start_receive(UARTDriver *uartp, size_t n, void *rxbuf) {
  uint8_t * obuf = (uint8_t *)rxbuf;
#if defined(__FUZZ__)
  fuzz_check_for_out_of_data(&uartp->buffer);
  uartp->rx_remaining = n;
  for (size_t i = 0; i < n && uartp->rx_remaining; ++i) {
    obuf[i] = fuzz_next_byte(&uartp->buffer, i == n - 1);
    if (uartp->config->rxchar_cb) {
      CH_IRQ_PROLOGUE()
      uartp->config->rxchar_cb(uartp, obuf[i]);
      CH_IRQ_EPILOGUE()
    }
    uartp->rx_remaining--;
    /* TODO(Reed): Fuzz timeouts and rx errors */
    /* see _uart_timeout_isr_code and _uart_rx */
  }
  // Complete the request
  CH_IRQ_PROLOGUE();
  _uart_rx_complete_isr_code(uartp);
  CH_IRQ_EPILOGUE();
#else
#error "Only fuzz mode is supported"
#endif
}

/**
 * @brief   Stops any ongoing receive operation.
 * @note    Stopping a receive operation also suppresses the receive callbacks.
 *
 * @param[in] uartp      pointer to the @p UARTDriver object
 *
 * @return              The number of data frames not received by the
 *                      stopped receive operation.
 *
 * @notapi
 */
size_t uart_lld_stop_receive(UARTDriver *uartp) {
  size_t tr = uartp->rx_remaining;
  uartp->rx_remaining = 0;

  return tr;
}

#endif /* HAL_USE_UART == TRUE */

/** @} */
