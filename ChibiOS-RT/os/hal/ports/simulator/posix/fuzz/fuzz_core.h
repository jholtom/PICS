#ifndef FUZZ_CORE_H
#define FUZZ_CORE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct fuzz_input {
  uint64_t timer_pattern;
  uint64_t timeout_pattern;
#ifdef POSIX_GPT_USE_GPT1
  uint64_t gpt1_pattern;
#endif
#if POSIX_I2C_USE_I2C1
  uint16_t i2c1_input_len;
#endif
#if POSIX_UART_USE_UART1
  uint16_t uart1_input_len;
#endif
#if POSIX_SPI_USE_SPI1
  uint16_t spi1_input_len;
#endif
  uint8_t data[];
} fuzz_input_t;

typedef struct fuzz_buffer_state {
  uint8_t * start;
  size_t len;
  size_t current;
  const char * name;
} fuzz_buffer_state_t;

typedef struct fuzz_system_state {
  // RNG state for timer
  uint64_t timer;
  // RNG state for timeout
  uint64_t timeout;
} fuzz_system_state_t;

extern fuzz_system_state_t * fuzz_state;

void fuzz_feed_data(fuzz_input_t * new_input);
bool fuzz_next_timer_bit(void);
bool fuzz_next_timeout_bit(void);
bool fuzz_next_bit(uint64_t * state);
uint8_t fuzz_next_byte(fuzz_buffer_state_t *buffer, bool allow_out_of_data);
void fuzz_check_for_out_of_data(fuzz_buffer_state_t * buffer);

#endif // FUZZ_CORE_H
