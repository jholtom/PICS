
#include "rf.h"
#include "regs_admin.h"

static PERSIST msg_t rf_buffer[elyNLMaxSlots + elyFWMaxSlots];
PERSIST MAILBOX_DECL(rf_mbox, rf_buffer, elyNLMaxSlots + elyFWMaxSlots);

thread_t * rf_thd;

static const PERSIST rf_events_t RFCfgMask = 0x3FF;
static const PERSIST rf_events_t AllRfEvents = 0x1FFFF;
static const PERSIST rf_events_t RFTxCfgMask = 0x155;
static const PERSIST rf_events_t RFRxCfgMask = 0xAA;
static eventmask_t events;

msg_t elyRFPostI(uint8_t * buffer) {
  msg_t msg;
  msg = chMBPostI(&rf_mbox, (msg_t)(buffer));
  if (MSG_OK == msg) {
    chEvtSignalI(rf_thd, RFPktAvailable);
  }
  return msg;
}

msg_t elyRFPost(uint8_t * buffer, systime_t timeout) {
  msg_t msg;
  chSysLock();
  msg = chMBPostS(&rf_mbox, (msg_t)(buffer), timeout);
  if (MSG_OK == msg) {
    chEvtSignalI(rf_thd, RFPktAvailable);
  }
  chSysUnlock();
  return msg;
}

void elyRFCfgMarkDirtyI(rf_events_t event) {
  chDbgAssert(chThdGetSelfX() != rf_thd, "can't set your own config dirty");
  chDbgAssert(event & RFCfgMask, "invalid event");

  chEvtSignalI(rf_thd, event);
}

void elyRFCfgMarkDirty(rf_events_t event) {
  chDbgAssert(chThdGetSelfX() != rf_thd, "can't set your own config dirty");
  chDbgAssert(event & RFCfgMask, "invalid event");

  chEvtSignal(rf_thd, event);
}

static eventmask_t get_next_event(eventmask_t mask) {
  mask ^= mask & (mask - (eventmask_t)1);
  return mask;
}

void __attribute__((weak)) elyRFChangeTxFreqS(SX1278Config * cfg) {
  chSysLock();
  cfg->freq = ( ((uint32_t)(bank0p[RegTXFreqLsb])) |
               ((uint32_t)(bank0p[RegTXFreqLmb]) << 8) |
               ((uint32_t)(bank0p[RegTXFreqHmb]) << 16) |
               ((uint32_t)(bank0p[RegTXFreqMsb]) << 24) );
  if (cfg->freq > TX_BAND_MAX) {
    cfg->freq = TX_BAND_MAX;
    bank0w[RegTXFreqLsb] = (cfg->freq & 0xFF);
    bank0w[RegTXFreqLmb] = ((cfg->freq >> 8) & 0xFF);
    bank0w[RegTXFreqHmb] = ((cfg->freq >> 16) & 0xFF);
    bank0w[RegTXFreqMsb] = (cfg->freq >> 24);
  }
  if (cfg->freq < TX_BAND_MIN) {
    cfg->freq = TX_BAND_MIN;
    bank0w[RegTXFreqLsb] = (cfg->freq & 0xFF);
    bank0w[RegTXFreqLmb] = ((cfg->freq >> 8) & 0xFF);
    bank0w[RegTXFreqHmb] = ((cfg->freq >> 16) & 0xFF);
    bank0w[RegTXFreqMsb] = (cfg->freq >> 24);
  }
  chSysUnlock();

  /* This is pretty fast */
  sx1278SetFrequency(&SX1278D1, cfg->freq);
}

