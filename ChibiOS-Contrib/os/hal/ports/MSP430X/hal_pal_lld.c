/*
    ChibiOS - Copyright (C) 2016 Andrew Wygle aka awygle

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
 * @file    MSP430X/hal_pal_lld.c
 * @brief   MSP430X PAL subsystem low level driver source.
 *
 * @addtogroup PAL
 * @{
 */

#include "hal.h"

#if (HAL_USE_PAL == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/* A callback map for each port.
 * 
 * The index is the pin number.
 * The result is the callback for that pin.
 */

#if defined(P11IE)
palcallback_t port_callback_map[12][8u];
#elif defined(P9IE)
palcallback_t port_callback_map[10][8u];
#elif defined(P7IE)
palcallback_t port_callback_map[8][8u];
#elif defined(P5IE)
palcallback_t port_callback_map[6][8u];
#elif defined(P3IE)
palcallback_t port_callback_map[4][8u];
#else
palcallback_t port_callback_map[2][8u];
#endif

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/**
 * @brief   PORT1 interrupt handler
 * 
 * @isr
 */
PORT_IRQ_HANDLER(PORT1_VECTOR) {
  palcallback_t cb;
  uint8_t pad;
  OSAL_IRQ_PROLOGUE();
  pad = (P1IV >> 1) - 1;
  cb = port_callback_map[0][pad];
  if (cb != NULL) {
    (*cb)();
  }
  
  OSAL_IRQ_EPILOGUE();
}
  
/**
 * @brief   PORT2 interrupt handler
 * 
 * @isr
 */
PORT_IRQ_HANDLER(PORT2_VECTOR) {
  palcallback_t cb;
  uint8_t pad;
  OSAL_IRQ_PROLOGUE();
  pad = (P2IV >> 1) - 1;
  cb = port_callback_map[1][pad];
  if (cb != NULL) {
    (*cb)();
  }
  
  OSAL_IRQ_EPILOGUE();
}
  
#if defined(P3IE) || defined(__DOXYGEN__)
/**
 * @brief   PORT3 interrupt handler
 * 
 * @isr
 */
PORT_IRQ_HANDLER(PORT3_VECTOR) {
  palcallback_t cb;
  uint8_t pad;
  OSAL_IRQ_PROLOGUE();
  pad = (P3IV >> 1) - 1;
  cb = port_callback_map[2][pad];
  if (cb != NULL) {
    (*cb)();
  }
  
  OSAL_IRQ_EPILOGUE();
}
  
/**
 * @brief   PORT4 interrupt handler
 * 
 * @isr
 */
PORT_IRQ_HANDLER(PORT4_VECTOR) {
  palcallback_t cb;
  uint8_t pad;
  OSAL_IRQ_PROLOGUE();
  pad = (P4IV >> 1) - 1;
  cb = port_callback_map[3][pad];
  if (cb != NULL) {
    (*cb)();
  }
  
  OSAL_IRQ_EPILOGUE();
}
#endif
  
#if defined(P5IE) || defined(__DOXYGEN__)
/**
 * @brief   PORT5 interrupt handler
 * 
 * @isr
 */
PORT_IRQ_HANDLER(PORT5_VECTOR) {
  palcallback_t cb;
  uint8_t pad;
  OSAL_IRQ_PROLOGUE();
  pad = (P5IV >> 1) - 1;
  cb = port_callback_map[4][pad];
  if (cb != NULL) {
    (*cb)(PAL_LINE(IOPORT3, pad));
  }
  
  OSAL_IRQ_EPILOGUE();
}
  
/**
 * @brief   PORT6 interrupt handler
 * 
 * @isr
 */
PORT_IRQ_HANDLER(PORT6_VECTOR) {
  palcallback_t cb;
  uint8_t pad;
  OSAL_IRQ_PROLOGUE();
  pad = (P6IV >> 1) - 1;
  cb = port_callback_map[5][pad];
  if (cb != NULL) {
    (*cb)(PAL_LINE(IOPORT3, (pad + 8) ));
  }
  
  OSAL_IRQ_EPILOGUE();
}
#endif
  
#if defined(P7IE) || defined(__DOXYGEN__)
/**
 * @brief   PORT7 interrupt handler
 * 
 * @isr
 */
