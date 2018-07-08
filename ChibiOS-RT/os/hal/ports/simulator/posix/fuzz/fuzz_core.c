#include "hal.h"
#include "fuzz_core.h"

#include <stdio.h>

fuzz_system_state_t * fuzz_state;

void fuzz_feed_data(fuzz_input_t * input) {
  uint8_t * buffer = input->data;
  fuzz_state->timer = input->timer_pattern;
#if POSIX_GPT_USE_GPT1
  GPTD1.state = input->gpt1_pattern;
#endif
#if POSIX_I2C_USE_I2C1
  I2CD1.buffer.start = buffer;
  I2CD1.buffer.len = input->i2c1_input_len;
  I2CD1.buffer.current = 0;
  I2CD1.buffer.name = "I2CD1";
#endif
#if POSIX_UART_USE_UART1
  UARTD1.buffer.start = buffer;
  UARTD1.buffer.len = input->uart1_input_len;
  UARTD1.buffer.current = 0;
  UARTD1.buffer.name = "UARTD1";
#endif
#if POSIX_SPI_USE_SPI1
  SPID1.buffer.start = buffer;
  SPID1.buffer.len = input->spi1_input_len;
  SPID1.buffer.current = 0;
  SPID1.buffer.name = "SPID1";
#endif
}

bool fuzz_next_timer_bit(void) {
  return fuzz_next_bit(&fuzz_state->timer);
}

bool fuzz_next_timeout_bit(void) {
  return fuzz_next_bit(&fuzz_state->timeout);
}

bool fuzz_next_bit(uint64_t * state) {
  // really crappy RNG
  *state ^= 0xF136352; // chosen by mashing keyboard
  *state = *state * 5;
  *state ^= *state << 32;
  *state ^= *state >> 32;
  return (*state) & 1;
}

uint8_t fuzz_next_byte(fuzz_buffer_state_t * buffer, bool allow_out_of_data) {
  if (buffer->current == buffer->len) {
    fprintf(stderr, "Out of data in buffer %s\n", buffer->name);
    if (allow_out_of_data) {
      exit(0);
    } else {
      exit(1);
    }
  }
  return buffer->start[buffer->current++];
}

void fuzz_check_for_out_of_data(fuzz_buffer_state_t * buffer) {
  if (buffer->current == buffer->len) {
    fprintf(stderr, "Out of data in buffer %s\n", buffer->name);
    exit(1);
  }
}
