/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <ch.h>
#include <msp430.h>
#include "uart.h"
#include "errors.h"

/* Disable watchdog because of lousy startup code in newlib */
#ifdef __FUZZ__
static void
#else
static void __attribute__((naked, section(".crt_0042disable_watchdog"), used))
#endif
disable_watchdog(void) {
  WDTCTL = WDTPW | WDTHOLD;
}

uint8_t PERSIST bootloader = 0;

/* Errors */
static uint16_t PERSIST err_log_lvl;
static uint16_t PERSIST err_rpt_lvl;
static uint16_t PERSIST signalled_errors;
static inline eventmask_t mask_from_error(uint8_t error) {
  return (1 << (error & 0x3F));
}

void elyErrorSignal(uint8_t error) {
  signalled_errors |= mask_from_error(error);
}

void elyErrorSetLogLvlS(uint8_t lvl) {
  err_log_lvl = lvl;
}

void elyErrorSetRptLvlS(uint8_t lvl) {
  err_rpt_lvl = lvl;
}

/* Events */
eventmask_t PERSIST logged_events;
eventmask_t PERSIST reported_events;
static eventmask_t PERSIST signalled_events;
size_t PERSIST event_resets = 0;
static inline eventmask_t mask_from_event(uint8_t event) {
  return (1 << (event & 0x3F));
}

void elyEventSignal(uint8_t event) {
  signalled_events |= mask_from_event(event);
}

void elyEventSubscribe(uint8_t event, uint16_t addr) {
  (void)(addr);
  reported_events |= mask_from_event(event);
}

void elyEventUnsubscribe(uint8_t event) {
  reported_events &= ~mask_from_event(event);
}

void elyEventLog(uint8_t event) {
  logged_events |= mask_from_event(event);
}

void elyEventUnlog(uint8_t event) {
  logged_events &= ~mask_from_event(event);
}

void elyEventReset() {
  logged_events = 0;
  signalled_events = 0;
  reported_events = 0;
  event_resets++;
}

/* Channels */
static eventmask_t PERSIST subscribed_channels;
static eventmask_t PERSIST logged_channels;
static size_t PERSIST chan_resets = 0;
static inline eventmask_t mask_from_chan(uint8_t chan) {
  return (1UL << (chan & 0x3F));
}

void elyChanSubscribe(uint8_t * buffer, uint8_t length, uint32_t interval) {
  (void)(interval);
  for (int i = 0; i < length; i++) {
    subscribed_channels |= mask_from_chan(buffer[i]);
  }
}

void elyChanUnsubscribe(uint8_t * buffer, uint8_t length) {
  for (int i = 0; i < length; i++) {
    subscribed_channels &= ~mask_from_chan(buffer[i]);
  }
}

void elyChanLog(uint8_t * buffer, uint8_t length, uint32_t interval) {
  (void)(interval);
  for (int i = 0; i < length; i++) {
    logged_channels |= mask_from_chan(buffer[i]);
  }
}

void elyChanUnlog(uint8_t * buffer, uint8_t length) {
  for (int i = 0; i < length; i++) {
    logged_channels &= ~mask_from_chan(buffer[i]);
  }
}

size_t elyChanGetValue(uint8_t * buffer, uint8_t id) {
  buffer[1] = id;
  return id;
}

void elyChanReset() {
  logged_channels = 0;
  subscribed_channels = 0;
  chan_resets++;
}

/* Telemetry */
static PERSIST uint8_t * last_buffer;
static PERSIST size_t last_n; 
static PERSIST telem_cfg_t telem_cfg;
void elyTelemPostBufferS(uint8_t * buffer, /* size_t n, */ telemcallback_t cb) {
  last_buffer = buffer;
  // last_n = n;
  cb(buffer);
}

void elyTelemUpdateConfigS(telem_cfg_t config) {
  chDbgCheckClassS();
  telem_cfg = config;
}

/*
 * Threads static table, one entry per thread. The number of entries must
 * match NIL_CFG_NUM_THREADS.
 */
THD_TABLE_BEGIN
  THD_TABLE_ENTRY(waCmdThd, "CmdThread", CmdThd, NULL)
  THD_TABLE_ENTRY(waUARTThd, "UARTThread", UARTThd, NULL)
  THD_TABLE_ENTRY(waRFThd, "RFThread", RFThd, NULL)
  THD_TABLE_ENTRY(waFramThd, "FramThread", FramThd, NULL)
THD_TABLE_END

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  WDTCTL = WDTPW | WDTHOLD;

  elyNLInit();
  halInit();
  chSysInit();

  /* This is now the idle thread loop, you may perform here a low priority
     task but you must never try to sleep or wait in this loop. Note that
     this tasks runs at the lowest priority level so any instruction added
     here will be executed after all other tasks have been started.*/
  while (true) {
  }
}
