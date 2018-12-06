
#include "sdlp.h"
#include <string.h>

/* Initialize the header to its default values */
/* Initialize only the header - C standard guarantees the rest filled with 0's */
static uint8_t tf_buffer[SDLP_TM_MAX_TF_LEN] =
  {0x02, 0xF0, 0x00, 0x00, 0x18, 0x00};
static PERSIST uint16_t tf_len = SDLP_TM_MAX_TF_LEN;
static PERSIST uint16_t tf_idx;
static PERSIST size_t tf_data_len = SDLP_TM_MAX_TF_LEN;
static PERSIST uint8_t mcfc = 0;
static PERSIST uint8_t vcfc[2] = {0, 0};
static PERSIST ely_dll_state_t dll_state;

/* TODO persistent, really? */
static PERSIST uint8_t tc_hdr_buff[SDLP_TC_PH_LEN];
static PERSIST uint8_t * rx_active_buffer;
static PERSIST uint16_t rx_idx;
static PERSIST uint16_t rx_pkt_len;
static PERSIST uint8_t curr_threshold;

/* Channels */
uint8_t PERSIST fec;
uint8_t PERSIST fecf;
uint8_t PERSIST seq_no;
uint8_t PERSIST missed_frames;
uint8_t PERSIST double_frames;
uint8_t PERSIST a_frames;
uint8_t PERSIST b_frames;

/* Idle Packet - TODO this should really be in SPP */
/* TODO someday, const correctness */
static PERSIST uint8_t idle_header[elyNLHeaderLen+1] = {0x03, 0xff, 0xC0,
  0x00, 0x00, 0x00, SPP_IDLE_DATA};

/* TX Packet Config */
sx1278_packet_config_t PERSIST DLLTxPktCfg = {
  SX1278Unlimited, /* packet length format */
  0, /* automatic whitening - doesn't conform to CCSDS */
  0, /* Manchester not used */
  0, /* CRC - seed or polynomial is wrong depending on mode */
  1, /* Preamble polarity - 1 is 0x55 - matches 1212 */
  0, /* Don't use automatic addressing */
  1 /* Use a length byte in Unlimited packet mode - ENH is this fo real */
};

/* RX Packet Config */
sx1212_packet_config_t PERSIST DLLRxPktCfg = {
  SX1212Variable, /* packet length format - ENH pretty sure this is just ignored on my fast-path */
  0, /* automatic whitening - doesn't conform to CCSDS */
  0, /* Manchester not used */
  0, /* CRC - seed or polynomial is wrong depending on mode */
  0, /* Don't use automatic addressing */
  0, /* no addressing */
  0 /* no CRC autoclear */
};

/* ENH - make these generally accessible to avoid duplication */
static uint8_t clamp(uint8_t value, uint8_t min, uint8_t max) {
  if (value < min) {
    value = min;
    elyErrorSignal(ErrRegClip);
  }
  else if (value > max) {
    value = max;
    elyErrorSignal(ErrRegClip);
  }
  return value;
}

/* Overrides the default (weak) one */
void elyRFChangeTxSyncS(SX1278Config * cfg) {
  chDbgCheckClassS();

  if (!(bank0p[RegDLLOptions] & BIT2)) { /* Compliant ASM */
    cfg->sync_word = ( (bank0p[RegTXSyncLsb]) |
                  ((uint32_t)(bank0p[RegTXSyncLmb]) << 8) |
                  ((uint32_t)(bank0p[RegTXSyncHmb]) << 16) |
                  ((uint32_t)(bank0p[RegTXSyncMsb]) << 24) );

    sx1278SetSync(&SX1278D1, cfg->sync_word);
  }

}

/* TODO CLTU compliant start sequence */

