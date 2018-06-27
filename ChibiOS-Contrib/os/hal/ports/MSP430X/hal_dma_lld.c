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
 * @file    MSP430X hal_dma_lld.c
 * @brief   MSP430X DMA subsystem low level driver source.
 *
 * @addtogroup MSP430X_DMA
 * @{
 */

#include "hal.h"
#include "ch.h"
#include "hal_dma_lld.h"

#if (HAL_USE_DMA == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

static msp430x_dma_ch_reg_t * const dma_regs =
    (msp430x_dma_ch_reg_t *)&DMA0CTL;

static msp430x_dma_cb_t callbacks[MSP430X_DMA_CHANNELS];
static threads_queue_t dma_queue;

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief     Set a DMA trigger using an index.
 *
 * @param[in] index   The index of the DMA channel whose trigger is set.
 * @param[in] trigger The trigger to use.
 * @note  This is all to get around weird MSP behavior when writing to memory-
 *        mapped registers using bytewise instructions.
 */
static void dma_trigger_set(uint8_t index, uint8_t trigger) {
  uint16_t * ctl = ((uint16_t *)((uintptr_t)(&DMACTL0)) + (index / 2));
  *ctl &= 0xFF00 >> (8 * (index % 2));
  *ctl |= trigger << (8 * (index % 2));
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

PORT_IRQ_HANDLER(DMA_VECTOR) {
  uint8_t index;
  OSAL_IRQ_PROLOGUE();

  index = (DMAIV >> 1) - 1;

  if (index < MSP430X_DMA_CHANNELS) {
    /* Set to idle mode (but still claimed) */
    dma_regs[index].ctl &= ~DMAEN;
    dma_trigger_set(index, DMA_TRIGGER_MNEM(DMAREQ));
    dma_regs[index].sz  = 0;
    dma_regs[index].ctl = DMAEN | DMAABORT;

    msp430x_dma_cb_t * cb = &callbacks[index];

    /* WARNING: CALLBACKS ARE CALLED IN AN ISR CONTEXT! */
    if (cb->callback != NULL) {
      cb->callback(cb->args);
    }
  }

  OSAL_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief Initialize the DMA engine.
 *
 * @init
 */
void dmaInit(void) {
  osalThreadQueueObjectInit(&dma_queue);
}

#if 0
/**
 * @brief   Requests a DMA transfer operation from the DMA engine.
 * @note    The DMA engine uses unclaimed DMA channels to provide DMA services
 *          for one-off or infrequent uses. If all channels are busy, and
 *          semaphores are enabled, the calling thread will sleep until a
 *          channel is available or the request times out. If semaphores are
 *          disabled, the calling thread will busy-wait instead of sleeping.
 * @param[out] channel    The handle of the channel allocated to the request.
 *                        Pass NULL if the handle isn't needed.
 * @param[in] request     The DMA request to be fulfilled.
 * @param[in] timeout     A timeout on the acquisition of a DMA channel.
 * @return  Returns MSG_OK if the request has been accepted, or MSG_TIMEOUT if
 *          the request has timed out.
 *         
 * @sclass
 */
msg_t dmaRequestS(msp430x_dma_ch_t * channel, msp430x_dma_req_t * request, 
    systime_t timeout) {
  
  osalDbgCheckClassS();
  
  /* Check if a DMA channel is available */
  if (queue_length >= MSP430X_DMA_CHANNELS) {
    msg_t queueresult = osalThreadEnqueueTimeoutS(&dma_queue, timeout);
    if (queueresult != MSG_OK)
      return queueresult;
  }

  /* Grab the correct DMA channel to use */
  int i = 0;
  for (i = 0; i < MSP430X_DMA_CHANNELS; i++) {
    if (!(dma_regs[i].ctl & DMAEN)) {
      break;
    }
  }
  
  /* Additional channel is in use */
  queue_length++;

  /* Make the request */
  init_request(request, i);
  
  if (channel != NULL) {
    channel->index = i;
    channel->registers = &dma_regs[i];
  }
  
  return MSG_OK;
}
#endif

msg_t dmaAcquireTimeoutS(msp430x_dma_ch_t * channel, systime_t timeout) {
  osalDbgCheckClassS();
  
  msg_t result = dmaAcquireI(channel);
  
  if (MSG_OK != result) {
    if (TIME_IMMEDIATE == timeout) {
      return MSG_TIMEOUT;
    }
    result = osalThreadEnqueueTimeoutS(&dma_queue, timeout);
  }
  return result;
}

/**
 * @brief   Acquires exclusive control of any free DMA channel.
 * @post    This channel must be interacted with using only the functions
 *          defined in this module.
 *
 * @param[out] channel    The channel handle. Must be pre-allocated.
 * @return                The operation status.
 * @retval false          no error, channel acquired.
 * @retval true           error, channel already acquired.
 * 
 * @iclass
 */
msg_t dmaAcquireI(msp430x_dma_ch_t * channel) {
  

  /* Grab the correct DMA channel to use */
  int i = 0;
  for (i = 0; i < MSP430X_DMA_CHANNELS; i++) {
    if (!(dma_regs[i].ctl & DMAEN)) {
      break;
    }
  }
  
  if (i >= MSP430X_DMA_CHANNELS)  {
    return MSG_TIMEOUT;
  }
  
  /* Acquire the channel in an idle mode */
  dma_trigger_set(i, DMA_TRIGGER_MNEM(DMAREQ));
  dma_regs[i].sz  = 0;
  dma_regs[i].ctl = DMAEN | DMAABORT;

  channel->registers = dma_regs + i;
  channel->index     = i;
  
  return MSG_OK;
}

/**
 * @brief   Claims exclusive control of a specific DMA channel.
 * @pre     The channel must not be already in use or an error is returned.
 * @post    This channel must be interacted with using only the functions
 *          defined in this module.
 *
 * @param[out] channel    The channel handle. Must be pre-allocated.
 * @param[in]  index      The index of the channel (< MSP430X_DMA_CHANNELS).
 * @return                The operation status.
 * @retval false          no error, channel claimed.
 * @retval true           error, channel in use.
 * 
 * @iclass
 */
msg_t dmaClaimI(msp430x_dma_ch_t * channel, uint8_t index) {

  /* Is the channel already acquired? */
  osalDbgAssert(index < MSP430X_DMA_CHANNELS, "invalid channel index");
  if (dma_regs[index].ctl & DMAEN) {
    return MSG_TIMEOUT;
  }

  /* Acquire the channel in an idle mode */
  dma_trigger_set(index, DMA_TRIGGER_MNEM(DMAREQ));
  dma_regs[index].sz  = 0;
  dma_regs[index].ctl = DMAEN | DMAABORT;

  channel->registers = dma_regs + index;
  channel->index     = index;
  
  return MSG_OK;
}

/**
 * @brief   Initiates a DMA transfer operation using an acquired channel.
 * @pre     The channel must have been acquired using @p dmaAcquire().
 *
 * @param[in] channel   pointer to a DMA channel from @p dmaAcquire().
 * @param[in] request   pointer to a DMA request object.
 */
void dmaTransfer(msp430x_dma_ch_t * channel, msp430x_dma_req_t * request) {
  
  osalSysLock();
  
  dmaTransferI(channel, request);
  
  osalSysUnlock();
}
  
/**
 * @brief   Initiates a DMA transfer operation using an acquired channel.
 * @pre     The channel must have been acquired using @p dmaAcquire().
 *
 * @param[in] channel   pointer to a DMA channel from @p dmaAcquire().
 * @param[in] request   pointer to a DMA request object.
 * 
 * @iclass
 */
void dmaTransferI(msp430x_dma_ch_t * channel, msp430x_dma_req_t * request) {


  channel->registers->ctl &= (~DMAEN);
  dma_trigger_set(channel->index, request->trigger);
  callbacks[channel->index] = request->callback;

#if defined(__MSP430X_LARGE__)
  asm ("movx.a %1, %0" : "=m"(channel->registers->sa) : "g"(request->source_addr) : );
  asm ("movx.a %1, %0" : "=m"(channel->registers->da) : "g"(request->dest_addr) : );
#else
  channel->registers->sa  = (uintptr_t)request->source_addr;
  channel->registers->da  = (uintptr_t)request->dest_addr;
#endif
  channel->registers->sz  = request->size;
  channel->registers->ctl = DMAIE | request->data_mode | request->addr_mode |
                            request->transfer_mode | DMAEN |
                            DMAREQ;
}

/**
 * @brief   Releases exclusive control of a DMA channel.
 * @details The channel is released from control and returned to the DMA
 *          engine pool. Trying to release an unallocated channel is an illegal
 *          operation and is trapped if assertions are enabled.
 * @pre     The channel must have been acquired using @p dmaAcquireI() 
 *          or claimed using @p dmaClaimI().
 * @post    The channel is returned to the DMA engine pool.
 */
void dmaReleaseX(msp430x_dma_ch_t * channel) {
  syssts_t sts;

  sts = osalSysGetStatusAndLockX();
  osalDbgCheck(channel != NULL);
  
  if (dmaIsClaimed(channel)) {

    /* Release the channel in an idle mode */
    channel->registers->ctl = DMAABORT;

    /* release the DMA counter */
    osalThreadDequeueNextI(&dma_queue, MSG_OK);
  }
  osalSysRestoreStatusX(sts);
}

/**
 * @brief   Cancels a DMA transfer in progress.
 * @details Any in-progress memory copy is completed before the transfer is
 *          cancelled.
 * @note    Trying to cancel a transaction on a channel which is not currently
 *          in use is not an error but it is a waste of time.
 * @note    Cancelling a transaction on a channel from the DMA engine pool will
 *          return the channel to the pool.
 */
void dmaCancelI(msp430x_dma_ch_t * channel) {
  
  osalDbgCheck(channel != NULL);
  
  if (!(channel->registers->ctl & DMAEN)) {
    return;
  }
  
  /* Cancel pending DMA transaction */
  channel->registers->ctl &= ~(DMAEN | DMAREQ);
  /* Set to idle mode (but still claimed */
  dma_trigger_set(channel->index, DMA_TRIGGER_MNEM(DMAREQ));
  channel->registers->sz  = 0;
  channel->registers->ctl = DMAEN | DMAABORT;
}

#endif /* HAL_USE_DMA == TRUE */

/** @} */
