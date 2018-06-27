
#include "rf.h"

msg_t elyRFPostI(uint8_t * buffer) {
  return elyUARTPostI(buffer);
}

msg_t elyRFPost(uint8_t * buffer, systime_t timeout) {
  return elyUARTPost(buffer, timeout);
}

void elyRFCfgMarkDirtyI(rf_events_t event) {
  (void)(event);
}

void elyRFCfgMarkDirty(rf_events_t event) {
  (void)(event);
}
