
#include "fram.h"

/* Queue for Requests */
PERSIST QUEUE_DECL(fram_queue, FRAM_QUEUE_LEN);

static fram_req_t PERSIST fram_queue_storage[FRAM_QUEUE_LEN];

/* TODO timeouts */
msg_t elyFramGetRequest(fram_req_t ** reqp) {
  if (!elyIsQueueInitialized(&fram_queue)) {
    elyQueueObjectInit(&fram_queue, sizeof(fram_req_t), (uint8_t *)(fram_queue_storage));
  }
  
  msg_t r = elyQueueGetEmptyBufferTimeout(&fram_queue, (uint8_t **)reqp, TIME_IMMEDIATE);
  /* ENH remove this hack - should be at PointOfCall */
  (*reqp)->special = 0;
  return r;
}

void elyFramPostRequestS(fram_req_t * req) {
  /* WARNING this is an abominable hack! */
  OSAL_IRQ_PROLOGUE();
  chSysLockFromISR();
    req->callback(req->buffer);
    elyQueueFreeBufferI(&fram_queue, (uint8_t *)(req));
  chSysUnlockFromISR();
  OSAL_IRQ_EPILOGUE();
}

void elyFramPostRequest(fram_req_t * req) {
  /* WARNING this is an abominable hack! */
  OSAL_IRQ_PROLOGUE();
  chSysLockFromISR();
    req->callback(req->buffer);
    elyQueueFreeBufferI(&fram_queue, (uint8_t *)(req));
  chSysUnlockFromISR();
  OSAL_IRQ_EPILOGUE();
}

msg_t elyFramGetRequestTimeoutS(fram_req_t ** reqp, systime_t timeout) {
  if (!elyIsQueueInitialized(&fram_queue)) {
    elyQueueObjectInit(&fram_queue, sizeof(fram_req_t), (uint8_t *)(fram_queue_storage));
  }
  
  msg_t r = elyQueueGetEmptyBufferTimeoutS(&fram_queue, (uint8_t **)reqp, timeout);
  (*reqp)->special = 0;
  return r;
}