void __attribute__((weak)) elyRFChangeRxFreqS(SX1212Config * cfg) {
  chSysLock();
  cfg->freq = ( ((uint32_t)(bank0p[RegRXFreqLsb])) |
               ((uint32_t)(bank0p[RegRXFreqLmb]) << 8) |
               ((uint32_t)(bank0p[RegRXFreqHmb]) << 16) |
               ((uint32_t)(bank0p[RegRXFreqMsb]) << 24) );
  if (cfg->freq > RX_BAND_MAX) {
    cfg->freq = RX_BAND_MAX;
    bank0w[RegRXFreqLsb] = (cfg->freq & 0xFF);
    bank0w[RegRXFreqLmb] = ((cfg->freq >> 8) & 0xFF);
    bank0w[RegRXFreqHmb] = ((cfg->freq >> 16) & 0xFF);
    bank0w[RegRXFreqMsb] = (cfg->freq >> 24);
  }
  if (cfg->freq < RX_BAND_MIN) {
    cfg->freq = RX_BAND_MIN;
    bank0w[RegRXFreqLsb] = (cfg->freq & 0xFF);
    bank0w[RegRXFreqLmb] = ((cfg->freq >> 8) & 0xFF);
    bank0w[RegRXFreqHmb] = ((cfg->freq >> 16) & 0xFF);
    bank0w[RegRXFreqMsb] = (cfg->freq >> 24);
  }
  chSysUnlock();

  /* This is INCREDIBLY slow */
  sx1212SetFrequency(&SX1212D1, cfg->freq);
}

void __attribute__((weak)) elyRFChangeTxBRS(SX1278Config * cfg) {
  chSysLock();
  cfg->bitrate = ( ((uint32_t)(bank0p[RegTXBRLsb])) |
                   ((uint32_t)(bank0p[RegTXBRLmb]) << 8) |
                   ((uint32_t)(bank0p[RegTXBRHmb]) << 16) |
                   ((uint32_t)(bank0p[RegTXBRMsb]) << 24) );
  if (cfg->bitrate > TX_BR_MAX) {
    cfg->bitrate = TX_BR_MAX;
    bank0w[RegTXBRLsb] = (cfg->bitrate & 0xFF);
    bank0w[RegTXBRLmb] = ((cfg->bitrate >> 8) & 0xFF);
    bank0w[RegTXBRHmb] = ((cfg->bitrate >> 16) & 0xFF);
    bank0w[RegTXBRMsb] = (cfg->bitrate >> 24);
  }
  if (cfg->bitrate < TX_BR_MIN) {
    cfg->bitrate = TX_BR_MIN;
    bank0w[RegTXBRLsb] = (cfg->bitrate & 0xFF);
    bank0w[RegTXBRLmb] = ((cfg->bitrate >> 8) & 0xFF);
    bank0w[RegTXBRHmb] = ((cfg->bitrate >> 16) & 0xFF);
    bank0w[RegTXBRMsb] = (cfg->bitrate >> 24);
  }
  chSysUnlock();

  /* This is fast */
  sx1278SetBitrate(&SX1278D1, cfg->bitrate);
}

void __attribute__((weak)) elyRFChangeRxBRS(SX1212Config * cfg) {
  chSysLock();
  cfg->bitrate = ( (uint32_t)(bank0p[RegRXBRLsb]) |
                   ((uint32_t)(bank0p[RegRXBRLmb]) << 8) |
                   ((uint32_t)(bank0p[RegRXBRHmb]) << 16) |
                   ((uint32_t)(bank0p[RegRXBRMsb]) << 24) );
  if (cfg->bitrate > RX_BR_MAX) {
    cfg->bitrate = RX_BR_MAX;
    bank0w[RegRXBRLsb] = (cfg->bitrate & 0xFF);
    bank0w[RegRXBRLmb] = ((cfg->bitrate >> 8) & 0xFF);
    bank0w[RegRXBRHmb] = ((cfg->bitrate >> 16) & 0xFF);
    bank0w[RegRXBRMsb] = (cfg->bitrate >> 24);
  }
  if (cfg->bitrate < RX_BR_MIN) {
    cfg->bitrate = RX_BR_MIN;
    bank0w[RegRXBRLsb] = (cfg->bitrate & 0xFF);
    bank0w[RegRXBRLmb] = ((cfg->bitrate >> 8) & 0xFF);
    bank0w[RegRXBRHmb] = ((cfg->bitrate >> 16) & 0xFF);
    bank0w[RegRXBRMsb] = (cfg->bitrate >> 24);
  }
  chSysUnlock();

  /* This is only moderately slow */
  sx1212SetBitrate(&SX1212D1, cfg->bitrate);
}

