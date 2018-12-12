
#ifndef _ELYSIUM_EVENTS_H_
#define _ELYSIUM_EVENTS_H_

#include "core.h"
#include "main.h"

extern eventmask_t PERSIST logged_events;
extern eventmask_t PERSIST reported_events;

typedef enum {
  EvtReset = 0xC0,
  EvtReload = 0xC1,
  EvtTimeChange = 0xC2,
  EvtGPOChange = 0xC3,
  EvtTelemRollover = 0xC4,
  EvtTXFreqChange = 0xC5,
  EvtRXFreqChange = 0xC6,
  EvtTXBRChange = 0xC7,
  EvtRXBRChange = 0xC8,
  EvtTXDevChange = 0xC9,
  EvtRXDevChange = 0xCA,
  EvtTXPowChange = 0xCB,
  EvtUARTBaudChange = 0xCC,
  EvtFrameRX = 0xCD,
  EvtFrameTX = 0xCE,
  EvtPktRX = 0xCF,
  EvtPktTX = 0xD0,
  EvtCmdRX = 0xD1,
  EvtReplyTX = 0xD2,
  EvtCoreMAX
} elysium_evts_t;

#include "nl_events.h"
#include "dll_events.h"

#ifdef __cplusplus
extern "C" {
#endif
  void elyEventSignal(uint8_t event);
  void elyEventSignalI(uint8_t event);
  void elyEventSubscribe(uint8_t event, uint16_t addr);
  void elyEventUnsubscribe(uint8_t event);
  void elyEventLog(uint8_t event);
  void elyEventUnlog(uint8_t event);
  void elyEventReset(void);
  THD_FUNCTION(EvtThd, arg);
#ifdef __cplusplus
}
#endif

extern THD_WORKING_AREA(waEvtThd, 128);

#endif
