
#ifndef _ELYSIUM_NL_ERRORS_H_
#define _ELYSIUM_NL_ERRORS_H_

typedef enum {
  ErrNLPVNMismatch = 0xA0,
  ErrNLPacketLengthMismatch = 0xA1,
  ErrNLMAX
} elysium_nl_errs_t;

#define NL_ERR_DEFAULTS \
  ERROR, /* PVNMismatch */\
  ERROR, /* PacketLengthMismatch */\
  0xFF,\
  0xFF,\
  0xFF,\
  0xFF,\
  0xFF,\
  0xFF,\
  0xFF,\
  0xFF,\
  0xFF,\
  0xFF,\
  0xFF,\
  0xFF,\
  0xFF,\
  0xFF

#endif

