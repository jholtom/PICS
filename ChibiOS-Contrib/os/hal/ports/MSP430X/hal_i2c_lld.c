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
 * @file    hal_i2c_lld.c
 * @brief   MSP430X I2C subsystem low level driver source.
 *
 * @addtogroup I2C
 * @{
 */

#include "hal.h"

#if (HAL_USE_I2C == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   I2CB0 driver identifier.
 */
#if (MSP430X_I2C_USE_I2CB0 == TRUE) || defined(__DOXYGEN__)
I2CDriver I2CDB0;
#endif

/**
 * @brief   I2CB1 driver identifier.
 */
#if (MSP430X_I2C_USE_I2CB1 == TRUE) || defined(__DOXYGEN__)
I2CDriver I2CDB1;
#endif

/**
 * @brief   I2CB2 driver identifier.
 */
#if (MSP430X_I2C_USE_I2CB2 == TRUE) || defined(__DOXYGEN__)
I2CDriver I2CDB2;
#endif

/**
 * @brief   I2CB3 driver identifier.
 */
#if (MSP430X_I2C_USE_I2CB3 == TRUE) || defined(__DOXYGEN__)
I2CDriver I2CDB3;
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

void tx_complete(void * args) {
  I2CDriver * i2cp = (I2CDriver *)(args);
  
  /* Enable TX IE to wake the thread or call the callback */
  i2cp->regs->ie |= UCTXIE0;
}

void rx_complete(void * args) {
  I2CDriver * i2cp = (I2CDriver *)(args);
  
  /* Generate stop condition */
  i2cp->regs->ctlw0 |= UCTXSTP;
  
  /* Enable RX interrupt to handle the last byte */
  i2cp->regs->ie |= UCRXIE0;
}

void rx_async_callback(void * args) {
  I2CDriver * i2cp = (I2CDriver *)(args);
  
  /* Call the callback with the buffer */
  if (NULL != i2cp->callback) {
    i2cp->callback(i2cp, i2cp->buffer, i2cp->req.size);
  }
  else {
    /* Default is to stop the transfer */
    i2cMSP430XEndTransferI(i2cp);
  }
}

#if MSP430X_I2C_USE_I2CB0
PORT_IRQ_HANDLER(USCI_B0_VECTOR) {
  OSAL_IRQ_PROLOGUE();
  switch(__even_in_range(UCB0IV, USCI_I2C_UCCLTOIFG)) {
    case USCI_NONE:
      break;
    case USCI_I2C_UCALIFG:
      /* Arbitration loss error */
      I2CDB0.errors |= I2C_ARBITRATION_LOST;
      break;
    case USCI_I2C_UCNACKIFG:
      /* NACK received */
      I2CDB0.errors |= I2C_ACK_FAILURE;
      break;
    case USCI_I2C_UCSTTIFG:
      osalDbgAssert(false, "Spurious interrupt in I2C driver");
      break;
    case USCI_I2C_UCSTPIFG:
      /* We've transmitted STOP - return to ready state */
      I2CDB0.state = I2C_READY;
      /* Resume the thread if we have one */
      if (I2CDB0.thread != NULL) {
        osalSysLockFromISR();
        osalThreadResumeI(&I2CDB0.thread, MSG_OK);
        osalSysUnlockFromISR();
      }
      /* Release the DMA if it's not claimed */
#if MSP430X_I2C_EXCLUSIVE_DMA == TRUE
      if (!(I2CDB0.dma_acquired)) {
#endif
        dmaReleaseX(&(I2CDB0.dma));
#if MSP430X_I2C_EXCLUSIVE_DMA == TRUE
      }
#endif
      break;
    case USCI_I2C_UCRXIFG3:
    case USCI_I2C_UCTXIFG3:
    case USCI_I2C_UCRXIFG2:
    case USCI_I2C_UCTXIFG2:
    case USCI_I2C_UCRXIFG1:
    case USCI_I2C_UCTXIFG1:
      osalDbgAssert(false, "Spurious interrupt in I2C driver");
      break;
    case USCI_I2C_UCRXIFG0:
      /* Handle final byte */
      ((uint8_t *)(I2CDB0.req.dest_addr))[I2CDB0.req.size] = UCB0RXBUF;
      /* Disable interrupt */
      UCB0IE &= ~(UCRXIE);
      break;
    case USCI_I2C_UCTXIFG0:
      /* Disable interrupt */
      UCB0IE &= ~(UCTXIE);
      /* Callback driven? */
      if (I2CDB0.thread == NULL) {
        /* Call the callback */
        if (NULL != I2CDB0.callback) {
          I2CDB0.callback(&I2CDB0, I2CDB0.buffer, I2CDB0.req.size);
        }
        else {
          osalSysLockFromISR();
          /* Default callback is to stop the transfer */
          i2cMSP430XEndTransferI(&I2CDB0);
          osalSysUnlockFromISR();
        }
      }
      else {
        /* Wake the thread */
        osalSysLockFromISR();
        osalThreadResumeI(&I2CDB0.thread, MSG_OK); 
        osalSysUnlockFromISR();
      }
      break;
    case USCI_I2C_UCBCNTIFG:
      osalDbgAssert(false, "Spurious interrupt in I2C driver");
      break;
    case USCI_I2C_UCCLTOIFG:
      /* Bus low timeout */
      I2CDB0.errors |= I2C_TIMEOUT;
      break;
  }
      
  if (I2CDB0.errors != I2C_NO_ERROR) {
    osalSysLockFromISR();
    uint16_t transferred = I2CDB0.req.size - I2CDB0.dma.registers->sz;
    /* Cancel DMA transaction */
    dmaCancelI(&I2CDB0.dma);
  
    if (I2CDB0.errors & I2C_TIMEOUT) {
      I2CDB0.state = I2C_LOCKED;
    }
    
    if (NULL != I2CDB0.callback) {
      I2CDB0.callback(&I2CDB0, I2CDB0.buffer, transferred);
    }
    
    if (I2CDB0.thread != NULL) {
      osalThreadResumeI(&I2CDB0.thread, MSG_RESET);
    }
    osalSysUnlockFromISR();
  }
  
    /* Turn off the TX IFG flag so that we'll start DMA correctly next time */
    /*I2CDB0.regs->ifg &= ~(UCTXIFG);*/
  
  OSAL_IRQ_EPILOGUE();
}
#endif

#if MSP430X_I2C_USE_I2CB1
PORT_IRQ_HANDLER(USCI_B1_VECTOR) {
  OSAL_IRQ_PROLOGUE();
  switch(__even_in_range(UCB1IV, USCI_I2C_UCCLTOIFG)) {
    case USCI_NONE:
      break;
    case USCI_I2C_UCALIFG:
      /* Arbitration loss error */
      I2CDB1.errors |= I2C_ARBITRATION_LOST;
      break;
    case USCI_I2C_UCNACKIFG:
      /* NACK received */
      I2CDB1.errors |= I2C_ACK_FAILURE;
      break;
    case USCI_I2C_UCSTTIFG:
    case USCI_I2C_UCSTPIFG:
    case USCI_I2C_UCRXIFG3:
    case USCI_I2C_UCTXIFG3:
    case USCI_I2C_UCRXIFG2:
    case USCI_I2C_UCTXIFG2:
    case USCI_I2C_UCRXIFG1:
    case USCI_I2C_UCTXIFG1:
      osalDbgAssert(false, "Spurious interrupt in I2C driver");
      break;
    case USCI_I2C_UCRXIFG0:
      /* Handle final byte */
      ((uint8_t *)(I2CDB1.req.dest_addr))[I2CDB1.req.size] = UCB1RXBUF;
      /* Disable interrupt */
      UCB1IE &= ~(UCRXIE);
      break;
    case USCI_I2C_UCTXIFG0:
      /* Disable interrupt */
      UCB1IE &= ~(UCTXIE);
      break;
    case USCI_I2C_UCBCNTIFG:
      osalDbgAssert(false, "Spurious interrupt in I2C driver");
      break;
    case USCI_I2C_UCCLTOIFG:
      /* Bus low timeout */
      I2CDB1.errors |= I2C_TIMEOUT;
      break;
  }
      
  osalSysLockFromISR();
  
  /* Cancel DMA transaction - either it's complete or there's been an error */
  dmaCancelI(&I2CDB1.dma);
  
  /* Turn off the TX IFG flag so that we'll start DMA correctly next time */
  I2CDB1.regs->ifg &= ~(UCTXIFG);
  
  /* Wake the thread */
  if (I2CDB1.errors != I2C_NO_ERROR) {
    osalThreadResumeI(&I2CDB1.thread, MSG_RESET);
  }
  else {
    osalThreadResumeI(&I2CDB1.thread, MSG_OK);
  }
  
  osalSysUnlockFromISR();
  
  OSAL_IRQ_EPILOGUE();
}
#endif

#if MSP430X_I2C_USE_I2CB2
PORT_IRQ_HANDLER(USCI_B2_VECTOR) {
  OSAL_IRQ_PROLOGUE();
  switch(__even_in_range(UCB2IV, USCI_I2C_UCCLTOIFG)) {
    case USCI_NONE:
      break;
    case USCI_I2C_UCALIFG:
      /* Arbitration loss error */
      I2CDB2.errors |= I2C_ARBITRATION_LOST;
      break;
    case USCI_I2C_UCNACKIFG:
      /* NACK received */
      I2CDB2.errors |= I2C_ACK_FAILURE;
      break;
    case USCI_I2C_UCSTTIFG:
    case USCI_I2C_UCSTPIFG:
    case USCI_I2C_UCRXIFG3:
    case USCI_I2C_UCTXIFG3:
    case USCI_I2C_UCRXIFG2:
    case USCI_I2C_UCTXIFG2:
    case USCI_I2C_UCRXIFG1:
    case USCI_I2C_UCTXIFG1:
      osalDbgAssert(false, "Spurious interrupt in I2C driver");
      break;
    case USCI_I2C_UCRXIFG0:
      /* Handle final byte */
      ((uint8_t *)(I2CDB2.req.dest_addr))[I2CDB2.req.size] = UCB2RXBUF;
      /* Disable interrupt */
      UCB2IE &= ~(UCRXIE);
      break;
    case USCI_I2C_UCTXIFG0:
      /* Disable interrupt */
      UCB2IE &= ~(UCTXIE);
      break;
    case USCI_I2C_UCBCNTIFG:
      osalDbgAssert(false, "Spurious interrupt in I2C driver");
      break;
    case USCI_I2C_UCCLTOIFG:
      /* Bus low timeout */
      I2CDB2.errors |= I2C_TIMEOUT;
      break;
  }
      
  osalSysLockFromISR();
  
  /* Cancel DMA transaction - either it's complete or there's been an error */
  dmaCancelI(&I2CDB2.dma);
  
  /* Turn off the TX IFG flag so that we'll start DMA correctly next time */
  I2CDB2.regs->ifg &= ~(UCTXIFG);
  
  /* Wake the thread */
  if (I2CDB2.errors != I2C_NO_ERROR) {
    osalThreadResumeI(&I2CDB2.thread, MSG_RESET);
  }
  else {
    osalThreadResumeI(&I2CDB2.thread, MSG_OK);
  }
  
  osalSysUnlockFromISR();
  
  OSAL_IRQ_EPILOGUE();
}
#endif

#if MSP430X_I2C_USE_I2CB3
PORT_IRQ_HANDLER(USCI_B3_VECTOR) {
  OSAL_IRQ_PROLOGUE();
  switch(__even_in_range(UCB3IV, USCI_I2C_UCCLTOIFG)) {
    case USCI_NONE:
      break;
    case USCI_I2C_UCALIFG:
      /* Arbitration loss error */
      I2CDB3.errors |= I2C_ARBITRATION_LOST;
      break;
    case USCI_I2C_UCNACKIFG:
      /* NACK received */
      I2CDB3.errors |= I2C_ACK_FAILURE;
      break;
    case USCI_I2C_UCSTTIFG:
    case USCI_I2C_UCSTPIFG:
    case USCI_I2C_UCRXIFG3:
    case USCI_I2C_UCTXIFG3:
    case USCI_I2C_UCRXIFG2:
    case USCI_I2C_UCTXIFG2:
    case USCI_I2C_UCRXIFG1:
    case USCI_I2C_UCTXIFG1:
      osalDbgAssert(false, "Spurious interrupt in I2C driver");
      break;
    case USCI_I2C_UCRXIFG0:
      /* Handle final byte */
      ((uint8_t *)(I2CDB3.req.dest_addr))[I2CDB3.req.size] = UCB3RXBUF;
      /* Disable interrupt */
      UCB3IE &= ~(UCRXIE);
      break;
    case USCI_I2C_UCTXIFG0:
      /* Disable interrupt */
      UCB3IE &= ~(UCTXIE);
      break;
    case USCI_I2C_UCBCNTIFG:
      osalDbgAssert(false, "Spurious interrupt in I2C driver");
      break;
    case USCI_I2C_UCCLTOIFG:
      /* Bus low timeout */
      I2CDB3.errors |= I2C_TIMEOUT;
      break;
  }
      
  osalSysLockFromISR();
  
  /* Cancel DMA transaction - either it's complete or there's been an error */
  dmaCancelI(&I2CDB3.dma);
  
  /* Turn off the TX IFG flag so that we'll start DMA correctly next time */
  I2CDB3.regs->ifg &= ~(UCTXIFG);
  
  /* Wake the thread */
  if (I2CDB3.errors != I2C_NO_ERROR) {
    osalThreadResumeI(&I2CDB3.thread, MSG_RESET);
  }
  else {
    osalThreadResumeI(&I2CDB3.thread, MSG_OK);
  }
  
  osalSysUnlockFromISR();
  
  OSAL_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level I2C driver initialization.
 *
 * @notapi
 */
void i2c_lld_init(void) {

#if MSP430X_I2C_USE_I2CB0 == TRUE
  i2cObjectInit(&I2CDB0);
  I2CDB0.regs = (msp430x_i2c_reg_t *)(&UCB0CTLW0);
  I2CDB0.req.data_mode = MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE;
  I2CDB0.req.transfer_mode = MSP430X_DMA_SINGLE;
  I2CDB0.txtrig = DMA_TRIGGER_MNEM(UCB0TXIFG0);
  I2CDB0.rxtrig = DMA_TRIGGER_MNEM(UCB0RXIFG0);
  I2CDB0.req.callback.callback = NULL;
  I2CDB0.req.callback.args = (void*)(&I2CDB0);
#endif
  
#if MSP430X_I2C_USE_I2CB1 == TRUE
  i2cObjectInit(&I2CDB1);
  I2CDB1.regs = (msp430x_i2c_reg_t *)(&UCB1CTLW0);
  I2CDB1.req.data_mode = MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE;
  I2CDB1.req.transfer_mode = MSP430X_DMA_SINGLE;
  I2CDB1.txtrig = DMA_TRIGGER_MNEM(UCB1TXIFG0);
  I2CDB1.rxtrig = DMA_TRIGGER_MNEM(UCB1RXIFG0);
  I2CDB1.req.callback.callback = NULL;
  I2CDB1.req.callback.args = (void*)(&I2CDB1);
#endif
  
#if MSP430X_I2C_USE_I2CB2 == TRUE
  i2cObjectInit(&I2CDB2);
  I2CDB2.regs = (msp430x_i2c_reg_t *)(&UCB2CTLW0);
  I2CDB2.req.data_mode = MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE;
  I2CDB2.req.transfer_mode = MSP430X_DMA_SINGLE;
  I2CDB2.txtrig = DMA_TRIGGER_MNEM(UCB2TXIFG0);
  I2CDB2.rxtrig = DMA_TRIGGER_MNEM(UCB2RXIFG0);
  I2CDB2.req.callback.callback = NULL;
  I2CDB2.req.callback.args = (void*)(&I2CDB2);
#endif
  
#if MSP430X_I2C_USE_I2CB3 == TRUE
  i2cObjectInit(&I2CDB3);
  I2CDB3.regs = (msp430x_i2c_reg_t *)(&UCB3CTLW0);
  I2CDB3.regs = &UCB3CTLW0;
  I2CDB3.req.data_mode = MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE;
  I2CDB3.req.transfer_mode = MSP430X_DMA_SINGLE;
  I2CDB3.txtrig = DMA_TRIGGER_MNEM(UCB3TXIFG0);
  I2CDB3.rxtrig = DMA_TRIGGER_MNEM(UCB3RXIFG0);
  I2CDB3.req.callback.callback = NULL;
  I2CDB3.req.callback.args = (void*)(&I2CDB3);
#endif
  
}

/**
 * @brief   Configures and activates the I2C peripheral.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
void i2c_lld_start(I2CDriver *i2cp) {

    /* Enables the peripheral.*/
#if MSP430X_I2C_USE_I2CB0 == TRUE
    if (&I2CDB0 == i2cp) {
      /* Stop the peripheral */
      UCB0CTLW0 |= 0x01;
      /* Configure bit rate */
      UCB0BRW = MSP430X_I2CB0_CLK_FREQ / I2CDB0.config->bit_rate;
      /* Clock low timeout 34 ms, manual STOP condition */
      UCB0CTLW1 = UCCLTO_3 | UCASTP_0;
  #if MSP430X_I2C_10BIT
      UCB0CTLW0 = (I2CDB0.config->long_addr << 14) | UCMST | UCMODE_3 | UCSYNC
        | MSP430X_I2CB0_UCSSEL;
  #else
      UCB0CTLW0 = UCMST | UCMODE_3 | UCSYNC | MSP430X_I2CB0_UCSSEL;
  #endif
      if (I2CDB0.regs->statw & BIT4) {
        /* Disable again */
        UCB0CTLW0 |= 0x01;
        /* Set the SCL pin as an output */
        P1SEL1 &= ~0x80;
        P1DIR |= 0x80;
        P1REN &= ~0x80;
        /* Toggle it 10 times */
        for (int i = 0 ; i < 10; i++) {
          P1OUT &= ~0x80;
          P1OUT |= 0x80;
        }
        /* Reset it to I2C mode */
        P1DIR &= ~0x80;
        P1SEL1 |= 0x80;
        P1REN |= 0x80;
        /* Re-enable the peripheral */
        UCB0CTLW0 &= ~0x01;
      }
      UCB0IE = UCCLTOIE | UCNACKIE | UCALIE | UCSTPIE;
      UCB0IFG = 0;
  #if MSP430X_I2C_EXCLUSIVE_DMA
      if (i2cp->dma_acquired) {
        dmaReleaseX(&I2CDB0.dma);
        I2CDB0.dma_acquired = false;
      }
      if (I2CDB0.config->dma_index < MSP430X_DMA_CHANNELS) {
        dmaClaimI(&I2CDB0.dma, I2CDB0.config->dma_index);
        I2CDB0.dma_acquired = true;
      }
  #endif
    }
#endif
#if MSP430X_I2C_USE_I2CB1 == TRUE
    if (&I2CDB1 == i2cp) {
      /* Stop the peripheral */
      UCB1CTLW0 |= 0x01;
      /* Configure bit rate */
      UCB1BRW = MSP430X_I2CB1_CLK_FREQ / I2CDB1.config->bit_rate;
      /* Clock low timeout 34 ms, STOP condition based on byte count */
      UCB1CTLW1 = UCCLTO_3 | UCASTP_0;
  #if MSP430X_I2C_10BIT
      UCB1CTLW0 = (I2CDB1.config->long_addr << 14) | UCMST | UCMODE_3 | UCSYNC
        | MSP430X_I2CB1_UCSSEL;
  #else
      UCB1CTLW0 = UCMST | UCMODE_3 | UCSYNC | MSP430X_I2CB1_UCSSEL;
  #endif
      UCB1IE = UCCLTOIE | UCNACKIE | UCALIE;
      UCB1IFG = 0;
  #if MSP430X_I2C_EXCLUSIVE_DMA
      if (I2CDB1.config->dma_index < MSP430X_DMA_CHANNELS) {
        osalSysLock();
        dmaAcquireI(&I2CDB1.dma, I2CDB1.config->dma_index);
        osalSysUnlock();
      }
  #endif
    }
#endif
#if MSP430X_I2C_USE_I2CB2 == TRUE
    if (&I2CDB2 == i2cp) {
      /* Stop the peripheral */
      UCB2CTLW0 |= 0x01;
      /* Configure bit rate */
      UCB2BRW = MSP430X_I2CB2_CLK_FREQ / I2CDB2.config->bit_rate;
      /* Clock low timeout 34 ms, STOP condition based on byte count */
      UCB2CTLW1 = UCCLTO_3 | UCASTP_0;
  #if MSP430X_I2C_10BIT
      UCB2CTLW0 = (I2CDB2.config->long_addr << 14) | UCMST | UCMODE_3 | UCSYNC
        | MSP430X_I2CB2_UCSSEL;
  #else
      UCB2CTLW0 = UCMST | UCMODE_3 | UCSYNC | MSP430X_I2CB2_UCSSEL;
  #endif
      UCB2IE = UCCLTOIE | UCNACKIE | UCALIE;
      UCB2IFG = 0;
  #if MSP430X_I2C_EXCLUSIVE_DMA
      if (I2CDB2.config->dma_index < MSP430X_DMA_CHANNELS) {
        osalSysLock();
        dmaAcquireI(&I2CDB2.dma, I2CDB2.config->dma_index);
        osalSysUnlock();
      }
  #endif
    }
#endif
#if MSP430X_I2C_USE_I2CB3 == TRUE
    if (&I2CDB3 == i2cp) {
      /* Stop the peripheral */
      UCB3CTLW0 |= UCSWRST;
      /* Configure bit rate */
      UCB3BRW = MSP430X_I2CB3_CLK_FREQ / I2CDB3.config->bit_rate;
      /* Clock low timeout 34 ms, STOP condition based on byte count */
      UCB3CTLW1 = UCCLTO_3 | UCASTP_0;
  #if MSP430X_I2C_10BIT
      UCB3CTLW0 = (I2CDB3.config->long_addr << 14) | UCMST | UCMODE_3 | UCSYNC
        | MSP430X_I2CB3_UCSSEL;
  #else
      UCB3CTLW0 = UCMST | UCMODE_3 | UCSYNC | MSP430X_I2CB3_UCSSEL;
  #endif
      UCB3IE = UCCLTOIE | UCNACKIE | UCALIE;
      UCB3IFG = 0;
  #if MSP430X_I2C_EXCLUSIVE_DMA
      if (I2CDB3.config->dma_index < MSP430X_DMA_CHANNELS) {
        osalSysLock();
        dmaAcquireI(&I2CDB3.dma, I2CDB3.config->dma_index);
        osalSysUnlock();
      }
  #endif
    }
#endif

}

/**
 * @brief   Deactivates the I2C peripheral.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
void i2c_lld_stop(I2CDriver *i2cp) {
  
  osalDbgAssert(i2cp->state != I2C_UNINIT, "Can't stop uninitialized driver");

  if (i2cp->state != I2C_STOP) {

    /* Disables the peripheral.*/
#if MSP430X_I2C_USE_I2CB0 == TRUE
    if (&I2CDB0 == i2cp) {
      osalSysLock();
      dmaCancelI(&I2CDB0.dma);
      osalSysUnlock();
      UCB0CTLW0 |= UCSWRST;
  #if MSP430X_I2C_EXCLUSIVE_DMA
      if (i2cp->dma_acquired) {
        dmaReleaseX(&I2CDB0.dma);
        I2CDB0.dma_acquired = false;
      }
  #endif
    }
#endif
#if MSP430X_I2C_USE_I2CB1 == TRUE
    if (&I2CDB1 == i2cp) {
      osalSysLock();
      dmaCancelI(&I2CDB1.dma);
      osalSysUnlock();
      UCB1CTLW0 |= UCSWRST;
  #if MSP430X_I2C_EXCLUSIVE_DMA
      if (I2CDB1.config->dma_index < MSP430X_DMA_CHANNELS) {
        dmaReleaseX(&I2CDB1.dma);
        I2CDB0.dma_acquired = false;
      }
  #endif
    }
#endif
#if MSP430X_I2C_USE_I2CB2 == TRUE
    if (&I2CDB2 == i2cp) {
      osalSysLock();
      dmaCancelI(&I2CDB2.dma);
      osalSysUnlock();
      UCB2CTLW0 |= UCSWRST;
  #if MSP430X_I2C_EXCLUSIVE_DMA
      if (I2CDB2.config->dma_index < MSP430X_DMA_CHANNELS) {
        dmaReleaseX(&I2CDB2.dma);
        I2CDB0.dma_acquired = false;
      }
  #endif
    }
#endif
#if MSP430X_I2C_USE_I2CB3 == TRUE
    if (&I2CDB3 == i2cp) {
      osalSysLock();
      dmaCancelI(&I2CDB3.dma);
      osalSysUnlock();
      UCB3CTLW0 |= UCSWRST;
  #if MSP430X_I2C_EXCLUSIVE_DMA
      if (I2CDB3.config->dma_index < MSP430X_DMA_CHANNELS) {
        dmaReleaseX(&I2CDB3.dma);
      }
  #endif
    }
#endif
  }
}

