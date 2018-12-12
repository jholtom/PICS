
#include "events.h"
#include "registers.h"

void elyEventSignal(uint8_t event) {
  (void)(event);
}

void elyEventSignalI(uint8_t event) {
  (void)(event);
}

void elyEventSubscribe(uint8_t event, uint16_t addr) {
  (void)(event);
  (void)(addr);
}

void elyEventUnsubscribe(uint8_t event) {
  (void)(event);
}

void elyEventLog(uint8_t event) {
  (void)(event);
}

void elyEventUnlog(uint8_t event) {
  (void)(event);
}

void elyEventReset() {
}

