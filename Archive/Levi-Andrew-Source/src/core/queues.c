#include "queues.h"

bool elyIsQueueInitialized( queue_t * queue ) {
  return (queue->mpool.pool.object_size != 0);
}

void elyQueueObjectInit( queue_t * queue, size_t buf_size, uint8_t * mpool_storage ) {
  chGuardedPoolObjectInit(&(queue->mpool), buf_size);
  chGuardedPoolLoadArray(&(queue->mpool), mpool_storage, queue->queue_len);
}

msg_t elyQueuePostFullBufferS(queue_t * queue, uint8_t * buffer, systime_t timeout) {
  chDbgCheckClassS();
  chDbgCheck(queue != NULL);
  
  return chMBPostS(&(queue->mbox), (msg_t)(buffer), timeout);
}

msg_t elyQueuePostFullBufferI(queue_t * queue, uint8_t * buffer) {
  chDbgCheckClassI();
  chDbgCheck(queue != NULL);
  
  return chMBPostI(&(queue->mbox), (msg_t)(buffer));
}

msg_t elyQueuePostFullBufferAheadS(queue_t * queue, uint8_t * buffer, systime_t timeout) {
  chDbgCheckClassS();
  chDbgCheck(queue != NULL);
  
  return chMBPostAheadS(&(queue->mbox), (msg_t)(buffer), timeout);
}

msg_t elyQueuePostFullBufferAheadI(queue_t * queue, uint8_t * buffer) {
  chDbgCheckClassI();
  chDbgCheck(queue != NULL);
  
  return chMBPostAheadI(&(queue->mbox), (msg_t)(buffer));
}

msg_t elyQueueGetEmptyBufferTimeoutS(queue_t * queue, uint8_t ** bufferp, systime_t timeout) {
  void * result = chGuardedPoolAllocTimeoutS(&(queue->mpool), timeout);
  if (result == NULL) {
    return MSG_TIMEOUT;
  }
  
  (*bufferp) = result;
  return MSG_OK;
}

msg_t elyQueueGetEmptyBufferTimeout(queue_t * queue, uint8_t ** bufferp, systime_t timeout) {
  msg_t result;
  chSysLock();
  result = elyQueueGetEmptyBufferTimeoutS(queue, bufferp, timeout);
  chSysUnlock();
  return result;
}

void elyQueueFreeBufferI(queue_t * queue, uint8_t * buffer) {
  chGuardedPoolFreeI(&(queue->mpool), buffer);
}

/* TODO add timeout variant using guarded pools */

msg_t elyQueuePendS(queue_t * queue, uint8_t ** bufferp, systime_t timeout) {
  chDbgCheckClassS();
  chDbgCheck(queue != NULL);
  
  return chMBFetchS(&(queue->mbox), (msg_t *)(bufferp), timeout);
}

msg_t elyQueuePend(queue_t * queue, uint8_t ** bufferp, systime_t timeout) {
  msg_t result;
  chSysLock();
  result = elyQueuePendS(queue, bufferp, timeout);
  chSysUnlock();
  return result;
}

msg_t elyQueuePendI(queue_t * queue, uint8_t ** bufferp) {
  chDbgCheckClassI();
  chDbgCheck(queue != NULL);
  
  return chMBFetchI(&(queue->mbox), (msg_t *)(bufferp));
}

msg_t elyQueueGetSizeI( queue_t * queue) {
  chDbgCheckClassI();
  chDbgCheck(queue != NULL);
  
  return chMBGetSizeI(&(queue->mbox));
}

msg_t elyQueueGetFreeCountI( queue_t * queue) {
  chDbgCheckClassI();
  chDbgCheck(queue != NULL);
  
  return chMBGetFreeCountI(&(queue->mbox));
}

msg_t elyQueueGetUsedCountI( queue_t * queue) {
  chDbgCheckClassI();
  chDbgCheck(queue != NULL);
  
  return chMBGetUsedCountI(&(queue->mbox));
}

