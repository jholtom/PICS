
#include "main.h"

static uint8_t PERSIST fw_storage[elyFWTotalBuffer];

void * fw_allocator(size_t size, unsigned align) {
  (void)(align);
  /* Provides memory blocks for the pool */
  static size_t PERSIST curr_index = 0;
  
  if (curr_index + size > elyFWTotalBuffer) {
    return NULL;
  }

  void * result = fw_storage + curr_index;
  
  curr_index += size;
  return result;
}

static PERSIST MEMORYPOOL_DECL(fw_mpool, elyFWBufferMaxSize, fw_allocator);
static SEMAPHORE_DECL(fw_sem, (cnt_t)(elyFWMaxSlots));

uint8_t * elyFWGetBufferI() {
  uint8_t * result =  chPoolAllocI(&fw_mpool);
  
  if (result == NULL) {
    return NULL;
  }
  
  /* semaphore must be >0 if we got a non-null result */
  chSemFastWaitI(&fw_sem);
  return elyNLToFW(result);
}

uint8_t * elyFWGetBuffer() {
  chSysLock();
  uint8_t * result = elyFWGetBufferI();
  chSysUnlock();
  
  return result;
}

uint8_t * elyFWGetBufferTimeoutS(systime_t timeout) {
  msg_t msg;
  
  msg = chSemWaitTimeoutS(&fw_sem, timeout);
  if (MSG_OK != msg) {
    return NULL;
  }
  
  uint8_t * result = chPoolAllocI(&fw_mpool);
  chDbgAssert(NULL != result, "Guarded pool failure");
  return elyNLToFW(result);
}

void elyFWFreeBufferI(uint8_t * buffer) {
  chPoolFreeI(&fw_mpool, buffer);
  chSemSignalI(&fw_sem);
}

void elyFWFreeBuffer(uint8_t * buffer) {
  chSysLock();
  elyFWFreeBufferI(buffer);
  chSchRescheduleS();
  chSysUnlock();
}

