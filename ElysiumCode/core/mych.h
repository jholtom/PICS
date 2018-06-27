
#ifndef _ELYSIUM_MYCH_H_
#define _ELYSIUM_MYCH_H_

#include "ch.h"

#ifdef __cplusplus
extern "C" {
#endif
  eventmask_t chEvtWaitAnyTimeoutS(eventmask_t mask, systime_t timeout);
#ifdef __cplusplus
}
#endif

#endif
