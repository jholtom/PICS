
#include "crc_sdlp.h"

#if ELYSIUM_CRC_DMA
static msp430x_dma_req_t dmareq = {
  NULL,
  &CRCDIRB_L,
  0,
  MSP430X_DMA_SRCINCR,
  MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE,
  MSP430X_DMA_BLOCK,
  DMA_TRIGGER_MNEM(DMAREQ),
  {
    NULL,
    NULL,
  }
};
#endif

/** @brief   Calculates the CRC value for a message according to SDLP rules,
 * then stores the result in the last two bytes of the message.
 *
 * @param message     pointer to the message buffer
 * @param[in] n       size of the message including the CRC field
 */
void crcGenSDLP(uint8_t * message, size_t n) {
  /* SDLP CRC is MSB first in and out (CRCDIRB+CRCINIRES), with an initial value
   * of 0xFFFF and no inversion */

  chDbgAssert(n >= 2, "message too small to CRC");

  /* Initialize the register to 0xFFFF */
  CRCINIRES = 0xFFFF;

#if ELYSIUM_CRC_DMA
  /* Use DMA engine to write byte-wise to CRCDI register */
  dmareq.source_addr = (void*)message;
  dmareq.size = (n-2);
  chSysLock();
  if (dmaRequestS(NULL, &dmareq, TIME_IMMEDIATE) < 0) {
    /* Fall back to byte-wise software copying */
#endif
    for (size_t i = 0; i < n-2; i++) {
      CRCDIRB_L = message[i];
    }
#if ELYSIUM_CRC_DMA
  }
  chSysUnlock();
#endif


  /* Write result to final 2 bytes */
  /* MSB comes first because of how things are transmitted */
  *(message+n-2) = (CRCINIRES_H);
  *(message+n-1) = (CRCINIRES_L);
}

/** @brief    Checks the CRC value for a message according to SDLP rules
 *
 * @param[in] message   pointer to the message buffer
 * @param[in] n         size of the message including the CRC field
 * @return  @p true if the signature is valid, @p false if not
 */
bool crcCheckSDLP(uint8_t * message, size_t n) {

  /* Initialize the register to 0xFFFF */
  CRCINIRES = 0xFFFF;

#if ELYSIUM_CRC_DMA
  /* Use DMA engine to write byte-wise to CRCDI register */
  dmareq.source_addr = (void*)message;
  dmareq.size = n-2;
  chSysLock();
  if (dmaRequestS(NULL, &dmareq, TIME_IMMEDIATE) < 0) {
    /* Fall back to byte-wise software copying */
#endif
    for (size_t i = 0; i < n-2; i++) {
      CRCDIRB_L = message[i];
    }
#if ELYSIUM_CRC_DMA
  }
  chSysUnlock();
#endif


  /* Final result should equal the saved checksum */
  return ((CRCINIRES_H == message[n-2]) && (CRCINIRES_L == message[n-1])) ;
}
