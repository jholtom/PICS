
#ifndef _ELYSIUM_NL_CHANNELS_H_
#define _ELYSIUM_NL_CHANNELS_H_

typedef enum {
  ChanNLPacketsReceived = 0x60,
  ChanNLPacketsSent = 0x61,
  ChanNLPacketsRelayed = 0x62,
  ChanNLMAX
} elysium_nl_channels_t;

extern uint8_t packets_received;
extern uint16_t packets_sent;
extern uint8_t packets_relayed;

#define NL_CHAN_INIT \
  LOG_CHAN_NODE(ChanCoreMAX-0x40+0, packets_received),\
  LOG_CHAN_NODE(ChanCoreMAX-0x40+1, packets_sent),\
  LOG_CHAN_NODE(ChanCoreMAX-0x40+2, packets_relayed)


#endif
