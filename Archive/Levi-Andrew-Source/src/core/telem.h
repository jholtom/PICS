
#ifndef _ELYSIUM_TELEM_H_
#define _ELYSIUM_TELEM_H_

#include "fram.h"
#include "main.h"
#include "rf.h"

#define telem_gpt GPTDA3
#define TELEM_OFFSET 0x00

typedef void (*telemcallback_t)(uint8_t * buffer);

typedef enum {
  TelemConfigUpdated = 0x01,
} telem_events_t;

typedef struct {
  int32_t timestamp_start;
  int32_t timestamp_end;
  uint8_t index_start;
  uint8_t index_end;
  bool use_index;
  bool use_timestamp;
} telem_cfg_t;

#ifdef __cplusplus
extern "C" {
#endif
  void elyTelemUpdateConfigS(telem_cfg_t config);
  void elyTelemPostBufferS(uint8_t * buffer, telemcallback_t cb);
  THD_FUNCTION(TelemThd, arg);
#ifdef __cplusplus
}
#endif

extern THD_WORKING_AREA(waTelemThd, 256);

#endif

