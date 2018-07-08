
#ifndef _ELYSIUM_UART_H_
#define _ELYSIUM_UART_H_

#include "ch.h"
#include "hal.h"
#include "core.h"
#include "registers.h"
#include "nl.h"
#include "main.h"
#include "uart_dll.h"

#if defined(POSIX_MCUCONF)
# define ELY_UART UARTD1
# define uart_gpt GPTD1
#elif defined(MSP430X_MCUCONF)
# if ELY_REVISION == B
#  define ELY_UART UARTDA1
# else
#  define ELY_UART UARTDA0
# endif
# define uart_gpt GPTDA1
#else
# error "Unsupported MCU"
#endif

typedef enum {
  UARTConfigUpdated = 0x01,
  UARTBufferPosted = 0x02,
  UARTRxBufferReady = 0x04
} uart_events_t;

typedef enum {
  UARTIdle = 2,
  UARTTXActive = 4,
  UARTCfg = 6
} uart_states_t;

#ifdef __cplusplus
extern "C" {
#endif
  void elyUARTCfgMarkDirty(void);
  void elyUARTCfgMarkDirtyI(void);
  msg_t elyUARTPost(uint8_t * buffer, systime_t timeout);
  msg_t elyUARTPostI(uint8_t * buffer);
  void elyUARTRxBufReadyI(void);
  THD_FUNCTION(UARTThd, arg);
#ifdef __cplusplus
}
#endif
  
extern THD_WORKING_AREA(waUARTThd, 256);
  
#endif
