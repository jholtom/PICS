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
 * @file    hal_uart_lld.c
 * @brief   MSP430X UART subsystem low level driver source.
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
 * @brief   UARTA0 driver identifier.
 */
#if (MSP430X_UART_USE_UARTA0 == TRUE) || defined(__DOXYGEN__)
UARTDriver UARTDA0;
#endif

/**
 * @brief   UARTA1 driver identifier.
 */
#if (MSP430X_UART_USE_UARTA1 == TRUE) || defined(__DOXYGEN__)
UARTDriver UARTDA1;
#endif

/**
 * @brief   UARTA2 driver identifier.
 */
#if (MSP430X_UART_USE_UARTA2 == TRUE) || defined(__DOXYGEN__)
UARTDriver UARTDA2;
#endif

/**
 * @brief   UARTA3 driver identifier.
 */
#if (MSP430X_UART_USE_UARTA3 == TRUE) || defined(__DOXYGEN__)
UARTDriver UARTDA3;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief     UCBRS calculation.
 * @details   This function calculates the UCBRS value for oversampled baud
 *            rates.
 *
 * @param[in] frac    Fractional part of baud rate division, times 10000.
 */
static uint8_t UCBRS(uint16_t frac) {
  /* TODO there must be a better way */
  if (frac < 529)
    return 0x00;
  else if (frac < 715)
    return 0x01;
  else if (frac < 835)
    return 0x02;
  else if (frac < 1001)
    return 0x04;
  else if (frac < 1252)
    return 0x08;
  else if (frac < 1430)
    return 0x10;
  else if (frac < 1670)
    return 0x20;
  else if (frac < 2147)
    return 0x11;
  else if (frac < 2224)
    return 0x21;
  else if (frac < 2503)
    return 0x22;
  else if (frac < 3000)
    return 0x44;
  else if (frac < 3335)
    return 0x25;
  else if (frac < 3575)
    return 0x49;
  else if (frac < 3753)
    return 0x4A;
  else if (frac < 4003)
    return 0x52;
  else if (frac < 4286)
    return 0x92;
  else if (frac < 4378)
    return 0x53;
  else if (frac < 5002)
    return 0x55;
  else if (frac < 5715)
    return 0xAA;
  else if (frac < 6003)
    return 0x6B;
  else if (frac < 6254)
    return 0xAD;
  else if (frac < 6432)
    return 0xB5;
  else if (frac < 6667)
    return 0xB6;
  else if (frac < 7001)
    return 0xD6;
  else if (frac < 7147)
    return 0xB7;
  else if (frac < 7503)
    return 0xBB;
  else if (frac < 7861)
    return 0xDD;
  else if (frac < 8004)
    return 0xED;
  else if (frac < 8333)
    return 0xEE;
  else if (frac < 8464)
    return 0xBF;
  else if (frac < 8572)
    return 0xDF;
  else if (frac < 8751)
    return 0xEF;
  else if (frac < 9004)
    return 0xF7;
  else if (frac < 9170)
    return 0xFB;
  else if (frac < 9288)
    return 0xFD;
  else
    return 0xFE;
}

static void set_baud(UARTDriver * uartp) {
  uint16_t n = uartp->freq / uartp->config->baud;
  uint16_t frac = (uartp->freq - (n * uartp->config->baud)) * 10000 / uartp->config->baud;
  if (n > 16) {
    uartp->regs->brw = (n >> 4);
    n = (n & 0x0F);
    uartp->regs->mctlw = ((UCBRS(frac) << 8) | (n << 4) | UCOS16);
  }
  else {
    uartp->regs->brw = n;
    uartp->regs->mctlw = (UCBRS(frac) << 8);
  }
}

static void init_transfer(msp430x_dma_ch_t * dma, msp430x_dma_req_t * req, bool *acquired) {
  if (!(*acquired)) {
    /* TODO fall back to interrupt-driven approach */
    dmaAcquireI(dma);
    (*acquired) = true;
  }
  dmaTransferI(dma, req);
}

