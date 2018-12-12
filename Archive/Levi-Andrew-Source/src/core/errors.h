
#ifndef _ELYSIUM_ERRORS_H_
#define _ELYSIUM_ERRORS_H_

#include "core.h"
#include "registers.h"

typedef enum {
  CRITICAL = 0x10,
  ERROR = 0x08,
  WARNING = 0x04,
  INFO = 0x02,
  DEBUG = 0x01
} elysium_err_lvl_t;

typedef uint8_t elysium_err_mask_t;

#define ELY_ALL_ERRORS 0x1F

typedef enum {
  ErrPAOvertemp = 0x80,
  ErrHSOvertemp = 0x81,
  ErrMCUOvertemp = 0x82,
  ErrGroundCommFault = 0x83,
  ErrSCCommFault = 0x84,
  ErrLNAOvercurrent = 0x85,
  ErrPAOvercurrent = 0x86,
  ErrMCUUndervolt = 0x87,
  ErrRegClip = 0x88,
  ErrInvalidOpcode = 0x89,
  ErrInvalidLength = 0x8A,
  ErrFCSError = 0x8B,
  ErrCmdFailure = 0x8C,
  ErrSubOverwrite = 0x8D,
  ErrReset = 0x8E,
  ErrUARTError = 0x8F,
  ErrCoreMAX
} elysium_errs_t;

#ifdef __cplusplus
extern "C" {
#endif
  void elyErrorSignal(uint8_t error);
  void elyErrorSignalI(uint8_t error);
  void elyErrorSetRptLvlS(uint8_t lvl);
  void elyErrorSetLogLvlS(uint8_t lvl);
#ifdef __cplusplus
}
#endif

#endif
