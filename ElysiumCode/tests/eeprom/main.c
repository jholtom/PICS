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

const char * start_msg  = "\r\n\r\nExecuting EEPROM test suite...\r\n";
const char * test_1_msg = "TEST 1: Standard\r\n";
const char * test_2_msg = "TEST 2: Exclusive\r\n";
const char * test_3_msg = "TEST 3: Exclusive DMA\r\n";
const char * test_4_msg = "TEST 4: Long transfer\r\n";
const char * test_5_msg = "TEST 5: Callback-driven\r\n";
const char * test_6_msg = "TEST 6: MSB-style\r\n";

const char * succeed_string = "SUCCESS\r\n\r\n";
const char * fail_string    = "FAILURE\r\n\r\n";

uint8_t inbuf[256];
uint8_t outbuf[256];

I2CConfig I2CDB0_config = {
  100000,     /* bit rate */
  0,          /* 10-bit address */
  7U          /* DMA index */
};

static BSEMAPHORE_DECL(callback_triggered, true);

static void rx_cb(I2CDriver * i2cp, uint8_t * buf) {
  (void)buf;
  /* Now be done */
  chSysLockFromISR();
  i2cMSP430XEndTransferI(i2cp);
  chBSemSignalI(&callback_triggered);
  chSysUnlockFromISR();
  
}

static void tx_cb(I2CDriver * i2cp, uint8_t * buf) {
  (void)buf;
  /* Now read back the numbers */
  chSysLockFromISR();
  i2cMSP430XStartReceiveI(i2cp, 0x50, 127, inbuf+127, rx_cb);
  chSysUnlockFromISR();
}

static void msb_cb(I2CDriver * i2cp, uint8_t * buf) {
  (void)buf;
  /* Write the numbers in MSB order */
  chSysLockFromISR();
  i2cMSP430XContinueTransmitMSBI(i2cp, 0x50, 127, outbuf+8, rx_cb);
  chSysUnlockFromISR();
}

/*
 * Thread 2.
 */