void i2cMSP430XStartReceiveToRegI(I2CDriver *i2cp, i2caddr_t addr, size_t n, 
    uint8_t * regp, i2ccallback_t callback) {
  
  osalDbgCheckClassI();
  
  osalDbgAssert(n > 0, "can't receive no bytes");
#if MSP430X_I2C_10BIT
  osalDbgAssert(!(addr & 0xFC00), "Invalid address");
#else
  osalDbgAssert(!(addr & 0x80), "Invalid address");
#endif
  
  /* Reset error flags */
  i2cp->errors = I2C_NO_ERROR;
  /* Set state */
  i2cp->state = I2C_ACTIVE_RX;
  
  i2cp->buffer = regp;
  
  /* DMA only works for more than 2 bytes */
  if (1 == n) {
    /* DMA request is referenced by interrupt handler */
    i2cp->req.dest_addr = regp;
    i2cp->req.size = 0;
    /* Receiver mode */
    i2cp->regs->ctlw0 &= ~UCTR;
    /* Generate Start */
    i2cp->regs->ctlw0 |= UCTXSTT;
    /* Generate Stop immediately */
    while (i2cp->regs->ctlw0 & UCTXSTT);
    /* Call the callback immediately as well */
    if (callback != NULL) {
      callback(i2cp, regp, 1);
    }
  }
  else {
    /* Set up DMA */
    i2cp->req.source_addr = &(i2cp->regs->rxbuf);
    i2cp->req.dest_addr = regp;
    i2cp->req.size = n - 1; /* have to stop early to generate STOP */
    i2cp->req.addr_mode = 0; /* neither increment nor decrement */
    i2cp->req.trigger = i2cp->rxtrig;
    /* Custom callback */
    i2cp->callback = callback;
    /* DMA callback handler */
    i2cp->req.callback.callback = &rx_async_callback;
    i2cp->req.callback.args = i2cp;
    
    if (!dmaIsClaimed(&(i2cp->dma))) {
      dmaAcquireI(&(i2cp->dma));
    }
    dmaTransferI(&(i2cp->dma), &(i2cp->req));
    
    /* Slave address first, then receiver mode, then START */
    i2cp->regs->i2csa = addr;
    i2cp->regs->ctlw0 &= ~UCTR;
    i2cp->regs->ctlw0 |= UCTXSTT;
  }
}

