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
#include "string.h"

#include "sx1212.h"

/* Disable watchdog because of lousy startup code in newlib */
static void __attribute__((naked, section(".crt_0042disable_watchdog"), used))
disable_watchdog(void) {
  WDTCTL = WDTPW | WDTHOLD;
}

const char * start_msg  = "\r\n\r\nBeginning SX1212 test...\r\n";

char instring[256];
char outstring[256];

static SPIConfig SPIDA1_config = {
  NULL, /* callback */
  LINE_SS_CONFIG_B, /* slave select line */
  25000, /* data rate */
  MSP430X_SPI_BO_MSB, /* bit order */
  MSP430X_SPI_DS_EIGHT, /* data size */
  0 /* SPI mode */
};

static SPIConfig SPIDA1_data = {
  NULL, /* callback */
  LINE_SS_DATA_B, /* slave select line */
  25000, /* data rate */
  MSP430X_SPI_BO_MSB, /* bit order */
  MSP430X_SPI_DS_EIGHT, /* data size */
  0 /* SPI mode */
};

static sx1212_packet_config_t packet_config = {
  Fixed, /* packet format */
  0, /* whitening */
  0, /* manchester encoding */
  0, /* crc */
  0, /* addressing */
  None, /* broadcast mode */
  1 /* crc autoclear */
};

static SX1212Config config = {
  &SPIDA1,
  &SPIDA1,
  &SPIDA1_config,
  &SPIDA1_data,
  9600, /* bitrate */
  434e6, /* center frequency- 434 MHz */
  4800, /* Frequency deviation - m = 1 */
  200e3, /* RX bandwidth */
  0x5EA6C11D, /* sync word */
  3, /* sync tolerance */
  LINE_SX1212_RESET, /* reset line */
  &packet_config, /* packet_config */
  0, /* Address */
  64 /* Length */
};

static SX1212Driver SX1212D1;

static int8_t rssi;

/*
 * Thread 2.
 */
THD_WORKING_AREA(waThread1, 4096);
THD_FUNCTION(Thread1, arg) {

  (void)arg;

  /*
   * Activate the serial driver 0 using the driver default configuration.
   */
  sdStart(&SD0, NULL);
  
  /* Initialize the SX1212 driver */
  sx1212ObjectInit(&SX1212D1);
  
  /* Start the SX1212 driver */
  sx1212Start(&SX1212D1, &config);

  /*chnWrite(&SD0, (const uint8_t *)start_msg, strlen(start_msg));*/
  /* Set it to RX mode so RSSI is valid */
  sx1212StartReceive(&SX1212D1, TIME_INFINITE);
  
  /* FOR DEBUGGING */
  sx1212CLKOUT(&SX1212D1, 0x0F);
  
  while (chnGetTimeout(&SD0, TIME_INFINITE)) {
    rssi = sx1212RSSI(&SX1212D1);
    chnWrite(&SD0, (uint8_t *)(&rssi), 1);
    
    chThdSleepMilliseconds(1000);
  }
}

/*
 * Threads static table, one entry per thread. The number of entries must
 * match NIL_CFG_NUM_THREADS.
 */
THD_TABLE_BEGIN
  THD_TABLE_ENTRY(waThread1, "sx1212_test", Thread1, NULL)
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

  /* This is now the idle thread loop, you may perform here a low priority
     task but you must never try to sleep or wait in this loop. Note that
     this tasks runs at the lowest priority level so any instruction added
     here will be executed after all other tasks have been started.*/
  while (true) {
  }
}
