/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio
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

/**
 * @file    hal_lld.c
 * @brief   POSIX simulator HAL subsystem low level driver code.
 *
 * @addtogroup POSIX_HAL
 * @{
 */

#include "hal.h"
#include <assert.h>
#include <string.h>

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/
// stkalign_t __main_thread_stack_base__;
// stkalign_t __main_thread_stack_end__;

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

#if !defined(__FUZZ__)
#define NEXT_TICK_NSEC (1000000000L / CH_CFG_ST_FREQUENCY)
#define NSEC_PER_SEC (1000000000L)

static struct timespec nextcnt;
#endif

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
static void read_stack_mapping();
static bool check_for_fuzz_interrupts();

static void read_stack_mapping() {
  FILE * f = fopen("/proc/self/maps", "r");
  assert(f);

  unsigned long mapping_start, mapping_end;
  char permissions[5];
  unsigned long offset;
  char dev[6];
  unsigned long inode;
  char path[4096];
  char buff[8192];

  while(fgets(buff, 8191, f) != NULL) {
    int read = sscanf(buff, "%lx-%lx %4s %lx %5s %ld %4096s",
           &mapping_start, &mapping_end,
           (char*)&permissions,
           &offset,
           (char*)&dev,
           &inode,
           (char*)&path);
    if (read == 7 && strcmp("[stack]", path) == 0) {
      // __main_thread_stack_base__ = mapping_start;
      // __main_thread_stack_end__ = mapping_end;
    }
  }
  fclose(f);
}

#if defined(__FUZZ__)
static bool check_for_fuzz_interrupts() {
  if (fuzz_next_timer_bit()) {
    return true;
  }
#if POSIX_GPT_USE_GPT1 == TRUE
  if (gpt_lld_next_bit(&GPTD1)) {
    return true;
  }
#endif
  return false;
}
#endif

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief Low level HAL driver initialization.
 */
void hal_lld_init(void) {
  printf("ChibiOS/RT simulator (POSIX)\n");
#if defined(__FUZZ__)
  printf("Fuzzing mode enabled\n");
#else
  clock_gettime(CLOCK_MONOTONIC, &nextcnt);
  nextcnt.tv_nsec += NEXT_TICK_NSEC;
  if (nextcnt.tv_nsec > NSEC_PER_SEC) {
    nextcnt.tv_nsec -= NSEC_PER_SEC;
    nextcnt.tv_sec += 1;
  }
#endif
  read_stack_mapping();

  fflush(stdout);
}

/**
 * @brief   Interrupt simulation.
 */
void _sim_check_for_interrupts(void) {
#if HAL_USE_SERIAL
  if (sd_lld_interrupt_pending()) {
    _dbg_check_lock();
    if (chSchIsPreemptionRequired())
      chSchDoReschedule();
    _dbg_check_unlock();
    return;
  }
#endif

  /* Interrupt Timer simulation */
#if defined(__FUZZ__)
  if (check_for_fuzz_interrupts()) {
#else
  struct timespec ctime;
  clock_gettime(CLOCK_MONOTONIC, &ctime);
  if (ctime.tv_sec > nextcnt.tv_sec || ctime.tv_nsec > nextcnt.tv_nsec) {
    nextcnt.tv_nsec += NEXT_TICK_NSEC;
    if (nextcnt.tv_nsec > NSEC_PER_SEC) {
      nextcnt.tv_nsec -= NSEC_PER_SEC;
      nextcnt.tv_sec += 1;
    }
#endif

    CH_IRQ_PROLOGUE();

    chSysLockFromISR();
    chSysTimerHandlerI();
    chSysUnlockFromISR();

    CH_IRQ_EPILOGUE();

    _dbg_check_lock();
    if (chSchIsPreemptionRequired())
      chSchDoReschedule();
    _dbg_check_unlock();
  }
}

/** @} */