void i2cMSP430XStartReceiveI(I2CDriver *i2cp, i2caddr_t addr, size_t n, 
    uint8_t * rxbuf, i2ccallback_t callback) {
  
  osalDbgCheckClassI();
  
  osalDbgAssert(n > 0, "can't receive no bytes");
#if MSP430X_I2C_10BIT
  osalDbgAssert(!(addr & 0xFC00), "Invalid address");
#else
  osalDbgAssert(!(addr & 0x80), "Invalid address");
#endif
  
  /* Reset error flags */
  i2cp->errors = I2C_NO_ERROR;
  /* Set state */
  i2cp->state = I2C_ACTIVE_RX;
  
  i2cp->buffer = rxbuf;
  
  /* DMA only works for more than 2 bytes */
  if (1 == n) {
    /* DMA request is referenced by interrupt handler */
    i2cp->req.dest_addr = rxbuf;
    i2cp->req.size = 0;
    /* Receiver mode */
    i2cp->regs->ctlw0 &= ~UCTR;
    /* Generate Start */
    i2cp->regs->ctlw0 |= UCTXSTT;
    /* Generate Stop immediately */
    while (i2cp->regs->ctlw0 & UCTXSTT);
    /* Call the callback immediately as well */
    if (callback != NULL) {
      chSysUnlockFromISR();
      callback(i2cp, rxbuf, 1);
      chSysLockFromISR();
    }
  }
  else {
    /* Set up DMA */
    i2cp->req.source_addr = &(i2cp->regs->rxbuf);
    i2cp->req.dest_addr = rxbuf;
    i2cp->req.size = n - 1; /* have to stop early to generate STOP */
    i2cp->req.addr_mode = MSP430X_DMA_DSTINCR;
    i2cp->req.trigger = i2cp->rxtrig;
    /* Custom callback */
    i2cp->callback = callback;
    /* DMA callback handler */
    i2cp->req.callback.callback = &rx_async_callback;
    i2cp->req.callback.args = i2cp;
    
    if (!dmaIsClaimed(&(i2cp->dma))) {
      dmaAcquireI(&(i2cp->dma));
    }
    dmaTransferI(&(i2cp->dma), &(i2cp->req));
    
    /* Slave address first, then receiver mode, then START */
    i2cp->regs->i2csa = addr;
    i2cp->regs->ctlw0 &= ~UCTR;
    i2cp->regs->ctlw0 |= UCTXSTT;
  }
}

