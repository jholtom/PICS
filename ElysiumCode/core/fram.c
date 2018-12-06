
#include "fram.h"

static const uint8_t slave_id = 0x50;
static const uint8_t device_select = 0x00;

static PERSIST uint8_t fram_storage[FRAM_REQ_STORAGE];

void * fram_mpool_alloc(size_t size, unsigned align) {
  (void)(align);
  /* Provides memory blocks for the pool */
  static size_t PERSIST curr_index = 0;

  if (curr_index + size > FRAM_REQ_STORAGE) {
    return NULL;
  }

  void * result = fram_storage + curr_index;

  curr_index += size;
  return result;
}

static PERSIST fram_req_t * fram_mbox_buffer[FRAM_QUEUE_LEN];
static PERSIST uint8_t fram_write_idx;
static PERSIST uint8_t fram_read_idx;
static PERSIST MEMORYPOOL_DECL(fram_mpool, sizeof(fram_req_t), fram_mpool_alloc);
static PERSIST SEMAPHORE_DECL(fram_pool_sem, FRAM_QUEUE_LEN);

#define QUEUE_FULL() ((fram_write_idx - fram_read_idx) == FRAM_QUEUE_LEN)
#define QUEUE_MASK(idx) (idx & (FRAM_QUEUE_LEN - 1))
#define QUEUE_EMPTY() (fram_write_idx == fram_read_idx)

static framcallback_t current_callback;
static fram_req_t * active_req;

void fram_handle_request(fram_req_t * req);

void end_cb(I2CDriver * i2cp, uint8_t * buffer, uint16_t n) {
  (void)(n);
  /* End the transfer */
  chSysLockFromISR();
  i2cMSP430XEndTransferI(i2cp);
  /* Remove request from queue */
  fram_read_idx++;
  /* Free the request buffer */
  chPoolFreeI(&fram_mpool, active_req);
  chSemSignalI(&fram_pool_sem);
  chSysUnlockFromISR();
  /* Call the FRAM callback */
  if (active_req->callback) {
    active_req->callback(buffer);
  }
  /* Get next request from queue if relevant */
  if (!QUEUE_EMPTY()) {
    /* Process next request */
    fram_handle_request(fram_mbox_buffer[QUEUE_MASK(fram_read_idx)]);
  }
}

void addr_cb(I2CDriver * i2cp, uint8_t * buffer, uint16_t n) {
  (void)(n);
  (void)(buffer);
  fram_req_t * req = active_req;

  chSysLockFromISR();
  if (req->read) {
    if (req->special) {
      i2cMSP430XStartReceiveToRegI(i2cp, req->device_id,
          req->size, req->buffer, end_cb);
    }
    else {
      i2cMSP430XStartReceiveI(i2cp, req->device_id, req->size,
          req->buffer, end_cb);
    }
  }
  else {
    if (req->special) {
      i2cMSP430XContinueTransmitMemsetI(i2cp, req->device_id,
          req->size, req->buffer, end_cb);
    }
    else {
      i2cMSP430XContinueTransmitI(i2cp, req->device_id, req->size,
          req->buffer, end_cb);
    }
  }
  current_callback = req->callback;
  chSysUnlockFromISR();
}

/* called from i-class */
void fram_handle_request(fram_req_t * req) {
  chDbgAssert(req->address < 0x00020000, "Invalid FRAM address");

  active_req = req;
  /* Build the 7-bit device address */
  req->device_id = (slave_id | device_select |
    (req->address >> 16));

  /* Issue a write to set the address */
  i2cMSP430XStartTransmitMSBI(&I2CDB0, req->device_id, 2,
      (uint8_t *)(&(req->address)),
      addr_cb);
}

void elyFramPostRequestI(fram_req_t * req) {
  chDbgCheckClassI();

  /* We should never hit this if we've managed to get a request in the first place */
  chDbgAssert(!QUEUE_FULL(), "internal buffer overflows should be impossible");

  fram_mbox_buffer[QUEUE_MASK(fram_write_idx++)] = req;
}

void elyFramPostRequestS(fram_req_t * req) {
  chDbgCheckClassS();

  if (QUEUE_EMPTY()) {
    elyFramPostRequestI(req);
    fram_handle_request(req);
  }
  else {
    elyFramPostRequestI(req);
  }
}

msg_t elyFramGetRequestI(fram_req_t ** reqp) {
  chDbgCheckClassI();

  cnt_t count = chSemGetCounterI(&fram_pool_sem);
  if (count <= (cnt_t)0) {
    return MSG_TIMEOUT;
  }

  /* semaphore counter known to be positive */
  chSemFastWaitI(&fram_pool_sem);

  (*reqp) = (fram_req_t *)(chPoolAllocI(&fram_mpool));
  chDbgAssert( (*reqp) != NULL, "internal pool overflow should be impossible");
  (*reqp)->special = 0;

  return MSG_OK;
}

msg_t elyFramGetRequestTimeoutS(fram_req_t ** reqp, systime_t timeout) {
  chDbgCheckClassS();

  msg_t r = chSemWaitTimeoutS(&fram_pool_sem, timeout);
  if (MSG_OK != r) {
    return r;
  }

  (*reqp) = (fram_req_t *)(chPoolAllocI(&fram_mpool));
  chDbgAssert( (*reqp) != NULL, "internal pool overflow should be impossible");
  (*reqp)->special = 0;
  return r;
}

static I2CConfig cfg = {
  400000, /* max bitrate supported by USCI */
};


void elyFramInit() {
  chDbgAssert((FRAM_QUEUE_LEN & (FRAM_QUEUE_LEN -1)) == 0,
      "queue length must be power of 2");
  i2cStart(&I2CDB0, &cfg);
}

