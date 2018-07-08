/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio.

    This file is part of ChibiOS.

    ChibiOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    SIMIA32/chcore.c
 * @brief   Simulator on IA32 port code.
 *
 * @addtogroup SIMIA32_GCC_CORE
 * @{
 */

#include "ch.h"

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

bool port_isr_context_flag;
syssts_t port_irq_sts;

/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * Performs a context switch between two threads.
 * @param otp the thread to be switched out
 * @param ntp the thread to be switched in
 */
__attribute__((used,fastcall))
void __dummy(thread_t *ntp, thread_t *otp) {
  (void)ntp; (void)otp;

  asm volatile (
#if defined(WIN32)
                ".globl @port_switch@8                          \n\t"
                "@port_switch@8:"
#elif defined(__APPLE__)
                ".globl _port_switch                            \n\t"
                "_port_switch:"
#else
                ".globl port_switch                             \n\t"
                "port_switch:"
#endif
                "pushl   %ebp                                  \n\t"
                "pushl   %ebx                                  \n\t"
                "pushl   %ecx                                  \n\t"
                "pushl   %edx                                  \n\t"
                "pushl   %esi                                  \n\t"
                "pushl   %edi                                  \n\t"
#ifdef _CHIBIOS_NIL_CONF_
                "movl    %esp, 0(%edx)                         \n\t"
                "movl    0(%ecx), %esp                         \n\t"
#elif _CHIBIOS_RT_CONF_
                "movl    %esp, 12(%edx)                         \n\t"
                "movl    12(%ecx), %esp                         \n\t"
#else
#error "Unsupported CHIBIOS"
#endif
                "popl    %edi                                  \n\t"
                "popl    %esi                                  \n\t"
                "popl    %edx                                  \n\t"
                "popl    %ecx                                  \n\t"
                "popl    %ebx                                  \n\t"
                "popl    %ebp                                  \n\t"
                "ret                                            \n\t"
                );
}

/**
 * @brief   Start a thread by invoking its work function.
 * @details If the work function returns @p chThdExit() is automatically
 *          invoked.
 */
CH_CDECL_NORETURN
void _port_thread_start(msg_t (*pf)(void *), void *p) {
  chSysUnlock();
  pf(p);
#ifndef _CHIBIOS_NIL_CONF_
  chThdExit(0);
#endif
  while(1);
}


/**
 * @brief   Returns the current value of the realtime counter.
 *
 * @return              The realtime counter value.
 */
#ifdef CH_POSIX
# include <time.h>
# if !defined(__FUZZ__)
rtcnt_t port_rt_get_counter_value(void) {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (rtcnt_t)(t.tv_sec * 1000000 + t.tv_nsec / 1000);
}
# endif // defined(__FUZZ__)
#else
#include <windows.h>
rtcnt_t port_rt_get_counter_value(void) {
  LARGE_INTEGER n;

  QueryPerformanceCounter(&n);

  return (rtcnt_t)(n.QuadPart / 1000LL);
}
#endif

/** @} */
