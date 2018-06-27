
#ifndef _ELYSIUM_REGISTERS_H_
#define _ELYSIUM_REGISTERS_H_

typedef enum {
  RegTXFreqLsb = 0x00,
  RegTXFreqLmb = 0x01,
  RegTXFreqHmb = 0x02,
  RegTXFreqMsb = 0x03,
  RegRXFreqLsb = 0x04,
  RegRXFreqLmb = 0x05,
  RegRXFreqHmb = 0x06,
  RegRXFreqMsb = 0x07,
  RegTXDevLsb = 0x08,
  RegTXDevLmb = 0x09,
  RegTXDevHmb = 0x0A,
  RegTXDevMsb = 0x0B,
  RegRXDevLsb = 0x0C,
  RegRXDevLmb = 0x0D,
  RegRXDevHmb = 0x0E,
  RegRXDevMsb = 0x0F,
  RegTXSyncLsb = 0x10,
  RegTXSyncLmb = 0x11,
  RegTXSyncHmb = 0x12,
  RegTXSyncMsb = 0x13,
  RegRXSyncLsb = 0x14,
  RegRXSyncLmb = 0x15,
  RegRXSyncHmb = 0x16,
  RegRXSyncMsb = 0x17,
  RegTXBRLsb = 0x18,
  RegTXBRLmb = 0x19,
  RegTXBRHmb = 0x1A,
  RegTXBRMsb = 0x1B,
  RegRXBRLsb = 0x1C,
  RegRXBRLmb = 0x1D,
  RegRXBRHmb = 0x1E,
  RegRXBRMsb = 0x1F,
  RegFilterParams = 0x20,
  RegOutputPower = 0x21,
  RegUARTBaudLsb = 0x22,
  RegUARTBaudLmb = 0x23,
  RegUARTBaudHmb = 0x24,
  RegUARTBaudMsb = 0x25,
  RegUARTParams = 0x26,
  RegErrorReportLsb = 0x27,
  RegErrorReportMsb = 0x28,
  RegFaultResponse = 0x29,
  RegResetErrLvl = 0x2A,
  RegUARTErrLvl = 0x2B,
  RegSubOverLvl = 0x2C,
  RegNRErrLvl = 0x2D,
  RegFCSLvl = 0x2E,
  RegLengthErrLvl = 0x2F,
  RegOpErrLvl = 0x30,
  RegRegErrLvl = 0x31,
  RegSCCommLvl = 0x32,
  RegGFLvl = 0x33,
  RegSCCommTimeLsb = 0x34,
  RegSCCommTimeLmb = 0x35,
  RegSCCommTimeHmb = 0x36,
  RegSCCommTimeMsb = 0x37,
  RegSCCommBaudLsb = 0x38,
  RegSCCommBaudLmb = 0x39,
  RegSCCommBaudHmb = 0x3A,
  RegSCCommBaudMsb = 0x3B,
  RegGFTimeLsb = 0x3C,
  RegGFTimeLmb = 0x3D,
  RegGFTimeHmb = 0x3E,
  RegGFTimeMsb = 0x3F,
  RegGFBank = 0x40,
  RegBeaconTimeLsb = 0x41,
  RegBeaconTimeLmb = 0x42,
  RegBeaconTimeHmb = 0x43,
  RegBeaconTimeMsb = 0x44,
  RegOTFaultTime = 0x45,
  RegBoardTempWARN = 0x46,
  RegBoardTempERR = 0x47,
  RegHSTempWARN = 0x48,
  RegHSTempERR = 0x49,
  RegPATempWARN = 0x4A,
  RegPATempERR = 0x4B,
  RegErrRptLvl = 0x4C,
  RegErrLogLvl = 0x4D,
  RegErrRptAddrLsb = 0x4E,
  RegErrRptAddrMsb = 0x4F,
  RegSrcAddrLsb = 0x50,
  RegSrcAddrMsb = 0x51,
  RegChanDefaultAddrLsb = 0x52,
  RegChanDefaultAddrMsb = 0x53,
  RegEventDefaultAddrLsb = 0x54,
  RegEventDefaultAddrMsb = 0x55,
  RegCoreMAX
} elysium_core_regs_t;

typedef enum {
  RegBootCount = 0x60,
  RegGPOState = 0x61,
  RegCRITErrors = 0x62,
  RegERRErrors = 0x63,
  RegWARNErrors = 0x64,
  RegINFOErrors = 0x65,
  RegDEBUGErrors = 0x66,
  RegLastError = 0x67,
  RegLastEvent = 0x68,
  RegActiveBank = 0x69,
  RegTelemIndex = 0x6A,
  RegUptimeLsb = 0x6B,
  RegUptimeLmb = 0x6C,
  RegUptimeHmb = 0x6D,
  RegUptimeMsb = 0x6E,
  RegRFConfigLsb = 0x6F,
  RegRFConfigMsb = 0x70,
  RegNetworkLayer = 0x71,
  RegDataLinkLayer = 0x72,
  RegMissionTimeLsb = 0x73,
  RegMissionTimeLmb = 0x74,
  RegMissionTimeHmb = 0x75,
  RegMissionTimeMsb = 0x76,
  RegSpecialMAX
} elysium_special_regs_t;

#include "nl_registers.h"
#include "dll_registers.h"
#include "nl.h"

extern const uint8_t * const bank0p;
#ifdef __cplusplus
extern "C" {
#endif
  void elyRegSet(uint8_t bank, uint8_t * buffer, uint8_t n);
  void elyRegGet(uint8_t bank, uint8_t * buffer, uint8_t n);
  void elyRegGetBlock(uint8_t bank, uint8_t * buffer, uint8_t addr, uint8_t n);
  void elyRegSetBlock(uint8_t bank, uint8_t * buffer, uint8_t addr, uint8_t n);
#ifdef __cplusplus
}
#endif
  
#endif
