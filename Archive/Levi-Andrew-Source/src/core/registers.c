
#include "core.h"
#include "cfg.h"
#include "registers.h"
#include "errors.h"
#include "fram.h"

#include "chbsem.h"

#include <string.h>

static uint8_t __attribute__((section(".persistent"))) bank0[256] = {
  TX_BAND_MID_LSB, /* RegTXFreqLsb */
  TX_BAND_MID_LMB, /* RegTXFreqLmb */
  TX_BAND_MID_HMB, /* RegTXFreqHmb */
  TX_BAND_MID_MSB, /* RegTXFreqMsb */
  RX_BAND_MID_LSB, /* RegRXFreqLsb */
  RX_BAND_MID_LMB, /* RegRXFreqLmb */
  RX_BAND_MID_HMB, /* RegRXFreqHmb */
  RX_BAND_MID_MSB, /* RegRXFreqMsb */
  TX_DEV_LSB, /* RegTXDevLsb */
  TX_DEV_LMB, /* RegTXDevLmb */
  TX_DEV_HMB, /* RegTXDevHmb */
  TX_DEV_MSB, /* RegTXDevMsb */ /* 5 kHz */
  RX_DEV_LSB, /* RegRXDevLsb */
  RX_DEV_LMB, /* RegRXDevLmb */
  RX_DEV_HMB, /* RegRXDevHmb */
  RX_DEV_MSB, /* RegRXDevMsb */ /* 50 kHz */
  TX_SYNC_LSB, /* RegTXSyncLsb */
  TX_SYNC_LMB, /* RegTXSyncLmb */
  TX_SYNC_HMB, /* RegTXSyncHmb */
  TX_SYNC_MSB, /* RegTXSyncMsb */
  RX_SYNC_LSB, /* RegRXSyncLsb */
  RX_SYNC_LMB, /* RegRXSyncLmb */
  RX_SYNC_HMB, /* RegRXSyncHmb */
  RX_SYNC_MSB, /* RegRXSyncMsb */
  TX_BR_LSB, /* RegTXBRLsb */
  TX_BR_LMB, /* RegTXBRLmb */
  TX_BR_HMB, /* RegTXBRHmb */
  TX_BR_MSB, /* RegTXBRMsb */ /* 4.8 kbps */
  RX_BR_LSB, /* RegRXBRLsb */
  RX_BR_LMB, /* RegRXBRLmb */
  RX_BR_HMB, /* RegRXBRHmb */
  RX_BR_MSB, /* RegRXBRMsb */ /* 25 kbps */
  FILTER_DEFAULT, /* RegFilterParams */ /* No Gaussian, 100 kHz */
  TX_POW_MIN, /* RegOutputPower */
  0x00, /* RegUARTBaudLsb */
  0xC2, /* RegUARTBaudLmb */
  0x01, /* RegUARTBaudHmb */
  0x00, /* RegUARTBaudMsb */ /* 115200 baud */
  0xF0, /* RegUARTParams */ /* 8N1 */
  0xFF, /* RegErrorReportLsb */
  0xFF, /* RegErrorReportMsb */ /* All reports enabled */
  0xFF, /* RegFaultResponse */ /* All fault responses enabled */
  0x08, /* RegResetErrLvl */ /* ERROR */
  0x04, /* RegUARTErrLvl */ /* WARNING */
  0x02, /* RegSubOverLvl */ /* INFO */
  0x04, /* RegNRErrLvl */ /* WARNING */
  0x04, /* RegFCSLvl */ /* WARNING */
  0x04, /* RegLengthErrLvl */ /* WARNING */
  0x04, /* RegOpErrLvl */ /* WARNING */
  0x04, /* RegRegErrLvl */ /* WARNING */
  0x08, /* RegSCCommLvl */ /* ERROR */
  0x08, /* RegGFLvl */ /* ERROR */
  0x00, /* RegSCCommTimeLsb */
  0xA3, /* RegSCCommTimeLmb */
  0x02, /* RegSCCommTimeHmb */
  0x00, /* RegSCCommTimeMsb */ /* 48 hours */
  0x80, /* RegSCCommBaudLsb */
  0x25, /* RegSCCommBaudLmb */
  0x00, /* RegSCCommBaudHmb */
  0x00, /* RegSCCommBaudMsb */ /* 9600 baud */
  0x00, /* RegGFTimeLsb */
  0xA3, /* RegGFTimeLmb */
  0x02, /* RegGFTimeHmb */
  0x00, /* RegGFTimeMsb */ /* 48 hours */
  0x04, /* RegGFBank */ /* Bank 4 */
  0x2C, /* RegBeaconTimeLsb */
  0x01, /* RegBeaconTimeLmb */
  0x00, /* RegBeaconTimeHmb */
  0x00, /* RegBeaconTimeMsb */ /* 5 minutes */
  0x1E, /* RegOTFaultTime */ /* 30 seconds */
  0x32, /* RegBoardTempWARN */ /* 50 degrees C */
  0x3C, /* RegBoardTempERR */ /* 60 degrees C */
  0x32, /* RegHSTempWARN */ /* 50 degrees C */
  0x46, /* RegHSTempERR */ /* 70 degrees C */
  0x3C, /* RegPATempWARN */ /* 60 degrees C */
  0x4B, /* RegPATempERR */ /* 75 degrees C */
  0x1C, /* RegErrRptLvl */ /* CRITICAL | ERROR | WARNING */
  0x18, /* RegErrLogLvl */ /* CRITICAL | ERROR */
  0xFF, /* RegErrRptAddrLsb */
  0xFF, /* RegErrRptAddrMsb */ /* Broadcast */
  0x01, /* RegSrcAddrLsb */
  0x00, /* RegSrcAddrMsb */ /* Invalid */ /* TODO give this a proper default */
  0xFF, /* RegChanDefaultAddrLsb */
  0xFF, /* RegChanDefaultAddrMsb */ /* Broadcast */
  0xFF, /* RegEventDefaultAddrLsb */
  0xFF, /* RegEventDefaultAddrMsb */ /* Broadcast */
  0xFF, /* 0x56 */
  0xFF, /* 0x57 */
  0xFF, /* 0x58 */
  0xFF, /* 0x59 */
  0xFF, /* 0x5A */
  0xFF, /* 0x5B */
  0xFF, /* 0x5C */
  0xFF, /* 0x5D */
  0xFF, /* 0x5E */
  0xFF, /* 0x5F */
  0x00, /* RegBootCount */
  0x00, /* RegGPOState */
  0x00, /* RegCRITErrors */
  0x00, /* RegERRErrors */
  0x00, /* RegWARNErrors */
  0x00, /* RegINFOErrors */
  0x00, /* RegDEBUGErrors */
  0xFF, /* RegLastError */ /* Invalid error code */
  0xFF, /* RegLastEvent */ /* Invalid event code */
  0x01, /* RegActiveBank */
  0x00, /* RegTelemIndex */
  0x00, /* RegUptimeLsb */
  0x00, /* RegUptimeLmb */
  0x00, /* RegUptimeHmb */
  0x00, /* RegUptimeMsb */
  RF_PARAMS_LSB, /* RegRFConfigLsb */
  RF_PARAMS_MSB, /* RegRFConfigMsb */
  NETWORK_LAYER_ID, /* RegNetworkLayer */
  DATALINK_LAYER_ID, /* RegDataLinkLayer */
  0xFF, /* 0x73 */
  0xFF, /* 0x74 */
  0xFF, /* 0x75 */
  0xFF, /* 0x76 */
  0xFF, /* 0x77 */
  0xFF, /* 0x78 */
  0xFF, /* 0x79 */
  0xFF, /* 0x7A */
  0xFF, /* 0x7B */
  0xFF, /* 0x7C */
  0xFF, /* 0x7D */
  0xFF, /* 0x7E */
  0xFF, /* 0x7F */
  NL_REG_DEFAULTS,
  DLL_REG_DEFAULTS
};

