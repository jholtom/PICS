
#ifndef _ELYSIUM_SDLP_H_
#define _ELYSIUM_SDLP_H_

#include "core.h"
#include "sx1278.h"
#include "nl.h"
#include "registers.h"
#include "errors.h"
#include "rf_admin.h"
#include "crc_sdlp.h"
#include "main.h"
#include "rf_dll.h"

#define SDLP_TM_MAX_TF_LEN 2048
#define SDLP_TC_MAX_TF_LEN 1024
#define SDLP_TM_PH_LEN 6
#define SDLP_TC_PH_LEN 5
#define SDLP_IDLE_PACKET_TIMEOUT MS2ST(100)

/* TODO move this to SPP */
#define SPP_IDLE_DATA 0xAA

typedef enum {
  DLL_STATE_HDR,
  DLL_STATE_PKT
} ely_dll_state_t;

typedef enum {
  FB_STATE_UNINIT,
  FB_STATE_HEADER,
  FB_STATE_PAYLOAD,
  FB_STATE_IDLE
} ely_framebuilder_state_t;

#endif
