
#include "crc_x25.h"

#if ELYSIUM_CRC_DMA
static msp430x_dma_req_t dmareq = {
  NULL,
  &CRCDI,
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

enum crc_state_t {
  CRC_STOPPED,
  CRC_RUNNING
};

static enum crc_state_t crc_state = CRC_STOPPED;

/** @brief   Calculates the CRC value for a message according to X.25 rules,
 * then stores the result in the last two bytes of the message.
 *
 * @param message     pointer to the message buffer 
 * @param[in] n       size of the message including the CRC field
 */
void crcGenX25(uint8_t * message, size_t n) {
  chDbgAssert(crc_state == CRC_STOPPED, "invalid call order");
  /* X.25 CRC is LSB first in and out (CRCDI+CRCRESR), with an initial value
   * of 0xFFFF and a final inversion */
   
  chDbgAssert(n >= 2, "message too small to CRC");
   
#if defined(MSP430X_MCUCONF)
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
      CRCDI_L = message[i];
    }
#if ELYSIUM_CRC_DMA
  }
  chSysUnlock();
#endif
  
   
  /* Write result to final 2 bytes - MSB comes first because network byte order */
  *(message+n-2) = ~(CRCRESR_H);
  *(message+n-1) = ~(CRCRESR_L);
#elif defined(POSIX_MCUCONF) && defined(__FUZZ__)
  // XXX(Reed): This isn't actually computing a CRC, but it should be fine for suzzing
  *(message+n-2) = 0xDE;
  *(message+n-1) = 0xAD;
#else
#error "unssuported MCU"
#endif
}

/** @brief    Checks the CRC value for a message according to X.25 rules
 * 
 * @param[in] message   pointer to the message buffer
 * @param[in] n         size of the message including the CRC field
 * @return  @p true if the signature is valid, @p false if not
 */
bool crcCheckX25(uint8_t * message, size_t n) {
  chDbgAssert(crc_state == CRC_STOPPED, "invalid call order");

#if defined(MSP430X_MCUCONF)
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
      CRCDI_L = message[i];
    }
#if ELYSIUM_CRC_DMA
  }
  chSysUnlock();
#endif


  /* Final result should be equal to calculated*/
  return (~CRCRESR == ((uint16_t)(message[n-2] << 8) | message[n-1]));
#elif defined(POSIX_MCUCONF) && defined(__FUZZ__)
  // XXX(Reed): We don't actually validate the CRC when fuzzing, since the
  // ground could send arbitrary garbage (but with a valid CRC)
  (void) message;
  (void) n;
  return true;
#else
#error "unsupported MCU"
#endif
}

#if defined(POSIX_MCUCONF)
// XXX(Reed): On POSIX systems, we just dump any CRC writes
static uint32_t crc_scratch;
#endif

/** @brief    Sets up to generate the CRC of a large message according to X.25 rules
 * 
 * @return    The address of the CRC module input
 */
volatile uint8_t * crcStart() {
  chDbgAssert(crc_state == CRC_STOPPED, "invalid call order");
#if defined(MSP430X_MCUCONF)
  /* Initialize the register to 0xFFFF */
  CRCINIRES = 0xFFFF;

  /* Change state */
  crc_state = CRC_RUNNING;

  /* Return the address of the input register */
  return &CRCDI_L;
#elif defined(POSIX_MCUCONF)
  return (uint8_t*) crc_scratch;
#else
#error "Unsupported MCU"
#endif
}

/** @brief    Returns the CRC of a large message according to X.25 rules
 * 
 * @return    The CRC of the large message
 */
uint16_t crcStop() {
  chDbgAssert(crc_state == CRC_RUNNING, "invalid call order");
  /* Change state */
  crc_state = CRC_STOPPED;

  /* Return the result */
#if defined(MSP430X_MCUCONF)
  return ~CRCRESR;
#elif defined(POSIX_MCUCONF)
  return 0xDEAD;
#else
#error "Unsupported MCU"
#endif
}