void i2cMSP430XStartTransmitMSBI(I2CDriver *i2cp, i2caddr_t addr, size_t n, 
    uint8_t * txbuf, i2ccallback_t callback) {
  
  osalDbgCheckClassI();
  
  osalDbgAssert(i2cp->state == I2C_READY, "not ready");
  
  osalDbgAssert(n > 0, "can't transmit no bytes");
  
#if MSP430X_I2C_10BIT
  osalDbgAssert(!(addr & 0xFC00), "Invalid address");
#else
  osalDbgAssert(!(addr & 0x80), "Invalid address");
#endif
  
  i2cp->regs->ifg &= ~UCTXIFG;
  i2cp->regs->ie &= ~UCTXIE;
  
  /* Reset error flags */
  i2cp->errors = I2C_NO_ERROR;
  /* Set state */
  i2cp->state = I2C_ACTIVE_TX;
  
  i2cp->buffer = txbuf;
  
  /* Set up DMA */
  i2cp->req.source_addr = txbuf + n - 1;
  i2cp->req.dest_addr = &(i2cp->regs->txbuf);
  i2cp->req.size = n;
  i2cp->req.addr_mode = MSP430X_DMA_SRCDECR;
  i2cp->req.trigger = i2cp->txtrig;
  /* Custom callback */
  i2cp->callback = callback;
  /* DMA callback handler */
  i2cp->req.callback.callback = &tx_complete;
  
  if (!dmaIsClaimed(&(i2cp->dma))) {
    dmaAcquireI(&(i2cp->dma));
  }
  dmaTransferI(&(i2cp->dma), &(i2cp->req));
    
  /* Slave address first, then transmitter mode and START */
  i2cp->regs->i2csa = addr;
  i2cp->regs->ctlw0 |= (UCTR | UCTXSTT);
  /* IFG is set automatically after START */
  
}

