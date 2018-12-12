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
#include "crc_x25.h"
#include "crc_sdlp.h"

/* Disable watchdog because of lousy startup code in newlib */
static void __attribute__((naked, section(".crt_0042disable_watchdog"), used))
disable_watchdog(void) {
  WDTCTL = WDTPW | WDTHOLD;
}

const char * start_msg  = "\r\n\r\nExecuting CRC test suite...\r\n";
const char * test_1_msg = "TEST 1: X25 Check\r\n";
const char * test_2_msg = "TEST 2: X25 Examples\r\n";
const char * test_3_msg = "TEST 3: CCSDS Check\r\n"; /* 29b1 */
const char * test_4_msg = "TEST 4: CCSDS Examples\r\n";

const char * succeed_string = "SUCCESS\r\n\r\n";
const char * fail_string    = "FAILURE\r\n\r\n";

uint8_t buf[256];

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

  while (chnGetTimeout(&SD0, TIME_INFINITE)) {
    chnWrite(&SD0, (const uint8_t *)start_msg, strlen(start_msg));
    
    /* Test 1 - X25 Check */
    chnWrite(&SD0, (const uint8_t *)test_1_msg, strlen(test_1_msg));
    chThdSleepMilliseconds(2000);
    /* Forwards */
    buf[0] = '1';
    buf[1] = '2';
    buf[2] = '3';
    buf[3] = '4';
    buf[4] = '5';
    buf[5] = '6';
    buf[6] = '7';
    buf[7] = '8';
    buf[8] = '9';
    crcGenX25(buf, 11);
    if ((buf[9] != 0x90) ||
        (buf[10] != 0x6E)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    /* Backwards */
    buf[0] = '1';
    buf[1] = '2';
    buf[2] = '3';
    buf[3] = '4';
    buf[4] = '5';
    buf[5] = '6';
    buf[6] = '7';
    buf[7] = '8';
    buf[8] = '9';
    buf[9] = 0x90;
    buf[10] = 0x6E;
    if (!crcCheckX25(buf, 11)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    
    /* Test 2 - X25 Examples */
    chThdSleepMilliseconds(2000);
    chnWrite(&SD0, (const uint8_t *)test_2_msg, strlen(test_2_msg));
    /* Forwards */
    buf[0] = 0x03;
    buf[1] = 0x3F;
    crcGenX25(buf, 4);
    if ((buf[3] != 0x5B) ||
        (buf[2] != 0xEC)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x01;
    buf[1] = 0x73;
    crcGenX25(buf, 4);
    if ((buf[3] != 0x83) ||
        (buf[2] != 0x57)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x01;
    buf[1] = 0x3F;
    crcGenX25(buf, 4);
    if ((buf[3] != 0xEB) ||
        (buf[2] != 0xDF)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x03;
    buf[1] = 0x73;
    crcGenX25(buf, 4);
    if ((buf[3] != 0x33) ||
        (buf[2] != 0x64)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x00;
    buf[1] = 0x00;
    buf[2] = 0x00;
    crcGenX25(buf, 5);
    if ((buf[4] != 0xCC) ||
        (buf[3] != 0xC6)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x0F;
    buf[1] = 0xAA;
    buf[2] = 0xFF;
    crcGenX25(buf, 5);
    if ((buf[4] != 0xFC) ||
        (buf[3] != 0xD1)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x0A;
    buf[1] = 0x12;
    buf[2] = 0x34;
    buf[3] = 0x56;
    crcGenX25(buf, 6);
    if ((buf[5] != 0x2C) ||
        (buf[4] != 0xF6)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }

    /* Backwards */
    buf[0] = 0x03;
    buf[1] = 0x3F;
    buf[3] = 0x5B;
    buf[2] = 0xEC;
    if (!crcCheckX25(buf, 4)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x01;
    buf[1] = 0x73;
    buf[3] = 0x83;
    buf[2] = 0x57;
    if (!crcCheckX25(buf, 4)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x01;
    buf[1] = 0x3F;
    buf[3] = 0xEB;
    buf[2] = 0xDF;
    if (!crcCheckX25(buf, 4)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x03;
    buf[1] = 0x73;
    buf[3] = 0x33;
    buf[2] = 0x64;
    if (!crcCheckX25(buf, 4)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x00;
    buf[1] = 0x00;
    buf[2] = 0x00;
    buf[4] = 0xCC;
    buf[3] = 0xC6;
    if (!crcCheckX25(buf, 5)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x0F;
    buf[1] = 0xAA;
    buf[2] = 0xFF;
    buf[4] = 0xFC;
    buf[3] = 0xD1;
    if (!crcCheckX25(buf, 5)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x0A;
    buf[1] = 0x12;
    buf[2] = 0x34;
    buf[3] = 0x56;
    buf[5] = 0x2C;
    buf[4] = 0xF6;
    if (!crcCheckX25(buf, 6)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }

    /* Test 3 - SDLP Check */
    chThdSleepMilliseconds(2000);
    chnWrite(&SD0, (const uint8_t *)test_3_msg, strlen(test_3_msg));
    /* Forwards */
    buf[0] = '1';
    buf[1] = '2';
    buf[2] = '3';
    buf[3] = '4';
    buf[4] = '5';
    buf[5] = '6';
    buf[6] = '7';
    buf[7] = '8';
    buf[8] = '9';
    crcGenSDLP(buf, 11);
    if ((buf[9] != 0x29) ||
        (buf[10] != 0xB1)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    /* Backwards */
    buf[0] = '1';
    buf[1] = '2';
    buf[2] = '3';
    buf[3] = '4';
    buf[4] = '5';
    buf[5] = '6';
    buf[6] = '7';
    buf[7] = '8';
    buf[8] = '9';
    buf[9] = 0x29;
    buf[10] = 0xb1;
    if (!crcCheckSDLP(buf, 11)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    
    /* Test 4 - SDLP Examples */
    chThdSleepMilliseconds(2000);
    chnWrite(&SD0, (const uint8_t *)test_4_msg, strlen(test_4_msg));
    /* Forwards */
    buf[0] = 0x30;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x07;
    buf[4] = 0x00;
    buf[5] = 0x00;
    crcGenSDLP(buf, 8);
    if ((buf[6] != 0x4C) ||
        (buf[7] != 0xA9)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x30;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x09;
    buf[4] = 0x00;
    buf[5] = 0x82;
    buf[6] = 0x00;
    buf[7] = 0x00;
    crcGenSDLP(buf, 10);
    if ((buf[8] != 0xF6) ||
        (buf[9] != 0xF0)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x30;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x09;
    buf[4] = 0x00;
    buf[5] = 0x82;
    buf[6] = 0x00;
    buf[7] = 0x10;
    crcGenSDLP(buf, 10);
    if ((buf[8] != 0xE4) ||
        (buf[9] != 0xC1)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x30;
    buf[1] = 0x1B;
    buf[2] = 0x04;
    buf[3] = 0x09;
    buf[4] = 0x00;
    buf[5] = 0x82;
    buf[6] = 0x00;
    buf[7] = 0x00;
    crcGenSDLP(buf, 10);
    if ((buf[8] != 0xF0) ||
        (buf[9] != 0x51)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x30;
    buf[1] = 0x1B;
    buf[2] = 0x04;
    buf[3] = 0x09;
    buf[4] = 0x00;
    buf[5] = 0x82;
    buf[6] = 0x00;
    buf[7] = 0x10;
    crcGenSDLP(buf, 10);
    if ((buf[8] != 0xE2) ||
        (buf[9] != 0x60)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x00;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x07;
    buf[4] = 0xFF;
    buf[5] = 0x01;
    crcGenSDLP(buf, 8);
    if ((buf[6] != 0x70) ||
        (buf[7] != 0xFB)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x00;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x08;
    buf[4] = 0x00;
    buf[5] = 0x01;
    buf[6] = 0x02;
    crcGenSDLP(buf, 9);
    if ((buf[7] != 0xBE) ||
        (buf[8] != 0x58)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x00;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x09;
    buf[4] = 0x01;
    buf[5] = 0x01;
    buf[6] = 0x02;
    buf[7] = 0x03;
    crcGenSDLP(buf, 10);
    if ((buf[8] != 0xF2) ||
        (buf[9] != 0x93)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x00;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x0A;
    buf[4] = 0x02;
    buf[5] = 0x01;
    buf[6] = 0x02;
    buf[7] = 0x03;
    buf[8] = 0x04;
    crcGenSDLP(buf, 11);
    if ((buf[9] != 0x3C) ||
        (buf[10] != 0xEB)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x00;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x0E;
    buf[4] = 0x06;
    buf[5] = 0x01;
    buf[6] = 0x02;
    buf[7] = 0x03;
    buf[8] = 0x04;
    buf[9] = 0x05;
    buf[10] = 0x06;
    buf[11] = 0x07;
    buf[12] = 0x08;
    crcGenSDLP(buf, 15);
    if ((buf[13] != 0x14) ||
        (buf[14] != 0xBB)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x00;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x0F;
    buf[4] = 0x07;
    buf[5] = 0x01;
    buf[6] = 0x02;
    buf[7] = 0x03;
    buf[8] = 0x04;
    buf[9] = 0x05;
    buf[10] = 0x06;
    buf[11] = 0x07;
    buf[12] = 0x08;
    buf[13] = 0x09;
    crcGenSDLP(buf, 16);
    if ((buf[14] != 0xCF) ||
        (buf[15] != 0x90)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x00;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x11;
    buf[4] = 0x00;
    buf[5] = 0xC0;
    buf[6] = 0x10;
    buf[7] = 0x00;
    buf[8] = 0xC0;
    buf[9] = 0x00;
    buf[10] = 0x00;
    buf[11] = 0x03;
    buf[12] = 0x2E;
    buf[13] = 0xAF;
    buf[14] = 0x8A;
    buf[15] = 0x06;
    crcGenSDLP(buf, 18);
    if ((buf[16] != 0x9F) ||
        (buf[17] != 0x71)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x00;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x12;
    buf[4] = 0x00;
    buf[5] = 0xC1;
    buf[6] = 0x10;
    buf[7] = 0x00;
    buf[8] = 0xFF;
    buf[9] = 0xFF;
    buf[10] = 0x00;
    buf[11] = 0x04;
    buf[12] = 0x01;
    buf[13] = 0x02;
    buf[14] = 0x03;
    buf[15] = 0x11;
    buf[16] = 0x82;
    crcGenSDLP(buf, 19);
    if ((buf[17] != 0x8D) ||
        (buf[18] != 0x80)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x00;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x12;
    buf[4] = 0x00;
    buf[5] = 0xC1;
    buf[6] = 0x11;
    buf[7] = 0x04;
    buf[8] = 0xC0;
    buf[9] = 0x00;
    buf[10] = 0x00;
    buf[11] = 0x04;
    buf[12] = 0x01;
    buf[13] = 0x02;
    buf[14] = 0x03;
    buf[15] = 0x72;
    buf[16] = 0x17;
    crcGenSDLP(buf, 19);
    if ((buf[17] != 0x8D) ||
        (buf[18] != 0x80)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x20;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x07;
    buf[4] = 0x00;
    buf[5] = 0xE1;
    crcGenSDLP(buf, 8);
    if ((buf[6] != 0xBB) ||
        (buf[7] != 0x22)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x20;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x11;
    buf[4] = 0x00;
    buf[5] = 0xC0;
    buf[6] = 0x10;
    buf[7] = 0x00;
    buf[8] = 0xC0;
    buf[9] = 0x00;
    buf[10] = 0x00;
    buf[11] = 0x03;
    buf[12] = 0x2E;
    buf[13] = 0xAF;
    buf[14] = 0x8A;
    buf[15] = 0x06;
    crcGenSDLP(buf, 18);
    if ((buf[16] != 0xD9) ||
        (buf[17] != 0x65)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    
    /* Backwards */
    buf[0] = 0x30;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x07;
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = 0x4C;
    buf[7] = 0xA9;
    if (!crcCheckSDLP(buf, 8)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x30;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x09;
    buf[4] = 0x00;
    buf[5] = 0x82;
    buf[6] = 0x00;
    buf[7] = 0x00;
    buf[8] = 0xF6;
    buf[9] = 0xF0;
    if (!crcCheckSDLP(buf, 10)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x30;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x09;
    buf[4] = 0x00;
    buf[5] = 0x82;
    buf[6] = 0x00;
    buf[7] = 0x10;
    buf[8] = 0xE4;
    buf[9] = 0xC1;
    if (!crcCheckSDLP(buf, 10)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x30;
    buf[1] = 0x1B;
    buf[2] = 0x04;
    buf[3] = 0x09;
    buf[4] = 0x00;
    buf[5] = 0x82;
    buf[6] = 0x00;
    buf[7] = 0x00;
    buf[8] = 0xF0;
    buf[9] = 0x51;
    if (!crcCheckSDLP(buf, 10)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x30;
    buf[1] = 0x1B;
    buf[2] = 0x04;
    buf[3] = 0x09;
    buf[4] = 0x00;
    buf[5] = 0x82;
    buf[6] = 0x00;
    buf[7] = 0x10;
    buf[8] = 0xE2;
    buf[9] = 0x60;
    if (!crcCheckSDLP(buf, 10)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x00;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x07;
    buf[4] = 0xFF;
    buf[5] = 0x01;
    buf[6] = 0x70;
    buf[7] = 0xFB;
    if (!crcCheckSDLP(buf, 8)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x00;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x08;
    buf[4] = 0x00;
    buf[5] = 0x01;
    buf[6] = 0x02;
    buf[7] = 0xBE;
    buf[8] = 0x58;
    if (!crcCheckSDLP(buf, 9)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x00;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x09;
    buf[4] = 0x01;
    buf[5] = 0x01;
    buf[6] = 0x02;
    buf[7] = 0x03;
    buf[8] = 0xF2;
    buf[9] = 0x93;
    if (!crcCheckSDLP(buf, 10)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x00;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x0A;
    buf[4] = 0x02;
    buf[5] = 0x01;
    buf[6] = 0x02;
    buf[7] = 0x03;
    buf[8] = 0x04;
    buf[9] = 0x3C;
    buf[10] = 0xEB;
    if (!crcCheckSDLP(buf, 11)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x00;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x0E;
    buf[4] = 0x06;
    buf[5] = 0x01;
    buf[6] = 0x02;
    buf[7] = 0x03;
    buf[8] = 0x04;
    buf[9] = 0x05;
    buf[10] = 0x06;
    buf[11] = 0x07;
    buf[12] = 0x08;
    buf[13] = 0x14;
    buf[14] = 0xBB;
    if (!crcCheckSDLP(buf, 15)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x00;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x0F;
    buf[4] = 0x07;
    buf[5] = 0x01;
    buf[6] = 0x02;
    buf[7] = 0x03;
    buf[8] = 0x04;
    buf[9] = 0x05;
    buf[10] = 0x06;
    buf[11] = 0x07;
    buf[12] = 0x08;
    buf[13] = 0x09;
    buf[14] = 0xCF;
    buf[15] = 0x90;
    if (!crcCheckSDLP(buf, 16)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x00;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x11;
    buf[4] = 0x00;
    buf[5] = 0xC0;
    buf[6] = 0x10;
    buf[7] = 0x00;
    buf[8] = 0xC0;
    buf[9] = 0x00;
    buf[10] = 0x00;
    buf[11] = 0x03;
    buf[12] = 0x2E;
    buf[13] = 0xAF;
    buf[14] = 0x8A;
    buf[15] = 0x06;
    buf[16] = 0x9F;
    buf[17] = 0x71;
    if (!crcCheckSDLP(buf, 18)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x00;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x12;
    buf[4] = 0x00;
    buf[5] = 0xC1;
    buf[6] = 0x10;
    buf[7] = 0x00;
    buf[8] = 0xFF;
    buf[9] = 0xFF;
    buf[10] = 0x00;
    buf[11] = 0x04;
    buf[12] = 0x01;
    buf[13] = 0x02;
    buf[14] = 0x03;
    buf[15] = 0x11;
    buf[16] = 0x82;
    buf[17] = 0x8D;
    buf[18] = 0x80;
    if (!crcCheckSDLP(buf, 19)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x00;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x12;
    buf[4] = 0x00;
    buf[5] = 0xC1;
    buf[6] = 0x11;
    buf[7] = 0x04;
    buf[8] = 0xC0;
    buf[9] = 0x00;
    buf[10] = 0x00;
    buf[11] = 0x04;
    buf[12] = 0x01;
    buf[13] = 0x02;
    buf[14] = 0x03;
    buf[15] = 0x72;
    buf[16] = 0x17;
    buf[17] = 0x8D;
    buf[18] = 0x80;
    if (!crcCheckSDLP(buf, 19)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x20;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x07;
    buf[4] = 0x00;
    buf[5] = 0xE1;
    buf[6] = 0xBB;
    buf[7] = 0x22;
    if (!crcCheckSDLP(buf, 8)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    buf[0] = 0x20;
    buf[1] = 0x1B;
    buf[2] = 0x00;
    buf[3] = 0x11;
    buf[4] = 0x00;
    buf[5] = 0xC0;
    buf[6] = 0x10;
    buf[7] = 0x00;
    buf[8] = 0xC0;
    buf[9] = 0x00;
    buf[10] = 0x00;
    buf[11] = 0x03;
    buf[12] = 0x2E;
    buf[13] = 0xAF;
    buf[14] = 0x8A;
    buf[15] = 0x06;
    buf[16] = 0xD9;
    buf[17] = 0x65;
    if (!crcCheckSDLP(buf, 18)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
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
