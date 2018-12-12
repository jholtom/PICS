
#ifndef _ELYSIUM_MAIN_H_
#define _ELYSIUM_MAIN_H_

#include "core.h"
#include "cmds.h"
#include "nl.h"
#include "uart.h"
#include "rf.h"

/* TODO this should be in config */
#define elyFWMaxSlots 4
#define elyFWBufferMaxSize (255+4+6)
#define elyFWTotalBuffer (elyFWBufferMaxSize * elyFWMaxSlots)

#ifdef __cplusplus
extern "C" {
#endif
  msg_t elyMainMBPost(uint8_t * buffer, systime_t timeout);
  msg_t elyMainMBPostS(uint8_t * buffer, systime_t timeout);
  msg_t elyMainMBPostI(uint8_t * buffer);
  uint8_t * elyFWGetBuffer(void);
  uint8_t * elyFWGetBufferI(void);
  uint8_t * elyFWGetBufferTimeoutS(systime_t timeout);
  void elyFWFreeBuffer(uint8_t * buffer);
  void elyFWFreeBufferI(uint8_t * buffer);
#ifdef __cplusplus
}
#endif

#endif
