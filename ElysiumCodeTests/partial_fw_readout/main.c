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

#include "ch.h"
#include "hal.h"
#include "hal_dma_lld.h"
#include "string.h"
#include "chbsem.h"

/* Disable watchdog because of lousy startup code in newlib */
static void __attribute__((naked, section(".crt_0042disable_watchdog"), used))
disable_watchdog(void) {
  WDTCTL = WDTPW | WDTHOLD;
}

const char * succeed_string = "SUCCESS\r\n\r\n";

uint16_t address;
uint8_t outbuf[4];
uint8_t inbuf[0x7E00] = {1};

I2CConfig I2CDB0_config = {
  400000,     /* bit rate */
};

BSEMAPHORE_DECL(i2c_sem, 0);

void end_cb(I2CDriver * i2cp, uint8_t * buffer, uint16_t n) {
  (void)(buffer);
  (void)(n);
  chSysLockFromISR();
  i2cMSP430XEndTransferI(i2cp);
  chBSemSignalI(&i2c_sem);
  chSysUnlockFromISR();
}

void addr_cb(I2CDriver * i2cp, uint8_t * buffer, uint16_t n) {
  (void)(buffer);
  (void)(n);
  chSysLockFromISR();
  i2cMSP430XStartReceiveI(i2cp, 0x51, 4, inbuf, end_cb);
  chSysUnlockFromISR();
}

/*
 * Thread 2.
 */
THD_WORKING_AREA(waThread1, 4096);
THD_FUNCTION(Thread1, arg) {

  (void)arg;
  
  /* Read from 0x10400 */
  outbuf[0] = 0x04;
  outbuf[1] = 0x00;
  /* Also read from 0x18200 */
  outbuf[2] = 0x82;
  outbuf[3] = 0x00;
  
  address = 0x0400;
  
  /*
   * Activate the serial driver 0 using the driver default configuration.
   */
  sdStart(&SD0, NULL);

  /* Activate the I2C driver B0 using its config */
  i2cStart(&I2CDB0, &I2CDB0_config);

  while (chnGetTimeout(&SD0, TIME_INFINITE)) {
    chThdSleepMilliseconds(2000);
    
    /* Test 4 - Long transfer */
    chThdSleepMilliseconds(2000);
    /* Issue a write to set the address */
    chSysLock();
    i2cMSP430XStartTransmitMSBI(&I2CDB0, 0x51, 2, 
        (uint8_t *)(&(address)), 
        addr_cb);
    /* wait for transaction to finish */
    chBSemWaitTimeoutS(&i2c_sem, TIME_INFINITE);
    chSysUnlock();
    

    
    
    chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    
    if (address == 0x0400) {
      address += 0x7C00;
    }
    else {
      address = 0x0400;
    }
  }
}

/*
 * Threads static table, one entry per thread. The number of entries must
 * match NIL_CFG_NUM_THREADS.
 */
THD_TABLE_BEGIN
  THD_TABLE_ENTRY(waThread1, "spi_test", Thread1, NULL)
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
