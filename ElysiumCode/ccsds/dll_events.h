
#ifndef _ELYSIUM_DLL_EVENTS_H_
#define _ELYSIUM_DLL_EVENTS_H_

typedef enum {
  EvtDLLSeqNoChanged = 0xF0,
  EvtDLLUnlock = 0xF1,
  EvtDLLBufferRelease = 0xF2,
  EvtDLLMAX
} elysium_dll_evts_t;

#endif
