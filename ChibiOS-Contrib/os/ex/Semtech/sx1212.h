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
 * @file    sx1212.h
 * @brief   SX1212 RF transceiver module header.
 * 
 * @{
 */
#ifndef _SX1212_H_
#define _SX1212_H_

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
 * @brief   SX1212 Clock Frequency
 * @details Set to the frequency of the SX1212 oscillator
 * @note    The default is @p 12800000 Hz - unlikely to need changing
 */
#if !defined(SX1212_CLK_FREQ) || defined(__DOXYGEN__)
#define SX1212_CLK_FREQ     12800000
#endif

/**
 * @brief   SX1212 shared SPI switch.
 * @details If set to @p TRUE the device acquires SPI bus ownership on each
 *          transaction.
 * @note    The default is @p FALSE. Requires SPI_USE_MUTUAL_EXCLUSION
 */
#if !defined(SX1212_SHARED_SPI) || defined(__DOXYGEN__)
#define SX1212_SHARED_SPI   FALSE
#endif

/**
 * @brief   SX1212 Sync Value Type
 * @details Set to the data type of the SX1212 sync value
 * @note    The default is @p uint32_t - 4 bytes
 */
#if !defined(SX1212_SYNC_TYPE) || defined(__DOXYGEN__)
#define SX1212_SYNC_TYPE     uint32_t
#endif

/**
 * @brief   SX1212 Length Type for Buffered Packet mode
 * @details Set to the data type of the SX1212 Length field for Unlimited
 *          packet mode
 * @note    The default is @p uint16_t - 2 bytes, allowing for 65535-byte
 *          packets including Address and Length fields.
 */
#if !defined(SX1212_LENGTH_TYPE) || defined(__DOXYGEN__)
#define SX1212_LENGTH_TYPE    uint16_t
#endif

/**
 * @brief   SX1212 IRQ_0 line
 * @details Set to the ioline_t corresponding to IRQ_0 for the SX1212
 * @note    The default is to leave this undefined. If left undefined, SPI
 *          register reads are used as fallbacks. This mode is much less
 *          efficient.
 */
#if !defined(SX1212_IRQ_0) || defined(__DOXYGEN__)
#define SX1212_IRQ_0          PAL_NOLINE
#undef SX1212_IRQ_0
#endif

/**
 * @brief   SX1212 IRQ_1 line
 * @details Set to the ioline_t corresponding to IRQ_1 for the SX1212
 * @note    The default is to leave this undefined. If left undefined, SPI
 *          register reads are used as fallbacks. This mode is much less
 *          efficient.
 */
#if !defined(SX1212_IRQ_1) || defined(__DOXYGEN__)
#define SX1212_IRQ_1          PAL_NOLINE
#undef SX1212_IRQ_1
#endif

/**
 * @brief   SX1212 PLL_LOCK line
 * @details Set to the ioline_t corresponding to PLL_LOCK for the SX1212
 * @note    The default is to leave this undefined. If left undefined, SPI
 *          register reads are used as fallbacks. This mode is less efficient.
 */
#if !defined(SX1212_PLL_LOCK) || defined(__DOXYGEN__)
#define SX1212_PLL_LOCK       PAL_NOLINE
#undef SX1212_PLL_LOCK
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !HAL_USE_SPI
#error "SX1212 support requires SPI support"
#endif

#if SX1212_SHARED_SPI && !SPI_USE_MUTUAL_EXCLUSION
#error "SX1212_SHARED_SPI support requires SPI_USE_MUTUAL_EXCLUSION"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   SX1212 register constants
 */
typedef enum {
  MCParam0 = 0,
  MCParam1 = 1,
  MCParam2 = 2,
  MCParam3 = 3,
  MCParam4 = 4,
  MCParam5 = 5,
  MCParam6 = 6,
  MCParam7 = 7,
  MCParam8 = 8,
  MCParam9 = 9,
  MCParam10 = 10,
  MCParam11 = 11,
  IRQParam0 = 12,
  IRQParam1 = 13,
  IRQParam2 = 14,
  IRQParam3 = 15,
  RXParam0 = 16,
  RXParam1 = 17,
  RXParam2 = 18,
  RXParam3 = 19,
  RXParam4 = 20,
  RXParam5 = 21,
  SYNCParam0 = 22,
  SYNCParam1 = 23,
  SYNCParam2 = 24,
  SYNCParam3 = 25,
  TXParam = 26,
  OSCParam = 27,
  PKTParam0 = 28,
  PKTParam1 = 29,
  PKTParam2 = 30,
  PKTParam3 = 31
} SX1212RegConstants;

/**
 * @brief   SX1212 IRQ mappings
 */
typedef enum {
  Sync = 0xC0,
  Fifoempty_B = 0x80, /* inverted */
  Write_byte = 0x40,
  Payload_ready = 0x00,
  Fifo_threshold = 0x30,
  RSSI = 0x20,
  Fifofull = 0x10,
  CRC_ok = 0x00
} SX1212IRQConstants;

/**
 * @brief   SX1212 system modes
 */
typedef enum {
  SX1212Sleep = 0x00,
  SX1212Stdby = 0x20,
  SX1212FS = 0x40,
  SX1212Rx = 0x60,
  SX1212Tx = 0x80
} SX1212ModeConstants;

/**
 * @brief SX1212 packet handling modes
 */
typedef enum {
  SX1212Fixed = 0x00,
  SX1212Variable = 0x01
} SX1212PacketModes;

/**
 * @brief SX1212 broadcast modes
 */
typedef enum {
  None = 0x00,
  Broadcast_00 = 0x02,
  Broadcast_FF = 0x03
} SX1212BroadcastModes;

/* TODO add the forward declaration */
typedef struct SX1212Driver_t SX1212Driver;
/**
 * @brief SX1212 data received callback
 */
typedef void (*sx1212_rx_cb)(SX1212Driver *devp, size_t n, uint8_t *rxbuf);

/**
 * @brief SX1212 data ready callback
 */
typedef void (*sx1212_ready_cb)(SX1212Driver *devp);

typedef struct {
  /**
   * @brief Packet format
   */
  uint8_t format:1;
  /* OOK not supported */
  /* Low power mode not supported - no one knows what this does */
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
  /**
   * @brief Use addressing
   */
  uint8_t addressing:1;
  /**
   * @brief Broadcast mode
   */
  uint8_t broadcast:2;
  /* preamble_len only relevant in Tx mode - someday */
  /**
   * @brief Auto-clear the Fifo on CRC failure
   */
  uint8_t crc_autoclear:1;
  /* length filtering required because i said so */
} sx1212_packet_config_t;
/* 8 total bits */

/**
 * @brief   SX1212 configuration structure.
 */
typedef struct {
  /**
   * @brief SPI driver associated to this SX1212.
   */
  SPIDriver       *spip;
  /**
   * @brief SPI configuration associated with the Config interface 
   */
  SPIConfig *spicfgp;
  /**
   * @brief Data SPI slave select line
   */
  ioline_t data_ss_b;
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
   * @brief Receiver bandwidth
   */
  uint32_t rx_bw;
  /* TODO add power someday when you add transmit */
  /**
   * @brief Sync word
   */
  SX1212_SYNC_TYPE sync_word;
  /**
   * @brief Sync tolerance - number of errors tolerated in the Sync word
   */
  uint8_t sync_tol;
  /**
   * @brief Reset line
   */
  ioline_t reset_line;
  /** 
   * @brief Packet handling configuration settings
   */
  sx1212_packet_config_t *packet_config;
  /**
   * @brief Address (when enabled)
   */
  uint8_t addr;
  /**
   * @brief Length (for fixed size packet mode), max length (for variable), or
   *        length value offset (for buffered)
   */
  uint8_t length;
} SX1212Config;

/**
 * @brief   Registers of SX1212 that need to be preserved
 */
typedef struct {
  /**
   * @brief Operating mode and frequency band register
   */
  uint8_t MCParam0;
  /**
   * @brief FIFO size and threshold register
   */
  uint8_t IRQParam0;
} SX1212Regs;

/**
 * @brief   Structure representing an SX1212 driver.
 */
/* TODO extract this out into a proper class once you've finished the driver */
/**
 * @brief   SX1212 RF transceiver class.
 */
struct SX1212Driver_t {
  /**
   * @brief Configuration
   */
  const SX1212Config *config;
  /**
   * @brief Preserved registers
   */
  SX1212Regs regs;
  /**
   * @brief Configured IRQs
   */
  SX1212IRQConstants irq_mappings[2];
#if defined(SX1212_IRQ_0) || defined(SX1212_IRQ_1)
  /**
   * @brief Ready callback
   */
  sx1212_ready_cb ready_callback;
  /**
   * @brief Receiver callback
   */
  sx1212_rx_cb rx_callback;
  /**
   *  @brief Active receive bytes remaining
   */
  size_t rx_toread;
  /**
   *  @brief Active receive bytes read
   */
  size_t rx_read;
  /**
   *  @brief Active receive buffer
   */
  uint8_t * active_rx_buf;
#endif
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern SX1212Driver SX1212D1;

#ifdef __cplusplus
extern "C" {
#endif
  void sx1212ObjectInit(SX1212Driver *devp);
  void sx1212Start(SX1212Driver *devp, const SX1212Config *config);
  void sx1212Stop(SX1212Driver *devp);
  msg_t sx1212ReceiveTimeout(SX1212Driver *devp, size_t n, uint8_t *buffer, 
      systime_t timeout);
  int8_t sx1212RSSI(SX1212Driver *devp);
  msg_t sx1212StartReceive(SX1212Driver *devp, size_t n, palcallback_t callback);
  void sx1212StopReceive(SX1212Driver *devp);
  void sx1212CLKOUT(SX1212Driver *devp, uint8_t div);
  void sx1212TUNE(SX1212Driver *devp);
  void sx1212SetFrequency(SX1212Driver *devp, uint32_t freq);
  void sx1212SetBitrate(SX1212Driver *devp, uint32_t rate);
  void sx1212SetDeviation(SX1212Driver *devp, uint32_t fdev);
  void sx1212SetSync(SX1212Driver *devp, SX1212_SYNC_TYPE sync);
  void sx1212SetRxBw(SX1212Driver *devp, uint8_t bandwidth);
  void sx1212ReceiveI(SX1212Driver * devp, size_t n, palcallback_t callback);
  void sx1212StopReceiveI(SX1212Driver *devp);
  void sx1212FifoRead(SX1212Driver * devp, size_t n, uint8_t * buffer);
#ifdef __cplusplus
}
#endif

#endif /* _SX1212_H_*/

/** @} */
