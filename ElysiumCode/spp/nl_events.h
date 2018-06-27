
#ifndef _ELYSIUM_NL_EVENTS_H_
#define _ELYSIUM_NL_EVENTS_H_

typedef enum {
  EvtNLPacketReceived = 0xE0,
  EvtNLPacketSent = 0xE1,
  EvtNLPacketRelayed = 0xE2,
  EvtNLMAX
} elysium_nl_evts_t;

#endif
