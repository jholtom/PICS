
#include "telem.h"

void elyTelemUpdateConfigS(telem_cfg_t config) {
  (void)(config);
}

void elyTelemPostBufferS(uint8_t * buffer, telemcallback_t cb) {
  /* WARNING this is an abominable hack! */
  OSAL_IRQ_PROLOGUE();
  chSysLockFromISR();
  cb(buffer);
  chSysUnlockFromISR();
  OSAL_IRQ_EPILOGUE();
}