const uint8_t * const bank0p = bank0;
uint8_t * const bank0w = bank0;

static BSEMAPHORE_DECL(regs_sem, 1); /* 1 is taken which is cnt == 0 */
static uint8_t regs_in_progress = 0;

static uint8_t clamp(uint8_t value, uint8_t min, uint8_t max) {
  if (value < min) {
    value = min;
    elyErrorSignal(ErrRegClip);
  }
  else if (value > max) {
    value = max;
    elyErrorSignal(ErrRegClip);
  }
  return value;
}

static uint8_t clamp_err(uint8_t value) {
  if ((value & 0x10) && value != 0x10) {
    elyErrorSignal(ErrRegClip);
    return 0x10;
  }
  if ((value & 0x08) && value != 0x08) {
    elyErrorSignal(ErrRegClip);
    return 0x08;
  }
  if ((value & 0x04) && value != 0x04) {
    elyErrorSignal(ErrRegClip);
    return 0x04;
  }
  if ((value & 0x02) && value != 0x02) {
    elyErrorSignal(ErrRegClip);
    return 0x02;
  }
  if ((value & 0x01) && value != 0x01) {
    elyErrorSignal(ErrRegClip);
    return 0x01;
  }
  if (value & 0xD0) {
    elyErrorSignal(ErrRegClip);
    return 0x00;
  }
  return value; /* must be 0x00 at this point */
}

