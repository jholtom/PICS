
#include "events.h"
#include "registers.h"

eventmask_t PERSIST logged_events = 0;
eventmask_t PERSIST reported_events = 0;
uint16_t PERSIST report_addrs[EvtCoreMAX+EvtNLMAX+EvtDLLMAX]; /* initialized to 0 */

static thread_t * event_thd;

static inline eventmask_t mask_from_event(uint8_t event) {
  return (1U << (event & 0x3F));
}

static inline elysium_evts_t event_from_mask(eventmask_t event) {
  size_t i = 0;
  while (!(event & 1)) {
    event >>= 1;
    i++;
  }
  return i + 0xC0;
}

void elyEventSignal(uint8_t event) {
  chEvtSignal(event_thd, mask_from_event(event));
}

void elyEventSignalI(uint8_t event) {
  chDbgCheckClassI();
  chEvtSignalI(event_thd, mask_from_event(event));
}

static uint8_t evt_idx(uint8_t event) {
  if (event > 0xF0) {
    return (event-0xF0)+(EvtCoreMAX-0xC0)+(EvtNLMAX-0xE0);
  }
  else if (event > 0xE0) {
    return (event-0xE0)+(EvtCoreMAX-0xC0);
  }
  return event - 0xC0;
}

void elyEventSubscribe(uint8_t event, uint16_t addr) {
  reported_events |= mask_from_event(event);
  report_addrs[evt_idx(event)] = addr;
  chSysLock();
  if (!NIL_THD_IS_READY(event_thd)) {
    chSchReadyI(event_thd, MSG_RESET);
  }
  chSysUnlock();
}

void elyEventUnsubscribe(uint8_t event) {
  reported_events &= ~(mask_from_event(event));
  chSysLock();
  if (!NIL_THD_IS_READY(event_thd)) {
    chSchReadyI(event_thd, MSG_RESET);
  }
  chSysUnlock();
}

void elyEventLog(uint8_t event) {
  logged_events |= mask_from_event(event);
  chSysLock();
  if (!NIL_THD_IS_READY(event_thd)) {
    chSchReadyI(event_thd, MSG_RESET);
  }
  chSysUnlock();
}

void elyEventUnlog(uint8_t event) {
  logged_events &= ~(mask_from_event(event));
  chSysLock();
  if (!NIL_THD_IS_READY(event_thd)) {
    chSchReadyI(event_thd, MSG_RESET);
  }
  chSysUnlock();
}

void elyEventReset() {
  logged_events = 0;
  reported_events = 0;
  chSysLock();
  if (!NIL_THD_IS_READY(event_thd)) {
    chSchReadyI(event_thd, MSG_RESET);
  }
  chSysUnlock();
}

/* TODO make sure this is right, port it to Errors */
static eventmask_t get_next_event(eventmask_t mask) {
  mask ^= mask & (mask - (eventmask_t)1);
  return mask;
}

static void make_evt_buffer(uint8_t * buffer, uint8_t event) {
  buffer[0] = event;
  buffer[1] = 0;
  buffer[2] = report_addrs[evt_idx(event)] >> 8;
  buffer[3] = report_addrs[evt_idx(event)] & 0xFF;
}

THD_WORKING_AREA(waEvtThd, 128);
THD_FUNCTION(EvtThd, arg) {
  (void)arg;
  eventmask_t result;
  eventmask_t evt;
  
  event_thd = chThdGetSelfX();
  
  while (true) {
    result = chEvtWaitAnyTimeout(logged_events | reported_events, TIME_INFINITE);
    while (result > 0) {
      /* Get the lowest index event */
      evt = get_next_event(result);
      
      /* Handle the event */
      if (logged_events & evt) {
        /* TODO log event */
      }
      
      if (reported_events & evt) {
        uint8_t * buffer = elyNLGetBuffer();
        make_evt_buffer(elyNLExtract(buffer), event_from_mask(evt));
        if (ELY_DEST_RF == elyNLSetDest(buffer)) {
          /* TODO timeout for safety */
          elyRFPost(buffer, TIME_INFINITE);
        }
        else {
          /* TODO timeout for safety */
          elyUARTPost(buffer, TIME_INFINITE);
        }
      }
      
      /* Remove the event from the mask */
      result &= ~evt;
    }
    /* Otherwise the masks have been updated - just go to the top of the loop */
  }
}