void i2cMSP430XStartTransmitI(I2CDriver *i2cp, i2caddr_t addr, size_t n, 
    uint8_t * txbuf, i2ccallback_t callback) {
  
  osalDbgCheckClassI();
  
  osalDbgAssert(i2cp->state == I2C_READY, "not ready");
  
  osalDbgAssert(n > 0, "can't transmit no bytes");
  
#if MSP430X_I2C_10BIT
  osalDbgAssert(!(addr & 0x03FF), "Invalid address");
#else
  osalDbgAssert(!(addr & 0x7F), "Invalid address");
#endif
  
  /* Reset error flags */
  i2cp->errors = I2C_NO_ERROR;
  /* Set state */
  i2cp->state = I2C_ACTIVE_TX;
  
  i2cp->buffer = txbuf;
  
  /* Set up DMA */
  i2cp->req.source_addr = txbuf;
  i2cp->req.dest_addr = &(i2cp->regs->txbuf);
  i2cp->req.size = n;
  i2cp->req.addr_mode = MSP430X_DMA_SRCINCR;
  i2cp->req.trigger = i2cp->txtrig;
  /* Custom callback */
  i2cp->callback = callback;
  /* DMA callback handler */
  i2cp->req.callback.callback = &tx_complete;
  
  if (!dmaIsClaimed(&(i2cp->dma))) {
    dmaAcquireI(&(i2cp->dma));
  }
  dmaTransferI(&(i2cp->dma), &(i2cp->req));
    
  /* Slave address first, then transmitter mode and START */
  i2cp->regs->i2csa = addr;
  i2cp->regs->ctlw0 |= (UCTR | UCTXSTT);
  /* IFG is set automatically after START */
  
}