/* TODO test that this cast actually works - pretty sure it's not standard-compliant*/
static uint8_t clamps(uint8_t value, int8_t min, int8_t max) {
  if ((int8_t)(value) < min) {
    elyErrorSignal(ErrRegClip);
    value = (uint8_t)(min);
  }
  else if ((int8_t)(value) > max) {
    elyErrorSignal(ErrRegClip);
    value = (uint8_t)(max);
  }
  return value;
}

void fram_reg_cb(uint8_t * buffer) {
  (void)(buffer);
  regs_in_progress--;
  if (regs_in_progress == 0) {
    chSysLockFromISR();
    chBSemSignalI(&regs_sem);
    chSysUnlockFromISR();
  }
}

void fram_reg(uint8_t read, uint8_t bank, uint8_t addr, uint8_t * valuep) {
  fram_req_t * req;
  
  chSysLock();
  /* TODO timeout for safety */
  elyFramGetRequestTimeoutS(&req, TIME_INFINITE);
  chSysUnlock();
  
  /* Fill out the request */
  req->address = FRAM_REG_BASE + ((bank-1) * 256) + addr;
  req->read = read;
  req->size = 1;
  req->buffer = valuep;
  req->callback = fram_reg_cb;
  
  /* TODO handle failure */
  chSysLock();
  elyFramPostRequestS(req);
  chSysUnlock();
}

void fram_block_cb(uint8_t * buffer) {
  (void)(buffer);
  chSysLockFromISR();
  chBSemSignalI(&regs_sem);
  chSysUnlockFromISR();
}

void fram_block(uint8_t read, uint8_t bank, uint8_t addr, uint8_t * buffer, uint8_t n) {
  fram_req_t * req;
  
  chSysLock();
  /* TODO timeout for safety */
  elyFramGetRequestTimeoutS(&req, TIME_INFINITE);
  chSysUnlock();
  
  /* Fill out the request */
  req->address = FRAM_REG_BASE + ((bank-1) * 256) + addr;
  req->read = read;
  req->size = n;
  req->buffer = buffer;
  req->callback = fram_block_cb;
  
  /* TODO handle failure */
  chSysLock();
  elyFramPostRequestS(req);
  chSysUnlock();
}