static uint8_t clamp_err(uint8_t value) {
  if ((value & 0x10) && value != 0x10) {
    elyErrorSignal(ErrRegClip);
    return 0x10;
  }
  if ((value & 0x08) && value != 0x08) {
    elyErrorSignal(ErrRegClip);
    return 0x08;
  }
  if ((value & 0x04) && value != 0x04) {
    elyErrorSignal(ErrRegClip);
    return 0x04;
  }
  if ((value & 0x02) && value != 0x02) {
    elyErrorSignal(ErrRegClip);
    return 0x02;
  }
  if ((value & 0x01) && value != 0x01) {
    elyErrorSignal(ErrRegClip);
    return 0x01;
  }
  if (value & 0xD0) {
    elyErrorSignal(ErrRegClip);
    return 0x00;
  }
  return value; /* must be 0x00 at this point */
}

uint8_t elyRFDLLClampReg(uint8_t addr, uint8_t value) {
  chDbgAssert(addr >= 0xC0 && addr < RegDLLMAX, "invalid address");
  switch (addr) {
    case RegDLLTFLengthLSB:
      value = clamp(value, 7, 0xFF);
      break;
    case RegDLLTFLengthMSB:
      value = clamp(value, 0, 0x40);
      break;
    case RegDLLTMFEC:
      if ((value & 0x38) == 0x38) {
        value &= ~BIT3;
      }
      if ((value & 0x07) > 5) {
        value &= ~BIT1;
        value |= BIT0;
      }
      break;
    case RegDLLTCFEC:
      if ((value & 0x03) == 0x03) {
        value &= ~BIT0;
      }
      break;
    case RegDLLWindowLength:
      value &= ~BIT0;
      break;
    case RegDLLFECLvl:
    case RegDLLFECFLvl:
    case RegDLLMissedFrameLvl:
    case RegDLLLockoutLvl:
    case RegDLLDoubleFrameLvl:
    case RegDLLInvalidIDLvl:
    case RegDLLShortFrameLvl:
    case RegDLLLongFrameLvl:
    case RegDLLWaitLvl:
      value = clamp_err(value);
      break;
    default:
      /* All other registers have ranges equal to their data types */
      break;
  }
  return value;
}

