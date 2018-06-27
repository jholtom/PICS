
#ifndef _ELYSIUM_UART_DLL_H_
#define _ELYSIUM_UART_DLL_H_

#include "uart.h"

#ifdef __cplusplus
extern "C" {
#endif
  void elyUARTDLLRxInit(UARTDriver * uartp);
  void elyUARTDLLRxCharCB(UARTDriver * uartp, uint16_t c);
  void elyUARTDLLRxCB(UARTDriver * uartp);
  void elyUARTDLLStartTx(UARTDriver * uartp, uint8_t * buffer);
  void elyUARTDLLTxCB(UARTDriver * uartp);
  void elyUARTDLLTimeoutCB(GPTDriver * gptp);
  void elyUARTDLLRxHandleBuffer(void);
#ifdef __cplusplus
}
#endif
  

#endif