void __attribute__((weak)) elyRFChangeTxDevS(SX1278Config * cfg) {
  chSysLock();
  cfg->fdev = ( ((uint32_t)(bank0p[RegTXDevLsb])) |
                     ((uint32_t)(bank0p[RegTXDevLmb]) << 8) |
                     ((uint32_t)(bank0p[RegTXDevHmb]) << 16) |
                     ((uint32_t)(bank0p[RegTXDevMsb]) << 24) );
  if (cfg->fdev > TX_DEV_MAX) {
    cfg->fdev = TX_DEV_MAX;
    bank0w[RegTXDevLsb] = (cfg->fdev & 0xFF);
    bank0w[RegTXDevLmb] = ((cfg->fdev >> 8) & 0xFF);
    bank0w[RegTXDevHmb] = ((cfg->fdev >> 16) & 0xFF);
    bank0w[RegTXDevMsb] = (cfg->fdev >> 24);
  }
  if (cfg->fdev < TX_DEV_MIN) {
    cfg->fdev = TX_DEV_MIN;
    bank0w[RegTXDevLsb] = (cfg->fdev & 0xFF);
    bank0w[RegTXDevLmb] = ((cfg->fdev >> 8) & 0xFF);
    bank0w[RegTXDevHmb] = ((cfg->fdev >> 16) & 0xFF);
    bank0w[RegTXDevMsb] = (cfg->fdev >> 24);
  }
  chSysUnlock();

  /* This is fast */
  sx1278SetDeviation(&SX1278D1, cfg->fdev);
}

void __attribute__((weak)) elyRFChangeRxDevS(SX1212Config * cfg) {
  chSysLock();
  cfg->fdev = ( ((uint32_t)(bank0p[RegRXDevLsb])) |
                     ((uint32_t)(bank0p[RegRXDevLmb]) << 8) |
                     ((uint32_t)(bank0p[RegRXDevHmb]) << 16) |
                     ((uint32_t)(bank0p[RegRXDevMsb]) << 24) );
  if (cfg->fdev > RX_DEV_MAX) {
    cfg->fdev = RX_DEV_MAX;
    bank0w[RegRXDevLsb] = (cfg->fdev & 0xFF);
    bank0w[RegRXDevLmb] = ((cfg->fdev >> 8) & 0xFF);
    bank0w[RegRXDevHmb] = ((cfg->fdev >> 16) & 0xFF);
    bank0w[RegRXDevMsb] = (cfg->fdev >> 24);
  }
  if (cfg->fdev < RX_DEV_MIN) {
    cfg->fdev = RX_DEV_MIN;
    bank0w[RegRXDevLsb] = (cfg->fdev & 0xFF);
    bank0w[RegRXDevLmb] = ((cfg->fdev >> 8) & 0xFF);
    bank0w[RegRXDevHmb] = ((cfg->fdev >> 16) & 0xFF);
    bank0w[RegRXDevMsb] = (cfg->fdev >> 24);
  }
  chSysUnlock();

  /* This is super fast */
  sx1212SetDeviation(&SX1212D1, cfg->fdev);
}

void __attribute__((weak)) elyRFChangeTxSyncS(SX1278Config * cfg) {
  chSysLock();
  cfg->sync_word = ( ((uint32_t)(bank0p[RegTXSyncLsb])) |
                ((uint32_t)(bank0p[RegTXSyncLmb]) << 8) |
                ((uint32_t)(bank0p[RegTXSyncHmb]) << 16) |
                ((uint32_t)(bank0p[RegTXSyncMsb]) << 24) );
  chSysUnlock();

  /* This is reasonably fast */
  sx1278SetSync(&SX1278D1, cfg->sync_word);
}

void __attribute__((weak)) elyRFChangeRxSyncS(SX1212Config * cfg) {
  chSysLock();
  cfg->sync_word = ( ((uint32_t)(bank0p[RegRXSyncLsb])) |
                ((uint32_t)(bank0p[RegRXSyncLmb]) << 8) |
                ((uint32_t)(bank0p[RegRXSyncHmb]) << 16) |
                ((uint32_t)(bank0p[RegRXSyncMsb]) << 24) );
  chSysUnlock();

  /* This is reasonably fast */
  sx1212SetSync(&SX1212D1, cfg->sync_word);
}

