/*
    ChibiOS - Copyright (C) 2016 Andrew Wygle aka awygle

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

/**
 * @file    sx1212.c
 * @brief   SX1212 RF transcevier module code.
 *
 * @addtogroup sx1212
 * @{
 */

#include "stdlib.h"
#include "math.h"

#include "hal.h"

#include "sx1212.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define BLOCK_RETURN_ERROR(x)                                               \
  suspend_result = (x);                                                     \
  if (suspend_result != MSG_OK)                                             \
    return suspend_result;

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

SX1212Driver SX1212D1;

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

thread_reference_t tr; /* For suspending thread while waiting for ISRs */
bool sending;
typedef msg_t (*packet_receiver)(SX1212Driver *devp, size_t n, uint8_t *rxbuf,
    systime_t timeout);
static msg_t suspend_result;

static void spi_end_cb(SPIDriver * spip) {
  (void)(spip);
  SX1212Driver * devp = &SX1212D1;
  /* Call the callback */
  devp->rx_callback(devp, devp->rx_read, devp->active_rx_buf);
}

void spi_mode_config(SX1212Driver * devp) {
  devp->config->spicfgp->end_cb = NULL;
  devp->config->spip->regs->ctlw0 &= ~UCMODE1;
}

void spi_mode_data(SX1212Driver * devp) {
  devp->config->spicfgp->end_cb = spi_end_cb;
  devp->config->spip->regs->ctlw0 |= UCMODE1;
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

static void blockuntil_callback(void) {
  osalSysLockFromISR();
  osalThreadResumeI(&tr, MSG_OK);
  osalSysUnlockFromISR();
}

static void ready_cb(void) {
  /* Just calls the ready callback */
  SX1212D1.ready_callback(&SX1212D1);
}

static void sx1212SetFifoThreshold(SX1212Driver *devp, uint8_t thresh);

static void active_read_cb(void) {
  SX1212Driver * devp = &SX1212D1;

  /* Calculate the threshold */
  uint8_t thresh = (devp->regs.IRQParam0 & 0x1F) + 1;

  /* Read threshold bytes out of the FIFO */
  osalDbgAssert(devp->config->data_ss_b == PAL_NOLINE,
      "Software chip select control not currently supported");
  chSysLockFromISR();
  /* Switch to the Data SPI interface */
  spi_mode_data(devp);
  spiStartReceiveI(devp->config->spip, thresh,
      devp->active_rx_buf + devp->rx_read);

  /* Modify bytes read and bytes to read */
  devp->rx_read += thresh;
  devp->rx_toread -= thresh;

  /* Stop short, remainder will be handled in the SPI callback */
  if (devp->rx_toread == 0) {
    chSysUnlockFromISR();
    return;
  }

  /* Re-set FIFO threshold */
  /* Switch to the Config SPI interface */
  spi_mode_config(devp);
  if (devp->rx_toread < 64) {
    sx1212SetFifoThreshold(devp, devp->rx_toread - 1);
  }
  else {
    sx1212SetFifoThreshold(devp, 47);
  }
  chSysUnlockFromISR();
}


/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Reads register value.
 * @pre     The SX1212 driver must be initialized and the NSS_CONFIG interface
 *          started and acquired (if enabled).
 * @post    The NSS_CONFIG interface is started and acquired (if enabled).
 *
 * @param[in] devp    pointer to the SX1212 driver
 * @param[in] reg     register address
 * @return            the read value
 */
static uint8_t sx1212ReadRegister(SX1212Driver *devp, uint8_t reg) {

  osalDbgAssert(reg <= 30, "register address out of bounds");
  /* TODO: Should also check that it is not above the register boundaries*/

  /* 01xxxxx0 - set read bit */
  reg = (0x40 | (reg << 1));

  /* assume we're already in config mode - see precondition */
  spiSelectI(devp->config->spip);
  spiPolledExchange(devp->config->spip, reg);
  reg = spiPolledExchange(devp->config->spip, 0xFFU);
  spiUnselectI(devp->config->spip);

  return reg;
}

/**
 * @brief   Sets register value.
 * @pre     The SX1212 driver must be initialized and the NSS_CONFIG interface
 *          started and acquired (if enabled).
 * @post    The NSS_CONFIG interface is started and acquired (if enabled).
 *
 * @param[in] devp    pointer to the SX1212 driver
 * @param[in] reg     register address
 * @param[in] value   the value to be written
 * @return            the previous value of the register
 */
static uint8_t sx1212SetRegister(SX1212Driver *devp, uint8_t reg,
    uint8_t value) {

  osalDbgAssert(reg <= 30, "register address out of bounds");

  osalDbgAssert(reg != 20, "setting read-only registers");

  /* 00xxxxx0 */
  reg = (reg << 1);

  spiSelectI(devp->config->spip);
  spiPolledExchange(devp->config->spip, reg);
  reg = spiPolledExchange(devp->config->spip, value);
  spiUnselectI(devp->config->spip);

  return reg;
}


/**
 * @brief   Helper function for waiting on the value of a bit in a SPI register
 * @pre     The SX1212 driver must be initialized and the NSS_CONFIG interface
 *          started and acquired (if enabled).
 * @post    The NSS_CONFIG interface is started and acquired (if enabled).
 *
 * @param[in] devp    pointer to the SX1212 driver
 * @param[in] reg     the register containing the bit to wait on
 * @param[in] bit     the mask of the bit in the register to wait on
 * @param[in] timeout how long to wait
 * @return  @p MSG_OK, or @p MSG_TIMEOUT if timeout occurs
 *
 * @notapi
 */
static msg_t block_on_bit(SX1212Driver *devp, uint8_t reg, uint8_t bit,
    systime_t timeout) {
  /* Assume Config SPI bus already acquired and started */
  systime_t start = osalOsGetSystemTimeX();
  while (!(sx1212ReadRegister(devp, reg) & bit)) {
    if (timeout == TIME_IMMEDIATE) return MSG_TIMEOUT;
    if (timeout != TIME_INFINITE &&
        !osalOsIsTimeWithinX(osalOsGetSystemTimeX(), start, start + timeout)) {
      return MSG_TIMEOUT;
    }
  }

  return MSG_OK;
}

/**
 * @brief   Helper function for suspending a thread until an IRQ event
 * @pre     The SX1212 driver must be initialized and the NSS_CONFIG interface
 *          started and acquired (if enabled).
 * @post    The NSS_CONFIG interface is started and acquired (if enabled).
 *
 * @param[in] devp    pointer to the SX1212 driver
 * @param[in] irq     the ioline_t to block on
 * @param[in] timeout how long to block for
 * @return  @p MSG_OK, or @p MSG_TIMEOUT if timeout occurs
 *
 * @notapi
 */
static msg_t irq_block_with_timeout(SX1212Driver *devp, ioline_t irq,
    systime_t timeout) {
  /* Assume Config SPI bus already acquired and started */
#if SX1212_SHARED_SPI
  spiReleaseBus(devp->config->spip);
#else
  /* suppress warning */
  (void)devp;
#endif
  osalSysLock();
  palLineEnableEventI(irq, PAL_EVENT_MODE_RISING_EDGE, blockuntil_callback);
  if (palReadLine(irq)) {
    suspend_result = MSG_OK;
  }
  else {
    suspend_result = osalThreadSuspendTimeoutS(&tr, timeout);
  }

  /* Disable interrupt */
  palLineDisableEventI(irq);
  osalSysUnlock();

  /* Maintain invariant */
#if SX1212_SHARED_SPI
  spiAcquireBus(devp->config->spip);
  spiStart(devp->config->spip, devp->config->spicfgp);
#endif

  return suspend_result;
}

/**
 * @brief   Helper function for waiting until there is data in the FIFO to read
 * @pre     The SX1212 driver must be initialized and the NSS_DATA interface
 *          started and acquired (if enabled).
 * @post    The NSS_DATA interface is started and acquired (if enabled).
 *
 * @param[in] devp    pointer to the SX1212 driver
 * @param[in] timeout how long to block for
 * @return  @p MSG_OK, or @p MSG_TIMEOUT if timeout occurs
 *
 * @notapi
 */
static msg_t block_fifo_empty(SX1212Driver *devp, systime_t timeout) {
#if defined(SX1212_IRQ_0)
  osalDbgAssert(devp->irq_mappings[0] == Fifoempty_B,
      "irq configuration incorrect");

  BLOCK_RETURN_ERROR(irq_block_with_timeout(devp, SX1212_IRQ_0, timeout));
#else
  /* Assume Data SPI acquired and started */
#if SX1212_SHARED_SPI
  spiReleaseBus(devp->config->dataspip);
  spiAcquireBus(devp->config->spip);
#endif /* shared spi */
  spiStart(devp->config->spip, devp->config->spicfgp);
  /* can't use the macro - SPI state change must be reversed before return */
  suspend_result = block_on_bit(devp, IRQParam1, BIT0, timeout);
#if SX1212_SHARED_SPI
  spiReleaseBus(devp->config->spip);
  spiAcquireBus(devp->config->dataspip);
#endif /* shared spi */
  spiStart(devp->config->dataspip, devp->config->spidatap);
#endif
  return suspend_result;
}

/**
 * @brief   Helper function for reading data from the FIFO while ensuring there
 *          is data to be read.
 * @pre     The SX1212 driver must be initialized and the NSS_CONFIG interface
 *          started and acquired (if enabled).
 * @post    The NSS_CONFIG interface is started and acquired (if enabled).
 *
 * @param[in] devp    pointer to the SX1212 driver
 * @param[in] n       number of bytes to be read
 * @param[out] buf    buffer into which to read bytes
 * @param[in] timeout how long to wait for additional bytes to arrive
 * @return  @p MSG_OK, or @p MSG_TIMEOUT if timeout occurs
 *
 * @notapi
 */
static msg_t fifo_read_unsafe(SX1212Driver *devp, size_t n, uint8_t *buf,
    systime_t timeout) {
  /* Assume Config SPI acquired and started */
  /* Switch to Data bus */
  spi_mode_data(devp);

  for (unsigned int i = 0; i < n; i++) {

    /* Make sure there's something to read */
    /* Can't use the macro - SPI state change must be reversed before return */
    suspend_result = block_fifo_empty(devp, timeout);
    if (suspend_result != MSG_OK) break;

    buf[i] = spiPolledExchange(devp->config->spip, 0xFF);
  }

  /* Restore invariant */
  spi_mode_config(devp);

  return MSG_OK;
}

/**
 * @brief   Helper function for reading data from the FIFO when it can be
 *          guaranteed that there are at least n bytes of data available
 * @pre     The SX1212 driver must be initialized and the NSS_CONFIG interface
 *          started and acquired (if enabled).
 * @post    The NSS_CONFIG interface is started and acquired (if enabled).
 *
 * @param[in] devp    pointer to the SX1212 driver
 * @param[in] n       number of bytes to be read
 * @param[out] buf    buffer into which to read bytes
 *
 * @notapi
 */
static void fifo_read_safe(SX1212Driver *devp, size_t n, uint8_t *buf) {
  /* Assume Config SPI acquired and started */
#if SX1212_SHARED_SPI
  spiReleaseBus(devp->config->spip);
  spiAcquireBus(devp->config->dataspip);
#endif
  /* Switch to Data bus */
  spi_mode_data(devp);

  for (unsigned int i = 0; i < n; i++) {
    /* Can't use spiReceive because of slave select framing needs */
    /* TODO pretty sure this whole function can go */
    buf[i] = spiPolledExchange(devp->config->spip, 0xFF);
  }

  /* Restore invariant */
  spi_mode_config(devp);
}

/* TODO test this */
/**
 * @brief   Sets the transceiver's bit rate
 * @pre     The SX1212 driver must be initialized and the NSS_CONFIG interface
 *          started and acquired (if enabled).
 * @post    The NSS_CONFIG interface is started and acquired (if enabled)
 *
 * @param[in] devp pointer to the SX1212 driver
 * @param[in] rate bit rate in bits per second to be used
*/
void sx1212SetBitrate(SX1212Driver *devp, uint32_t rate) {
  uint16_t tmp;

  osalDbgAssert(rate < 150000, "bit rate too high");

  /* Calculate the multiplier */
  rate = (SX1212_CLK_FREQ >> 1)/rate;

  if (rate - 1 <= 255) {
    sx1212SetRegister(devp, MCParam3, (rate & 0x00FF));
    sx1212SetRegister(devp, MCParam4, 0);
  }
  else {
    uint8_t bestc = 0;
    uint8_t bestd = 0;
    uint8_t besterr = 0xffu;
    uint8_t err;
    /* i is (C+1), up to the largest multiplier's square root */
    for (unsigned int i = (rate / 255) + 1; i <= 92; i++) {
      /* Divide by i - this is (D+1) */
      tmp = rate / i;
      err = rate - (tmp * i);
      if (err < besterr) {
        bestc = i - 1;
        bestd = tmp - 1;
      }
      if (err == 0 || i*i > rate) break;
    }
    sx1212SetRegister(devp, MCParam3, bestc);
    sx1212SetRegister(devp, MCParam4, bestd);
  }
}

/**
 * @brief   Sets the transceiver's frequency deviation
 * @pre     The SX1212 driver must be initialized and the NSS_CONFIG interface
 *          started and acquired (if enabled).
 * @post    The NSS_CONFIG interface is started and acquired (if enabled)
 *
 * @param[in] devp      pointer to the SX1212 driver
 * @param[in] deviation frequency deviation to be programmed, in Hz
*/
void sx1212SetDeviation(SX1212Driver *devp, uint32_t fdev) {
  uint16_t regs = ((SX1212_CLK_FREQ >> 5) + (fdev >> 1))/ fdev;

  osalDbgAssert(fdev < 200000, "deviation too high");

  sx1212SetRegister(devp, MCParam2, regs - 1);
}

/**
 * @brief   Sets the transceiver's center frequency
 * @pre     The SX1212 driver must be initialized and the NSS_CONFIG interface
 *          started and acquired (if enabled).
 * @post    The NSS_CONFIG interface is started and acquired (if enabled)
 *
 * @param[in] devp      pointer to the SX1212 driver
 * @param[in] deviation center frequency to be programmed, in Hz
*/
void sx1212SetFrequency(SX1212Driver *devp, uint32_t freq) {
  /* Frequency parameter is ignored */
  /* Manual RPS register configuration for frequency */
  sx1212SetRegister(devp, MCParam6, 143);
  sx1212SetRegister(devp, MCParam7, 60);
  sx1212SetRegister(devp, MCParam8, 17);  /* PIC A (459.2MHz) */
//  sx1212SetRegister(devp, MCParam8, 15);  /* PIC B (459.0MHz) */

  /* Set frequency band registers */
  devp->regs.MCParam0 = (devp->regs.MCParam0 & 0xE0) /* clear bits 4-0 */
          | 0x10 /* 430-470 MHz band */ | 0x02 /* 3rd subband (450MHz) */;
  sx1212SetRegister(devp, MCParam0, devp->regs.MCParam0);
}

/**
 * @brief   Sets the transceiver's sync value
 * @pre     The SX1212 driver must be initialized and the NSS_CONFIG interface
 *          started and acquired (if enabled).
 * @post    The NSS_CONFIG interface is started and acquired (if enabled)
 *
 * @param[in] devp  pointer to the SX1212 driver
 * @param[in] bytes length of the sync value in bytes
 * @param[in] value pointer to the sync value
*/
void sx1212SetSync(SX1212Driver *devp, SX1212_SYNC_TYPE sync) {

  osalDbgAssert(sizeof(sync) <= 4 && sizeof(sync) >= 1,
      "requested sync length invalid");
  /* Sync is required - we don't support turning it off (why would you?) */

  for (unsigned int i = 0; i < sizeof(sync); i++) {
    sx1212SetRegister(devp, SYNCParam0 + i,
        sync >> (8 * (sizeof(sync) - 1 - i)));
  }
}

/**
 * @brief   Resets the transceiver
 *
 * @param[in] devp pointer to the SX1212 driver
 */
static void sx1212Reset(SX1212Driver *devp) {

  osalDbgAssert(devp->config->reset_line != PAL_NOLINE, "reset line required");

  palSetLineMode(devp->config->reset_line, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLine(devp->config->reset_line);
  osalThreadSleepMicroseconds(100);
  palClearLine(devp->config->reset_line);
  palSetLineMode(devp->config->reset_line, PAL_MODE_INPUT_PULLDOWN);
  osalThreadSleepMilliseconds(5);
}

/**
 * @brief   Sets the SX1212 system mode
 * @pre     The SX1212 driver must be initialized and the NSS_CONFIG interface
 *          started and acquired (if enabled).
 * @post    The NSS_CONFIG interface is started and acquired (if enabled)
 *
 * @param[in] devp pointer to the SX1212 driver
 * @param[in] mode the mode to transition the driver to
 */
static void sx1212SetMode(SX1212Driver *devp, SX1212ModeConstants mode) {

  devp->regs.MCParam0 = (devp->regs.MCParam0 & 0x1F) | mode;
  sx1212SetRegister(devp, MCParam0, devp->regs.MCParam0);
}

/**
 * @brief   Configures the SX1212 IRQs
 * @pre     The SX1212 driver must be initialized and the NSS_CONFIG interface
 *          started and acquired (if enabled).
 * @post    The NSS_CONFIG interface is started and acquired (if enabled)
 *
 * @param[in] devp  pointer to the SX1212 driver
 * @param[in] irq0  first IRQ value to configure
 * @param[in] irq1 second IRQ value to configure
 */
static void sx1212MapIRQs(SX1212Driver *devp, SX1212IRQConstants irq0,
    SX1212IRQConstants irq1) {

  devp->irq_mappings[0] = irq0;
  devp->irq_mappings[1] = irq1;

  sx1212SetRegister(devp, IRQParam1, irq0 | irq1);
}

static msg_t go_to_receive(SX1212Driver *devp, systime_t timeout) {
  /* Assume config SPI already started if not shared */
  /* Further assume both SPIs released if shared */
#if SX1212_SHARED_SPI
  spiAcquireBus(devp->config->spip);
  spiStart(devp->config->spip, devp->config->spicfgp);
#endif

  /* Assume we're in Sleep mode */
  /* Go to Rx, by degrees */
  /* Standby */
  sx1212SetMode(devp, SX1212Stdby);
#if SX1212_SHARED_SPI
  /* Release SPI bus - 5 ms is 80,000 cycles at 16 MHz */
  spiReleaseBus(devp->config->spip);
#endif

  /* Wait TS_OSC for the oscillator to wake up */
  osalThreadSleepMilliseconds(5);

#if SX1212_SHARED_SPI
  /* Reclaim SPI bus */
  spiAcquireBus(devp->config->spip);
  spiStart(devp->config->spip, devp->config->spicfgp);
#endif

  /* FS */
  sx1212SetMode(devp, SX1212FS);

  /* Wait for PLL lock */
  /* can't use the macro - SPI state change must be reversed before return */
#if defined(SX1212_PLL_LOCK)
  #if SX1212_SHARED_SPI
    /* Release SPI bus - 800 us is 12,800 cycles at 16 MHz */
    spiReleaseBus(devp->config->spip);
  #endif
  suspend_result = irq_block_with_timeout(devp, SX1212_PLL_LOCK, timeout);
  #if SX1212_SHARED_SPI
    /* Reclaim SPI bus */
    spiAcquireBus(devp->config->spip);
    spiStart(devp->config->spip, devp->config->spicfgp);
  #endif
#else
  /* Can't drop the SPI bus since we're falling back to it */
  suspend_result = block_on_bit(devp, IRQParam2, BIT1, timeout);
#endif

  if (suspend_result != MSG_OK) {
    return suspend_result;
  }

  /* Receive */
  sx1212SetMode(devp, SX1212Rx);

  /* Reset RX state machine */
  sx1212SetRegister(devp, IRQParam2, 0x51);

#if SX1212_SHARED_SPI
  /* Release SPI bus */
  spiReleaseBus(devp->config->spip);
#endif

  return MSG_OK;
}

/**
 * @brief   Sets the SX1212 FIFO IRQ threshold
 * @pre     The SX1212 driver must be initialized and the NSS_CONFIG interface
 *          started and acquired (if enabled).
 * @post    The NSS_CONFIG interface is started and acquired (if enabled)
 *
 * @param[in] devp    pointer to the SX1212 driver
 * @param[in] thresh  the threshold to set
 */
static void sx1212SetFifoThreshold(SX1212Driver *devp, uint8_t thresh) {

  osalDbgAssert(thresh < 64, "threshold too high");

  sx1212SetRegister(devp, IRQParam0, (0xC0 | thresh));
  devp->regs.IRQParam0 = (0xC0 | thresh);
}

/**
 * @brief   Sets the SX1212 receiver bandwidth
 * @pre     The SX1212 driver must be initialized and the NSS_CONFIG interface
 *          started and acquired (if enabled).
 * @post    The NSS_CONFIG interface is started and acquired (if enabled)
 *
 * @param[in] devp      pointer to the SX1212 driver
 * @param[in] bandwidth the receiver bandwidth to use minus one, in steps of 25 kHz
 *
 * @note The bandwidth specifiedd here is for the active filter. Passive
 *       filter bandwidth is set automatically according to datasheet
 */
void sx1212SetRxBw(SX1212Driver *devp, uint8_t bandwidth) {
  static const uint8_t passive[16] = {
    1, 4, 7, 9, 10, 12, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15
  };

  osalDbgAssert(bandwidth < 16, "requested bandwidth is too wide");

  /* Passive filter bandwidth to be between 3 and 4 times active filter
   * bandwidth, per datasheet. This is precalculated because there's no obvious
   * mapping. */
  sx1212SetRegister(devp, RXParam0, (passive[bandwidth] << 4) | bandwidth);
}

/**
 * @brief   Sets the SX1212 packet configuration
 * @pre     The SX1212 driver must be initialized and the NSS_CONFIG interface
 *          started and acquired (if enabled).
 * @post    The NSS_CONFIG interface is started and acquired (if enabled)
 *
 * @param[in] devp          pointer to the SX1212 driver
 * @param[in] packet_config the packet configuration to apply
 */
static void sx1212ConfigurePackets(SX1212Driver* devp,
    sx1212_packet_config_t *packet_config) {
  /* Configure packet mode */

  /* Buffered - Data_mode == 01b */
  /* Fixed - Data_mode == 10b, Pkt_format == 0 */
  /* Variable - Data_mode == 10b, Pkt_format == 1 */

  /* MCParam1 - Data_mode */
  sx1212SetRegister(devp, MCParam1, 0xA4);

  osalDbgAssert(!(packet_config->whitening && packet_config->manchester),
      "Whitening and Manchester encoding are mutually exclusive");

  /* PKTParam0 - Manchester_on, Payload_length */
  osalDbgAssert(devp->config->length < 128, "requested packet size too long");
  sx1212SetRegister(devp, PKTParam0, (packet_config->manchester << 7) |
     devp->config->length);

  /* PKTParam2 - Pkt_format, preamble_size, Whitening_on, CRC_on, Adrs_filt */
  if (!packet_config->broadcast) {
    packet_config->broadcast = packet_config->addressing;
  }

  /* PKTParam1 - Address */
  if (packet_config->broadcast) {
    sx1212SetRegister(devp, PKTParam1, devp->config->addr);
  }

  sx1212SetRegister(devp, PKTParam2,
      ((packet_config->format == SX1212Variable ? 1 : 0) << 7) |
      /* (packet_config->preamble_len << 5) | */
      (packet_config->whitening << 4) |
      (packet_config->crc << 3) |
      (packet_config->broadcast << 1));

  /* PktParam3 - CRC_autoclr */
  sx1212SetRegister(devp, PKTParam3, ((!packet_config->crc_autoclear) << 7));
}

/**
 * @brief   Helper function for receiving packets in packet modes.
 * @pre     NSS_CONFIG interface started and acquired (if enabled)
 * @post    NSS_CONFIG interface started and acquired (if enabled)
 *
 * @param[in] devp    pointer to the SX1212 driver
 * @param[in] n       number of bytes to be read
 * @param[out] rxbuf    buffer into which to read bytes
 * @param[in] timeout how long to wait for additional bytes to arrive
 * @return  @p MSG_OK, or @p MSG_TIMEOUT if timeout occurs
 *
 * @notapi
 */
static msg_t packet_handler(SX1212Driver *devp, size_t n, uint8_t *rxbuf,
    systime_t timeout) {

  osalDbgAssert(n <= devp->config->length, "buffer too small for packet");

  /* Three variables: Use CRC, IRQ_0 routed, IRQ_1 routed.
   * Relevant configurations:
   * Yes, X, X - Wait for CRC_ok
   * No, Yes, X - Wait for Payload_ready
   * No, No, X - Wake up on Fifoempty_B, read them out one at a time */
  /* Configure IRQs */
  sx1212MapIRQs(devp, Payload_ready, CRC_ok);

  /* Block until whole packet received */
  if (devp->config->packet_config->crc &&
      devp->config->packet_config->crc_autoclear) {
#if !defined(SX1212_IRQ_1)
    BLOCK_RETURN_ERROR(block_on_bit(devp, PKTParam1, BIT0, timeout));
#else
    BLOCK_RETURN_ERROR(irq_block_with_timeout(devp, SX1212_IRQ_1, timeout));
#endif

    if (devp->config->packet_config->format == SX1212Variable) {
      /* Read length byte from Fifo */
      fifo_read_safe(devp, 1, rxbuf);
      n = rxbuf[0];
      rxbuf += 1;
    }

    /* Read packet from Fifo */
    fifo_read_safe(devp, n, rxbuf);
  }
  else {
#if defined(SX1212_IRQ_0)
    /* Wait for Payload_ready interrupt */
    BLOCK_RETURN_ERROR(irq_block_with_timeout(devp, SX1212_IRQ_0, timeout));

    /* Go to Standby */
    sx1212SetMode(devp, SX1212Stdby);

    if (devp->config->packet_config->format == SX1212Variable) {
      /* Read length byte from Fifo */
      fifo_read_safe(devp, 1, rxbuf);
      n = rxbuf[0];
      rxbuf += 1;
    }

    /* Read packet from Fifo */
    fifo_read_safe(devp, n, rxbuf);
#else
    if (devp->config->packet_config->format == SX1212Variable) {
      /* Read length byte from Fifo */
      fifo_read_safe(devp, 1, rxbuf);
      n = rxbuf[0];
      rxbuf += 1;
    }

    /* Read packet from Fifo one byte at a time */
    BLOCK_RETURN_ERROR(fifo_read_unsafe(devp, n, rxbuf, timeout));
#endif
  }

  return MSG_OK;
}

/**
 * @brief   Helper function for receiving packets in packet modes.
 * @pre     NSS_CONFIG interface started and acquired (if enabled)
 * @post    NSS_CONFIG interface started and acquired (if enabled)
 *
 * @param[in] devp    pointer to the SX1212 driver
 * @param[in] n       number of bytes to be read
 * @param[out] rxbuf    buffer into which to read bytes
 * @param[in] timeout how long to wait for additional bytes to arrive
 * @return  @p MSG_OK, @p MSG_TIMEOUT if timeout occurs, or @p MSG_RESET if the
 *          received packet is too long for the buffer (length > n).
 *
 * @notapi
 */
static msg_t buffered_handler(SX1212Driver *devp, size_t n, uint8_t *rxbuf,
    systime_t timeout) {

  /* Configure IRQs */
  sx1212MapIRQs(devp, Fifoempty_B, Fifo_threshold);

  /* Read until length value offset */
  size_t offset = 0;
  BLOCK_RETURN_ERROR(
      fifo_read_unsafe(devp, devp->config->length + sizeof(SX1212_LENGTH_TYPE),
        rxbuf, timeout)
    );
  offset += devp->config->length;

  /* Evaluate length value */
  SX1212_LENGTH_TYPE templen = 0;
  for (size_t i = 0; i < sizeof(SX1212_LENGTH_TYPE); i++) {
    templen |= (rxbuf[offset] << (sizeof(SX1212_LENGTH_TYPE) - i - 1));
    offset++;
  }

  if (templen > n) {
    return MSG_RESET;
  }

  /* Read the remainder of the packet */
#if defined(SX1212_IRQ_1)
  size_t toread;
  while (offset < n) {
    if (n - offset < 64) { /* Fifo depth */
      toread = n - offset;
      /* We're done receiving - go to Standby */
      sx1212SetMode(devp, SX1212Stdby);
    }
    else {
      toread = 48; /* 75% of Fifo depth */
    }

    sx1212SetFifoThreshold(devp, toread - 1);

    /* Wait for the Fifo threshold */
    BLOCK_RETURN_ERROR(irq_block_with_timeout(devp, SX1212_IRQ_1, timeout));

    /* Read the available bytes */
    fifo_read_safe(devp, toread, rxbuf + offset);

    offset += toread;
  }

#else
  /* No SPI fallback for Fifo_threshold */
  BLOCK_RETURN_ERROR(
      fifo_read_unsafe(devp, n - offset, rxbuf + offset, timeout)
    );
#endif

  /* Reset for next packet reception */
  sx1212SetRegister(devp, IRQParam2, 0x51);

  return MSG_OK;
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/


/**
 * @brief   Restores Settings to SX1212 Driver Object
 *
 * @param[in] devp      pointer to the @p SX1212Driver object
 *
 * @api
 */
void sx122ObjectInit(SX1212Driver *devp){

}

/**
 * @brief   Configures and activates SX1212 Complex Driver peripheral.
 * @pre     NSS_CONFIG and NSS_DATA busses are both released (if enabled)
 * @post    NSS_CONFIG and NSS_DATA busses are both released (if enabled).
 *          SX1212 is in Sleep mode.
 *
 * @param[in] devp      pointer to the @p SX1212Driver object
 * @param[in] config    pointer to the @p SX1212Config object
 *
 * @api
 */
void sx1212Start(SX1212Driver *devp, const SX1212Config *config) {

  devp->config = config;
  /* Reset the device to get it into a known state */
  sx1212Reset(devp);
  /* Set all the stored registers to their reset states */
  devp->regs.MCParam0 = 0x30;
  devp->regs.IRQParam0 = 0x0F;

  /* Start device SPI interface */
  spiStart(config->spip, config->spicfgp);
  /* Put the SPI interface into Config mode */
  spi_mode_config(devp);

  /* Put the device into sleep mode */
  sx1212SetMode(devp, SX1212Sleep);
  /* Set device configuration */
  sx1212SetFrequency(devp, config->freq);
  sx1212SetDeviation(devp, config->fdev);
  sx1212SetBitrate(devp, config->bitrate);
  sx1212SetRxBw(devp, config->rx_bw);
  sx1212SetSync(devp, config->sync_word);
  if (config->packet_config != NULL) {
    sx1212ConfigurePackets(devp, devp->config->packet_config);
  }
  else {
    /* Buffered mode */
    sx1212SetRegister(devp, MCParam1, 0x64);
  }

  /* RXParam2 - Sync_tol + Sync_size */
  osalDbgAssert(config->sync_tol <= 3, "sync tolerance too loose");
  sx1212SetRegister(devp, RXParam2, 0x20 |
      ((sizeof(SX1212_SYNC_TYPE) - 1) << 3) |
      (config->sync_tol << 1));

  /* IRQParam2 - enable PLL lock */
  sx1212SetRegister(devp, IRQParam2, 0x51);

  /* Set Fifo size to 64 */
  sx1212SetRegister(devp, IRQParam0, 0xC0);
  devp->regs.IRQParam0 = 0xC0;

#if SX1212_SHARED_SPI
  spiReleaseBus(config->spip);
#endif

}

/**
 * @brief   Deactivates the SX1212 Complex Driver peripheral.
 * @pre     NSS_CONFIG and NSS_DATA busses are both released (if enabled)
 * @post    NSS_CONFIG and NSS_DATA busses are both released (if enabled)
 *          SX1212 is in Sleep mode.
 *
 * @param[in] devp       pointer to the @p SX1212Driver object
 *
 * @api
 */
void sx1212Stop(SX1212Driver *devp) {

  /* Reset the device to get it into a known state */
  sx1212Reset(devp);
#if SX1212_SHARED_SPI
  spiAcquireBus(devp->config->spip);
#endif
  /* Start device SPI interface - in case of repeated stops */
  spiStart(devp->config->spip, devp->config->spicfgp);
  /* Put the device into sleep mode */
  sx1212SetMode(devp, SX1212Sleep);
  /* Set all the stored registers to their reset states */
  devp->regs.MCParam0 = 0x30;
  spiStop(devp->config->spip);
#if SX1212_SHARED_SPI
  spiReleaseBus(devp->config->spip);
#endif
}

/**
 * @brief   Prepare the SX1212 Complex Driver peripheral to receive data
 * @pre     NSS_CONFIG and NSS_DATA busses are both released (if enabled).
 *          SX1212 is in Sleep mode.
 * @post    NSS_CONFIG and NSS_DATA busses are both released (if enabled).
 *          SX1212 is in Receive mode.
 *
 * @param[in] devp    pointer to the SX1212 driver
 * @param[in] callback  callback function called after first byte received
 *
 */
msg_t sx1212StartReceive(SX1212Driver *devp, size_t n, palcallback_t callback) {

  /* TODO real state machines */

  /* Configure IRQs */
  sx1212MapIRQs(devp, Fifoempty_B, Fifo_threshold);

  chSysLock();
  sx1212ReceiveI(devp, n, callback);
  chSysUnlock();

  /* Start receiving */
  /* TODO add a timeout */
  BLOCK_RETURN_ERROR(
      go_to_receive(devp, TIME_INFINITE)
      );

  return MSG_OK;
}

void sx1212ReceiveI(SX1212Driver * devp, size_t n, palcallback_t callback) {
  osalDbgCheckClassI();
  /* TODO state machine */

  osalDbgAssert(n < 64, "invalid threshold");
  /* IRQs are Fifoempty_B and Fifo_threshold */

  /* Configure the Fifo_threshold callback appropriately */
  palLineEnableEventI(SX1212_IRQ_1, PAL_EVENT_MODE_RISING_EDGE, callback);

  /* Set Fifo threshold */
  sx1212SetFifoThreshold(devp, n);
}

void sx1212FifoReadAsync(SX1212Driver * devp, size_t n, uint8_t * buffer, sx1212_rx_cb callback) {
  osalDbgAssert(devp->config->data_ss_b == PAL_NOLINE, "can't do async");

  spi_mode_data(devp);

  devp->rx_callback = callback;
  spiStartReceive(devp->config->spip, n, buffer);
}

void sx1212FifoRead(SX1212Driver * devp, size_t n, uint8_t * buffer) {

  for (size_t i = 0; i < n; i++) {
    palClearLine(devp->config->data_ss_b);
    buffer[i] = spiPolledExchange(devp->config->spip, 0xFF);
    palSetLine(devp->config->data_ss_b);
  }

}

/**
 * @brief   End reception of current packet
 * @pre     NSS_CONFIG and NSS_DATA busses are both released (if enabled).
 *          SX1212 is in Receive mode.
 * @post    NSS_CONFIG and NSS_DATA busses are both released (if enabled).
 *          SX1212 is in Sleep mode.
 *
 * @param[in] devp    pointer to the SX1212 driver
 */
void sx1212StopReceiveI(SX1212Driver *devp) {

  /* Assume config SPI already started if not shared */
  /* Further assume both SPIs released if shared */
#if SX1212_SHARED_SPI
  spiAcquireBus(devp->config->spip);
  spiStart(devp->config->spip, devp->config->spicfgp);
#endif

  /* Reset for next packet reception */
  sx1212SetRegister(devp, IRQParam2, 0x51);
  /* Go to Sleep */
  sx1212SetMode(devp, SX1212Sleep);

  /* Release SPI bus */
#if SX1212_SHARED_SPI
  spiReleaseBus(devp->config->spip);
#endif
}

/**
 * @brief   End reception and put the SX1212 Complex Driver peripheral to sleep
 * @pre     NSS_CONFIG and NSS_DATA busses are both released (if enabled).
 *          SX1212 is in Receive mode.
 * @post    NSS_CONFIG and NSS_DATA busses are both released (if enabled).
 *          SX1212 is in Sleep mode.
 *
 * @param[in] devp    pointer to the SX1212 driver
 */
void sx1212StopReceive(SX1212Driver *devp) {

  /* Assume config SPI already started if not shared */
  /* Further assume both SPIs released if shared */
#if SX1212_SHARED_SPI
  spiAcquireBus(devp->config->spip);
  spiStart(devp->config->spip, devp->config->spicfgp);
#endif

  /* Reset for next packet reception */
  sx1212SetRegister(devp, IRQParam2, 0x51);
  /* Go to Sleep */
  sx1212SetMode(devp, SX1212Sleep);

  /* Release SPI bus */
#if SX1212_SHARED_SPI
  spiReleaseBus(devp->config->spip);
#endif
}

/**
 * @brief   Receive operation on the SX1212 Complex Driver peripheral, with
 *          timeout
 * @pre     NSS_CONFIG and NSS_DATA busses are both released (if enabled).
 *          SX1212 is in Receive mode.
 * @post    NSS_CONFIG and NSS_DATA busses are both released (if enabled)
 *          SX1212 is in Receive mode.
 *
 * @param[in] devp      pointer to the @p SX1212Driver object
 * @param[in] n         Length of rxbuf in bytes (maximum packet size)
 * @param[in] rxbuf     pointer to the receive buffer
 *
 * @api
 */
msg_t sx1212ReceiveTimeout(SX1212Driver *devp, size_t n, uint8_t * rxbuf,
    systime_t timeout) {

  /* Assume config SPI already started if not shared */
  /* Further assume both SPIs released if shared */
#if SX1212_SHARED_SPI
  spiAcquireBus(devp->config->spip);
  spiStart(devp->config->spip, devp->config->spicfgp);
#endif

  suspend_result =
    (devp->config->packet_config ?
     packet_handler(devp, n, rxbuf, timeout) :
     buffered_handler(devp, n, rxbuf, timeout));

  /* Restore invariants */
  /* Release SPI bus */
#if SX1212_SHARED_SPI
  spiReleaseBus(devp->config->spip);
#endif

  return suspend_result; /* good or bad */
}

/**
 * @brief   Returns RSSI value in dBm
 * @pre     The SX1212 driver must be initialized. NSS_CONFIG and NSS_DATA are
 *          both released (if enabled). SX1212 is in Rx mode.
 * @pre     NSS_CONFIG and NSS_DATA are both released (if enabled). SX1212 is
 *          in Rx mode.
 *
 * @param[in] devp  pointer to the @p SX1212Driver object
 * @return    RSSI value in dBm (-115 to 12)
 *
 * @note    Accuracy is very poor outside of -100 to -40 range
 */
int8_t sx1212RSSI(SX1212Driver *devp) {
  uint8_t rssi = 0;

  /* Assume config SPI already started if not shared */
  /* Further assume both SPIs released if shared */
#if SX1212_SHARED_SPI
  spiAcquireBus(devp->config->spip);
  spiStart(devp->config->spip, devp->config->spicfgp);
#endif

  /* Assume we're already in RX mode */
  rssi = sx1212ReadRegister(devp, RXParam4);

#if SX1212_SHARED_SPI
  spiReleaseBus(devp->config->spip);
#endif

  return ((int8_t)(rssi >> 1)) - 115;
}

void sx1212CLKOUT(SX1212Driver *devp, uint8_t div) {

  sx1212SetRegister(devp, OSCParam, 0x80 | (div << 2));
}

void sx1212TUNE(SX1212Driver *devp) {
  sx1212SetMode(devp, SX1212Tx);
}

/** @} */
