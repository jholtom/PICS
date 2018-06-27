/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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

#include "hal.h"
#include "ch.h"
#include <string.h>

const char * start_msg  = "\r\n\r\nExecuting quick SPI test...\r\n";
const char * test_1_msg = "TEST 1: Read a register from SX1212\r\n";
const char * test_2_msg = "TEST 2: Read a register from SX1278\r\n";

const char * succeed_string = "SUCCESS\r\n\r\n";
const char * fail_string    = "FAILURE\r\n\r\n";

SPIConfig SPIDA1_config = {
  spi_callback,         /* callback */
  LINE_SX1278_SS_B,           /* hardware slave select line */
  25000,               /* data rate */
  MSP430X_SPI_BO_MSB,   /* bit order */
  MSP430X_SPI_DS_EIGHT, /* data size */
  0,                    /* SPI mode */
  0xFFU,                /* no exclusive TX DMA */
  0xFFU                 /* no exclusive RX DMA */
};

static uint8_t sx1212_read_register(uint8_t reg) {
  static uint8_t buf;
  
  osalDbgAssert(!(reg & 0x1F), "register address out of bounds");
  
  buf = (0x40 | (reg << 1));
  
  
  return buf;
}

static uint8_t sx1278_read_register(uint8_t reg) {
  static uint8_t buf;
  
  osalDbgAssert(!(reg & 0x7F), "register address out of bounds");
  
  buf = reg;
  
  SPIDA1_config.ss_line = LINE_SX1278_SS_B;
  spiStart(&SPIDA1, &SPIDA1_config);
  
  spiSelect(&SPIDA1);
  spiPolledExchange(&SPIDA1, buf);
  buf = spiPolledExchange(0xFF);
  spiUnselect(&SPIDA1);
  
  return buf;
}

static void sx1212_reset(void) {
  palSetLineMode(LINE_SX1212_RESET, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLine(LINE_SX1212_RESET);
  chThdSleepMilliseconds(1);
  palClearLine(LINE_SX1212_RESET);
  palSetLineMode(LINE_SX1212_RESET, PAL_MODE_INPUT_PULLDOWN);
  chThdSleepMilliseconds(5);
}

static void sx1278_reset(void) {
  palSetLineMode(LINE_SX1278_RESET_B, PAL_MODE_OUTPUT_PUSHPULL);
  palClearLine(LINE_SX1278_RESET_B);
  chThdSleepMilliseconds(1);
  palSetLine(LINE_SX1278_RESET_B);
  palSetLineMode(LINE_SX1212_RESET, PAL_MODE_INPUT_PULLUP);
  chThdSleepMilliseconds(5);
}

/*
 * Thread 2.
 */
THD_WORKING_AREA(waThread2, 2048);
THD_FUNCTION(Thread2, arg) {

  (void)arg;

  /*
   * Activate the serial driver 0 using the driver default configuration.
   */
  sdStart(&SD0, NULL);

  chnWrite(&SD0, (const uint8_t *)start_msg, strlen(start_msg));
  
  sx1278_reset();
  
  while (1) {
    uint8_t buf;
    
    chThdSleepMilliseconds(2000);
    
    /* Test 2 - Read an SX1278 register */
    chnWrite(&SD0, (const uint8_t *)test_2_msg, strlen(test_2_msg));
    buf = sx1278_read_register(0x42); /* Silicon version register */
    if (buf == 0x12) { /* From datasheet */
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
  }
}

/*
 * Threads static table, one entry per thread. The number of entries must
 * match NIL_CFG_NUM_THREADS.
 */
THD_TABLE_BEGIN
  THD_TABLE_ENTRY(waThread2, "hello", Thread2, NULL)
THD_TABLE_END

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  WDTCTL = WDTPW | WDTHOLD;
  

  halInit();
  chSysInit();
  dmaInit();
  
  /* This is now the idle thread loop, you may perform here a low priority
     task but you must never try to sleep or wait in this loop. Note that
     this tasks runs at the lowest priority level so any instruction added
     here will be executed after all other tasks have been started.*/
  while (true) {
  }
}
