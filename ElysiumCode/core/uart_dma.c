
#include "uart.h"

static PERSIST msg_t uart_buffer[elyNLMaxSlots + elyFWMaxSlots];
static PERSIST MAILBOX_DECL(uart_mbox, uart_buffer, elyNLMaxSlots + elyFWMaxSlots);
static PERSIST uint8_t * tx_active_buffer = NULL;

static uart_events_t UARTEvtMask;
static eventmask_t events;

static thread_t * uart_thd;

void elyUARTCfgMarkDirtyI() {
  chDbgAssert(chThdGetSelfX() != uart_thd, "can't set your own config dirty");
  chEvtSignalI(uart_thd, UARTConfigUpdated);
}

void elyUARTCfgMarkDirty() {
  chDbgAssert(chThdGetSelfX() != uart_thd, "can't set your own config dirty");
  chEvtSignal(uart_thd, UARTConfigUpdated);
}

msg_t elyUARTPostI(uint8_t * buffer) {
  msg_t msg;
  msg = chMBPostI(&uart_mbox, (msg_t)(buffer));
  if (MSG_OK == msg) {
    chEvtSignalI(uart_thd, UARTBufferPosted);
  }
  else {
    /* don't try to handle it if we fail */
    /* TODO signal an error here */
    chDbgAssert(false, "UART msg failed to Post");
  }
  return msg;
}

msg_t elyUARTPost(uint8_t * buffer, systime_t timeout) {
  msg_t msg;
  chSysLock();
  /* timeout for safety, handle at point of call */
  msg = chMBPostS(&uart_mbox, (msg_t)(buffer), timeout);
  chEvtSignalI(uart_thd, UARTBufferPosted);
  chSysUnlock();
  return msg;
}

void elyUARTRxBufReadyI() {
  chEvtSignalI(uart_thd, UARTRxBufferReady);
}

void send_complete_cb(UARTDriver * uartp) {
  static volatile unsigned count = 0;
  (void)(uartp);
  chSysLockFromISR();
  elyNLFreeBufferCheckedI(tx_active_buffer);
  /* Null out the active buffer so we'll get a new one */
  tx_active_buffer = NULL;
  /* Allow us to handle config and buffer events again */
  UARTEvtMask |= (UARTConfigUpdated | UARTBufferPosted);
  /* Event mask modified, need to kick the thread */
  if (!NIL_THD_IS_READY(uart_thd)) {
    /* Reset this because we might have had more than one in the mailbox */
    if (chMBGetUsedCountI(&uart_mbox) > 0) {
      events |= UARTBufferPosted;
    }
    chSchReadyI(uart_thd, MSG_RESET);
  }
  chSysUnlockFromISR();
  count++;
}

/* Using UARTDA1 */
UARTConfig __attribute__((section(".persistent"))) A1_cfg = {
  elyUARTDLLTxCB, /* TX callback - DMA complete */
  send_complete_cb, /* TX callback - transmission complete */
  elyUARTDLLRxCB, /* RX callback - DMA complete  - SLIP DLL responsibility */
  elyUARTDLLRxCharCB, /* RX single-char callback - SLIP DLL responsibility */
  NULL, /* Error callback */
  UART_BAUD, /* Baud rate */
  MSP430X_UART_PARITY_NONE, /* Parity */
  MSP430X_UART_BO_LSB, /* Bit order */
  MSP430X_UART_DS_EIGHT, /* Data size */
  MSP430X_UART_ONE_STOP, /* Single stop bit */
  0, /* Autobaud */
  8,
  8
};

static const GPTConfig uart_timer_cfg = {
  1000, /* milliseconds */
  elyUARTDLLTimeoutCB
};

THD_WORKING_AREA(waUARTThd, 512);
THD_FUNCTION(UARTThd, arg) {
  (void)arg;
  
  /* Store a thread pointer for later use */
  uart_thd = chThdGetSelfX();
  
  /* Set the kinds of events we can handle */
  UARTEvtMask = (UARTConfigUpdated | UARTBufferPosted | UARTRxBufferReady);
  
  /* Get events into a consistent state with mailbox */
  chSysLock();
  if (tx_active_buffer != NULL || chMBGetUsedCountI(&uart_mbox) > 0) {
    events |= UARTBufferPosted;
  }
  chSysUnlock();
  
  /* Require configuration load */
  events |= UARTConfigUpdated;
  
  /* Initialize the timeout timer */
  gptStart(&uart_gpt, &uart_timer_cfg);
  
  /* Initialize the RX state machine */
  elyUARTDLLRxInit(&ELY_UART);
  
  msg_t result;
  while (true) {
    if (events & UARTConfigUpdated) {
      /* Update config based on register values */
      chSysLock();
      /* Parity */
      A1_cfg.parity = bank0p[RegUARTParams] & 0x03;
      /* Stop bits */
      A1_cfg.stop_bits = ((bank0p[RegUARTParams] >> 2) & 0x01);
      /* Autobaud */
      A1_cfg.autobaud = ((bank0p[RegUARTParams] >> 3) & 0x01);
      /* Baud rate */
      A1_cfg.baud = ( ((uint32_t)(bank0p[RegUARTBaudLsb])) |
                      ((uint32_t)(bank0p[RegUARTBaudLmb]) << 8) |
                      ((uint32_t)(bank0p[RegUARTBaudHmb]) << 16) |
                      ((uint32_t)(bank0p[RegUARTBaudMsb]) << 24) );
      
      chSysUnlock();
      /* Re-initialize the UART */
      uartStart(&ELY_UART, &A1_cfg);
      events &= ~UARTConfigUpdated;
    }
    if (events & UARTBufferPosted) {
      if (NULL == tx_active_buffer) {
        /* Get the buffer */
        chSysLock();
        result = chMBFetchS(&uart_mbox, (msg_t *)(&tx_active_buffer), TIME_IMMEDIATE);
        if (result != MSG_OK) {
          events &= ~UARTBufferPosted;
          chSysUnlock();
          /* Skip to the next loop */
          continue;
        }
        chSysUnlock();
      }
        
      /* Otherwise we already have a valid buffer to post */
      elyUARTDLLStartTx(&ELY_UART, tx_active_buffer);
      
      /* Don't handle config events or buffer events until transmission ends */
      UARTEvtMask &= ~(UARTConfigUpdated | UARTBufferPosted);
      
      events &= ~UARTBufferPosted; /* This will be re-set from the callback */
    }
    if (events & UARTRxBufferReady) {
      
      elyUARTDLLRxHandleBuffer();
      events &= ~UARTRxBufferReady; /* Again, re-set from a callback */
    }
    /* Get next events */
    chSysLock();
    events |= chEvtWaitAnyTimeoutS(UARTEvtMask, TIME_INFINITE);
    chSysUnlock();
  }
}

