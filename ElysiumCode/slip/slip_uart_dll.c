
#include "slip_uart_dll.h"

static const uint8_t SLIP_ESC_ESC_ESC[2] = {SLIP_ESC, SLIP_ESC_ESC};
static const uint8_t SLIP_ESC_ESC_END[2] = {SLIP_ESC, SLIP_ESC_END};

typedef struct {
  uint8_t buf[2][SLIP_RX_BUF_LEN];
  uint16_t write_idx;
  uint16_t read_idx;
} double_buff;

static uint8_t * dbuf_get_write(double_buff * rb) {
  return rb->buf[rb->write_idx++ & 1];
}

static uint8_t * dbuf_get_read(double_buff * rb) {
  return rb->buf[rb->read_idx++ & 1];
}

static void dbuf_init(double_buff * rb) {
  chDbgAssert(SLIP_RX_BUF_LEN <= ((UINT16_MAX - 1) / 2),
      "RX buffer length too long");

  rb->write_idx = 0;
  rb->read_idx = 0;
}

static double_buff rx_buf;
static slip_uart_states_t rx_state = ELY_SLIP_RESET;
static PERSIST uint8_t * rx_active_buffer = NULL;
static uint8_t * rx_write_ptr;
static size_t bytes_available;

static uint8_t * tx_active_buffer;
static size_t tx_n;
static uint8_t * tx_read_ptr;
static size_t next_cnt;
static const uint8_t * next_tx_ptr;
static slip_uart_states_t tx_state = ELY_SLIP_NOT_ESCAPED;

static thread_t * uart_thd;

void calc_next_buff(void) {

  switch (tx_state) {
    case ELY_SLIP_NOT_ESCAPED:
      next_tx_ptr = tx_read_ptr;
      next_cnt = 0;
      while (next_cnt != tx_n &&
          *tx_read_ptr != SLIP_END &&
          *tx_read_ptr != SLIP_ESC) {
        next_cnt++;
        tx_read_ptr++;
      }
      tx_state = ELY_SLIP_ESCAPED;
      if (next_cnt > 0) {
        break;
      }
    case ELY_SLIP_ESCAPED:
      {
        switch(*tx_read_ptr++) {
          case SLIP_ESC:
            next_tx_ptr = SLIP_ESC_ESC_ESC;
            break;
          case SLIP_END:
            next_tx_ptr = SLIP_ESC_ESC_END;
            break;
          default:
            chDbgAssert(false, "state machine error");
            /* if we decrement tx_n here, it cancels out the
             * increment below and all that happens is we send
             * two bytes */
            tx_n--;
            break;
        }
        next_cnt = 2;
        tx_n++; /* need another byte - sending 2 for 1 */
        tx_state = ELY_SLIP_NOT_ESCAPED;
      }
      break;
    default:
      chDbgAssert(false, "State machine error");
      /* ACTUALLY can't happen */
      break;
  }

}

void elyUARTDLLTxCB(UARTDriver * uartp) {
  static bool done = false;
  static const uint8_t END = SLIP_END;

  if (done) {
    done = false;
    return;
  }

  chSysLockFromISR();
  uartStartSendI(uartp, next_cnt, next_tx_ptr);
  chSysUnlockFromISR();

  if (tx_n == 0) {
    done = true;
    return;
  }

  tx_n -= next_cnt;

  if (tx_n == 0) {
    chSysLockFromISR();
    elyNLFreeBufferCheckedI(tx_active_buffer);
    chSysUnlockFromISR();
    next_tx_ptr = &END;
    next_cnt = 1;
    return;
  }

  calc_next_buff();
}

void elyUARTDLLStartTx(UARTDriver * uartp, uint8_t * buffer) {
  static const uint8_t END = SLIP_END;
  /* Calculate the length of the packet - store it in n */
  tx_n = elyNLGetLength(buffer);
  chDbgAssert(tx_n != 0 && tx_n <= elyNLMaxLen, "invalid packet length");
  /* The below should be impossible and should be deleted if Assert is Abort */
  if (tx_n == 0 || tx_n > elyNLMaxLen) {
      chDbgAssert(false, "Impossible case of output being longer than supported");
      elyNLFreeBufferChecked(buffer);
  }
  tx_read_ptr = buffer;
  tx_active_buffer = buffer;
  tx_state = ELY_SLIP_NOT_ESCAPED;

  calc_next_buff();

  chSysLock();
  uartStartSendI(uartp, next_cnt, next_tx_ptr);

  tx_n -= next_cnt;

  if (tx_n == 0) {
    elyNLFreeBufferCheckedI(tx_active_buffer);
    next_tx_ptr = &END;
    next_cnt = 1;
    chSysUnlock();
    return;
  }

  calc_next_buff();
  chSysUnlock();

}

void elyUARTDLLRxInit(UARTDriver * uartp) {
  (void)(uartp);

  uart_thd = chThdGetSelfX();
}

