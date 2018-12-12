
#ifndef _ELYSIUM_SLIP_UART_DLL_H_
#define _ELYSIUM_SLIP_UART_DLL_H_

#include "core.h"
#include "main.h"
#include "slip.h"
#include "nl.h"
#include "uart_dll.h"

/* This must be a power of 2 and <= (UINT16_MAX+1) / 2 */
#define SLIP_RX_BUF_LEN 256
#define SLIP_TX_CHUNK_LEN 32

typedef enum {
  ELY_SLIP_RESET,
  ELY_SLIP_ESCAPED,
  ELY_SLIP_NOT_ESCAPED
} slip_uart_states_t;

#endif