void i2cMSP430XContinueTransmitMSBI(I2CDriver *i2cp, i2caddr_t addr, size_t n, 
    uint8_t * txbuf, i2ccallback_t callback) {
  
  osalDbgCheckClassI();
  
  osalDbgAssert(i2cp->state == I2C_ACTIVE_TX, "can't continue");
  
  osalDbgAssert(n > 0, "can't transmit no bytes");
  
#if MSP430X_I2C_10BIT
  osalDbgAssert(!(addr & 0x03FF), "Invalid address");
#else
  osalDbgAssert(!(addr & 0x7F), "Invalid address");
#endif
  
  i2cp->buffer = txbuf;
  
  /* Set up DMA */
  i2cp->req.source_addr = txbuf + n - 1;
  i2cp->req.dest_addr = &(i2cp->regs->txbuf);
  i2cp->req.size = n;
  i2cp->req.addr_mode = MSP430X_DMA_SRCDECR;
  i2cp->req.trigger = i2cp->txtrig;
  /* Custom callback */
  i2cp->callback = callback;
  /* DMA callback handler */
  i2cp->req.callback.callback = &tx_complete;
  
  osalDbgAssert(dmaIsClaimed(&(i2cp->dma)), "continuation should have DMA");
  dmaTransferI(&(i2cp->dma), &(i2cp->req));
    
  /* Reset IFG to restart the DMA */
  i2cp->regs->ifg |= UCTXIFG;
  
}