PORT_IRQ_HANDLER(PORT7_VECTOR) {
  palcallback_t cb;
  uint8_t pad;
  OSAL_IRQ_PROLOGUE();
  pad = (P7IV >> 1) - 1;
  cb = port_callback_map[6][pad];
  if (cb != NULL) {
    (*cb)(PAL_LINE(IOPORT4, pad));
  }
  
  OSAL_IRQ_EPILOGUE();
}
  
/**
 * @brief   PORT8 interrupt handler
 * 
 * @isr
 */
PORT_IRQ_HANDLER(PORT8_VECTOR) {
  palcallback_t cb;
  uint8_t pad;
  OSAL_IRQ_PROLOGUE();
  pad = (P8IV >> 1) - 1;
  cb = port_callback_map[7][pad];
  if (cb != NULL) {
    (*cb)(PAL_LINE(IOPORT4, (pad + 8) ));
  }
  
  OSAL_IRQ_EPILOGUE();
}
#endif
  
#if defined(P9IE) || defined(__DOXYGEN__)
/**
 * @brief   PORT9 interrupt handler
 * 
 * @isr
 */
PORT_IRQ_HANDLER(PORT9_VECTOR) {
  palcallback_t cb;
  uint8_t pad;
  OSAL_IRQ_PROLOGUE();
  pad = (P9IV >> 1) - 1;
  cb = port_callback_map[8][pad];
  if (cb != NULL) {
    (*cb)(PAL_LINE(IOPORT5, pad));
  }
  
  OSAL_IRQ_EPILOGUE();
}
  
/**
 * @brief   PORT10 interrupt handler
 * 
 * @isr
 */
PORT_IRQ_HANDLER(PORT10_VECTOR) {
  palcallback_t cb;
  uint8_t pad;
  OSAL_IRQ_PROLOGUE();
  pad = (P10IV >> 1) - 1;
  cb = port_callback_map[9][pad];
  if (cb != NULL) {
    (*cb)(PAL_LINE(IOPORT5, (pad + 8) ));
  }
  
  OSAL_IRQ_EPILOGUE();
}
#endif
  
#if defined(P11IE) || defined(__DOXYGEN__)
/**
 * @brief   PORT11 interrupt handler
 * 
 * @isr
 */
PORT_IRQ_HANDLER(PORT11_VECTOR) {
  palcallback_t cb;
  uint8_t pad;
  OSAL_IRQ_PROLOGUE();
  pad = (P11IV >> 1) - 1;
  cb = port_callback_map[10][pad];
  if (cb != NULL) {
    (*cb)(PAL_LINE(IOPORT6, pad));
  }
  
  OSAL_IRQ_EPILOGUE();
}
  
/**
 * @brief   PORT12 interrupt handler
 * 
 * @isr
 */
PORT_IRQ_HANDLER(PORT12_VECTOR) {
  palcallback_t cb;
  uint8_t pad;
  OSAL_IRQ_PROLOGUE();
  pad = (P12IV >> 1) - 1;
  cb = port_callback_map[11][pad];
  if (cb != NULL) {
    (*cb)(PAL_LINE(IOPORT6, (pad + 8) ));
  }
  
  OSAL_IRQ_EPILOGUE();
}
#endif
  
/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   MSP430X I/O ports configuration.
 * @details GPIO registers initialization
 *
 * @param[in] config    the MSP430X ports configuration
 *
 * @notapi
 */
void _pal_lld_init(const PALConfig *config) {

  PAOUT = config->porta.out;
  PADIR = config->porta.dir;
  PAREN = config->porta.ren;
  PASEL0 = config->porta.sel0;
  PASEL1 = config->porta.sel1;
#if defined(PB_BASE) || defined(__DOXYGEN__)
  PBOUT = config->portb.out;
  PBDIR = config->portb.dir;
  PBREN = config->portb.ren;
  PBSEL0 = config->portb.sel0;
  PBSEL1 = config->portb.sel1;
#endif
#if defined(PC_BASE) || defined(__DOXYGEN__)
  PCOUT = config->portc.out;
  PCDIR = config->portc.dir;
  PCREN = config->portc.ren;
  PCSEL0 = config->portc.sel0;
  PCSEL1 = config->portc.sel1;
#endif
#if defined(PD_BASE) || defined(__DOXYGEN__)
  PDOUT = config->portd.out;
  PDDIR = config->portd.dir;
  PDREN = config->portd.ren;
  PDSEL0 = config->portd.sel0;
  PDSEL1 = config->portd.sel1;
#endif
#if defined(PE_BASE) || defined(__DOXYGEN__)
  PEOUT = config->porte.out;
  PEDIR = config->porte.dir;
  PEREN = config->porte.ren;
  PESEL0 = config->porte.sel0;
  PESEL1 = config->porte.sel1;
#endif
#if defined(PF_BASE) || defined(__DOXYGEN__)
  PFOUT = config->portf.out;
  PFDIR = config->portf.dir;
  PFREN = config->portf.ren;
  PFSEL0 = config->portf.sel0;
  PFSEL1 = config->portf.sel1;
#endif
  PJOUT = config->portj.out;
  PJDIR = config->portj.dir;
  PJREN = config->portj.ren;
  PJSEL0 = config->portj.sel0;
  PJSEL1 = config->portj.sel1;
  
  PM5CTL0 &= ~LOCKLPM5;
}

