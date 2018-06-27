
#ifndef _ELYSIUM_RF_H_
#define _ELYSIUM_RF_H_

#include "core.h"
#include "registers.h"
#include "sx1212.h"
#include "sx1278.h"
#include "rf_dll.h"
#include "main.h"

extern sx1278_packet_config_t DLLTxPktCfg;
extern sx1212_packet_config_t DLLRxPktCfg;

#if ELY_REVISION == B
#define rf_spi SPIDA0
#else
#define rf_spi SPIDA1
#endif

typedef enum {
  RFTxFreqUpdated = 0x01,
  RFRxFreqUpdated = 0x02,
  RFTxBRUpdated = 0x04,
  RFRxBRUpdated = 0x08,
  RFTxDevUpdated = 0x10,
  RFRxDevUpdated = 0x20,
  RFTxSyncUpdated = 0x40,
  RFRxSyncUpdated = 0x80,
  RFTxPowerUpdated = 0x100,
  RFFilterParamsUpdated = 0x200,
  RFPktAvailable = 0x400, 
  RFTxFrameReady = 0x800,
  RFRxFifoThresh = 0x1000,
  RFSpiAvailable = 0x2000,
  RFRxIdle = 0x4000,
  RFTxFifoLevel = 0x8000,
  RFTxIdle = 0x10000,
} rf_events_t;

#ifdef __cplusplus
extern "C" {
#endif
  void elyRFCfgMarkDirty(rf_events_t event);
  void elyRFCfgMarkDirtyI(rf_events_t event);
  msg_t elyRFPost(uint8_t * buffer, systime_t timeout);
  msg_t elyRFPostI(uint8_t * buffer);
  THD_FUNCTION(RFThd, arg);
#ifdef __cplusplus
}
#endif
  
extern THD_WORKING_AREA(waRFThd, 256);

#endif