static void handle_buffer(size_t bytes_available, const uint8_t * buffer) {
  static size_t rx_n;
  static bool header;
  /* We have now received 256 bytes of data somewhere in rx_buf */

  const uint8_t * end = buffer + bytes_available;
  const uint8_t * cp = buffer;

  while (cp < end) {
    uint8_t c = *cp;

    if (rx_state == ELY_SLIP_RESET) {
      /* allocate a working buffer from the main pool */
      /* TODO this section is dangerous re: resets. Figure out how to fix it. */
      if (NULL == rx_active_buffer) {
        rx_active_buffer = elyNLGetBuffer();
      }
      /* end TODO */
      if (NULL == rx_active_buffer) {
        /* TODO signal a buffer overflow here */
        return;
      }
      rx_write_ptr = rx_active_buffer;
      rx_n = elyNLHeaderLen;
      rx_state = ELY_SLIP_NOT_ESCAPED;
      header = true;
    }

    switch (rx_state) {
      case ELY_SLIP_ESCAPED:
        {
          switch(c) {
            case SLIP_ESC_END:
              *rx_write_ptr++ = SLIP_END;
              break;
            case SLIP_ESC_ESC:
              *rx_write_ptr++ = SLIP_ESC;
              break;
            default:
              rx_state = ELY_SLIP_RESET;
              continue;
          }
          rx_n--;
          rx_state = ELY_SLIP_NOT_ESCAPED;
          cp++;
        }
        break;
      case ELY_SLIP_NOT_ESCAPED:
        {
          switch(c) {
            case SLIP_ESC:
              if (rx_n == 0) {
                rx_state = ELY_SLIP_RESET;
                continue;
              }
              else {
                rx_state = ELY_SLIP_ESCAPED;
                cp++;
              }
              break;
            case SLIP_END:
              if (rx_n == 0) {
                elyNLRouteUART(rx_active_buffer);
                rx_active_buffer = NULL;
              }
              rx_state = ELY_SLIP_RESET;
              cp++;
              continue;
            default:
              if (rx_n == 0) {
                rx_state = ELY_SLIP_RESET;
                continue;
              }
              else {
                *rx_write_ptr++ = c;
                rx_n--;
                cp++;
              }
          }
        }
        break;
      default:
        chDbgAssert(false, "shouldn't happen - unreachable code");
        /* ACTUALLY can't happen */
        break;
    }

    if (rx_n == 0 && header) { /* and we haven't reset */
      if (elyNLValidate(rx_active_buffer)) {
        rx_n = elyNLGetLength(rx_active_buffer) - elyNLHeaderLen;
        header = false;
      }
      else {
        /* junk - reset state machine */
        rx_state = ELY_SLIP_RESET;
      }
    }
  }

  if (bytes_available < SLIP_RX_BUF_LEN) {
    if (NULL != rx_active_buffer) {
      elyNLFreeBuffer(rx_active_buffer);
      rx_active_buffer = NULL;
    }
    rx_state = ELY_SLIP_RESET;
  }

}

void elyUARTDLLRxHandleBuffer() {
  handle_buffer(bytes_available, dbuf_get_read(&rx_buf));
}

/* TODO we might be able to refactor this out by signalling from
 * the callbacks and using an S-class timeout API */
void elyUARTDLLTimeoutCB(GPTDriver * gptp) {
  (void)(gptp);
  chSysLockFromISR();
  size_t bytes_missed = uartStopReceiveI(&ELY_UART);

  /* Count the number of bytes that are available */
  bytes_available = SLIP_RX_BUF_LEN - bytes_missed;
  /* Signal the thread that a buffer is ready */
  chEvtSignalI(uart_thd, UARTRxBufferReady);
  chSysUnlockFromISR();

}

void elyUARTDLLRxCharCB(UARTDriver * uartp, uint16_t c) {
  chDbgAssert(c < 0x100, "invalid char");
  dbuf_init(&rx_buf);

  uint8_t * buf = dbuf_get_write(&rx_buf);
  buf[0] = (uint8_t)(c);
  chSysLockFromISR();
  uartStartReceiveI(uartp, SLIP_RX_BUF_LEN-1, buf+1);
  gptStartOneShotI(&uart_gpt, UART_TIMEOUT_MS);


  chSysUnlockFromISR();
}

void elyUARTDLLRxCB(UARTDriver * uartp) {
  /* Get the second batch of data into reception ASAP */
  chSysLockFromISR();
  uartStartReceiveI(uartp, SLIP_RX_BUF_LEN, dbuf_get_write(&rx_buf));
  gptStopTimerI(&uart_gpt);
  gptStartOneShotI(&uart_gpt, UART_TIMEOUT_MS);
  /* All the bytes are available */
  bytes_available = SLIP_RX_BUF_LEN;
  /* Signal the thread that a buffer is ready */
  chEvtSignalI(uart_thd, UARTRxBufferReady);

  chSysUnlockFromISR();
}

