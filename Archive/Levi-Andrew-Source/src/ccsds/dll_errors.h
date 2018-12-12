
#ifndef _ELYSIUM_DLL_ERRORS_H_
#define _ELYSIUM_DLL_ERRORS_H_

typedef enum {
  ErrDLLFEC = 0xB0,
  ErrDLLFECF = 0xB1,
  ErrDLLMissedFrame = 0xB2,
  ErrDLLLockout = 0xB3,
  ErrDLLDoubleFrame = 0xB4,
  ErrDLLInvalidID = 0xB5,
  ErrDLLShortFrame = 0xB6,
  ErrDLLLongFrame = 0xB7,
  ErrDLLWait = 0xB8,
  ErrDLLMAX
} elysium_dll_errs_t;

#define DLL_ERR_DEFAULTS \
  WARNING, /* FEC */\
  ERROR, /* FECF */\
  ERROR, /* MissedFrame */\
  ERROR, /* Lockout */\
  INFO, /* DoubleFrame */\
  ERROR, /* InvalidID */\
  ERROR, /* ShortFrame */\
  WARNING, /* LongFrame */\
  ERROR, /* Wait */\
  0xFF,\
  0xFF,\
  0xFF,\
  0xFF,\
  0xFF,\
  0xFF,\
  0xFF 

#endif