uint8_t elyClampReg(uint8_t addr, uint8_t value) {
  switch(addr) {
    case RegTXFreqMsb:
      value = clamp(value, TX_BAND_MIN_MSB, TX_BAND_MAX_MSB);
      break;
    case RegRXFreqMsb:
      value = clamp(value, RX_BAND_MIN_MSB, RX_BAND_MAX_MSB);
      break;
    case RegTXDevMsb:
      value = clamp(value, TX_DEV_MIN_MSB, TX_DEV_MAX_MSB);
      break;
    case RegRXDevMsb:
      value = clamp(value, RX_DEV_MIN_MSB, RX_DEV_MAX_MSB);
      break;
    case RegTXBRMsb:
      value = clamp(value, TX_BR_MIN_MSB, TX_BR_MAX_MSB);
      break;
    case RegRXBRMsb:
      value = clamp(value, RX_BR_MIN_MSB, RX_BR_MAX_MSB);
      break;
    case RegOutputPower:
      value = clamp(value, TX_POW_MIN, TX_POW_MAX);
    case RegUARTBaudLsb:
    case RegSCCommBaudLsb:
      value = clamp(value, 1, 0xFF);
      break;
    case RegUARTBaudHmb:
    case RegSCCommBaudHmb:
      value = clamp(value, 0, UART_BAUD_MAX_HMB);
      break;
    case RegUARTBaudMsb:
    case RegSCCommBaudMsb:
      return 0; /* this is purely efficiency */
    case RegResetErrLvl:
    case RegUARTErrLvl:
    case RegSubOverLvl:
    case RegNRErrLvl:
    case RegFCSLvl:
    case RegLengthErrLvl:
    case RegOpErrLvl:
    case RegRegErrLvl:
    case RegSCCommLvl:
    case RegGFLvl:
      value = clamp_err(value);
      break;
    case RegGFBank:
      value = clamp(value, 0, 4);
      break;
    case RegBoardTempWARN:
    case RegBoardTempERR:
    case RegHSTempWARN:
    case RegHSTempERR:
    case RegPATempWARN:
    case RegPATempERR:
      value = clamps(value, TEMP_MIN, TEMP_MAX);
      break;
    default:
      if (addr > 0x80 && addr < 0xBF) {
        value = elyNLClampReg(addr, value);
      }
      else if (addr > 0xC0) {
        value = elyRFDLLClampReg(addr, value);
      }
      /* Other Core Registers have ranges equal to their data type's */
  }
  return value;
}

void elyRegGet(uint8_t bank, uint8_t * buffer, uint8_t n) {
  regs_in_progress = n;
  
  if (bank == 0) {
    for (int i = 0; i < n; i++) {
      uint8_t addr = buffer[i];
      *(buffer+i) = bank0p[addr];
    }
  }
  else {
    for (int i = 0; i < n; i++) {
      uint8_t addr = buffer[i];
  
      /* Get the register */
      fram_reg(1, bank, addr, (buffer+i));
    }
    /* Wait for all writes to complete */
    /* TODO timeout for safety */
    chBSemWaitTimeout(&regs_sem, TIME_INFINITE);
  }
}

void elyRegSet(uint8_t bank, uint8_t * buffer, uint8_t n) {
  regs_in_progress = n;
  
  for (int i = 0; i < n * 2; i += 2) {
    uint8_t addr = buffer[i];
    uint8_t *valuep = &buffer[i+1];
    (*valuep) = elyClampReg(addr, (*valuep));
  
    /* Write the register */
    fram_reg(0, bank, addr, valuep);
  }
  
  /* Wait for all writes to complete */
  /* TODO timeout for safety */
  chBSemWaitTimeout(&regs_sem, TIME_INFINITE);
}

void elyRegGetBlock(uint8_t bank, uint8_t * buffer, uint8_t addr, uint8_t n) {
  
  if (bank == 0) {
    memcpy(buffer, bank0p + addr, n);
  }
  else {
    /* Read the block */
    fram_block(1, bank, addr, buffer, n);
    /* Wait for the write to complete */
    /* TODO timeout for safety */
    chBSemWaitTimeout(&regs_sem, TIME_INFINITE);
  }
  
}

void elyRegSetBlock(uint8_t bank, uint8_t * buffer, uint8_t addr, uint8_t n) {
  
  for (int i = 0; i < n; i++) {
    buffer[i] = elyClampReg(addr + i, buffer[i]);
  }
  
  /* Write the block */
  fram_block(0, bank, addr, buffer, n);
  
  /* Wait for the write to complete */
  /* TODO timeout for safety */
  chBSemWaitTimeout(&regs_sem, TIME_INFINITE);
  
}








