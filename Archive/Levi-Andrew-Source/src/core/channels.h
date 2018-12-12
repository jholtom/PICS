
#ifndef _ELYSIUM_CHANNELS_H_
#define _ELYSIUM_CHANNELS_H_

#include "core.h"

#define chan_gpt GPTDA2

typedef enum {
  ChanRSSI = 0x40,
  ChanOutputPower = 0x41,
  ChanUplinkFrames = 0x42,
  ChanDownlinkFrames = 0x43,
  ChanUplinkPackets = 0x44,
  ChanDownlinkPackets = 0x45,
  ChanRadioRXBytes = 0x46,
  ChanRadioTXBytes = 0x47,
  ChanUARTRXBytes = 0x48,
  ChanUARTTXBytes = 0x49,
  ChanCmdReceivedCount = 0x4A,
  ChanReplySentCount = 0x4B,
  ChanMCUTemp = 0x4C,
  ChanPATemp = 0x4D,
  ChanHSTemp = 0x4E,
  ChanLastGroundContact = 0x4F,
  ChanLastSCContact = 0x50,
  ChanCoreMAX
} elysium_channels_t;

#include "nl_channels.h"
#include "dll_channels.h"

typedef struct channel_node channel_node_t;

struct channel_node {
  channel_node_t * prev;
  void * chan_data;
  channel_node_t * next;
  uint8_t size : 2;
  uint8_t chan_id : 6;
};

typedef struct interval interval_t;

struct interval {
  uint32_t interval;
  channel_node_t * chan_list;
  interval_t * next;
};

#ifdef __cplusplus
extern "C" {
#endif
  void elyChanSubscribe(uint8_t * buffer, uint8_t length, uint32_t interval);
  void elyChanUnsubscribe(uint8_t * buffer, uint8_t length);
  void elyChanLog(uint8_t * buffer, uint8_t length, uint32_t interval);
  void elyChanUnlog(uint8_t * buffer, uint8_t length);
  size_t elyChanGetValue(uint8_t * buffer, uint8_t id);
  void elyChanReset(void);
#ifdef __cplusplus
}
#endif

#endif

