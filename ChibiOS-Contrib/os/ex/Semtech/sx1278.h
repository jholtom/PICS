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
 * @file    sx1278.h
 * @brief   SX1278 RF transceiver module header.
 * 
 * @{
 */
#ifndef _SX1278_H_
#define _SX1278_H_

#include "hal.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   SX1278 Clock Frequency
 * @details Set to the frequency of the SX1278 oscillator
 * @note    The default is @p 32000000 Hz - unlikely to need changing
 */
#if !defined(SX1278_CLK_FREQ) || defined(__DOXYGEN__)
#define SX1278_CLK_FREQ     32000000
#endif

/**
 * @brief   SX1278 shared SPI switch.
 * @details If set to @p TRUE the device acquires SPI bus ownership on each
 *          transaction.
 * @note    The default is @p FALSE. Requires SPI_USE_MUTUAL_EXCLUSION
 */
#if !defined(SX1278_SHARED_SPI) || defined(__DOXYGEN__)
#define SX1278_SHARED_SPI   FALSE
#endif

/**
 * @brief   SX1278 Sync Value Type
 * @details Set to the data type of the SX1278 sync value
 * @note    The default is @p uint32_t - 4 bytes
 */
#if !defined(SX1278_SYNC_TYPE) || defined(__DOXYGEN__)
#define SX1278_SYNC_TYPE     uint32_t
#endif

/**
 * @brief   SX1278 Length Type for Unlimited Packet mode
 * @details Set to the data type of the SX1278 Length field for Unlimited
 *          packet mode
 * @note    The default is @p uint16_t - 2 bytes, allowing for 65535-byte
 *          packets including Address and Length fields.
 */
#if !defined(SX1278_LENGTH_TYPE) || defined(__DOXYGEN__)
#define SX1278_LENGTH_TYPE    uint16_t
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !HAL_USE_SPI
#error "SX1278 support requires SPI support"
#endif

#if SX1278_SHARED_SPI && !SPI_USE_MUTUAL_EXCLUSION
#error "SX1278_SHARED_SPI support requires SPI_USE_MUTUAL_EXCLUSION"
#endif

#define SX1278_FSTEP  (SX1278_CLK_FREQ >> 19)

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   SX1278 register constants
 */
typedef enum {
  RegFifo = 0x00,
  RegOpMode = 0x01,
  RegBitrateMsb = 0x02,
  RegBitrateLsb = 0x03,
  RegFdevMsb = 0x04,
  RegFdevLsb = 0x05,
  RegFrfMsb = 0x06,
  RegFrfMid = 0x07,
  RegFrfLsb = 0x08,
  RegPaConfig = 0x09,
  RegPaRamp = 0x0A,
  RegOcp = 0x0B,
  RegLna = 0x0C,
  RegRxConfig = 0x0D,
  RegRssiConfig = 0x0E,
  RegRssiCollision = 0x0F,
  RegRssiThresh = 0x10,
  RegRssiValue = 0x11,
  RegRxBw = 0x12,
  RegAfcBw = 0x13,
  RegOokPeak = 0x14,
  RegOokFix = 0x15,
  RegOokAvg = 0x16,
  RegRes17 = 0x17,
  RegRes18 = 0x18,
  RegRes19 = 0x19,
  RegAfcFei = 0x1A,
  RegAfcMsb = 0x1B,
  RegAfcLsb = 0x1C,
  RegFeiMsb = 0x1D,
  RegFeiLsb = 0x1E,
  RegPreambleDetect = 0x1F,
  RegRxTimeout1 = 0x20,
  RegRxTimeout2 = 0x21,
  RegRxTimeout3 = 0x22,
  RegRxDelay = 0x23,
  RegOsc = 0x24,
  RegPreambleMsb = 0x25,
  RegPreambleLsb = 0x26,
  RegSyncConfig = 0x27,
  RegSyncValue1 = 0x28,
  RegSyncValue2 = 0x29,
  RegSyncValue3 = 0x2A,
  RegSyncValue4 = 0x2B,
  RegSyncValue5 = 0x2C,
  RegSyncValue6 = 0x2D,
  RegSyncValue7 = 0x2E,
  RegSyncValue8 = 0x2F,
  RegPacketConfig1 = 0x30,
  RegPacketConfig2 = 0x31,
  RegPayloadLength = 0x32,
  RegNodeAdrs = 0x33,
  RegBroadcastAdrs = 0x34,
  RegFifoThresh = 0x35,
  RegSeqConfig1 = 0x36,
  RegSeqConfig2 = 0x37,
  RegTimerResol = 0x38,
  RegTimer1Coef = 0x39,
  RegTimer2Coef = 0x3A,
  RegImageCal = 0x3B,
  RegTemp = 0x3C,
  RegLowBat = 0x3D,
  RegIrqFlags1 = 0x3E,
  RegIrqFlags2 = 0x3F,
  RegDioMapping1 = 0x40,
  RegDioMapping2 = 0x41,
  RegVersion = 0x42,
  RegPllHop = 0x44,
  RegTcxo = 0x4B,
  RegPaDac = 0x4D,
  RegFormerTemp = 0x5B,
  RegBitrateFrac = 0x5D,
  RegAgcRef = 0x61,
  RegAgcThresh1 = 0x62,
  RegAgcThresh2 = 0x63,
  RegAgcThresh3 = 0x64,
  RegPll = 0x70
} SX1278RegConstants;

/**
 * @brief   SX1278 IRQ sources
 */
typedef enum {
  SX1278PacketSent = 0x00,
  SX1278FifoLevel,
  IRQMax
} SX1278IRQConstants;

/**
 * @brief   SX1278 system modes
 */
typedef enum {
  SX1278Sleep = 0x00,
  SX1278Stdby = 0x01,
  SX1278FSTx = 0x02,
  SX1278Tx = 0x03,
  SX1278FSRx = 0x04,
  SX1278Rx = 0x05
} SX1278ModeConstants;

/**
 * @brief SX1278 packet handling modes
 */
typedef enum {
  SX1278Fixed = 0x00,
  SX1278Variable = 0x01,
  SX1278Unlimited = 0x02
} SX1278PacketModes;

/**
 * @brief Driver state machine possible states
 */
typedef enum {
  SX1278_UNINIT = 0, /**< Not initialized */
  SX1278_STOP = 1, /**< Stopped. */
  SX1278_IDLE = 2, /**< Idle. */
  SX1278_ACTIVE = 3, /**< Active. */
  SX1278_SLEEP = 4, /**< Sleep. */
} sx1278_state_t;

/**
 * @brief SX1278 filter params
 */
typedef enum {
  NO_FILTER = 0,
  BT_1_0 = 1,
  BT_0_5 = 2,
  BT_0_3 = 3
} sx1278_filter_t;

typedef struct {
  /**
   * @brief Packet format
   */
  uint8_t format:2;
  /**
   * @brief Whitening
   */
  uint8_t whitening:1;
  /**
   * @brief Manchester encoding
   */
  uint8_t manchester:1;
  /**
   * @brief CRC generation
   */
  uint8_t crc:1;
  /* IBM CRC and whitening not supported due to Errata */
  /**
   * @brief Preamble polarity (0 for 0xAA, 1 for 0x55)
   */
  uint8_t preamble_polarity:1;
  /**
   * @brief Use addressing
   */
  uint8_t addressing:1;
  /**
   * @brief Use length byte (for Unlimited packet mode)
   */
  uint8_t length:1;
} sx1278_packet_config_t;

/**
 * @brief   SX1278 configuration structure.
 */
typedef struct {
  /**
   * @brief SPI driver associated to this SX1278.
   */
  SPIDriver       *spip;
  /**
   * @brief SPI configuration associated to this SX1278.
   */
  SPIConfig *spicfgp;
  /**
   * @brief Bit rate
   */
  uint32_t bitrate;
  /**
   * @brief Center frequency
   */
  uint32_t freq;
  /**
   * @brief Frequency deviation
   */
  uint32_t fdev;
  /**
   * @brief Filtering
   */
  sx1278_filter_t filter;
  /**
   * @brief Output power
   */
  uint8_t pow;
  /**
   * @brief Preamble length
   */
  uint16_t preamble_len;
  /**
   * @brief Sync word
   */
  SX1278_SYNC_TYPE sync_word;
  /**
   * @brief IRQ mapping
   * @note This is a mapping from IRQ sources to DIO pins. An index >5 means
   *       the source is not mapped.
   */
  uint8_t irq_map[IRQMax];
  /**
   * @brief Reset line
   */
  ioline_t reset_line;
  /**
   * @brief Slave select line
   */
  ioline_t ss_line;
  /** 
   * @brief Packet handling configuration settings
   */
  sx1278_packet_config_t *packet_config;
  /**
   * @brief Address (when enabled)
   */
  uint8_t addr;
  /**
   * @brief Length (for fixed size packet mode)
   */
  uint16_t length;
  /**
   * @brief DIO mapping
   * @note This is a mapping from DIO pins to ioline_ts. PAL_NOLINE means the
   *       DIO is not mapped.
   */
  ioline_t dio_map[6];
} SX1278Config;

/**
 * @brief   Registers of SX1278 that need to be preserved
 */
typedef struct {
  /**
   * @brief Operating mode register
   */
  uint8_t opmode;
  /**
   * @brief Top Level Sequencer config register
   */
  uint8_t seq_config;
} SX1278Regs;

/**
 * @brief   Structure representing an SX1278 driver.
 */
typedef struct SX1278Driver SX1278Driver;

/* TODO extract this out into a proper class once you've finished the driver */
/**
 * @brief   SX1278 RF transceiver class.
 */
struct SX1278Driver {
  /**
   * @brief Configuration
   */
  const SX1278Config *config;
  /**
   * @brief Driver state
   */
  sx1278_state_t state;
  /**
   * @brief Preserved registers
   */
  SX1278Regs regs;
  spicallback_t callback;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern SX1278Driver SX1278D1;

#ifdef __cplusplus
extern "C" {
#endif
  void sx1278ObjectInit(SX1278Driver *devp);
  void sx1278Start(SX1278Driver *devp, const SX1278Config *config);
  void sx1278Stop(SX1278Driver *devp);
  msg_t sx1278Send(SX1278Driver *devp, uint8_t *buffer, SX1278_LENGTH_TYPE len,
      systime_t timeout);
  void sx1278Sleep(SX1278Driver *devp);
  void sx1278SetFrequency(SX1278Driver *devp, uint32_t freq);
  void sx1278SetBitrate(SX1278Driver *devp, uint32_t rate);
  void sx1278SetDeviation(SX1278Driver *devp, uint32_t fdev);
  void sx1278SetFilterParams(SX1278Driver *devp, sx1278_filter_t filter);
  void sx1278SetSync(SX1278Driver *devp, SX1278_SYNC_TYPE sync);
  size_t sx1278StartTransmit(SX1278Driver * devp, size_t n, uint8_t * buffer,
    palcallback_t pal_cb, spicallback_t spi_cb);
  void sx1278FifoWriteAsyncS(SX1278Driver * devp, size_t n, uint8_t * buffer,
    spicallback_t cb);
  void sx1278FifoWriteAsync(SX1278Driver * devp, size_t n, uint8_t * buffer,
    spicallback_t cb);
  void sx1278SetPower(SX1278Driver * devp, uint8_t pow);
  uint8_t sx1278SetRegister(SX1278Driver *devp, uint8_t reg, uint8_t value);
  uint8_t sx1278ReadRegister(SX1278Driver *devp, uint8_t reg);
#ifdef __cplusplus
}
#endif

#endif /* _SX1278_H_*/

/** @} */
