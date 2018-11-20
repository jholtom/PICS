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
 * @file    sx1278.c
 * @brief   SX1278 RF transcevier module code.
 *
 * @addtogroup sx1278
 * @{
 */

#include "stdlib.h"
#include "math.h"

#include "hal.h"

#include "sx1278.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define BLOCK_RETURN_ERROR(x)                                               \
  suspend_result = (x);                                                     \
  if (suspend_result != MSG_OK) {                                           \
    devp->state = SX1278_IDLE;                                              \
    return suspend_result;                                                  \
  }

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

SX1278Driver SX1278D1;

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

static thread_reference_t tr; /* For suspending thread while waiting for ISRs */
static const uint8_t irqbits[IRQMax] = { 10, 12 }; /* Bit index of IRQ src */
static msg_t suspend_result;

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

static void blocking_callback(void) {
  osalSysLockFromISR();
  osalThreadResumeI(&tr, MSG_OK);
  osalSysUnlockFromISR();
}

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Reads register value.
 * @pre     The SX1278 driver must be initialized
 * 
 * @param[in] devp    pointer to the SX1278 driver
 * @param[in] reg     register address
 * @return            the read value
 */
uint8_t sx1278ReadRegister(SX1278Driver *devp, uint8_t reg) {
  
  osalDbgAssert(reg & 0x7FU, "register address out of bounds");
  
  palClearLine(devp->config->ss_line);
  spiPolledExchange(devp->config->spip, reg);
  reg = spiPolledExchange(devp->config->spip, 0xFFU);
  palSetLine(devp->config->ss_line);
  
  return reg;
}
  
/**
 * @brief   Sets register value.
 * @pre     The SX1278 driver must be initialized
 * 
 * @param[in] devp    pointer to the SX1278 driver
 * @param[in] reg     register address
 * @param[in] value   the value to be written
 * @return            the previous value of the register
 */
uint8_t sx1278SetRegister(SX1278Driver *devp, uint8_t reg, 
    uint8_t value) {
  
  osalDbgAssert(reg & 0x7FU, "register address out of bounds");
  
  osalDbgAssert(reg != 0x11U && reg != 0x3CU && reg != 0x42U, 
      "setting read-only registers");
  
  palClearLine(devp->config->ss_line);
  spiPolledExchange(devp->config->spip, (reg | 0x80));
  reg = spiPolledExchange(devp->config->spip, value);
  palSetLine(devp->config->ss_line);
  
  return reg;
}

/**
 * @brief   Reads multiple register values.
 * @pre     The SX1278 driver must be initialized
 * 
 * @param[in]  devp   pointer to the SX1278 driver
 * @param[in]  start  first register address to be read
 * @param[in]  count  number of sequential registers to read from
 * @param[out] values buffer to be filled with the read values
 */
static void sx1278ReadRegisters(SX1278Driver *devp, uint8_t start, 
    uint8_t *values, uint8_t count) {
  
  osalDbgAssert(!(start & 0x7FU), "register address out of bounds");
  
  palClearLine(devp->config->ss_line);
  spiPolledExchange(devp->config->spip, start);
  spiReceive(devp->config->spip, count, values);
  palSetLine(devp->config->ss_line);
}

/**
 * @brief   Writes multiple register values.
 * @pre     The SX1278 driver must be initialized
 * 
 * @param[in]  devp   pointer to the SX1278 driver
 * @param[in]  start  first register address to be written to
 * @param[in]  count  number of sequential registers to write to
 * @param[out] values buffer of values to be written
 */
static void sx1278SetRegisters(SX1278Driver *devp, uint8_t start, 
    uint8_t *values, uint8_t count) {
  
  osalDbgAssert(!(start & 0x7FU), "register address out of bounds");
  
  palClearLine(devp->config->ss_line);
  spiPolledExchange(devp->config->spip, (start | 0x80));
  spiSend(devp->config->spip, count, values);
  palSetLine(devp->config->ss_line);
}

/**
 * @brief   Sets the transceiver's bit rate
 * @pre     The SX1278 driver must be initialized
 * 
 * @param[in] devp pointer to the SX1278 driver
 * @param[in] rate bit rate in bits per second to be used
*/
void sx1278SetBitrate(SX1278Driver *devp, uint32_t rate) {
  uint32_t tmp = (SX1278_CLK_FREQ << 4)/ rate;
  uint16_t regs = tmp >> 4;
  uint8_t frac = tmp & 0x000F;
  
  osalDbgAssert(rate < 300000, "bit rate too high");
  
  sx1278SetRegister(devp, RegBitrateMsb, (regs >> 8));
  sx1278SetRegister(devp, RegBitrateLsb, (regs & 0x00FF));
  sx1278SetRegister(devp, RegBitrateFrac, frac);
}

/**
 * @brief   Sets the transceiver's frequency deviation
 * @pre     The SX1278 driver must be initialized
 * 
 * @param[in] devp      pointer to the SX1278 driver
 * @param[in] deviation frequency deviation to be programmed, in Hz
*/
void sx1278SetDeviation(SX1278Driver *devp, uint32_t fdev) {
  /* Fancy math for freq / FSTEP to correct for roundings */
  uint16_t regs = ((fdev << 11) + (SX1278_CLK_FREQ >> 9)) / 
    (SX1278_CLK_FREQ >> 8);
  
  osalDbgAssert(fdev < 200000, "deviation too high");
  
  sx1278SetRegister(devp, RegFdevMsb, (regs >> 8));
  sx1278SetRegister(devp, RegFdevLsb, (regs & 0x00FF));
}

/**
 * @brief   Sets the transceiver's center frequency
 * @pre     The SX1278 driver must be initialized
 * 
 * @param[in] devp      pointer to the SX1278 driver
 * @param[in] deviation center frequency to be programmed, in Hz
*/
void sx1278SetFrequency(SX1278Driver *devp, uint32_t freq) {
  /* Fancy math for freq / FSTEP to keep everything from rounding */
  uint32_t regs = (((freq + (SX1278_CLK_FREQ >> 12)) / 
        (SX1278_CLK_FREQ >> 11)) << 8);
  
  if ((freq >= 137000000 && freq <= 175000000) || 
      (freq >= 410000000 && freq <= 525000000)) {
    /* Bands 2 + 3 */
    devp->regs.opmode |= (1 << 3);
  }
  else if (freq >= 862000000 && freq <= 1020000000) {
    /* Band 1 */
    devp->regs.opmode &= ~(1 << 3);
  }
  else {
    osalDbgAssert(false, "frequency out of range");
  }
  
  osalDbgAssert(!(regs & 0xFF000000), "incorrect register value calculated");
  
  sx1278SetRegister(devp, RegOpMode, devp->regs.opmode);
  sx1278SetRegister(devp, RegFrfMsb, ((regs >> 16) & 0x000000FF));
  sx1278SetRegister(devp, RegFrfMid, ((regs >> 8) & 0x000000FF));
  sx1278SetRegister(devp, RegFrfLsb, (regs & 0x000000FF));
}

/**
 * @brief   Sets the transmitter's output power
 * @pre     The SX1278 driver must be initialized
 * 
 * @param[in] devp pointer to the SX1278 driver
 * @param[in] pow  output power will be programmed as [10+(.2 * pow)], in dBm
*/
void sx1278SetPower(SX1278Driver * devp, uint8_t pow) {
  /* TODO figure out how to set power properly. also add closed loop. */
  sx1278SetRegister(devp, RegPaConfig, pow);
  return;
}

/**
 * @brief   Sets the length of the transceiver's preamble
 * @pre     The SX1278 driver must be initialized
 * 
 * @param[in] devp   pointer to the SX1278 driver
 * @param[in] length length of the preamble
*/
void sx1278SetFilterParams(SX1278Driver *devp, sx1278_filter_t filter) {
  osalDbgAssert(filter < 4, "I miss real enums...");
 
  //sx1278SetRegister(devp, RegPaRamp, (filter << 5));
  //Build my own PaRamp setting, because why not do it right???
  sx1278SetRegister(devp, RegPaRamp, 111);
}

/**
 * @brief   Sets the length of the transceiver's preamble
 * @pre     The SX1278 driver must be initialized
 * 
 * @param[in] devp   pointer to the SX1278 driver
 * @param[in] length length of the preamble
*/
static void sx1278SetPreambleLength(SX1278Driver *devp, uint16_t length) {
  
  sx1278SetRegister(devp, RegPreambleMsb, length >> 8);
  sx1278SetRegister(devp, RegPreambleLsb, length & 0x00FF);
}

/**
 * @brief   Sets the transceiver's sync value
 * @pre     The SX1278 driver must be initialized
 * 
 * @param[in] devp  pointer to the SX1278 driver
 * @param[in] bytes length of the sync value in bytes
 * @param[in] value pointer to the sync value
*/
void sx1278SetSync(SX1278Driver *devp, SX1278_SYNC_TYPE sync) {
  uint8_t reg = 0x00;
  
  osalDbgAssert(sizeof(sync) <= 8, "requested sync length too large");

  reg |= devp->config->packet_config->preamble_polarity << 5;
  
  if (sync == 0) {
    reg |= 0x80;
    return;
  }
  else {
    reg |= 0x90 | (sizeof(sync) - 1);
  }
  
  sx1278SetRegister(devp, RegSyncConfig, reg);
  
  for (unsigned int i = 0; i < sizeof(sync); i++) {
    osalDbgAssert(((sync >> (8 * (sizeof(sync) - 1 - i))) & 0xFF) != 0x00, 
        "sync can't contain 0 bytes");
    sx1278SetRegister(devp, RegSyncValue1 + i, 
        (sync >> (8 * (sizeof(sync) - 1 - i)) & 0xFF));
  }
}

/**
 * @brief   Resets the transceiver
 * 
 * @param[in] devp pointer to the SX1278 driver
 */
static void sx1278Reset(SX1278Driver *devp) {
  
  osalDbgAssert(devp->config->reset_line != PAL_NOLINE, "reset line required");
  
  palSetLineMode(devp->config->reset_line, PAL_MODE_OUTPUT_PUSHPULL);
  palClearLine(devp->config->reset_line);
  osalThreadSleepMicroseconds(100);
  palSetLine(devp->config->reset_line);
  /*palSetLineMode(devp->config->reset_line, PAL_MODE_INPUT_PULLUP);*/
  osalThreadSleepMilliseconds(5);
}

/**
 * @brief   Sets the SX1278 system mode
 * @pre     The SX1278 driver must be initialized
 * 
 * @param[in] devp pointer to the SX1278 driver
 * @param[in] mode the mode to transition the driver to
 */
static void sx1278SetMode(SX1278Driver *devp, SX1278ModeConstants mode) {
  
  osalDbgAssert(mode < 0x06U, "requested invalid transceiver mode");
  
  devp->regs.opmode = (devp->regs.opmode & 0xF8) | mode;
  sx1278SetRegister(devp, RegOpMode, devp->regs.opmode);
}

/**
 * @brief   Sets the SX1278 Digital I/O mapping
 * @pre     The SX1278 driver must be initialized and started
 * 
 * @param[in] devp    pointer to the SX1278 driver
 * @param[in] dio_map the mapping of SX1278 IRQ signals to DIO pins
 */
static void sx1278MapIRQs(SX1278Driver *devp, 
    const uint8_t irq_map[IRQMax]) {
  /* Configure IRQ mapping. Mapping PacketSent to DIO0 and FifoLevel to DIO1 
   * is recommended. */
  /* Presently this whole function does nothing but assert the reset state.
   * Someday we might want to use other IRQs and we'd add them here */
  
  for (SX1278IRQConstants i = 0; i < IRQMax; i++) {
    if (irq_map[i] > 5) continue;
    switch (i) {
      case SX1278PacketSent:
        osalDbgAssert(irq_map[i] == 0, "IRQ mapping invalid");
        /* The below is obviously a no-op - included for clarity */
        /*dio_reg |= (0x00 << 6);*/
        break;
      case SX1278FifoLevel:
        osalDbgAssert(irq_map[i] == 1, "IRQ mapping invalid");
        /* The below is obviously a no-op - included for clarity */
        /*dio_reg |= (0x00 << 4);*/
        break;
      default:
        osalDbgAssert(false, "Invalid IRQ mapping requested");
        break;
    }
  }
  
  sx1278SetRegister(devp, RegDioMapping1, 0);
  sx1278SetRegister(devp, RegDioMapping2, 0x40);
}

/**
 * @brief   Sets the SX1278 packet configuration
 * @pre     The SX1278 driver must be initialized and started
 * 
 * @param[in] devp          pointer to the SX1278 driver
 * @param[in] packet_config the packet configuration to apply
 */
static void sx1278ConfigurePackets(SX1278Driver* devp, 
    sx1278_packet_config_t *packet_config) {
  /* Configure packet mode */
  uint8_t reg = 0x00;
  
  /* Unlimited - PacketFormat == 0, PayloadLength == 0 */
  if (packet_config->format == SX1278Variable) {
    reg |= 1 << 7;
  }
  /* else {
   *  regs |= 0 << 7;
   * }*/
  
  osalDbgAssert(!(packet_config->whitening && packet_config->manchester), 
      "Whitening and Manchester encoding are mutually exclusive");
  reg |= packet_config->whitening << 6;
  reg |= packet_config->manchester << 5;
  
  reg |= packet_config->crc << 4;
  
  sx1278SetRegister(devp, RegPacketConfig1, reg);
  
  if (packet_config->format == SX1278Fixed) {
    sx1278SetRegister(devp, RegPacketConfig2, 
        0x40 | (devp->config->length >> 8));
    sx1278SetRegister(devp, RegPayloadLength, (devp->config->length & 0x00FF));
  }
  else {
    sx1278SetRegister(devp, RegPacketConfig2, 0x40);
    sx1278SetRegister(devp, RegPayloadLength, 0x00);
  }
}

static msg_t sx1278BlockTimeout(SX1278Driver *devp, SX1278IRQConstants irq, 
    uint8_t edge, systime_t timeout) {
  uint8_t dio_index;
  uint8_t reg;
  uint8_t bit;
  
  osalDbgAssert (irq < IRQMax, "IRQ source not supported");
  
  dio_index = devp->config->irq_map[irq];
  
  if (dio_index > 5 || devp->config->dio_map[dio_index] == PAL_NOLINE) {
    /* BusyWait on SPI */
    if (irqbits[irq] > 7) {
      reg = RegIrqFlags2;
      bit = (1 << (irqbits[irq] >> 8));
    }
    else {
      reg = RegIrqFlags1;
      bit = (1 << irqbits[irq]);
    }
    systime_t start = osalOsGetSystemTimeX();
    while ((!(sx1278ReadRegister(devp, reg) & bit)) == 
        (edge == PAL_EVENT_MODE_RISING_EDGE)) {
      if (timeout == TIME_IMMEDIATE) return MSG_TIMEOUT;
      if (timeout != TIME_INFINITE && 
          !osalOsIsTimeWithinX(osalOsGetSystemTimeX(), start, start+timeout)) {
        return MSG_TIMEOUT;
      }
    }
    
    return MSG_OK;
  }
  else {
    /* Block on interrupt, yield CPU */
#if SX1278_SHARED_SPI
    spiReleaseBus(devp->config->spip);
#endif
    osalSysLock();
    palLineEnableEventI(devp->config->dio_map[dio_index], edge, blocking_callback); 
    if (palReadLine(devp->config->dio_map[dio_index])) {
      suspend_result = MSG_OK;
    }
    else {
      suspend_result = osalThreadSuspendTimeoutS(&tr, timeout);
    }
    
    /* Disable interrupt */
    palLineDisableEventI(devp->config->dio_map[dio_index]);
    osalSysUnlock();
    
#if SX1278_SHARED_SPI
    spiAcquireBus(devp->config->spip);
    spiStart(devp->config->spip);
#endif
    
    return suspend_result;
  }
}

static msg_t sx1278SendPacket(SX1278Driver *devp, uint8_t *buf, 
    SX1278_LENGTH_TYPE len, systime_t timeout) {
  SX1278_LENGTH_TYPE index = 0;
  uint8_t towrite;
  
  do {
    /* Fill the FIFO */
    /* At least 75% is available, so write max(75%, packet size) */
    towrite = (len - index > 64 ? 48 : len - index);
    sx1278SetRegisters(devp, RegFifo, buf + index, towrite);
    index += towrite;
    
    /* Wait until we see a FifoLevel falling edge */
    BLOCK_RETURN_ERROR(
      sx1278BlockTimeout(devp, SX1278FifoLevel, PAL_EVENT_MODE_FALLING_EDGE, timeout)
      );
  } while(index < len);
  
  /* Wait until we see a PacketSent rising edge */
  BLOCK_RETURN_ERROR(
    sx1278BlockTimeout(devp, SX1278PacketSent, PAL_EVENT_MODE_RISING_EDGE, timeout)
    );
  
  return MSG_OK;
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Initializes an instance.
 *
 * @param[out] devp     pointer to the @p SX1278Driver object
 *
 * @init
 */
void sx1278ObjectInit(SX1278Driver *devp) {
  
  /* Set all the stored registers to their reset states */
  devp->regs.opmode = 0x09;
  devp->regs.seq_config = 0x00;
  
  devp->state = SX1278_STOP;
}

void spi_callback(SPIDriver *spip) {
  palSetLine(SX1278D1.config->ss_line);
  if (SX1278D1.callback != NULL) {
    SX1278D1.callback(spip);
  }
}

/**
 * @brief   Configures and activates SX1278 Complex Driver peripheral.
 *
 * @param[in] devp      pointer to the @p SX1278Driver object
 * @param[in] config    pointer to the @p SX1278Config object
 *
 * @api
 */
void sx1278Start(SX1278Driver *devp, const SX1278Config *config) {
  
  devp->config = config;
  /* Reset the device to get it into a known state */
  sx1278Reset(devp);
#if SX1278_SHARED_SPI
  spiAcquireBus(config->spip);
#endif
  /* Add the SPI callback to the SPI config */
  config->spicfgp->end_cb = spi_callback;
  /* Start device SPI interface */
  spiStart(config->spip, config->spicfgp);
  /* Put the device into sleep mode */
  sx1278SetMode(devp, SX1278Sleep);
  /* Set device configuration */
  sx1278SetFrequency(devp, config->freq);
  sx1278SetDeviation(devp, config->fdev);
  sx1278SetBitrate(devp, config->bitrate);
  sx1278SetPower(devp, config->pow);
  sx1278SetPreambleLength(devp, config->preamble_len);
  sx1278SetSync(devp, config->sync_word);
  
  /* Configure the Top Level Sequencer to transmit when FiFo is filled */
  /* Set FromStart bits to 11 
  Don't set SequencerStart, SequencerStop, FromTransmit, and LowPowerSelection
  Don't care FromIdle, FromReceive, FromRxTimeout, FromPacketReceived */
  devp->regs.seq_config = 0x18;
  
  sx1278SetRegister(devp, RegSeqConfig1, devp->regs.seq_config);
  
  sx1278ConfigurePackets(devp, devp->config->packet_config);
  
  sx1278MapIRQs(devp, devp->config->irq_map);

#if SX1278_SHARED_SPI
  spiReleaseBus(config->spip);
#endif
  
  devp->state = SX1278_IDLE;
}

/**
 * @brief   Deactivates the SX1278 Complex Driver peripheral.
 *
 * @param[in] devp       pointer to the @p SX1278Driver object
 *
 * @api
 */
void sx1278Stop(SX1278Driver *devp) {
  
  /* Reset the device to get it into a known state */
  sx1278Reset(devp);
#if SX1278_SHARED_SPI
  spiAcquireBus(devp->config->spip);
#endif
  /* Start device SPI interface - in case of repeated stops */
  spiStart(devp->config->spip, devp->config->spicfgp);
  /* Put the device into sleep mode */
  sx1278SetMode(devp, SX1278Sleep);
  /* Set all the stored registers to their reset states */
  devp->regs.opmode = 0x09;
  spiStop(devp->config->spip);
#if SX1278_SHARED_SPI
  spiReleaseBus(devp->config->spip);
#endif
  
  devp->state = SX1278_STOP;
}

/**
 * @brief   Puts the SX1278 Complex Driver peripheral to sleep
 *
 * @param[in] devp       pointer to the @p SX1278Driver object
 *
 * @api
 */
void sx1278Sleep(SX1278Driver *devp) {
  
#if SX1278_SHARED_SPI
  spiAcquireBus(devp->config->spip);
  spiStart(devp->config->spip, devp->config->spicfgp);
#endif
  /* Put the device into sleep mode */
  sx1278SetMode(devp, SX1278Sleep);
#if SX1278_SHARED_SPI
  spiReleaseBus(devp->config->spip);
#endif
  
  devp->state = SX1278_SLEEP;
}

void sx1278FifoWriteAsync(SX1278Driver * devp, size_t n, uint8_t * buffer,
    spicallback_t cb) {
  devp->callback = cb;
  palClearLine(devp->config->ss_line);
  spiPolledExchange(devp->config->spip, (RegFifo | 0x80));
  devp->config->spicfgp->end_cb = spi_callback;
  spiStartSend(devp->config->spip, n, buffer);
}

void sx1278FifoWriteAsyncS(SX1278Driver * devp, size_t n, uint8_t * buffer,
    spicallback_t cb) {
  devp->callback = cb;
  palClearLine(devp->config->ss_line);
  spiPolledExchange(devp->config->spip, (RegFifo | 0x80));
  devp->config->spicfgp->end_cb = spi_callback;
  spiStartSendI(devp->config->spip, n, buffer);
}

size_t sx1278StartTransmit(SX1278Driver * devp, size_t n, uint8_t * buffer, 
    palcallback_t pal_cb, spicallback_t spi_cb) {
  
  /* We're in Sleep or Standby, TLS isn't started yet */
  /* Set FifoThreshold to 25% or packet size, whichever's smaller */
  sx1278SetRegister(devp, RegFifoThresh, (n > 24 ? 23 : n-1));
  /* Stop TLS */
  devp->regs.seq_config &= ~(0x80);
  sx1278SetRegister(devp, RegSeqConfig1, devp->regs.seq_config | 0x40);
  /* Start TLS */
  devp->regs.seq_config |= 0x80;
  sx1278SetRegister(devp, RegSeqConfig1, devp->regs.seq_config);
  
  /* Configure FifoLevel interrupt */
  chSysLock();
  palLineEnableEventI(devp->config->dio_map[devp->config->irq_map[SX1278FifoLevel]],
      PAL_EVENT_MODE_FALLING_EDGE, pal_cb);
  chSysUnlock();
  /* Fill the FIFO */
  uint8_t to_write = (n > 47 ? 47 : n);
  sx1278FifoWriteAsync(devp, to_write, buffer, spi_cb);
  return to_write;
}

/**
 * @brief   Send a packet of data
 * @pre     The SX1278 driver must be initialized and started
 * 
 * @param[in] devp pointer to the SX1278 driver
 * @param[in] buf  pre-allocated buffer for the packet, including room for
 *                 address and length bytes if used.
 * @param[in] len  length of the buffer, including the address and length bytes
 *                 if used.
 *                 
 * @api
 */
msg_t sx1278Send(SX1278Driver *devp, uint8_t *buf, SX1278_LENGTH_TYPE len, 
    systime_t timeout) {
  
  osalDbgAssert(devp->state == SX1278_IDLE || devp->state == SX1278_SLEEP, 
      "State machine error!");
  
  devp->state = SX1278_ACTIVE;
  
  if (devp->config->packet_config->format == SX1278Fixed) {
    osalDbgAssert(len == devp->config->length, "Fixed packet wrong length");
    
    /* Tack on address if used */
    if (devp->config->packet_config->addressing) {
      buf[0] = devp->config->addr;
    }
  }
  
  else if (devp->config->packet_config->format == SX1278Variable) {
    osalDbgAssert(len <= 256, "Variable length packet too large");
    
    /* Calculate length byte */
    buf[0] = (len-1);
    
    /* Tack on address if used */
    if (devp->config->packet_config->addressing) {
      buf[1] = devp->config->addr;
    }
  }
  
  else if (devp->config->packet_config->format == SX1278Unlimited) {
    /* TODO document that length is the length offset */
      
    if (devp->config->packet_config->length) {
      
      osalDbgAssert((!devp->config->packet_config->addressing) || 
          devp->config->length > 0, "Length offset conflicts with address");
      
      /* Insert length value */
      /* Send length in network byte order - big endian */
      /* This should work regardless of host endianness */
      for (int i = sizeof(len)-1; i >= 0; i--) {
        buf[devp->config->length + i] = (len >> (8 * i)) & 0xFFu;
      }
    }
      
    /* Tack on address if used */
    if (devp->config->packet_config->addressing) {
      buf[0] = devp->config->addr;
    }
  }
  
#if SX1278_SHARED_SPI
  spiAcquireBus(config->spip);
  spiStart(devp->config->spip, devp->config->spicfgp);
#endif
  
  /* We're in Sleep or Standby, TLS isn't started yet */
  /* Set FifoThreshold to 25% or packet size, whichever's smaller */
  sx1278SetRegister(devp, RegFifoThresh, (len > 16 ? 15 : len-1));
  /* Start TLS */
  devp->regs.seq_config &= ~(0x40);
  devp->regs.seq_config |= 0x80;
  sx1278SetRegister(devp, RegSeqConfig1, devp->regs.seq_config);
  
  /* Send packet */
  suspend_result = sx1278SendPacket(devp, buf, len, timeout);
  
  /* Stop TLS */
  devp->regs.seq_config &= ~(0x80);
  devp->regs.seq_config |= 0x40;
  sx1278SetRegister(devp, RegSeqConfig1, devp->regs.seq_config);
  
  /* Restore invariants */
#if SX1278_SHARED_SPI
  spiReleaseBus(devp->config->spip);
#endif
  devp->state = SX1278_IDLE;
  
  return suspend_result;
}

/** @} */
