/* This file defines the interface to the MSP's CRC hardware, which uses the
 * CRC-CCIT-BR generating polynomial, 0x1021 */

#ifndef _ELYSIUM_CRC_X25_H_
#define _ELYSIUM_CRC_X25_H_

/* 
 * X.25 - POLY 0x1021, INIT 0xFFFF, REFIN=TRUE, REFOUT=TRUE, XOROUT=0xFFFF
 * SDLP - POLY 0x1021, INIT 0xFFFF, REFIN=FALSE, REFOUT=FALSE, XOROUT=0x0000
 */

#include "hal.h"
#include "hal_dma_lld.h"

#if defined(HAL_USE_DMA) 
  #if !defined(ELYSIUM_CRC_DMA)
    #define ELYSIUM_CRC_DMA FALSE
  #endif
#else
  #define ELYSIUM_CRC_DMA FALSE
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void crcGenX25(uint8_t * message, size_t n);
  bool crcCheckX25(uint8_t * message, size_t n);
  volatile uint8_t * crcStart(void);
  uint16_t crcStop(void);
#ifdef __cplusplus
}
#endif

#endif