/* TODO learn how to deal with non-SPP packets */
void elyRFDLLBuildFrame(void) {
  static ely_framebuilder_state_t state = FB_STATE_UNINIT;
  static size_t idle_len = 0;
  static size_t idle_idx = 0;

  size_t tf_idx = SDLP_TM_PH_LEN;
  bool fhp = false;

  static uint8_t * active_packet;
  static size_t pkt_len;
  static size_t pkt_idx = 0;

  chEvtSignal(rf_thd, RFPktAvailable);

  /* HEADER entry actions */
  /* Set SCID */
  tf_buffer[0] |= ( ((bank0p[RegDLLIDsMSB] & 0x03) << 4) |
                    ((bank0p[RegDLLIDsLSB] & 0xF0) >> 4));
  /* Set VCID (VCID 0) */
  tf_buffer[1] |= ( ((bank0p[RegDLLIDsLSB] & 0x0F) << 4) |
      ((bank0p[RegDLLIDsMSB] & 0x1C) >> 1));
  /* Set MCFC */
  tf_buffer[2] = mcfc;
  /* Set VCFC */
  tf_buffer[3] = vcfc[0];
  /* Increment the frame counts */
  mcfc++;
  vcfc[0]++;
  /* Clear Synch (packets) */
  tf_buffer[4] &= ~BIT6;
  /* Set FHP to all 1's */
  tf_buffer[4] |= 0x07;
  tf_buffer[5] = 0xFF;
  /* TODO add FSH if needed */

  if (state == FB_STATE_UNINIT) {
    /* Init actions */
    /* Idle_len already initialized to zero */
    if (MSG_OK != chMBFetch(&rf_mbox, (msg_t *)(&active_packet),
          TIME_IMMEDIATE)) {
      /* Fatal error */
      chDbgAssert(false, "should have a packet available");
    }
    pkt_len = elyNLGetLength(active_packet);
    /* FHP points to first octet */
    tf_buffer[4] &= ~0x07;
    tf_buffer[5] = 0;
    fhp = true;
    idle_len = 0;
    idle_idx = 0;
    pkt_idx = 0;
  }

  state = FB_STATE_HEADER;

  while (tf_idx != tf_data_len) {
    switch(state) {
      case FB_STATE_HEADER:
        if (idle_len > 0) {
          state = FB_STATE_IDLE;
        }
        else {
          state = FB_STATE_PAYLOAD;
        }
        continue;
      case FB_STATE_PAYLOAD:
        if (pkt_idx == pkt_len) {
          pkt_idx = 0; /* either way - needs to be 0 when we come back around */
          elyNLFreeBufferChecked(active_packet);
          if (!fhp) {
            tf_buffer[4] |= (tf_idx-SDLP_TM_PH_LEN) >> 8;
            tf_buffer[5] = (tf_idx-SDLP_TM_PH_LEN) & 0xFF;
            fhp = true;
          }
          msg_t r = chMBFetch(&rf_mbox, (msg_t *)(&active_packet), SDLP_IDLE_PACKET_TIMEOUT);
          if (MSG_OK != r) {
            idle_len = tf_data_len - tf_idx;
            if (idle_len < 7) {
              idle_len += tf_data_len;
            }
            idle_header[4] = (idle_len - 7) >> 8;
            idle_header[5] = (idle_len - 7) & 0xFF;
            idle_idx = 0;
            state = FB_STATE_IDLE;
            continue;
          }
          else { /* we have another packet */
            pkt_len = elyNLGetLength(active_packet);
          }
        }
        tf_buffer[tf_idx++] = active_packet[pkt_idx++];
        break;
      case FB_STATE_IDLE:
        if (idle_idx < 6) {
          tf_buffer[tf_idx++] = idle_header[idle_idx++];
        }
        else {
          tf_buffer[tf_idx++] = SPP_IDLE_DATA;
          idle_idx++;
        }
        if (idle_idx == idle_len) {
          chDbgAssert(tf_idx == tf_data_len, "invalid framebuilder state");
          idle_len = 0;
          chEvtWaitAnyTimeout(RFPktAvailable, TIME_IMMEDIATE);
          state = FB_STATE_UNINIT;
          continue;
        }
        break;
      default:
        chDbgAssert(false, "invalid framebuilder state");
        break;
    }
  }

  /* Finished copying - add the footers */
  if (bank0p[RegDLLOptions] & BIT4) {
    /* TODO add CLCW to OCF */
  }
  if (bank0p[RegDLLTMFEC] & BIT7) {
    /* add CRC */
    crcGenSDLP(tf_buffer, tf_len);
  }
  /* Put R=1/2 K=7 CCSDS Convolutional Code Here - nkohls 
   * tf_buffer of tf_len (bytes)*/
  /* Frame is ready */
  chEvtSignal(rf_thd, RFTxFrameReady);
}

void rxfifothresh_callback(void) {
  chSysLockFromISR();
  chEvtSignalI(rf_thd, RFRxFifoThresh);
  chSysUnlockFromISR();
}

void packet_callback(SX1212Driver * devp, size_t n, uint8_t *rxbuf) {
  (void)(rxbuf);
  /* TODO run FARM checks and free the buffer if it fails */

  rx_idx += n;
  chSysLockFromISR();
  if (rx_idx < rx_pkt_len) {
    curr_threshold = (rx_pkt_len < 64 ? rx_pkt_len : 48);
    /* Keep receiving */
    sx1212ReceiveI(devp, curr_threshold, rxfifothresh_callback);
    chEvtSignalI(rf_thd, RFSpiAvailable);
  }
  else {
    /* Otherwise, post the buffer */
    /* TODO implement this */
    elyNLRouteRFI(rx_active_buffer);
    /* End the packet */

    sx1212StopReceiveI(devp);
    /* Start trying to receive the next header */
    sx1212ReceiveI(devp, SDLP_TC_PH_LEN, rxfifothresh_callback);
    chEvtSignalI(rf_thd, RFSpiAvailable | RFRxIdle);
  }
  chSysUnlockFromISR();
}