uint8_t rfic_pow_to_reg(int16_t pow) {
  /* from -4.2 to 14 by 0.2s, adjusted for Pmax weirdness */
  static const uint8_t lut[] = {
    0x0, 0x0, 0x0, 0x10, 0x20, 0x1, 0x1, 0x1, 0x11, 0x21, 0x2, 0x70, 0x70, 0x12, 0x22, 0x3, 0x71,
    0x71, 0x13, 0x23, 0x4, 0x72, 0x72, 0x14, 0x24, 0x5, 0x73, 0x73, 0x15, 0x25, 0x6, 0x74, 0x74,
    0x16,  0x26, 0x7, 0x75, 0x75, 0x17, 0x27, 0x8, 0x76, 0x76, 0x18, 0x28, 0x9, 0x77, 0x77, 0x19,
    0x29, 0xa, 0x78, 0x78, 0x1a, 0x2a, 0xb, 0x79, 0x79, 0x1b, 0x2b, 0xc, 0x7a, 0x7a, 0x1c, 0x2c,
    0xd, 0x7b, 0x7b, 0x1d, 0x2d, 0xe, 0x7c, 0x7c, 0x1e, 0x2e, 0xf, 0x7d, 0x7d, 0x1f, 0x2f, 0x3f,
    0x7e,  0x7e, 0x7e, 0x4f, 0x6f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f
  };

  return lut[(pow / 2) + 21];
}

void __attribute__((weak)) elyRFChangeTxPowerS(SX1278Config * cfg) {

  /*int16_t prog_pow = 100 + 2 * bank0p[RegOutputPower];
  int16_t rfic_pow = prog_pow - 300 + (FIXED_ATTEN*10);
  cfg->pow = rfic_pow_to_reg(rfic_pow);
  What the actual heck....*/
  //cfg->pow = 0;
  //cfg->pow = 68; //0b010001011: bit7 = 0, RFO pins, bit6-4 = 0b100 = 4, MaxPower of 13.2, bit3-0: 0b1011 = 11, Sets Pout to about 9
  cfg->pow = 127; //Maxed everything out...
  /* This is reasonably fast because I made it kind of sloppy */
  sx1278SetPower(&SX1278D1, cfg->pow);
}

void __attribute__((weak)) elyRFChangeTxFilterParamsS(SX1278Config * tx_cfg) {

  tx_cfg->filter = ((bank0p[RegFilterParams] >> 4) & 0x03);
  /* This is very fast */
  sx1278SetFilterParams(&SX1278D1, tx_cfg->filter);
}

void __attribute__((weak)) elyRFChangeRxFilterParamsS(SX1212Config * rx_cfg) {

  rx_cfg->rx_bw = (bank0p[RegFilterParams] & 0x0F);
  /* This is fast and I made it kind of sloppy anyway */
  sx1212SetRxBw(&SX1212D1, rx_cfg->rx_bw);
}

SPIConfig __attribute__((section(".persistent"))) rf_spi_cfg = {
  NULL, /* TODO end_cb */
  LINE_SS_CONFIG_B, /* slave select */
  SPI_BR, /* bit rate */
  MSP430X_SPI_BO_MSB, /* MSB first - required by SX1278 */
  MSP430X_SPI_DS_EIGHT,
  0 /* mode 0 for SX1278 */
    /* currently no SPI exclusive DMA - may need later */
};

static SX1278Config __attribute__((section(".persistent"))) tx_cfg = {
  &rf_spi,
  &rf_spi_cfg,
  TX_BR, /* Initial bitrate */
  434000000, /* Initial frequency */
  TX_DEV, /* Initial deviation */
  TX_FILTER_DEFAULT, /* Initial filter setting */
  0, /* Initial output power */
  8, /* Preamble length - picked to match RX */
  TX_SYNC, /* Sync word */
  {
    0,
    1,
  }, /* PacketSent on DIO0, FifoLevel on DIO1 - ENH currently this does basically nothing, rethink? */
  LINE_SX1278_RESET_B, /* reset line */
  LINE_SX1278_SS_B, /* slave select line */
  &DLLTxPktCfg,
  0, /* no address */
  0,  /* not using fixed length packet mode */
  {
    LINE_SX1278_DIO0,
    LINE_SX1278_DIO1,
    LINE_SX1278_DIO2,
    LINE_SX1278_DIO3,
    LINE_SX1278_DIO4,
    LINE_SX1278_DIO5,
  }, /* DIO line mapping */
};