void i2cMSP430XContinueTransmitMemsetI(I2CDriver *i2cp, i2caddr_t addr, size_t n, 
    uint8_t * value, i2ccallback_t callback) {
  
  osalDbgCheckClassI();
  
  osalDbgAssert(i2cp->state == I2C_ACTIVE_TX, "can't continue");
  
  osalDbgAssert(n > 0, "can't transmit no bytes");
  
#if MSP430X_I2C_10BIT
  osalDbgAssert(!(addr & 0xFC00), "Invalid address");
#else
  osalDbgAssert(!(addr & 0x80), "Invalid address");
#endif
  
  i2cp->buffer = value;
  
  /* Set up DMA */
  i2cp->req.source_addr = value;
  i2cp->req.dest_addr = &(i2cp->regs->txbuf);
  i2cp->req.size = n;
  i2cp->req.addr_mode = 0; /* neither increment nor decrement */
  i2cp->req.trigger = i2cp->txtrig;
  /* Custom callback */
  i2cp->callback = callback;
  /* DMA callback handler */
  i2cp->req.callback.callback = &tx_complete;
  
  osalDbgAssert(dmaIsClaimed(&(i2cp->dma)), "continuation should have DMA");
  dmaTransferI(&(i2cp->dma), &(i2cp->req));
    
  /* Reset IFG to restart the DMA */
  i2cp->regs->ifg |= UCTXIFG;
  
}

void i2cMSP430XContinueTransmitI(I2CDriver *i2cp, i2caddr_t addr, size_t n, 
    uint8_t * txbuf, i2ccallback_t callback) {
  
  osalDbgCheckClassI();
  
  osalDbgAssert(i2cp->state == I2C_ACTIVE_TX, "can't continue");
  
  osalDbgAssert(n > 0, "can't transmit no bytes");
  
#if MSP430X_I2C_10BIT
  osalDbgAssert(!(addr & 0xFC00), "Invalid address");
#else
  osalDbgAssert(!(addr & 0x80), "Invalid address");
#endif
  
  i2cp->buffer = txbuf;
  
  /* Set up DMA */
  i2cp->req.source_addr = txbuf;
  i2cp->req.dest_addr = &(i2cp->regs->txbuf);
  i2cp->req.size = n;
  i2cp->req.addr_mode = MSP430X_DMA_SRCINCR;
  i2cp->req.trigger = i2cp->txtrig;
  /* Custom callback */
  i2cp->callback = callback;
  /* DMA callback handler */
  i2cp->req.callback.callback = &tx_complete;
  
  osalDbgAssert(dmaIsClaimed(&(i2cp->dma)), "continuation should have DMA");
  dmaTransferI(&(i2cp->dma), &(i2cp->req));
    
  /* Reset IFG to restart the DMA */
  i2cp->regs->ifg |= UCTXIFG;
  
}

void i2cMSP430XEndTransferI(I2CDriver *i2cp) {
  
  osalDbgCheckClassI();
  
  osalDbgAssert(i2cp->state == I2C_ACTIVE_RX || i2cp->state == I2C_ACTIVE_TX,
      "can't cancel a transaction that's not happening");
  
  /* Generate STOP condition*/
  i2cp->regs->ctlw0 |= UCTXSTP;
  
  if (i2cp->state == I2C_ACTIVE_RX) {
    /* Enable RX interrupt to handle the last byte */
    /*i2cp->regs->ie |= UCRXIE0;*/
    
    /* Ensure we don't accidentally continue before receiving the last byte */
    /*i2cp->state = I2C_ACTIVE_RX;*/
    
    /* Wait for last RX byte */
    while (!(i2cp->regs->ifg & UCRXIFG0));
    
    if (i2cp->req.addr_mode) {
      ((uint8_t *)(i2cp->req.dest_addr))[i2cp->req.size] = i2cp->regs->rxbuf;
    }
    else {
      *((uint8_t *)(i2cp->req.dest_addr)) = i2cp->regs->rxbuf;
    }
    
    while (i2cp->regs->ctlw0 & UCTXSTP) ;
    i2cp->regs->ifg &= ~UCSTPIFG;
      
    if (i2cp->regs->ifg & UCRXIFG0) {
      /* Need to read the buffer to reset the state */
      volatile uint8_t throwaway;
      throwaway = i2cp->regs->rxbuf;
      (void)(throwaway);
    }
  }
  else {
    while (i2cp->regs->ctlw0 & UCTXSTP) ;
    i2cp->regs->ifg &= ~UCSTPIFG;
  }
  
  i2cp->state = I2C_READY;
}

/**
 * @brief   Receives data via the I2C bus as master.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 * @param[in] addr      slave device address
 * @param[out] rxbuf    pointer to the receive buffer
 * @param[in] rxbytes   number of bytes to be received
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The operation status.
 * @retval MSG_OK       if the function succeeded.
 * @retval MSG_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval MSG_TIMEOUT  if a timeout occurred before operation end. <b>After a
 *                      timeout the driver must be stopped and restarted
 *                      because the bus is in an uncertain state</b>.
 *
 * @notapi
 */
