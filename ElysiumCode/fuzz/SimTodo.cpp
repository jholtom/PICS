#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <iostream>

extern "C" {
#include "hal.h"
#include "sx1212.h"
#include "sx1278.h"
  #include "fram.h"
  /********************************************************************************\
   *  Global structure definitions                                                *
  \********************************************************************************/
  I2CDriver I2CDB0;
  SX1278Driver SX1278D1;
  thread_t * rf_thd;

  /********************************************************************************\
   *  Elysium functions                                                           *
  \********************************************************************************/
  void elyErrorSignal(uint8_t error) {}
  void elyNLRouteRF(uint8_t * buffer) {}
  void elyNLRouteRFI(uint8_t * buffer) {}
  msg_t elyFramGetRequestTimeoutS(fram_req_t ** reqp, systime_t timeout) { return 0; }
  void elyFramPostRequestS(fram_req_t * req) {}
  uint8_t * elyNLGetBufferI() { return NULL; }

  /********************************************************************************\
   *  MSP430 functions                                                            *
  \********************************************************************************/
  void _enable_interrupts() {}
  void _disable_interrupts() {}

  /********************************************************************************\
   *  ChibiOS functions                                                           *
  \********************************************************************************/
  /*
  msg_t chSemWaitTimeout(semaphore_t *sp, systime_t time) { return 0; }
  msg_t chSemWaitTimeoutS(semaphore_t *sp, systime_t timeout) { return 0; }
  void chEvtSignal(thread_t *tp, eventmask_t events) { std::cout << "chEvtSignal(" << tp << ", " << events << ")" << std::endl; }
  void chEvtSignalI(thread_t *tp, eventmask_t events) {}
  void chEvtSignalI(thread_t *tp, eventmask_t mask);
  void chSemSignal(semaphore_t *sp) {}
  void chSemSignalI(semaphore_t *sp) {}
  void chSysHalt(const char * reason) { assert(0); }
  void chSchRescheduleS(void) {}
  void i2cMSP430XContinueTransmitI(I2CDriver *i2cp, i2caddr_t addr, size_t n, uint8_t * txbuf, i2ccallback_t callback) {}
  void i2cStart(I2CDriver *i2cp, const I2CConfig *config) {}
*/

  /********************************************************************************\
   *  ChibiOS mailbox functions                                                   *
  \********************************************************************************/
/*
  msg_t chMBFetchI(mailbox_t *mbp, msg_t *msgp) { return 0; }
  msg_t chMBFetchS(mailbox_t *mbp, msg_t *msgp, systime_t timeout) { return 0; }
  msg_t chMBPostAheadI(mailbox_t *mbp, msg_t msg) { return 0; }
  msg_t chMBPostAhead(mailbox_t *mbp, msg_t msg, systime_t timeout) { return 0; }
  msg_t chMBPostAheadS(mailbox_t *mbp, msg_t msg, systime_t timeout) { return 0; }
  msg_t chMBPostI(mailbox_t *mbp, msg_t msg) { return 0; }
  msg_t chMBPostS(mailbox_t *mbp, msg_t msg, systime_t timeout) { return 0; }
*/
  
  /*
  void crcStartCalc(CRCDriver *crcp, size_t n, const void *buf) {}
  void crcStartCalcI(CRCDriver *crcp, size_t n, const void *buf) {}
  */

}
