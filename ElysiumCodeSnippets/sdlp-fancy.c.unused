
#include "sdlp.h"

/* Initialize the header to its default values */
static PERSIST uint8_t tf_header[SDLP_TM_PH_LEN] = 
  {0x02, 0xF0, 0x00, 0x00, 0x18, 0x00};
static PERSIST uint16_t tf_len = SDLP_TM_MAX_TF_LEN;
static PERSIST uint8_t mcfc = 0;
static PERSIST uint8_t vcfc[2] = {0, 0};
static PERSIST ely_dll_state_t dll_state;

/* TODO persistent, really? */
static PERSIST uint8_t tc_hdr_buff[SDLP_TC_PH_LEN];
static PERSIST uint8_t * rx_active_buffer;
static PERSIST uint16_t curr_idx;
static PERSIST uint16_t curr_pkt_len;
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
  SX1278Unlimited, /* packet length format - TODO is this right? */
  0, /* automatic whitening - doesn't conform to CCSDS */
  0, /* Manchester not used */
  0, /* CRC - seed or polynomial is wrong depending on mode */
  1, /* Preamble polarity - 1 is 0x55 - doesn't matter */
  0, /* Don't use automatic addressing */
  1 /* Use a length byte in Unlimited packet mode - TODO is this fo real */
};

