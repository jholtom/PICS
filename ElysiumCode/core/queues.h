
#ifndef _ELYSIUM_QUEUES_H_
#define _ELYSIUM_QUEUES_H_

#include "core.h"

#if CH_CFG_USE_MEMPOOLS == FALSE
#error "Elysium queues require CH_CFG_USE_MEMPOOLS"
#endif
#if CH_CFG_USE_MAILBOXES == FALSE
#error "Elysium queues require CH_CFG_USE_MAILBOXES"
#endif

#include "chmempools.h"
#include "chmboxes.h"

#define QUEUE_DECL( name, queue_len ) \
  static PERSIST msg_t name ## _mbox_buffer[queue_len]; \
  static PERSIST queue_t name = { \
    queue_len, \
    _MAILBOX_DATA( name , name ## _mbox_buffer , queue_len ), \
    _GUARDEDMEMORYPOOL_DATA( name , 0 ), \
  }

typedef struct {
  size_t queue_len; /* TODO maybe remove this if the static allocator works */
  mailbox_t mbox;
  guarded_memory_pool_t mpool;
} queue_t;


#ifdef __cplusplus
extern "C" {
#endif
  void elyQueueObjectInit( queue_t * queue, size_t buf_size, uint8_t * mpool_storage );
  bool elyIsQueueInitialized( queue_t * queue );
  msg_t elyQueuePostFullBufferS(queue_t * queue, uint8_t * buffer, systime_t timeout);
  msg_t elyQueuePostFullBufferI(queue_t * queue, uint8_t * buffer);
  msg_t elyQueuePostFullBufferAheadS(queue_t * queue, uint8_t * buffer, systime_t timeout);
  msg_t elyQueuePostFullBufferAheadI(queue_t * queue, uint8_t * buffer);
  msg_t elyQueueGetEmptyBufferI(queue_t * queue, uint8_t ** bufferp);
  msg_t elyQueueGetEmptyBuffer(queue_t * queue, uint8_t ** bufferp);
  void elyQueueFreeBufferI(queue_t * queue, uint8_t * buffer);
  msg_t elyQueuePend(queue_t * queue, uint8_t ** bufferp, systime_t timeout);
  msg_t elyQueuePendS(queue_t * queue, uint8_t ** bufferp, systime_t timeout);
  msg_t elyQueueFetchI(queue_t * queue, uint8_t ** bufferp);
  msg_t elyQueueGetSizeI( queue_t * queue);
  msg_t elyQueueGetFreeCountI( queue_t * queue);
  msg_t elyQueueGetUsedCountI( queue_t * queue);
  msg_t elyQueueGetEmptyBufferTimeout(queue_t * queue, uint8_t ** bufferp, systime_t timeout);
  msg_t elyQueueGetEmptyBufferTimeoutS(queue_t * queue, uint8_t ** bufferp, systime_t timeout);
#ifdef __cplusplus
}
#endif
  
#endif