void header_callback(SX1212Driver * devp, size_t n, uint8_t *rxbuf) {
  (void)(rxbuf);
  chDbgAssert(n == SDLP_TC_PH_LEN, "invalid header size - weird");

  /* Pull out header length */
  rx_pkt_len = ((tc_hdr_buff[2] & 0x03) << 8) | (tc_hdr_buff[3]);
  rx_pkt_len -= SDLP_TC_PH_LEN;
  chSysLockFromISR();
  /* Grab a working buffer */
  /* TODO handle failure */
  rx_active_buffer = elyNLGetBufferI();
  if (rx_active_buffer == NULL) {
    /* Restart state machine */
  }
  curr_threshold = (rx_pkt_len < 64 ? rx_pkt_len : 48);
  rx_idx = 0;
  /* Receive the packet */
  sx1212ReceiveI(devp, curr_threshold, rxfifothresh_callback);
  chEvtSignalI(rf_thd, RFSpiAvailable);
  /* Update state */
  dll_state = DLL_STATE_PKT;
  chSysUnlockFromISR();
}

void elyRFDLLRxInit(SX1212Driver * devp) {
  /* End the packet */
  sx1212StopReceive(devp);
  /* Put us in RX mode (param is/will be FifoThreshold) */
  /* TODO deal with errors */
  /* TODO add a timeout for going to RX */
  sx1212StartReceive(devp, SDLP_TC_PH_LEN, rxfifothresh_callback);
  /* Initial state */
  /* TODO think about persistence */
  dll_state = DLL_STATE_HDR;
  chEvtSignal(rf_thd, RFSpiAvailable | RFRxIdle);
}

void elyRFDLLHandleRxFifo(SX1212Driver * devp) {
  /* FIFO is full */
  if (dll_state == DLL_STATE_HDR) {
#if 0
    sx1212FifoReadAsync(devp, SDLP_TC_PH_LEN, tc_hdr_buff, header_callback);
#else /* rev A */
    sx1212FifoRead(devp, SDLP_TC_PH_LEN, tc_hdr_buff);
    /* Header integrity checks */
    /* PVN and CCSDS Reserved bits */
//    if (tc_hdr_buff[0] & 0xCC) {
      /* Restart state machine */
//      elyRFDLLRxInit(devp);
//      return;
//    }
    /* SCID */
    if (((tc_hdr_buff[0] & 0x03) != (bank0p[RegDLLIDsMSB] & 0x03)) ||
        (tc_hdr_buff[1] != bank0p[RegDLLIDsLSB])) {
      /* Restart state machine */
      elyRFDLLRxInit(devp);
      return;
    }
    /* VCID */
//    if ((tc_hdr_buff[2] & 0xFC) != (bank0p[RegDLLIDsMSB] & 0xFC)) {
      /* Restart state machine */
//      elyRFDLLRxInit(devp);
//      return;
//    }

    /* Pull out header length */
    rx_pkt_len = (((tc_hdr_buff[2] & 0x03) << 8) | (tc_hdr_buff[3]))+1;
//    if (rx_pkt_len > SDLP_TC_MAX_TF_LEN) {
      /* Restart state machine */
//      elyRFDLLRxInit(devp);
//      return;
//    }
    rx_pkt_len -= SDLP_TC_PH_LEN;
    chSysLock();
    /* Grab a working buffer */
    rx_active_buffer = elyNLGetBufferI();
    if (rx_active_buffer == NULL) {
      /* TODO signal a buffer overrun here */
      /* Restart state machine */
      elyRFDLLRxInit(devp);
      return;
    }
    curr_threshold = (rx_pkt_len < 64 ? rx_pkt_len : 32);
    rx_idx = 0;
    /* Receive the packet */
    sx1212ReceiveI(devp, curr_threshold, rxfifothresh_callback);
    chEvtSignalI(rf_thd, RFSpiAvailable);
    /* Update state */
    dll_state = DLL_STATE_PKT;
    chSysUnlock();
#endif
  }
  else { /* DLL_STATE_PKT */
#if 0
    sx1212FifoReadAsync(devp, curr_threshold, rx_active_buffer + rx_idx,
        packet_callback);
#else /* rev A */
    sx1212FifoRead(devp, curr_threshold, rx_active_buffer + rx_idx);

    /* TODO run FARM checks and free the buffer if it fails */

    rx_idx += curr_threshold;
    if (rx_idx < rx_pkt_len) {
      chSysLock();
      uint16_t temp = rx_pkt_len - rx_idx; // Find how much of the frame we have left to read
      curr_threshold = (temp < 64 ? temp : 48);
      /* Keep receiving */
      sx1212ReceiveI(devp, curr_threshold, rxfifothresh_callback);
      chEvtSignalI(rf_thd, RFSpiAvailable);
      chSysUnlock();
    }
    else {
      /* Otherwise, post the buffer */
      elyNLRouteRF(rx_active_buffer);
      /* Restart state machine */
      elyRFDLLRxInit(devp);
      chEvtSignal(rf_thd, RFSpiAvailable);
    }
#endif
  }

}

