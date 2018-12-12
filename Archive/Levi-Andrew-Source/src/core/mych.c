#include "mych.h"

#if CH_CFG_USE_EVENTS == TRUE
eventmask_t chEvtWaitAnyTimeoutS(eventmask_t mask, systime_t timeout) {
  chDbgCheckClassS();
  thread_t *ctp = nil.current;
  eventmask_t m;

  if ((m = (ctp->epmask & mask)) == (eventmask_t)0) {
    if (TIME_IMMEDIATE == timeout) {

      return (eventmask_t)0;
    }
    ctp->u1.ewmask = mask;
    if (chSchGoSleepTimeoutS(NIL_STATE_WTOREVT, timeout) < MSG_OK) {

      return (eventmask_t)0;
    }
    m = ctp->epmask & mask;
  }
  ctp->epmask &= ~m;

  return m;
}
#endif /* CH_CFG_USE_EVENTS == TRUE */
