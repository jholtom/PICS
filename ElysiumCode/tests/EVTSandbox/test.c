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
static void __attribute__((naked, section(".crt_0042disable_watchdog"), used))
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
static inline eventmask_t mask_from_event(uint8_t event) {
  return (1 << (event & 0x3F));
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

size_t elyChanGetValue(uint8_t * buffer) {
  buffer[1] = 0;
  return 1;
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
void elyTelemPostBufferS(uint8_t * buffer, size_t n, telemcallback_t cb) {
  last_buffer = buffer;
  last_n = n;
  cb(buffer);
}

void elyTelemUpdateConfigS(telem_cfg_t config) {
  chDbgCheckClassS();
  telem_cfg = config;
}

/* RF */
static rf_events_t PERSIST cfg_dirt = 0;
void elyRFCfgMarkDirty(rf_events_t event) {
  cfg_dirt |= mask_from_event(event);
}

msg_t elyRFPostI(uint8_t * buffer) {
  return elyUARTPostI(buffer);
}

msg_t elyRFPost(uint8_t * buffer) {
  return elyUARTPost(buffer);
}

/* DLL */
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

uint8_t elyDLLClampReg(uint8_t addr, uint8_t value) {
  chDbgAssert(addr >= 0xC0 && addr < RegDLLMAX, "invalid address");
  switch (addr) {
    case RegDLLTFLengthLSB:
      value = clamp(value, 7, 0xFF);
      break;
    case RegDLLTFLengthMSB:
      value = clamp(value, 0, 0x40);
      break;
    case RegDLLTMFEC:
      if ((value & 0x38) == 0x38) {
        value &= ~BIT3;
      }
      if ((value & 0x07) > 5) {
        value &= ~BIT1;
        value |= BIT0;
      }
      break;
    case RegDLLTCFEC:
      if ((value & 0x03) == 0x03) {
        value &= ~BIT0;
      }
      break;
    case RegDLLWindowLength:
      value &= ~BIT0;
      break;
    case RegDLLFECLvl:
    case RegDLLFECFLvl:
    case RegDLLMissedFrameLvl:
    case RegDLLLockoutLvl:
    case RegDLLDoubleFrameLvl:
    case RegDLLInvalidIDLvl:
    case RegDLLShortFrameLvl:
    case RegDLLLongFrameLvl:
    case RegDLLWaitLvl:
      value = clamp_err(value);
      break;
    default:
      /* All other registers have ranges equal to their data types */
      break;
  }
  return value;
}

/*
 * Threads static table, one entry per thread. The number of entries must
 * match NIL_CFG_NUM_THREADS.
 */
THD_TABLE_BEGIN
  THD_TABLE_ENTRY(waMainThd, "MainThread", MainThd, NULL)
  THD_TABLE_ENTRY(waUARTThd, "UARTThread", UARTThd, NULL)
  THD_TABLE_ENTRY(waFramThd, "FramThread", FramThd, NULL)
  THD_TABLE_ENTRY(waEvtThd, "EvtThread", EvtThd, NULL)
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

  halInit();
  chSysInit();

  /* This is now the idle thread loop, you may perform here a low priority
     task but you must never try to sleep or wait in this loop. Note that
     this tasks runs at the lowest priority level so any instruction added
     here will be executed after all other tasks have been started.*/
  while (true) {
  }
}