/**
 * @brief   Pads mode setup.
 * @details This function programs a pads group belonging to the same port
 *          with the specified mode.
 * @note    @p PAL_MODE_UNCONNECTED is implemented as input with pullup.
 *
 * @param[in] port      the port identifier
 * @param[in] mask      the group mask
 * @param[in] mode      the mode
 *
 * @notapi
 */
void _pal_lld_setgroupmode(ioportid_t port,
                           ioportmask_t mask,
                           iomode_t mode) {
  
  switch (mode) {
    case PAL_MODE_RESET:
    case PAL_MODE_INPUT:
      port->dir &= ~mask;
      port->ren &= ~mask;
      if ((port->sel0 & mask) && (port->sel1 & mask))
        port->selc = mask;
      else {
        port->sel0 &= ~mask;
        port->sel1 &= ~mask;
      }
      break;
    case PAL_MODE_UNCONNECTED:
    case PAL_MODE_INPUT_PULLUP:
      port->dir &= ~mask;
      port->ren |= mask;
      port->out |= mask;
      if ((port->sel0 & mask) && (port->sel1 & mask))
        port->selc = mask;
      else {
        port->sel0 &= ~mask;
        port->sel1 &= ~mask;
      }
      break;
    case PAL_MODE_INPUT_PULLDOWN:
      port->dir &= ~mask;
      port->ren |= mask;
      port->out &= ~mask;
      if ((port->sel0 & mask) && (port->sel1 & mask))
        port->selc = mask;
      else {
        port->sel0 &= ~mask;
        port->sel1 &= ~mask;
      }
      break;
    case PAL_MODE_OUTPUT_PUSHPULL:
      port->dir |= mask;
      if ((port->sel0 & mask) && (port->sel1 & mask))
        port->selc = mask;
      else {
        port->sel0 &= ~mask;
        port->sel1 &= ~mask;
      }
      break;
    case PAL_MSP430X_ALTERNATE_1:
      if (!(port->sel0 & mask) && (port->sel1 & mask)) 
        port->selc = mask;
      else {
        port->sel0 |= mask;
        port->sel1 &= ~mask;
      }
      break;
    case PAL_MSP430X_ALTERNATE_2:
      if ((port->sel0 & mask) && !(port->sel1 & mask))
        port->selc = mask;
      else {
        port->sel0 &= ~mask;
        port->sel1 |= mask;
      }
      break;
    case PAL_MSP430X_ALTERNATE_3:
      if (!(port->sel0 & mask) && !(port->sel1 & mask))
        port->selc = mask;
      else {
        port->sel0 |= mask;
        port->sel1 |= mask;
      }
      break;
  }
}

void _pal_lld_enablepadevent(ioportid_t port, uint8_t pad, ioeventmode_t mode,
    palcallback_t callback) {
  /* This is ugly but very efficient. Check the MSP header files. */
  uint8_t portindex = ((((intptr_t)(port)) >> 4) & 0x0F) + (pad > 7 ? 2 : 1);
  
  port_callback_map[portindex - 1][pad & 0x07] = callback;

  if (mode == PAL_EVENT_MODE_RISING_EDGE) {
    port->ies &= ~(1 << pad);
  }
  else if (mode == PAL_EVENT_MODE_FALLING_EDGE) {
    port->ies |= (1 << pad);
  }
  
  if (mode == PAL_EVENT_MODE_DISABLED) {
    port->ie &= ~(1 << pad);
  }
  else {
    port->ifg &= ~(1 << pad);
    port->ie |= (1 << pad);
  }
}

void _pal_lld_disablepadevent(ioportid_t port, uint8_t pad) {
  
  port->ie &= ~(1 << pad);
}
#endif /* HAL_USE_PAL == TRUE */

/** @} */
