
#ifndef _ELYSIUM_CMDS_H_
#define _ELYSIUM_CMDS_H_

#include "msp430.h"
#include "core.h"
#include "crc_x25.h"
#include "events.h"
#include "cfg.h"
#include "rf.h"
#include "uart.h"
#include "channels.h"
#include "fram.h"
#include "telem.h"

typedef enum {
  CmdReset = 0x00,
  CmdGetGPO = 0x01,
  CmdSetGPO = 0x02,
  CmdGetActiveBank = 0x03,
  CmdGetRegs = 0x04,
  CmdSetRegs = 0x05,
  CmdGetBlock = 0x06,
  CmdSetBlock = 0x07,
  CmdGetTXFreq = 0x08,
  CmdSetTXFreq = 0x09,
  CmdGetRXFreq = 0x0A,
  CmdSetRXFreq = 0x0B,
  CmdGetTXRate = 0x0C,
  CmdSetTXRate = 0x0D,
  CmdGetRXRate = 0x0E,
  CmdSetRXRate = 0x0F,
  CmdGetTXDev = 0x10,
  CmdSetTXDev = 0x11,
  CmdGetRXDev = 0x12,
  CmdSetRXDev = 0x13,
  CmdGetTXPow = 0x14,
  CmdSetTXPow = 0x15,
  CmdGetBaud = 0x16,
  CmdSetBaud = 0x17,
  CmdReloadConfig = 0x18,
  CmdChannelSub = 0x19,
  CmdChannelUnsub = 0x1A,
  CmdLogChan = 0x1B,
  CmdUnlogChan = 0x1C,
  CmdGetChan = 0x1D,
  CmdResetChan = 0x1E,
  CmdEventSub = 0x1F,
  CmdEventUnsub = 0x20,
  CmdLogEvent = 0x21,
  CmdUnlogEvent = 0x22,
  CmdResetEvent = 0x23,
  CmdSetTime = 0x24,
  CmdGetTime = 0x25,
  CmdGetErr = 0x26,
  CmdSetErr = 0x27,
  CmdGetLog = 0x28,
  CmdSetLog = 0x29,
  CmdUploadFW = 0x2A,
  CmdVerifyFW = 0x2B,
  CmdCancelFW = 0x2C,
  CmdInstallFW = 0x2D,
  CmdStoreTelem = 0x2E,
  CmdGetTelem = 0x2F,
  CMD_MAX
} elysium_opcodes_t;

typedef struct {
  uint8_t crc : 1;
  uint8_t reply : 1;
  uint8_t opcode : 6;
  uint8_t length;
  uint16_t reply_addr;
} elysium_cmd_hdr_t;

typedef void (*cmdhandler_t)(uint8_t * buffer, elysium_cmd_hdr_t hdr);

#ifdef __cplusplus
extern "C" {
#endif
  elysium_cmd_hdr_t elyCmdParse(uint8_t * buffer);
  bool elyCmdValidate(elysium_cmd_hdr_t hdr, uint8_t * buff);
  void elyCmdDispatch(elysium_cmd_hdr_t hdr, uint8_t * buff);
  void elyCmdSendReply(uint8_t * buff, uint16_t dest_addr);
  void elyCmdSendReplyI(uint8_t * buff, uint16_t dest_addr);
  THD_FUNCTION(CmdThd, arg);
#ifdef __cplusplus
}
#endif

extern THD_WORKING_AREA(waCmdThd, 256);

#endif