/* RX Packet Config */
sx1212_packet_config_t PERSIST DLLRxPktCfg = {
  SX1212Variable, /* packet length format - TODO is this right? */
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

uint8_t elyDLLClampReg(uint8_t addr, uint8_t value) {
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

static void next_header(void) {
  /* Build the header */
  /* Set VCID (VCID 0) */
  tf_buffer[1] |= ((bank0p[RegDLLIDsMSB] & 0x1C) >> 1);
  /* Set MCFC */
  tf_buffer[2] = mcfc;
  /* Set VCFC */
  tf_buffer[3] = vcfc[0];
  /* Increment the frame counts */
  mcfc++;
  vcfc[0]++;
  /* Clear Synch (packets) */
  tf_buffer[4] &= ~BIT6;
}

void txlvl_cb(void) {
  to_eat = 48;
  tx_state(to_eat);
}

void trampoline(SPIDriver * spip) {
  (void)(spip);
  if (to_eat > 0) {
    tx_state(to_eat);
  }
}

void idle_state (uint8_t to_eat) {
  static uint8_t idle_idx = 0;
  
  uint8_t to_transmit = (
      (tf_len - tf_idx) < to_eat ?
      (tf_len - tf_idx) :
      to_eat );
  
  if (to_transmit > 0) {
    if (idle_idx < 7) {
      to_transmit = (idle_idx < to_transmit ? idle_idx : to_transmit);
      chSysLockFromISR();
      sx1278FifoWriteAsyncI(&SX1278D1, to_transmit, idle_header+idle_idx, trampoline);
      chSysUnlockFromISR();
    }
    else {
      chSysLockFromISR();
      sx1278FifoWriteAsyncConstI(&SX1278D1, to_transmit, SDLP_IDLE_DATA, trampoline);
      chSysUnlockFromISR();
    }
    idle_idx += to_transmit;
    tf_idx += to_transmit;
    to_eat -= to_transmit;
  }
  
  if (tf_len == tf_idx) {
    if (idle_idx < 7) {
      tx_state = header_state;
    }
    else {
      tx_state = NULL;
    }
  }
  
}

void packet_state(uint8_t to_eat) {
  static uint8_t pkt_idx = 0;
  static size_t pkt_len = 0;
  static uint8_t * active_packet = NULL;
  
  uint8_t to_transmit;
  
  if (pkt_len - pkt_idx < to_eat && pkt_len - pkt_idx < tf_len - tf_idx) {
    to_transmit = pkt_len - pkt_idx;
  }
  else if (tf_len - tf_idx < pkt_len - pkt_idx && tf_len - tf_idx < to_eat) {
    to_transmit = tf_len - tf_idx;
  }
  else { /* to_eat is min */
    to_transmit = to_eat;
  }
  
  /* TODO dealing with 0's wrong */
  if (to_transmit > 0) {
    chSysLockFromISR();
    sx1278FifoWriteAsyncI(&SX1278D1, to_transmit, active_packet+pkt_idx, trampoline);
    chSysUnlockFromISR();
    pkt_idx += to_transmit;
    tf_idx += to_transmit;
    to_eat -= to_transmit;
  }
  
  if (pkt_len == pkt_idx) {
    if (MSG_OK != chMBFetchS(&rf_mbox, (msg_t *)(&active_packet))) {
      pkt_idx = 0;
      pkt_len = 0;
      tx_state = idle_state;
    }
    pkt_idx = 0;
    pkt_len = elyNLGetLength(active_packet);
  }
  if (tf_len == tf_idx) {
    tx_state = header_state;
  }
}

void header_state(uint8_t to_eat) {
  static uint8_t hdr_idx = 0;
  
  uint8_t to_transmit = (
      (SDLP_TM_PH_LEN - hdr_idx) < to_eat ?
      (SDLP_TM_PH_LEN - hdr_idx) :
      to_eat );
  
  if (to_transmit > 0) {
    /* X-class - can be called from HandlePacket too */
    syssts_t s = chSysGetStatusAndLockX();
    sx1278FifoWriteAsyncI(&SX1278D1, to_transmit, tf_header+hdr_idx, trampoline);
    chSysRestoreStatusX(s);
    hdr_idx += to_transmit;
    tf_idx += to_transmit;
    to_eat -= to_transmit;
  }
  
  if (hdr_idx == SDLP_TM_PH_LEN) {
    hdr_idx = 0;
    tx_state = packet_state;
  }
}

/* TODO learn how to deal with non-SPP packets */
void elyRFDLLHandlePacket() {
#if ELY_DISCRETE_PA_CTL
  /* Activate the PA */
  palSetLine(LINE_PA_PC_EN);
#endif
  
  /* Go to TX mode */
  sx1278StartTransmit(devp);
  
  /* Initialize variables */
  tf_idx = 0;
  
  /* Enter Header state */
  tx_state = header_state;
  header_state(64);
}

    
  
  static uint8_t * active_packet;
  static uint8_t * packet_reader;
  static size_t packet_len;
  
  bool idle = false;
  bool fhp = false;
  
  /* Reset frame_data pointer */
  uint8_t * frame_data = tf_buffer + SDLP_TM_PH_LEN + 
    ((bank0p[RegDLLOptions] & BIT5) >> 5);
  
  while (true) {
    if (!idle && packet_reader == active_packet + packet_len) {
      chSysLock();
      elyNLFreeBufferI(active_packet);
      if (MSG_OK != chMBFetchS(&rf_mbox, (msg_t *)(&active_packet), 
            SDLP_IDLE_PACKET_TIMEOUT)) {
        /* reset buffer flag */
        chEvtWaitAnyTimeoutS(RFBufferPosted, TIME_IMMEDIATE);
        chSysUnlock();
        active_packet = idle_header;
        packet_len = (tf_buffer + tf_len) - frame_data;
        if (packet_len < 7) {
          packet_len = 7; /* minimum length of a Space Packet */
        }
        idle = true;
      }
      else {
        chSysUnlock();
        packet_len = elyNLGetLength(active_packet);
      }
      packet_reader = active_packet;
      if (!fhp) {
        /* Set first header pointer */
        /* How far into frame are we? */
        uint16_t pointer = frame_data - (tf_buffer + SDLP_TM_PH_LEN); 
        chDbgAssert(pointer < 0x400, "invalid fhp");
        tf_buffer[4] |= (pointer >> 8);
        tf_buffer[5] = (pointer & 0xFF);
        fhp = true; /* we've set the first header pointer */
      }
    }
    if (frame_data >= tf_data_end) {
      /* Finished copying - add the footers */
      if (bank0p[RegDLLOptions] & BIT4) {
        /* TODO add OCF */
      }
      if (bank0p[RegDLLTMFEC] & BIT7) {
        /* add CRC */
        crcGenSDLP(tf_buffer, tf_len);
      }
      /* Frame is ready */
      chEvtSignal(rf_thd, RFTxFrameReady);
      fhp = false;
      return;
    }
    if (idle && packet_reader > active_packet + elyNLHeaderLen) {
      (*frame_data++) = SPP_IDLE_DATA;
      packet_reader++; /* maintain "end of packet" invariant */
    }
    else {
      (*frame_data++) = (*packet_reader++);
    }
  }
}

void rxfifothresh_callback(void) {
  chSysLockFromISR();
  chEvtSignalI(rf_thd, RFRxFifoThresh);
  chSysUnlockFromISR();
}

void packet_callback(SX1212Driver * devp, size_t n, uint8_t *rxbuf) {
  (void)(rxbuf);
  /* TODO run FARM checks and free the buffer if it fails */
  
  curr_idx += n;
  chSysLockFromISR();
  if (curr_idx < curr_pkt_len) {
    curr_threshold = (curr_pkt_len <= 64 ? curr_pkt_len : 48);
    /* Keep receiving */
    sx1212ReceiveI(devp, curr_threshold, rxfifothresh_callback);
    chEvtSignalI(rf_thd, RFSpiAvailable);
  }
  else {
    /* Otherwise, post the buffer */
    elyMainMBPostI(rx_active_buffer);
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
  curr_pkt_len = ((tc_hdr_buff[2] & 0x03) << 8) | (tc_hdr_buff[3]);
  chSysLockFromISR();
  /* Grab a working buffer */
  /* TODO handle failure */
  rx_active_buffer = elyNLGetBufferI();
  curr_threshold = (curr_pkt_len <= 64 ? curr_pkt_len : 48);
  curr_idx = 0;
  /* Receive the packet */
  sx1212ReceiveI(devp, curr_threshold, rxfifothresh_callback);
  /* Update state */
  dll_state = DLL_STATE_PKT;
  chSysUnlockFromISR();
}

void elyRFDLLRxInit(SX1212Driver * devp) {
  /* Put us in RX mode (param is/will be FifoThreshold) */
  /* TODO deal with errors */
  /* TODO add a timeout for going to RX */
  sx1212StartReceive(devp, SDLP_TC_PH_LEN, rxfifothresh_callback);
  /* Initial state */
  /* TODO think about persistence */
  dll_state = DLL_STATE_HDR;
}


void elyRFDLLHandleRxFifo(SX1212Driver * devp) {
  /* FIFO is full */
  if (dll_state == DLL_STATE_HDR) {
#if ELY_REVISION == B
    sx1212FifoReadAsync(devp, SDLP_TC_PH_LEN, tc_hdr_buff, header_callback);
#else /* rev A */
    sx1212FifoRead(devp, SDLP_TC_PH_LEN, tc_hdr_buff);
    /* Pull out header length */
    curr_pkt_len = ((tc_hdr_buff[2] & 0x03) << 8) | (tc_hdr_buff[3]);
    chSysLock();
    /* Grab a working buffer */
    /* TODO handle failure */
    rx_active_buffer = elyNLGetBufferI();
    curr_threshold = (curr_pkt_len <= 64 ? curr_pkt_len : 48);
    curr_idx = 0;
    /* Receive the packet */
    sx1212ReceiveI(devp, curr_threshold, rxfifothresh_callback);
    /* Update state */
    dll_state = DLL_STATE_PKT;
    chSysUnlock();
#endif
  }
  else { /* DLL_STATE_PKT */
#if ELY_REVISION == B
    sx1212FifoReadAsync(devp, curr_threshold, rx_active_buffer + curr_idx, 
        packet_callback);
#else /* rev A */
    sx1212FifoRead(devp, curr_threshold, rx_active_buffer + curr_idx);
    
    
    /* TODO run FARM checks and free the buffer if it fails */
    
    curr_idx += curr_threshold;
    chSysLock();
    if (curr_idx < curr_pkt_len) {
      curr_threshold = (curr_pkt_len <= 64 ? curr_pkt_len : 48);
      /* Keep receiving */
      sx1212ReceiveI(devp, curr_threshold, rxfifothresh_callback);
      chEvtSignalI(rf_thd, RFSpiAvailable);
    }
    else {
      /* Otherwise, post the buffer */
      elyMainMBPostI(rx_active_buffer);
      /* End the packet */
      sx1212StopReceiveI(devp);
      /* Start trying to receive the next header */
      sx1212ReceiveI(devp, SDLP_TC_PH_LEN, rxfifothresh_callback);
      chEvtSignalI(rf_thd, RFSpiAvailable | RFRxIdle);
    }
    chSysUnlock();
#endif
  }
  
}

void elyRFDLLInitiateTransmit(SX1278Driver * devp) {
  uint8_t to_transmit = (tf_len <= 64 ? tf_len : 64);
  
}