msg_t i2c_lld_master_receive_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                     uint8_t *rxbuf, size_t rxbytes,
                                     systime_t timeout) {
  systime_t start;

#if MSP430X_I2C_10BIT
  osalDbgAssert(!(addr & 0x03FF), "Invalid address");
#else
  osalDbgAssert(!(addr & 0x7F), "Invalid address");
#endif
  
  /* So that setup doesn't add to timeout calc */
  start = osalOsGetSystemTimeX();
  
  /* Reset error flags */
  i2cp->errors = I2C_NO_ERROR;
  
  if (1 == rxbytes) {
    /* DMA request is referenced by interrupt handler */
    i2cp->req.dest_addr = rxbuf;
    i2cp->req.size = 0;
    /* Receiver mode */
    i2cp->regs->ctlw0 &= ~UCTR;
    /* Generate Start */
    i2cp->regs->ctlw0 |= UCTXSTT;
    /* Generate Stop immediately */
    while (i2cp->regs->ctlw0 & UCTXSTT);
    i2cp->regs->ctlw0 |= UCTXSTP;
    /* Enable RX interrupt to handle the single received byte */
    i2cp->regs->ie |= UCRXIE0;
  }
  else {
    /* Set up DMA */
    i2cp->req.source_addr = &(i2cp->regs->rxbuf);
    i2cp->req.dest_addr = rxbuf;
    i2cp->req.size = rxbytes - 1; /* have to stop early to generate STOP */
    i2cp->req.addr_mode = MSP430X_DMA_DSTINCR;
    i2cp->req.trigger = i2cp->rxtrig;
    i2cp->req.callback.callback = &rx_complete;
    
  if (!dmaIsClaimed(&(i2cp->dma))) {
    dmaAcquireI(&(i2cp->dma));
    /* TODO fallback */
  }
    
    /* Slave address first, then receiver mode, then START */
    i2cp->regs->i2csa = addr;
    i2cp->regs->ctlw0 &= ~UCTR;
    i2cp->regs->ctlw0 |= UCTXSTT;
  }
  
  /* Wait until timeout, error, or transfer complete */
  msg_t msg;
  msg = osalThreadSuspendTimeoutS(&(i2cp->thread), 
      timeout - (osalOsGetSystemTimeX() - start));
  
#if (MSP430X_I2C_EXCLUSIVE_DMA == TRUE)
  if (!(i2cp->dma_acquired)) {
    dmaReleaseX(&(i2cp->dma));
  }
#else
  dmaReleaseX(&(i2cp->dma));
#endif
  
  return msg;
}

/**
 * @brief   Transmits data via the I2C bus as master.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 * @param[in] addr      slave device address
 * @param[in] txbuf     pointer to the transmit buffer
 * @param[in] txbytes   number of bytes to be transmitted
 * @param[out] rxbuf    pointer to the receive buffer
 * @param[in] rxbytes   number of bytes to be received
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The operation status.
 * @retval MSG_OK       if the function succeeded.
 * @retval MSG_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval MSG_TIMEOUT  if a timeout occurred before operation end. <b>After a
 *                      timeout the driver must be stopped and restarted
 *                      because the bus is in an uncertain state</b>.
 *
 * @notapi
 */
msg_t i2c_lld_master_transmit_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                      const uint8_t *txbuf, size_t txbytes,
                                      uint8_t *rxbuf, size_t rxbytes,
                                      systime_t timeout) {
  systime_t start;
  msg_t result;

#if MSP430X_I2C_10BIT
  osalDbgAssert(!(addr & 0xFC00), "Invalid address");
#else
  osalDbgAssert(!(addr & 0x80), "Invalid address");
#endif
  
  /* So that setup doesn't add to timeout calc */
  start = osalOsGetSystemTimeX();
  
  /* Reset error flags */
  i2cp->errors = I2C_NO_ERROR;
  
  /* Write first */
  /* Set up DMA */
  i2cp->req.source_addr = txbuf;
  i2cp->req.dest_addr = &(i2cp->regs->txbuf);
  i2cp->req.size = txbytes;
  i2cp->req.addr_mode = MSP430X_DMA_SRCINCR;
  i2cp->req.trigger = i2cp->txtrig;
  i2cp->req.callback.callback = &tx_complete;
  
  if (!dmaIsClaimed(&(i2cp->dma))) {
    dmaAcquireI(&(i2cp->dma));
    /* TODO fallback */
  }
    
  dmaTransferI(&(i2cp->dma), &(i2cp->req));
  
  /* Slave address first, then transmitter mode and START */
  i2cp->regs->i2csa = addr;
  i2cp->regs->ctlw0 |= (UCTR | UCTXSTT);
  /* IFG is set automatically after START */
  
  /* Wait until timeout or until transfer complete */
  result = osalThreadSuspendTimeoutS(&i2cp->thread, 
      timeout - (osalOsGetSystemTimeX() - start));
  
  if (MSG_OK != result) {
    return result;
  }
  
  /* Now the read */
  /* If no read, short circuit that logic path */
  if (0 == rxbytes) {
    /* Generate stop */
    i2cp->regs->ctlw0 |= UCTXSTP;
    /*return MSG_OK;*/
  }
  else if (1 == rxbytes) {
    /* DMA request is referenced by interrupt handler */
    i2cp->req.dest_addr = rxbuf;
    i2cp->req.size = 0;
    /* Receiver mode */
    i2cp->regs->ctlw0 &= ~UCTR;
    /* Generate restart */
    i2cp->regs->ctlw0 |= UCTXSTT;
    /* Generate stop immediately */
    while (i2cp->regs->ctlw0 & UCTXSTT);
    i2cp->regs->ctlw0 |= UCTXSTP;
    /* Enable RX interrupt to handle the single received byte */
    i2cp->regs->ie |= UCRXIE0;
  }
  else {
    /* Set up DMA */
    i2cp->req.source_addr = &(i2cp->regs->rxbuf);
    i2cp->req.dest_addr = rxbuf;
    i2cp->req.size = rxbytes - 1;
    i2cp->req.addr_mode = MSP430X_DMA_DSTINCR;
    i2cp->req.trigger = i2cp->rxtrig;
    i2cp->req.callback.callback = &rx_complete;
    
    dmaTransferI(&(i2cp->dma), &(i2cp->req));
    
    /* Receiver mode */
    i2cp->regs->ctlw0 &= ~UCTR;
    /* Generate restart */
    i2cp->regs->ctlw0 |= UCTXSTT;
  }
    
  /* Wait until timeout or until transfer complete */
  result = osalThreadSuspendTimeoutS(&i2cp->thread, 
      timeout - (osalOsGetSystemTimeX() - start));
  
#if (MSP430X_I2C_EXCLUSIVE_DMA == TRUE)
  if (!(i2cp->dma_acquired)) {
    dmaReleaseX(&(i2cp->dma));
  }
#else
  dmaReleaseX(&(i2cp->dma));
#endif
  
  return result;
}

#endif /* HAL_USE_I2C == TRUE */

/** @} */
