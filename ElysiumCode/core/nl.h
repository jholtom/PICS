
#ifndef _ELYSIUM_NL_H_
#define _ELYSIUM_NL_H_

#include "core.h"

typedef enum {
  ELY_DEST_UART,
  ELY_DEST_RF,
  ELY_DEST_FW
} elysium_destinations_t;

#include "nl_constants.h"
#include "nl_errors.h"
#include "nl_registers.h"

extern size_t elyNLMaxLen;

#ifdef __cplusplus
extern "C" {
#endif
  bool elyNLValidate(const uint8_t * buffer);
  elysium_destinations_t elyNLGetDest(uint8_t * buffer, uint16_t dest_addr);
  elysium_destinations_t elyNLSetDest(uint8_t * buffer);
  size_t elyNLGetLength(const uint8_t * buffer);
  size_t elyNLGetPayloadLength(const uint8_t * buffer);
  uint8_t * elyNLExtract(uint8_t * buffer);
  uint8_t * elyNLPack(uint8_t * buffer);
  void elyNLChangeMaxLength(size_t len);
  uint8_t * elyNLGetBuffer(void);
  void elyNLFreeBufferChecked(uint8_t * buffer);
  void elyNLFreeBufferCheckedI(uint8_t * buffer);
  void elyNLFreeBuffer(uint8_t * buffer);
  uint8_t * elyNLGetBufferI(void);
  void elyNLFreeBufferI(uint8_t * buffer);
  void elyNLRouteUART(uint8_t * buffer);
  void elyNLRouteRF(uint8_t * buffer);
  void elyNLRouteRFI(uint8_t * buffer);
  uint8_t * elyNLToFW(uint8_t * buffer);
  uint8_t * elyNLFromFW(uint8_t * buffer);
  void elyNLSetHeader(uint8_t * buffer, uint16_t length, uint16_t dest_addr);
  void elyNLInit(void);
#ifdef __cplusplus
}
#endif
  

#endif
