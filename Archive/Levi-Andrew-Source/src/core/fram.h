
#ifndef _ELYSIUM_FRAM_H_
#define _ELYSIUM_FRAM_H_

#include "core.h"
#include "queues.h"
#include "hal.h"
#include "chbsem.h"

typedef void (*framcallback_t)(uint8_t * buffer);

typedef struct {
  uint32_t address;
  uint32_t special : 1; /* special function flag */
  uint32_t read : 1; /* read or write */
  uint32_t size : 17; /* number of bytes */
  uint8_t * buffer;
  framcallback_t callback;
  uint8_t device_id;
} fram_req_t;

#define FRAM_QUEUE_LEN 4
#define FRAM_REQ_STORAGE (FRAM_QUEUE_LEN * sizeof(fram_req_t))
#define FRAM_FW_BASE 0x10400UL
#define FRAM_TELEM_BASE 0x00000000UL
#define FRAM_REG_BASE 0x00010000UL
#define FRAM_FW_SIZE (0x20000UL - FRAM_FW_BASE)

#ifdef __cplusplus
extern "C" {
#endif
  msg_t elyFramGetRequestI(fram_req_t ** reqp);
  void elyFramPostRequestS(fram_req_t * req);
  void elyFramPostRequestI(fram_req_t * req);
  msg_t elyFramGetRequestTimeoutS(fram_req_t ** reqp, systime_t timeout);
  void elyFramInit(void);
#ifdef __cplusplus
}
#endif

#endif