static void tx_cb(void * arg) {
  UARTDriver * uartp = (UARTDriver *)(arg);
  
  /* Call the callback function */
  _uart_tx1_isr_code(uartp);
  
  if (uartp->txstate == UART_TX_IDLE) {
    while (!(uartp->regs->ifg & UCTXCPTIFG));
    /* Clear the TXCPTIFG to prevent spurious interrupts */
    uartp->regs->ifg &= ~UCTXCPTIFG;
    
    /* Enable the interrupt for the next callback function */
    uartp->regs->ie |= UCTXCPTIE;
#if MSP430X_UART_EXCLUSIVE_DMA == TRUE
    if (uartp->config->dmatx_index >= MSP430X_DMA_CHANNELS) {
      dmaReleaseX(&(uartp->dma_tx));
      uartp->dma_acquired_tx = false;
    }
#else
    dmaReleaseX(&(uartp->dmta_tx));
    uartp->dma_acquired_tx = false;
#endif
  }
}

static void uart_enter_rx_idle_loop(UARTDriver* uartp) {
  /* Re-enable RX interrupt */
  uartp->regs->ie |= UCRXIE;
}

static void rx_cb(void * arg){
  UARTDriver * uartp = (UARTDriver *)(arg);
  
  /* Call the callback function */
  _uart_rx_complete_isr_code(uartp);
  if (uartp->rxstate == UART_RX_IDLE) {
#if MSP430X_UART_EXCLUSIVE_DMA == TRUE
    if (uartp->config->dmarx_index >= MSP430X_DMA_CHANNELS) {
      dmaReleaseX(&(uartp->dma_rx));
      uartp->dma_acquired_rx = false;
    }
#else
    dmaReleaseX(&(uartp->dmta_rx));
    uartp->dma_acquired_rx = false;
#endif
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if MSP430X_UART_USE_UARTA0 == TRUE
PORT_IRQ_HANDLER(USCI_A0_VECTOR) {
  OSAL_IRQ_PROLOGUE();

  switch(__even_in_range(UCA0IV, UCTXCPTIFG)) {
    case 0x00:
      /* nothing has happened */
      break;
    case USCI_UART_UCRXIFG:
      if (UCA0STATW & UCRXERR) {
        /* Error has occurred */
        uartflags_t sts = 0;
        
        if (UCA0STATW & UCBRK)
          sts |= UART_BREAK_DETECTED;
        if (UCA0STATW & UCOE)
          sts |= UART_OVERRUN_ERROR;
        if (UCA0STATW & UCFE)
          sts |= UART_FRAMING_ERROR;
        if (UCA0STATW & UCPE)
          sts |= UART_PARITY_ERROR;
        /* Call the error handling callback */
        _uart_rx_error_isr_code(&UARTDA0, sts);
        /* Don't stop the reception - let it keep going */
      }
      if (UARTDA0.config->rxchar_cb != NULL) {
        /* Call the RX char callback */
        UARTDA0.config->rxchar_cb(&UARTDA0, UCA0RXBUF);
      }
      else {
        /* Clear the flag so we can keep going */
        UCA0IFG &= ~UCRXIFG;
      }
      break;
    case USCI_UART_UCTXIFG:
    case USCI_UART_UCSTTIFG:
      osalDbgAssert(false, "Spurious interrupt in USCI A0 UART");
      break;
    case USCI_UART_UCTXCPTIFG:
      /* Call the second TX callback */
      _uart_tx2_isr_code(&UARTDA0);
      /* Disable the interrupt until next time */
      UCA0IE &= ~UCTXCPTIE;
      /* Reset the flag? */
      UCA0IFG &= ~UCTXCPTIFG;
      break;
  }
  
  OSAL_IRQ_EPILOGUE();
}
#endif

#if MSP430X_UART_USE_UARTA1 == TRUE
PORT_IRQ_HANDLER(USCI_A1_VECTOR) {
  OSAL_IRQ_PROLOGUE();

  switch(__even_in_range(UCA1IV, UCTXCPTIFG)) {
    case 0x00:
      /* nothing has happened */
      break;
    case USCI_UART_UCRXIFG:
      if (UCA1STATW & UCRXERR) {
        /* Error has occurred */
        uartflags_t sts = 0;
        
        if (UCA1STATW & UCBRK)
          sts |= UART_BREAK_DETECTED;
        if (UCA1STATW & UCOE)
          sts |= UART_OVERRUN_ERROR;
        if (UCA1STATW & UCFE)
          sts |= UART_FRAMING_ERROR;
        if (UCA1STATW & UCPE)
          sts |= UART_PARITY_ERROR;
        /* Call the error handling callback */
        _uart_rx_error_isr_code(&UARTDA1, sts);
        /* Don't stop the reception - let it keep going */
      }
      if (UARTDA1.config->rxchar_cb != NULL) {
        /* Call the RX char callback */
        UARTDA1.config->rxchar_cb(&UARTDA1, UCA1RXBUF);
      }
      else {
        /* Clear the flag so we can keep going */
        UCA1IFG &= ~UCRXIFG;
      }
      break;
    case USCI_UART_UCTXIFG:
    case USCI_UART_UCSTTIFG:
      osalDbgAssert(false, "Spurious interrupt in USCI A1 UART");
      break;
    case USCI_UART_UCTXCPTIFG:
      /* Call the second TX callback */
      _uart_tx2_isr_code(&UARTDA1);
      /* Disable the interrupt until next time */
      UCA1IE &= ~UCTXCPTIE;
      /* Reset the flag? */
      UCA1IFG &= ~UCTXCPTIFG;
      break;
  }
  
  OSAL_IRQ_EPILOGUE();
}
#endif

#if MSP430X_UART_USE_UARTA2 == TRUE
PORT_IRQ_HANDLER(USCI_A2_VECTOR) {
  OSAL_IRQ_PROLOGUE();

  switch(__even_in_range(UCA2IV, UCTXCPTIFG)) {
    case 0x00:
      /* nothing has happened */
      break;
    case USCI_UART_UCRXIFG:
      if (UCA2STATW & UCRXERR) {
        /* Error has occurred */
        uartflags_t sts = 0;
        
        if (UCA2STATW & UCBRK)
          sts |= UART_BREAK_DETECTED;
        if (UCA2STATW & UCOE)
          sts |= UART_OVERRUN_ERROR;
        if (UCA2STATW & UCFE)
          sts |= UART_FRAMING_ERROR;
        if (UCA2STATW & UCPE)
          sts |= UART_PARITY_ERROR;
        /* Call the error handling callback */
        _uart_rx_error_isr_code(&UARTDA2, sts);
        /* Don't stop the reception - let it keep going */
      }
      if (UARTDA2.config->rxchar_cb != NULL) {
        /* Call the RX char callback */
        UARTDA2.config->rxchar_cb(&UARTDA2, UCA2RXBUF);
      }
      else {
        /* Clear the flag so we can keep going */
        UCA2IFG &= ~UCRXIFG;
      }
      break;
    case USCI_UART_UCTXIFG:
    case USCI_UART_UCSTTIFG:
      osalDbgAssert(false, "Spurious interrupt in USCI A0 UART");
      break;
    case USCI_UART_UCTXCPTIFG:
      if (UARTDA2.txcpt_first) {
        UARTDA2.txcpt_first = false;
      }
      else {
        /* Call the second TX callback */
        _uart_tx2_isr_code(&UARTDA2);
        /* Disable the interrupt until next time */
        UCA2IE &= ~UCTXCPTIE;
        /* Reset the flag? */
        UCA2IFG &= ~UCTXCPTIFG;
        UARTDA2.txcpt_first = true;
      }
      break;
  }
  
  OSAL_IRQ_EPILOGUE();
}
#endif

#if MSP430X_UART_USE_UARTA3 == TRUE
PORT_IRQ_HANDLER(USCI_A3_VECTOR) {
  OSAL_IRQ_PROLOGUE();

  switch(__even_in_range(UCA3IV, UCTXCPTIFG)) {
    case 0x00:
      /* nothing has happened */
      break;
    case USCI_UART_UCRXIFG:
      if (UCA3STATW & UCRXERR) {
        /* Error has occurred */
        uartflags_t sts = 0;
        
        if (UCA3STATW & UCBRK)
          sts |= UART_BREAK_DETECTED;
        if (UCA3STATW & UCOE)
          sts |= UART_OVERRUN_ERROR;
        if (UCA3STATW & UCFE)
          sts |= UART_FRAMING_ERROR;
        if (UCA3STATW & UCPE)
          sts |= UART_PARITY_ERROR;
        /* Call the error handling callback */
        _uart_rx_error_isr_code(&UARTDA3, sts);
        /* Don't stop the reception - let it keep going */
      }
      if (UARTDA3.config->rxchar_cb != NULL) {
        /* Call the RX char callback */
        UARTDA3.config->rxchar_cb(&UARTDA3, UCA3RXBUF);
      }
      else {
        /* Clear the flag so we can keep going */
        UCA3IFG &= ~UCRXIFG;
      }
      break;
    case USCI_UART_UCTXIFG:
    case USCI_UART_UCSTTIFG:
      osalDbgAssert(false, "Spurious interrupt in USCI A0 UART");
      break;
    case USCI_UART_UCTXCPTIFG:
      if (UARTDA3.txcpt_first) {
        UARTDA3.txcpt_first = false;
      }
      else {
        /* Call the second TX callback */
        _uart_tx2_isr_code(&UARTDA3);
        /* Disable the interrupt until next time */
        UCA3IE &= ~UCTXCPTIE;
        /* Reset the flag? */
        UCA3IFG &= ~UCTXCPTIFG;
        UARTDA3.txcpt_first = true;
      }
      break;
  }
  
  OSAL_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level UART driver initialization.
 *
 * @notapi
 */
void uart_lld_init(void) {

#if MSP430X_UART_USE_UARTA0 == TRUE
  /* Driver initialization.*/
  uartObjectInit(&UARTDA0);
  UARTDA0.regs = (msp430x_uart_reg_t  *)(&UCA0CTLW0);
  UARTDA0.freq = MSP430X_UARTA0_CLK_FREQ;
  UARTDA0.dmareq_tx.dest_addr = (void*)(&UCA0TXBUF);
  UARTDA0.dmareq_tx.addr_mode = MSP430X_DMA_SRCINCR;
  UARTDA0.dmareq_tx.data_mode = (MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE);
  UARTDA0.dmareq_tx.transfer_mode = MSP430X_DMA_SINGLE;
  UARTDA0.dmareq_tx.trigger = DMA_TRIGGER_MNEM(UCA0TXIFG);
  UARTDA0.dmareq_rx.source_addr = (void*)(&UCA0RXBUF);
  UARTDA0.dmareq_rx.addr_mode = MSP430X_DMA_DSTINCR;
  UARTDA0.dmareq_rx.data_mode = (MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE);
  UARTDA0.dmareq_rx.transfer_mode = MSP430X_DMA_SINGLE;
  UARTDA0.dmareq_rx.trigger = DMA_TRIGGER_MNEM(UCA0RXIFG);
#endif
  
#if MSP430X_UART_USE_UARTA1 == TRUE
  /* Driver initialization.*/
  uartObjectInit(&UARTDA1);
  UARTDA1.regs = (msp430x_uart_reg_t  *)(&UCA1CTLW0);
  UARTDA1.freq = MSP430X_UARTA1_CLK_FREQ;
  UARTDA1.dmareq_tx.dest_addr = (void*)(&UCA1TXBUF);
  UARTDA1.dmareq_tx.addr_mode = MSP430X_DMA_SRCINCR;
  UARTDA1.dmareq_tx.data_mode = (MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE);
  UARTDA1.dmareq_tx.transfer_mode = MSP430X_DMA_SINGLE;
  UARTDA1.dmareq_tx.trigger = DMA_TRIGGER_MNEM(UCA1TXIFG);
  UARTDA1.dmareq_rx.source_addr = (void*)(&UCA1RXBUF);
  UARTDA1.dmareq_rx.addr_mode = MSP430X_DMA_DSTINCR;
  UARTDA1.dmareq_rx.data_mode = (MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE);
  UARTDA1.dmareq_rx.transfer_mode = MSP430X_DMA_SINGLE;
  UARTDA1.dmareq_rx.trigger = DMA_TRIGGER_MNEM(UCA1RXIFG);
#endif
  
#if MSP430X_UART_USE_UARTA2 == TRUE
  /* Driver initialization.*/
  uartObjectInit(&UARTDA2);
  UARTDA2.regs = (msp430x_uart_reg_t  *)(&UCA2CTLW0);
  UARTDA2.freq = MSP430X_UARTA2_CLK_FREQ;
  UARTDA2.dmareq_tx.dest_addr = (void*)(&UCA2TXBUF);
  UARTDA2.dmareq_tx.addr_mode = MSP430X_DMA_SRCINCR;
  UARTDA2.dmareq_tx.data_mode = (MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE);
  UARTDA2.dmareq_tx.transfer_mode = MSP430X_DMA_SINGLE;
  UARTDA2.dmareq_tx.trigger = DMA_TRIGGER_MNEM(UCA2TXIFG);
  UARTDA2.dmareq_rx.source_addr = (void*)(&UCA2RXBUF);
  UARTDA2.dmareq_rx.addr_mode = MSP430X_DMA_DSTINCR;
  UARTDA2.dmareq_rx.data_mode = (MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE);
  UARTDA2.dmareq_rx.transfer_mode = MSP430X_DMA_SINGLE;
  UARTDA2.dmareq_rx.trigger = DMA_TRIGGER_MNEM(UCA2RXIFG);
  UARTDA2.txcpt_first = true;
#endif
  
#if MSP430X_UART_USE_UARTA3 == TRUE
  /* Driver initialization.*/
  uartObjectInit(&UARTDA3);
  UARTDA3.regs = (msp430x_uart_reg_t  *)(&UCA3CTLW0);
  UARTDA3.freq = MSP430X_UARTA3_CLK_FREQ;
  UARTDA3.dmareq_tx.dest_addr = (void*)(&UCA3TXBUF);
  UARTDA3.dmareq_tx.addr_mode = MSP430X_DMA_SRCINCR;
  UARTDA3.dmareq_tx.data_mode = (MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE);
  UARTDA3.dmareq_tx.transfer_mode = MSP430X_DMA_SINGLE;
  UARTDA3.dmareq_tx.trigger = DMA_TRIGGER_MNEM(UCA3TXIFG);
  UARTDA3.dmareq_rx.source_addr = (void*)(&UCA3RXBUF);
  UARTDA3.dmareq_rx.addr_mode = MSP430X_DMA_DSTINCR;
  UARTDA3.dmareq_rx.data_mode = (MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE);
  UARTDA3.dmareq_rx.transfer_mode = MSP430X_DMA_SINGLE;
  UARTDA3.dmareq_rx.trigger = DMA_TRIGGER_MNEM(UCA3RXIFG);
  UARTDA3.txcpt_first = true;
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

  if (uartp->state != UART_STOP) {
    /* Waits for previous transmission to complete */
    while (!(uartp->regs->ifg & UCTXIFG));
    
    /* Releases previously held channels */
    if (uartp->dma_acquired_tx) {
      dmaReleaseX(&(uartp->dma_tx));
    }
    if (uartp->dma_acquired_rx) {
      dmaReleaseX(&(uartp->dma_rx));
    }
  }
  /* Configures the peripheral.*/
  /* Put the peripheral in reset */
  uartp->regs->ctlw0 |= UCSWRST;
  set_baud(uartp);
  uartp->regs->abctl = (UCDELIM1 | UCDELIM0 | uartp->config->autobaud);
#if MSP430X_UART_EXCLUSIVE_DMA == TRUE
  /* Acquire channels */
  bool b;
  if (uartp->config->dmatx_index < MSP430X_DMA_CHANNELS) {
    b = dmaClaimI(&(uartp->dma_tx), uartp->config->dmatx_index);
    osalDbgAssert(!b, "stream already allocated");
    uartp->dma_acquired_tx = !b;
  }
  else {
    uartp->dma_acquired_tx = false;
  }
  if (uartp->config->dmarx_index < MSP430X_DMA_CHANNELS) {
    b = dmaClaimI(&(uartp->dma_rx), uartp->config->dmarx_index);
    osalDbgAssert(!b, "stream already allocated");
    uartp->dma_acquired_rx = !b;
  }
  else {
    uartp->dma_acquired_rx = false;
  }
#endif
  /* Configure callbacks */
  uartp->dmareq_tx.callback.callback = tx_cb;
  uartp->dmareq_tx.callback.args = uartp;
  uartp->dmareq_rx.callback.callback = rx_cb;
  uartp->dmareq_rx.callback.args = uartp;
#if MSP430X_UART_USE_UARTA0 == TRUE
  if (&UARTDA0 == uartp) {
    uartp->regs->ctlw0 = ((uartp->config->parity << 14) | 
        (uartp->config->order << 13) | 
        (uartp->config->char_size << 12) | 
        (uartp->config->stop_bits << 11) |
        (uartp->config->autobaud << 10) | (uartp->config->autobaud << 9) |
        (MSP430X_UARTA0_UCSSEL) | 
        (UCRXEIE) | (UCBRKIE));

  }
#endif
#if MSP430X_UART_USE_UARTA1 == TRUE
  if (&UARTDA1 == uartp) {
    uartp->regs->ctlw0 = ((uartp->config->parity << 14) | 
        (uartp->config->order << 13) | 
        (uartp->config->char_size << 12) | 
        (uartp->config->stop_bits << 11) |
        (uartp->config->autobaud << 10) | (uartp->config->autobaud << 9) |
        (MSP430X_UARTA1_UCSSEL) | 
        (UCRXEIE) | (UCBRKIE));

  }
#endif
#if MSP430X_UART_USE_UARTA2 == TRUE
  if (&UARTDA2 == uartp) {
    uartp->regs->ctlw0 = ((uartp->config->parity << 14) | 
        (uartp->config->order << 13) | 
        (uartp->config->char_size << 12) | 
        (uartp->config->stop_bits << 11) |
        (uartp->config->autobaud << 10) | (uartp->config->autobaud << 9) |
        (MSP430X_UARTA2_UCSSEL) | 
        (UCRXEIE) | (UCBRKIE));

  }
#endif
#if MSP430X_UART_USE_UARTA3 == TRUE
  if (&UARTDA3 == uartp) {
    uartp->regs->ctlw0 = ((uartp->config->parity << 14) | 
        (uartp->config->order << 13) | 
        (uartp->config->char_size << 12) | 
        (uartp->config->stop_bits << 11) |
        (uartp->config->autobaud << 10) | (uartp->config->autobaud << 9) |
        (MSP430X_UARTA3_UCSSEL) | 
        (UCRXEIE) | (UCBRKIE));

  }
#endif
  uartp->regs->ie |= UCRXIE;
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
    /* Stops ongoing DMA */
    dmaCancelI(&(uartp->dma_tx));
    dmaCancelI(&(uartp->dma_rx));
    /* Resets and disables the peripheral.*/
    uartp->regs->ctlw0 = UCSWRST;
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

  uartp->dmareq_tx.source_addr = txbuf;
  uartp->dmareq_tx.size = n;
  
  while (!(uartp->regs->ifg & UCTXIFG)) ;
  uartp->regs->ifg &= ~UCTXIFG;

  init_transfer(&(uartp->dma_tx), &(uartp->dmareq_tx), &(uartp->dma_acquired_tx));
  
  uartp->regs->ifg |= UCTXIFG;
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

  dmaCancelI(&(uartp->dma_tx));
#if MSP430X_UART_EXCLUSIVE_DMA == TRUE
  if (uartp->config->dmatx_index >= MSP430X_DMA_CHANNELS) {
    dmaReleaseX(&(uartp->dma_tx));
    uartp->dma_acquired_tx = false;
  }
#else
  dmaReleaseX(&(uartp->dmta_tx));
  uartp->dma_acquired_tx = false;
#endif
  
  uartp->regs->ifg &= ~UCTXIFG;

  return (uartp->dmareq_tx.size - uartp->dma_tx.registers->sz);
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

  uartp->dmareq_rx.dest_addr = rxbuf;
  uartp->dmareq_rx.size = n;
  
  /* Disable the Idle state RX interrupt in order to allow DMA to happen */
  uartp->regs->ie &= ~UCRXIE;

  init_transfer(&(uartp->dma_rx), &(uartp->dmareq_rx), &(uartp->dma_acquired_rx));
  
  if (uartp->regs->ifg & UCRXIFG) {
    /* Toggle UCRXIFG to DMA the byte out of RXBUF */
    uartp->regs->ifg &= ~UCRXIFG;
    uartp->regs->ifg |= UCRXIFG;
  }
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

  size_t result = uartp->dma_rx.registers->sz;
  dmaCancelI(&(uartp->dma_rx));
#if MSP430X_UART_EXCLUSIVE_DMA == TRUE
  if (uartp->config->dmarx_index >= MSP430X_DMA_CHANNELS) {
    dmaReleaseX(&(uartp->dma_rx));
    uartp->dma_acquired_rx = false;
  }
#else
  dmaReleaseX(&(uartp->dma_rx));
  uartp->dma_acquired_rx = false;
#endif
  
  uartp->regs->ifg &= ~UCRXIFG;
  uartp->regs->ie |= UCRXIE; /* to re-enable CharCB */

  return result;
}

#endif /* HAL_USE_UART == TRUE */

/** @} */