void txlvl_callback(void) {
  chSysLockFromISR();
  chEvtSignalI(rf_thd, RFTxFifoLevel);
  chSysUnlockFromISR();
}

void write_cb(SPIDriver * spip) {
  (void)(spip);
  chSysLockFromISR();
  palSetLine(SX1278D1.config->ss_line);
  if (tf_idx == tf_len) {
    /* TODO in this function, check for another available packet or frame and
     * if none exists, go to sleep mode */
    /* Done transmitting. Do the stuff */
#if ELY_DISCRETE_PA_CTL
    palClearLine(LINE_PA_PC_EN);
#endif
    palLineDisableEventI(LINE_SX1278_DIO1);

    chEvtSignalI(rf_thd, RFSpiAvailable | RFTxIdle);
    tf_idx = 0;
  }
  else {
    chEvtSignalI(rf_thd, RFSpiAvailable);
  }
  chSysUnlockFromISR();
}

void elyRFDLLTxInit(SX1278Driver * devp) {
  chSysLock();
  tf_len = ((uint16_t)(bank0p[RegDLLTFLengthMSB] << 8)) | ((uint16_t)(bank0p[RegDLLTFLengthLSB]));
  if (bank0p[RegDLLOptions] & BIT5) {
    tf_data_len = tf_len - 4;
  }
  else {
    tf_data_len = tf_len;
  }
  if (bank0p[RegDLLOptions] & BIT2) {
    /* TODO this depends on the FEC */
    sx1278SetSync(devp, 0x1ACFFC1D);
  }
  /* TODO data whitening, FARM */
  chSysUnlock();
}

void elyRFDLLInitiateTransmit(SX1278Driver * devp) {

#if ELY_DISCRETE_PA_CTL
  palSetLine(LINE_PA_PC_EN);
#endif

  tf_idx = sx1278StartTransmit(devp, tf_len, tf_buffer, txlvl_callback, write_cb);
}

void elyRFDLLHandleTxFifo(SX1278Driver * devp) {
  /* FIFO is sufficiently empty */
  uint8_t to_transmit = (tf_len - tf_idx <= 31 ? tf_len - tf_idx : 31);
  chSysLock();
  sx1278FifoWriteAsyncS(devp, to_transmit, tf_buffer + tf_idx, write_cb);
  tf_idx += to_transmit;
  chSysUnlock();
}




