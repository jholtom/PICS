/* This file defines the interface to the MSP's CRC hardware, which uses the
 * CRC-CCIT-BR generating polynomial, 0x1021, when used for the SDLP Data Link
 * Layer's Frame Error Control Field */

#ifndef _ELYSIUM_CRC_SDLP_H_
#define _ELYSIUM_CRC_SDLP_H_

/* 
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
  void crcGenSDLP(uint8_t * message, size_t n);
  bool crcCheckSDLP(uint8_t * message, size_t n);
#ifdef __cplusplus
}
#endif

#endif