static SX1212Config __attribute__((section(".persistent"))) rx_cfg = {
  &rf_spi,
  &rf_spi_cfg,
  LINE_SS_DATA_B,
  RX_BR,
  RX_BAND_MID,
  RX_DEV,
  RX_FILTER_DEFAULT,
  RX_SYNC,
  0,
  LINE_SX1212_RESET,
  NULL,
  0,
  0
};

static bool bits_set(eventmask_t events, eventmask_t bits) {
  return ((events & bits) == bits);
}

THD_WORKING_AREA(waRFThd, 256);
THD_FUNCTION(RFThd, arg) {
  (void)arg;

  /* Store a thread pointer for later use */
  rf_thd = chThdGetSelfX();

  /* Build the TX config out of the registers */
  chSysLock();
  tx_cfg.bitrate = ( ((uint32_t)(bank0p[RegTXBRLsb])) |
                   ((uint32_t)(bank0p[RegTXBRLmb]) << 8) |
                   ((uint32_t)(bank0p[RegTXBRHmb]) << 16) |
                   ((uint32_t)(bank0p[RegTXBRMsb]) << 24) );
  tx_cfg.freq = ( ((uint32_t)(bank0p[RegTXFreqLsb])) |
               ((uint32_t)(bank0p[RegTXFreqLmb]) << 8) |
               ((uint32_t)(bank0p[RegTXFreqHmb]) << 16) |
               ((uint32_t)(bank0p[RegTXFreqMsb]) << 24) );
  tx_cfg.fdev = ( ((uint32_t)(bank0p[RegTXDevLsb])) |
                     ((uint32_t)(bank0p[RegTXDevLmb]) << 8) |
                     ((uint32_t)(bank0p[RegTXDevHmb]) << 16) |
                     ((uint32_t)(bank0p[RegTXDevMsb]) << 24) );
  tx_cfg.filter = ((bank0p[RegFilterParams] >> 4) & 0x03);
  int16_t prog_pow = 100 + 2 * bank0p[RegOutputPower];
  int16_t rfic_pow = prog_pow - 300 + (FIXED_ATTEN*10);
  tx_cfg.pow = rfic_pow_to_reg(rfic_pow);
  tx_cfg.sync_word = ( ((uint32_t)(bank0p[RegTXSyncLsb])) |
                ((uint32_t)(bank0p[RegTXSyncLmb]) << 8) |
                ((uint32_t)(bank0p[RegTXSyncHmb]) << 16) |
                ((uint32_t)(bank0p[RegTXSyncMsb]) << 24) );
  chSysUnlock();

  /* Start the transmitter driver and initiate the transmit loop */
  sx1278ObjectInit(&SX1278D1);
  sx1278Start(&SX1278D1, &tx_cfg);

  elyRFDLLTxInit(&SX1278D1);

  /* Build the RX config out of the registers */
  chSysLock();
  rx_cfg.bitrate = ( ((uint32_t)(bank0p[RegRXBRLsb])) |
                   ((uint32_t)(bank0p[RegRXBRLmb]) << 8) |
                   ((uint32_t)(bank0p[RegRXBRHmb]) << 16) |
                   ((uint32_t)(bank0p[RegRXBRMsb]) << 24) );
  rx_cfg.freq = ( ((uint32_t)(bank0p[RegRXFreqLsb])) |
               ((uint32_t)(bank0p[RegRXFreqLmb]) << 8) |
               ((uint32_t)(bank0p[RegRXFreqHmb]) << 16) |
               ((uint32_t)(bank0p[RegRXFreqMsb]) << 24) );
  rx_cfg.fdev = ( ((uint32_t)(bank0p[RegRXDevLsb])) |
                     ((uint32_t)(bank0p[RegRXDevLmb]) << 8) |
                     ((uint32_t)(bank0p[RegRXDevHmb]) << 16) |
                     ((uint32_t)(bank0p[RegRXDevMsb]) << 24) );
  rx_cfg.rx_bw = (bank0p[RegFilterParams] & 0x0F);
  rx_cfg.sync_word = ( ((uint32_t)(bank0p[RegRXSyncLsb])) |
                ((uint32_t)(bank0p[RegRXSyncLmb]) << 8) |
                ((uint32_t)(bank0p[RegRXSyncHmb]) << 16) |
                ((uint32_t)(bank0p[RegRXSyncMsb]) << 24) );
  chSysUnlock();


  /* Start the receiver driver and initiate the receive loop */
  sx1212Start(&SX1212D1, &rx_cfg);

  elyRFDLLRxInit(&SX1212D1);

  events = (RFSpiAvailable | RFTxIdle);

  /* Get events into a consistent state with mailbox */
  chSysLock();
  if (chMBGetUsedCountI(&rf_mbox) > 0) {
    events |= RFPktAvailable;
  }
  chSysUnlock();

  while (true) {
    if (bits_set(events, (RFRxFifoThresh | RFSpiAvailable))) {
      /*
       * Read FifoThresh bytes from the Fifo using spiStartReceive
       *  In the callback - signal SpiAvailable, and post buffer and signal RxIdle if applicable
       * Clear RxFifoThresh and SpiAvailable from Events variable
       */
      elyRFDLLHandleRxFifo(&SX1212D1);
      events &= ~(RFSpiAvailable | RFRxFifoThresh);
    }
    if (bits_set(events, (RFSpiAvailable | RFTxFrameReady | RFTxIdle))) {
      elyRFDLLInitiateTransmit(&SX1278D1);
      events &= ~(RFSpiAvailable | RFTxFrameReady | RFTxIdle);
    }
    if (bits_set(events, (RFTxFifoLevel | RFSpiAvailable))) {
      /*
       * Write FifoSize or BytesRemaining bytes to the Fifo using spiStartSend
       *  In the callback - signal SpiAvailable, and TxIdle if applicable
       *  TxFifoThresh signaled by PAL callback
       * Clear TxFifoThresh and SpiAvailable from Events variable
       */
      elyRFDLLHandleTxFifo(&SX1278D1);
      events &= ~(RFSpiAvailable | RFTxFifoLevel);
    }
    if (bits_set(events, (RFPktAvailable | RFTxIdle)) && !(events & RFTxFrameReady)) {
      /*
       * Turn the NL packet into a DLL frame. Do it outside a lock zone.
       * Once the frame is ready, signal TxFrameReady to start transmission
       * Clear RFPktAvailable from Events variable
       */
      elyRFDLLBuildFrame();
      events &= ~RFPktAvailable;
      events |= (RFTxFrameReady);
    }
    if (bits_set(events, (RFTxIdle | RFSpiAvailable)) && (events & RFTxCfgMask)) {
      while (events & RFTxCfgMask) {
        eventmask_t evt = get_next_event(events & RFTxCfgMask);

        switch (evt) {
          case RFTxFreqUpdated:
            elyRFChangeTxFreqS(&tx_cfg);
            break;
          case RFTxBRUpdated:
            elyRFChangeTxBRS(&tx_cfg);
            break;
          case RFTxDevUpdated:
            elyRFChangeTxDevS(&tx_cfg);
            break;
          case RFTxSyncUpdated:
            elyRFChangeTxSyncS(&tx_cfg);
          case RFTxPowerUpdated:
            elyRFChangeTxPowerS(&tx_cfg);
            break;
          case RFFilterParamsUpdated:
            elyRFChangeTxFilterParamsS(&tx_cfg);
            break;
          default:
            chDbgAssert(false, "shouldn't happen");
        }

        /* Remove event from mask */
        events &= ~evt;
      }
    }
    if (bits_set(events, (RFRxIdle | RFSpiAvailable)) && (events & RFRxCfgMask)) {
      while (events & RFRxCfgMask) {
        eventmask_t evt = get_next_event(events & RFRxCfgMask);

        switch (evt) {
          case RFRxFreqUpdated:
            elyRFChangeRxFreqS(&rx_cfg);
            break;
          case RFRxBRUpdated:
            elyRFChangeRxBRS(&rx_cfg);
            break;
          case RFRxDevUpdated:
            elyRFChangeRxDevS(&rx_cfg);
            break;
          case RFRxSyncUpdated:
            elyRFChangeRxSyncS(&rx_cfg);
          case RFFilterParamsUpdated:
            elyRFChangeRxFilterParamsS(&rx_cfg);
            break;
          default:
            chDbgAssert(false, "shouldn't happen");
        }
        /* Remove event from mask */
        events &= ~evt;
      }
    }

    events |= chEvtWaitAnyTimeout(AllRfEvents, TIME_INFINITE);
  }

}
