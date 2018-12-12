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
  NULL,         /* callback */
  LINE_SS_CONFIG_B,           /* config slave select line */
  1000000,               /* max data rate */
  MSP430X_SPI_BO_MSB,   /* bit order */
  MSP430X_SPI_DS_EIGHT, /* data size */
  0                    /* SPI mode */
};

static uint8_t sx1212_read_register(SPIDriver * spip, uint8_t reg) {
  static uint8_t buf;
  
  osalDbgAssert(!(reg & 0x1F), "register address out of bounds");
  
  SPIDA1_config.ss_line = LINE_SS_CONFIG_B;
  SPIDA1_config.spi_mode = 0;
  spiStart(spip, &SPIDA1_config);
  spiSelect(spip);
  buf = (0x40 | (reg << 1));
  spiPolledExchange(spip, buf); /* Send "read register" comand w/ addr */
  buf = spiPolledExchange(spip, buf); /* Receive result */
  /*spiSend(spip, 1, &buf); /* Send "read register" comand w/ addr */
  /*  while (spip->state != SPI_READY)
      ; /* wait for transaction to finish */
  /*spiReceive(spip, 1, &buf); /* Receive result */
  /*  while (spip->state != SPI_READY)
      ; /* wait for transaction to finish */
  spiUnselect(spip);
  return buf;
}

static uint8_t sx1278_read_register(SPIDriver * spip, uint8_t reg) {
  static uint8_t buf;
  
  osalDbgAssert(!(reg & 0x7F), "register address out of bounds");
  
  SPIDA1_config.ss_line = LINE_SX1278_SS_B;
  SPIDA1_config.spi_mode = 0;
  spiStart(spip, &SPIDA1_config);
  spiSelect(spip);
  buf = reg;
  spiSend(spip, 1, &buf); /* Send "read register" comand w/ addr */
    while (spip->state != SPI_READY)
      ; /* wait for transaction to finish */
  spiReceive(spip, 1, &buf); /* Receive result */
    while (spip->state != SPI_READY)
      ; /* wait for transaction to finish */
  spiUnselect(spip);
  return buf;
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
  spiStart(&SPIDA1, &SPIDA1_config);

  chnWrite(&SD0, (const uint8_t *)start_msg, strlen(start_msg));
  while (chnGetTimeout(&SD0, TIME_INFINITE)) {
    uint8_t buf;
    
    chThdSleepMilliseconds(2000);
    
    /* Test 1 - Read an SX1212 register */
    chnWrite(&SD0, (const uint8_t *)test_1_msg, strlen(test_1_msg));
    buf = sx1212_read_register(&SPIDA1, 0x10); /* Filter register */
    if (buf == 0xA3) { /* Reset value 10100011 */
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
      
    /* Test 2 - Read an SX1278 register */
    for (int i = 0; i < 5; i++) {
    chnWrite(&SD0, (const uint8_t *)test_2_msg, strlen(test_2_msg));
    buf = sx1278_read_register(&SPIDA1, 0x42); /* Silicon version register */
    if (buf == 0x12) { /* From datasheet */
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    }
      
    chThdSleepMilliseconds(2000);
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