THD_WORKING_AREA(waThread1, 4096);
THD_FUNCTION(Thread1, arg) {

  (void)arg;
  
  /* Write to 0x4400 */
  outbuf[0] = 0x44;
  outbuf[1] = 0x00;
  /* Write 0xAA55AA55 */
  outbuf[2] = 0xAA;
  outbuf[3] = 0x55;
  outbuf[4] = 0xAA;
  outbuf[5] = 0x55;
  /* Write to 0x2000 */
  outbuf[6] = 0x20;
  outbuf[7] = 0x00;
  /* Write increasing numbers up to 127 */
  outbuf[8] = 1;
  outbuf[9] = 2;
  outbuf[10] = 3;
  outbuf[11] = 4;
  outbuf[12] = 5;
  outbuf[13] = 6;
  outbuf[14] = 7;
  outbuf[15] = 8;
  outbuf[16] = 9;
  outbuf[17] = 10;
  outbuf[18] = 11;
  outbuf[19] = 12;
  outbuf[20] = 13;
  outbuf[21] = 14;
  outbuf[22] = 15;
  outbuf[23] = 16;
  outbuf[24] = 17;
  outbuf[25] = 18;
  outbuf[26] = 19;
  outbuf[27] = 20;
  outbuf[28] = 21;
  outbuf[29] = 22;
  outbuf[30] = 23;
  outbuf[31] = 24;
  outbuf[32] = 25;
  outbuf[33] = 26;
  outbuf[34] = 27;
  outbuf[35] = 28;
  outbuf[36] = 29;
  outbuf[37] = 30;
  outbuf[38] = 31;
  outbuf[39] = 32;
  outbuf[40] = 33;
  outbuf[41] = 34;
  outbuf[42] = 35;
  outbuf[43] = 36;
  outbuf[44] = 37;
  outbuf[45] = 38;
  outbuf[46] = 39;
  outbuf[47] = 40;
  outbuf[48] = 41;
  outbuf[49] = 42;
  outbuf[50] = 43;
  outbuf[51] = 44;
  outbuf[52] = 45;
  outbuf[53] = 46;
  outbuf[54] = 47;
  outbuf[55] = 48;
  outbuf[56] = 49;
  outbuf[57] = 50;
  outbuf[58] = 51;
  outbuf[59] = 52;
  outbuf[60] = 53;
  outbuf[61] = 54;
  outbuf[62] = 55;
  outbuf[63] = 56;
  outbuf[64] = 57;
  outbuf[65] = 58;
  outbuf[66] = 59;
  outbuf[67] = 60;
  outbuf[68] = 61;
  outbuf[69] = 62;
  outbuf[70] = 63;
  outbuf[71] = 64;
  outbuf[72] = 65;
  outbuf[73] = 66;
  outbuf[74] = 67;
  outbuf[75] = 68;
  outbuf[76] = 69;
  outbuf[77] = 70;
  outbuf[78] = 71;
  outbuf[79] = 72;
  outbuf[80] = 73;
  outbuf[81] = 74;
  outbuf[82] = 75;
  outbuf[83] = 76;
  outbuf[84] = 77;
  outbuf[85] = 78;
  outbuf[86] = 79;
  outbuf[87] = 80;
  outbuf[88] = 81;
  outbuf[89] = 82;
  outbuf[90] = 83;
  outbuf[91] = 84;
  outbuf[92] = 85;
  outbuf[93] = 86;
  outbuf[94] = 87;
  outbuf[95] = 88;
  outbuf[96] = 89;
  outbuf[97] = 90;
  outbuf[98] = 91;
  outbuf[99] = 92;
  outbuf[100] = 93;
  outbuf[101] = 94;
  outbuf[102] = 95;
  outbuf[103] = 96;
  outbuf[104] = 97;
  outbuf[105] = 98;
  outbuf[106] = 99;
  outbuf[107] = 100;
  outbuf[108] = 101;
  outbuf[109] = 102;
  outbuf[110] = 103;
  outbuf[111] = 104;
  outbuf[112] = 105;
  outbuf[113] = 106;
  outbuf[114] = 107;
  outbuf[115] = 108;
  outbuf[116] = 109;
  outbuf[117] = 110;
  outbuf[118] = 111;
  outbuf[119] = 112;
  outbuf[120] = 113;
  outbuf[121] = 114;
  outbuf[122] = 115;
  outbuf[123] = 116;
  outbuf[124] = 117;
  outbuf[125] = 118;
  outbuf[126] = 119;
  outbuf[127] = 120;
  outbuf[128] = 121;
  outbuf[129] = 122;
  outbuf[130] = 123;
  outbuf[131] = 124;
  outbuf[132] = 125;
  outbuf[133] = 126;
  outbuf[134] = 127;

  /*
   * Activate the serial driver 0 using the driver default configuration.
   */
  sdStart(&SD0, NULL);

  /* Activate the I2C driver B0 using its config */
  i2cStart(&I2CDB0, &I2CDB0_config);

  while (chnGetTimeout(&SD0, TIME_INFINITE)) {
    chnWrite(&SD0, (const uint8_t *)start_msg, strlen(start_msg));
    chThdSleepMilliseconds(2000);
    
    /* Test 1 - Standard */
    chnWrite(&SD0, (const uint8_t *)test_1_msg, strlen(test_1_msg));
    chThdSleepMilliseconds(2000);
    outbuf[2] = 0xAA;
    outbuf[3] = 0x55;
    /* Write 0xAA55AA55 to address 0x4400 on an ST M24M01 */
    i2cMasterTransmitTimeout(&I2CDB0, 0x50, outbuf, 6, NULL, 0, TIME_INFINITE);
    
    while (I2CDB0.state != I2C_READY)
      ; /* wait for transaction to finish */
    /* Wait max required time for write cycle */
    chThdSleepMilliseconds(6);
    /* Read back 0xAA as random address read */
    i2cMasterTransmitTimeout(&I2CDB0, 0x50, outbuf, 2, inbuf, 1, TIME_INFINITE);
    while (I2CDB0.state != I2C_READY)
      ; /* wait for transaction to finish */
    /* Wait max required time for write cycle */
    chThdSleepMilliseconds(6);
    /* Read back 0x55AA55 as sequential current read */
    i2cMasterReceiveTimeout(&I2CDB0, 0x50, inbuf + 1, 3, TIME_INFINITE);
    while (I2CDB0.state != I2C_READY)
      ; /* wait for transaction to finish */
    if ((inbuf[0] != 0xAA) ||
        (inbuf[1] != 0x55) ||
        (inbuf[2] != 0xAA) ||
        (inbuf[3] != 0x55)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    
    /* Test 2 - Exclusive */
    chnWrite(&SD0, (const uint8_t *)test_2_msg, strlen(test_2_msg));
    chThdSleepMilliseconds(2000);
    outbuf[2] = 0xAB;
    outbuf[3] = 0x56;
    /* Acquire bus */
    i2cAcquireBus(&I2CDB0);
    /* Write 0xAB56 to address 0x4400 on an ST M24M01 */
    i2cMasterTransmitTimeout(&I2CDB0, 0x50, outbuf, 4, NULL, 0, TIME_INFINITE);
    while (I2CDB0.state != I2C_READY)
      ; /* wait for transaction to finish */
    chThdSleepMilliseconds(6);
    /* Read back 0xAB as random address read */
    i2cMasterTransmitTimeout(&I2CDB0, 0x50, outbuf, 2, inbuf, 1, TIME_INFINITE);
    while (I2CDB0.state != I2C_READY)
      ; /* wait for transaction to finish */
    /* Read back 0x56 as sequential current read */
    i2cMasterReceiveTimeout(&I2CDB0, 0x50, inbuf + 1, 1, TIME_INFINITE);
    while (I2CDB0.state != I2C_READY)
      ; /* wait for transaction to finish */
    /* Release bus */
    i2cReleaseBus(&I2CDB0);
    
    if ((inbuf[0] != 0xAB) ||
        (inbuf[1] != 0x56)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }

    /* Test 3 - Exclusive DMA */
    chnWrite(&SD0, (const uint8_t *)test_3_msg, strlen(test_3_msg));
    chThdSleepMilliseconds(2000);
    outbuf[2] = 0xAC;
    outbuf[3] = 0x57;
    /* Reconfigure for exclusive DMA on channel 0 */
    I2CDB0_config.dma_index = 0;
    i2cStart(&I2CDB0, &I2CDB0_config);
    /* Write 0xAC57 to address 0x4400 on an ST M24M01 */
    i2cMasterTransmitTimeout(&I2CDB0, 0x50, outbuf, 4, NULL, 0, TIME_INFINITE);
    while (I2CDB0.state != I2C_READY)
      ; /* wait for transaction to finish */
    chThdSleepMilliseconds(6);
    /* Read back 0xAC as random address read */
    i2cMasterTransmitTimeout(&I2CDB0, 0x50, outbuf, 2, inbuf, 1, TIME_INFINITE);
    while (I2CDB0.state != I2C_READY)
      ; /* wait for transaction to finish */
    /* Read back 0x57 as sequential current read */
    i2cMasterReceiveTimeout(&I2CDB0, 0x50, inbuf + 1, 1, TIME_INFINITE);
    while (I2CDB0.state != I2C_READY)
      ; /* wait for transaction to finish */
    
    /* Revert DMA situation */
    I2CDB0_config.dma_index = 7U;
    i2cStart(&I2CDB0, &I2CDB0_config);
    
    if ((inbuf[0] != 0xAC) ||
        (inbuf[1] != 0x57)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }

    /* Test 4 - Long transfer */
    chnWrite(&SD0, (const uint8_t *)test_4_msg, strlen(test_4_msg));
    chThdSleepMilliseconds(2000);
    /* Write increasing numbers to address 0x2000 on an ST M24M01 */
    i2cMasterTransmitTimeout(&I2CDB0, 0x50, outbuf+6, 129, NULL, 0, TIME_INFINITE);
    while (I2CDB0.state != I2C_READY)
      ; /* wait for transaction to finish */
    chThdSleepMilliseconds(6);
    /* Read back increasing numbers */
    i2cMasterTransmitTimeout(&I2CDB0, 0x50, outbuf+6, 2, inbuf, 127, TIME_INFINITE);
    while (I2CDB0.state != I2C_READY)
      ; /* wait for transaction to finish */
    
    for (int i = 0; i < 127; i++) {
      if (inbuf[i] != i+1) {
        chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
        break;
      }
    }
    chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    
    /* Test 5 - Callback-driven */
    chnWrite(&SD0, (const uint8_t *)test_5_msg, strlen(test_5_msg));
    chThdSleepMilliseconds(2000);
    /* Read increasing numbers from address 0x2000 on an ST M24M01 */
    chSysLock();
    /* Address first */
    i2cMSP430XStartTransmitI(&I2CDB0, 0x50, 2, outbuf+6, tx_cb);
    chSysUnlock();
    
    chBSemWait(&callback_triggered);
    while (I2CDB0.state != I2C_READY)
      ; /* wait for last byte */
    
    for (int i = 127; i < 254; i++) {
      if (inbuf[i] != i-126) {
        chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
        break;
      }
    }
    chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    
    /* Test 6 - MSB-style */
    chnWrite(&SD0, (const uint8_t *)test_6_msg, strlen(test_6_msg));
    chThdSleepMilliseconds(2000);
    /* Write decreasing numbers to address 0x2000 on an ST M24M01 */
    chSysLock();
    /* Address first */
    uint16_t address = 0x2000;
    i2cMSP430XStartTransmitMSBI(&I2CDB0, 0x50, 2, (uint8_t *)(&address), msb_cb);
    chSysUnlock();
    
    chBSemWait(&callback_triggered);
    while (I2CDB0.state != I2C_READY)
      ; /* wait for last byte */
    
    chThdSleepMilliseconds(6);
    /* Read back increasing numbers */
    i2cMasterTransmitTimeout(&I2CDB0, 0x50, outbuf+6, 2, inbuf, 127, TIME_INFINITE);
    
    while (I2CDB0.state != I2C_READY)
      ; /* wait for transaction to finish */
    
    for (int i = 0; i < 127; i++) {
      if (inbuf[i] != 127-i) {
        chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
        break;
      }
    }
    chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    
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
