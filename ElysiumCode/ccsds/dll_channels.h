
#ifndef _ELYSIUM_DLL_CHANNELS_H_
#define _ELYSIUM_DLL_CHANNELS_H_

typedef enum {
  ChanDLLFEC = 0x70,
  ChanDLLFECF = 0x71,
  ChanDLLSeqNo = 0x72,
  ChanDLLMissedFrames = 0x73,
  ChanDLLDoubleFrames = 0x74,
  ChanDLLAFrames = 0x75,
  ChanDLLBFrames = 0x76,
  ChanDLLMAX
} elysium_dll_channels_t;

extern uint8_t fec;
extern uint8_t fecf;
extern uint8_t seq_no;
extern uint8_t missed_frames;
extern uint8_t double_frames;
extern uint8_t a_frames;
extern uint8_t b_frames;

#define DLL_CHAN_INIT \
  LOG_CHAN_NODE(ChanCoreMAX-0x40+ChanNLMAX-0x60+0, fec),\
  LOG_CHAN_NODE(ChanCoreMAX-0x40+ChanNLMAX-0x60+1, fecf),\
  LOG_CHAN_NODE(ChanCoreMAX-0x40+ChanNLMAX-0x60+2, seq_no),\
  LOG_CHAN_NODE(ChanCoreMAX-0x40+ChanNLMAX-0x60+3, missed_frames),\
  LOG_CHAN_NODE(ChanCoreMAX-0x40+ChanNLMAX-0x60+4, double_frames),\
  LOG_CHAN_NODE(ChanCoreMAX-0x40+ChanNLMAX-0x60+5, a_frames),\
  LOG_CHAN_NODE(ChanCoreMAX-0x40+ChanNLMAX-0x60+6, b_frames)

#endif
