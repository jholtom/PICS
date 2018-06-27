
#include "cmds.h"
#include "string.h"
#include "regs_admin.h"
#include "errors.h"

static msg_t main_buffer[elyNLMaxSlots + elyFWMaxSlots];
static MAILBOX_DECL(main_mbox, main_buffer, elyNLMaxSlots + elyFWMaxSlots);
static uint8_t * active_buffer = NULL;

msg_t elyMainMBPost(uint8_t * buffer, systime_t timeout) {
  return chMBPost(&main_mbox, (msg_t)(buffer), timeout);
}

msg_t elyMainMBPostS(uint8_t * buffer, systime_t timeout) {
  chDbgCheckClassS();
  return chMBPostS(&main_mbox, (msg_t)(buffer), timeout);
}

msg_t elyMainMBPostI(uint8_t * buffer) {
  return chMBPostI(&main_mbox, (msg_t)(buffer));
}

/* ENH this is ugly, fix it some other time */
static elysium_cmd_hdr_t stored_hdr;
static elysium_cmd_hdr_t verify_stored_hdr;
static elysium_cmd_hdr_t telem_stored_hdr;
static uint16_t verify_crc = 0;

static BSEMAPHORE_DECL(cancel_sem, 1);

extern uint8_t bootloader;

static void write_success(uint8_t * reply_buff, elysium_cmd_hdr_t hdr) {
  reply_buff[0] = hdr.opcode; /* opcode w/o CRC or Reply */
  reply_buff[2] = bank0p[RegSrcAddrMsb];
  reply_buff[3] = bank0p[RegSrcAddrLsb];
  reply_buff[4] = 1;
  if (hdr.crc) {
    reply_buff[1] = 3;
    crcGenX25(reply_buff, 7);
  }
  else {
    reply_buff[1] = 1;
  }
}

static void gen_success(elysium_cmd_hdr_t hdr) {
  uint8_t * reply_buff = elyFWGetBuffer();
  if (reply_buff == NULL) {
    /* TODO signal a buffer overflow here */
    return;
  }
  write_success(reply_buff, hdr);
  
  elyCmdSendReply(reply_buff, hdr.reply_addr);
}

static void write_failure(uint8_t * reply_buff, elysium_cmd_hdr_t hdr) {
  
  reply_buff[0] = hdr.opcode; /* opcode w/o CRC or Reply */
  reply_buff[1] = (hdr.crc ? 2 : 0);
  reply_buff[2] = bank0p[RegSrcAddrMsb];
  reply_buff[3] = bank0p[RegSrcAddrLsb];
  if (hdr.crc) {
    crcGenX25(reply_buff, 6);
  }
}

static void gen_failure(elysium_cmd_hdr_t hdr) {
  uint8_t * reply_buff = elyFWGetBuffer();
  if (reply_buff == NULL) {
    /* TODO signal a buffer overflow here */
    return;
  }
  
  write_failure(reply_buff, hdr);
  
  elyCmdSendReply(reply_buff, hdr.reply_addr);
}

static void get_u32(const uint8_t addr, elysium_cmd_hdr_t hdr, 
    const uint32_t max, const uint32_t min) {
  
  if (hdr.reply) {
    uint8_t * reply_buff = elyFWGetBuffer();
    if (reply_buff == NULL) {
      /* TODO signal a buffer overflow here */
      return;
    }
    
    reply_buff[0] = hdr.opcode;
    reply_buff[2] = bank0p[RegSrcAddrMsb];
    reply_buff[3] = bank0p[RegSrcAddrLsb];
    /* ENH this could be more efficient */
    uint32_t val = 0;
    for (int i = 0; i < 4; i++) {
      val |= ((uint32_t)(bank0p[addr + i]) << (8 * i));
    }
    
    if (val > max) {
      val = max;
      for (int i = 0; i < 4; i++) {
        bank0w[addr + i] = ((val >> (8 * i)) & 0xFF);
      }
    }
    else if (val < min) {
      val = min;
      for (int i = 0; i < 4; i++) {
        bank0w[addr + i] = ((val >> (8 * i)) & 0xFF);
      }
    }
    
    for (int i = 0; i < 4; i++) {
      reply_buff[i + 4] = ((val >> (8 * (3 - i))) & 0xFF);
    }
    
    if (hdr.crc) {
      reply_buff[1] = 6;
      crcGenX25(reply_buff, 10);
    }
    else {
      reply_buff[1] = 4;
    }
    elyCmdSendReply(reply_buff, hdr.reply_addr);
  }
  
}

static void set_u32(const uint8_t addr, const uint8_t * buffer, elysium_cmd_hdr_t hdr,
    const uint32_t max, const uint32_t min) {
  uint8_t hdr_ext = (hdr.reply ? 4 : 2);
  uint32_t val = 0;
  
  /* ENH this could be more efficient */
  for (int i = 0; i < 4; i++) {
    val |= ((uint32_t)(buffer[hdr_ext + i]) << (8 * (3-i)));
  }
  
  if (val > max || 
      val < min) {
    if (hdr.reply) {
      /* Send a FAILURE message */
      gen_failure(hdr);
    }
    else {
      elyErrorSignal(ErrCmdFailure);
    }
    return;
  }
  
  bank0w[addr+3] = buffer[hdr_ext];
  bank0w[addr+2] = buffer[hdr_ext+1];
  bank0w[addr+1] = buffer[hdr_ext+2];
  bank0w[addr] = buffer[hdr_ext+3];
  
  if (hdr.reply) {
    /* Send a SUCCESS message */
    gen_success(hdr);
  }
  
}

static void reset(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  (void)(hdr);
  elyNLFreeBuffer(elyNLFromFW(buffer));
  active_buffer = NULL; /* have to null before reset */
  PMMCTL0 = PMMPW | SVSHE | PMMSWPOR; /* SW POR */
}

static void get_gpo(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  if (hdr.reply) { /* reply requested */
    uint8_t * reply_buff = elyFWGetBuffer();
    if (reply_buff == NULL) {
      /* TODO signal a buffer overflow here */
      return;
    }
    
    reply_buff[0] = hdr.opcode; /* opcode w/o CRC or Reply */
    reply_buff[1] = 1 + (hdr.crc << 1);
    reply_buff[2] = bank0p[RegSrcAddrMsb];
    reply_buff[3] = bank0p[RegSrcAddrLsb];
    reply_buff[4] = bank0p[RegGPOState];
    if (hdr.crc) {
      crcGenX25(reply_buff, 7);
    }
    elyCmdSendReply(reply_buff, hdr.reply_addr);
  }
  
}

static void set_gpo(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  uint8_t arg = buffer[(hdr.reply ? 4 : 2)];
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  if (arg > 1) {
    if (hdr.reply) {
      /* Send a FAILURE message */
      gen_failure(hdr);
    }
    else {
      /* Unreported command failure */
      elyErrorSignal(ErrCmdFailure);
    }
    return;
  }
  
  if (bank0p[RegGPOState] != arg) {
#if ELY_REVISION == B
    palWriteLine(LINE_GPO, arg);
#endif
    elyEventSignal(EvtGPOChange);
    bank0w[RegGPOState] = arg;
  }
  
  if (hdr.reply) {
    /* Send a SUCCESS message */
    gen_success(hdr);
  }
  
}

static void get_active_bank(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  if (hdr.reply) { /* reply requested */
    uint8_t * reply_buff = elyFWGetBuffer();
    if (reply_buff == NULL) {
      /* TODO signal a buffer overflow here */
      return;
    }
    
    reply_buff[0] = hdr.opcode; /* opcode w/o CRC or Reply */
    reply_buff[1] = 1 + (hdr.crc << 1);
    reply_buff[2] = bank0p[RegSrcAddrMsb];
    reply_buff[3] = bank0p[RegSrcAddrLsb];
    reply_buff[4] = bank0p[RegActiveBank];
    if (hdr.crc) {
      crcGenX25(reply_buff, 7);
    }
    elyCmdSendReply(reply_buff, hdr.reply_addr);
  }
  
}

static void get_regs(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  if (hdr.reply) {
    uint8_t * reply_buff = elyFWGetBuffer();
    if (reply_buff == NULL) {
      /* TODO signal a buffer overflow here */
      elyNLFreeBuffer(elyNLFromFW(buffer));
      return;
    }
    
    uint8_t bank = buffer[4];
    const uint8_t regs_offset = 5;
    const uint8_t num_regs = hdr.length - (hdr.crc ? 2 : 0) - 3;
    
    /* We want to free the big buffer before we suspend, so we copy
     * the addresses to the reply buffer before passing it to the FRAM subsystem */
    for (int i = 0; i < num_regs; i++) {
      uint8_t addr = buffer[i + regs_offset];
      /* We can validate at the same time, and the loop isn't slower than memcpy */
      if ((bank > 4) ||
          (addr >= RegCoreMAX && addr < 0x60) ||
          (addr >= RegSpecialMAX && addr < 0x80) ||
          (addr >= RegNLMAX && addr < 0xC0) ||
          (addr >= RegDLLMAX)) {
        write_failure(reply_buff, hdr);
        elyCmdSendReply(reply_buff, hdr.reply_addr);
        elyNLFreeBuffer(elyNLFromFW(buffer));
        return;
      }
      
      reply_buff[i+4] = addr;
    }
    
    /* Done with the big buffer */
    elyNLFreeBuffer(elyNLFromFW(buffer));
    
    reply_buff[0] = hdr.opcode; /* opcode w/o CRC or Reply */
    reply_buff[2] = bank0p[RegSrcAddrMsb];
    reply_buff[3] = bank0p[RegSrcAddrLsb];
    
    /* this can no longer fail */
    elyRegGet(bank, reply_buff + 4, num_regs);
    
    if (hdr.crc) {
      reply_buff[1] = num_regs + 2;
      crcGenX25(reply_buff, num_regs + 6);
    }
    else {
      reply_buff[1] = num_regs;
    }
    
    elyCmdSendReply(reply_buff, hdr.reply_addr);
  }
  else {
    elyNLFreeBuffer(elyNLFromFW(buffer));
  }
}

static void set_regs(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  uint8_t bank = (hdr.reply ? buffer[4] : buffer[2]);
  const uint8_t regs_offset = (hdr.reply ? 5 : 3);
  const uint8_t num_regs = (hdr.length - (hdr.crc ? 2 : 0) - (hdr.reply ? 2 : 0)) / 2;
  uint8_t * reply_buff = elyFWGetBuffer();
  if (reply_buff == NULL) {
    /* TODO signal a buffer overflow here */
    elyNLFreeBuffer(elyNLFromFW(buffer));
    return;
  }
    
  
  for (int i = 0; i < num_regs * 2; i += 2) {
    uint8_t addr = buffer[i + regs_offset];
    
    if ((bank == 0) || (bank > 4) ||
        (addr >= RegCoreMAX && addr < 0x80) ||
        (addr >= RegNLMAX && addr < 0xC0) ||
        (addr >= RegDLLMAX)) {
      if (hdr.reply) {
        write_failure(reply_buff, hdr);
        elyCmdSendReply(reply_buff, hdr.reply_addr);
      }
      elyNLFreeBuffer(elyNLFromFW(buffer));
      return;
    }
    
    reply_buff[i+regs_offset-1] = addr;
    reply_buff[i+regs_offset] = buffer[i+regs_offset+1];
  }
  
  /* Done with the big buffer */
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  /* this can no longer fail */
  elyRegSet(bank, reply_buff + regs_offset-1, num_regs);
  
  if (hdr.reply) {
    /* send a SUCCESS message */
    write_success(reply_buff, hdr);
    elyCmdSendReply(reply_buff, hdr.reply_addr);
  }
  else {
    elyFWFreeBuffer(elyNLFromFW(reply_buff));
  }

}

static void get_block(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  if (hdr.reply) { /* reply requested */
    uint8_t * reply_buff = elyFWGetBuffer();
    if (reply_buff == NULL) {
      /* TODO signal a buffer overflow here */
      elyNLFreeBuffer(elyNLFromFW(buffer));
      return;
    }
    
    uint8_t bank = buffer[4];
    uint8_t addr = buffer[5];
    uint8_t count = buffer[6];
    
    /* Done with the big buffer */
    elyNLFreeBuffer(elyNLFromFW(buffer));
    
    if (bank > 4 || 
        (addr >= RegCoreMAX - count + 1 && addr < 0x60) ||
        (addr >= RegSpecialMAX && addr < 0x80) ||
        (addr >= RegNLMAX - count + 1 && addr < 0xC0) ||
        (addr >= RegDLLMAX - count + 1)) {
      write_failure(reply_buff, hdr);
      elyCmdSendReply(reply_buff, hdr.reply_addr);
      return; 
    }
    
    elyRegGetBlock(bank, reply_buff + 4, addr, count);
    
    reply_buff[0] = hdr.opcode; /* opcode w/o CRC or Reply */
    reply_buff[2] = bank0p[RegSrcAddrMsb];
    reply_buff[3] = bank0p[RegSrcAddrLsb];
    
    if (hdr.crc) {
      reply_buff[1] = count + 2; 
      crcGenX25(reply_buff, count + 6);
    }
    else {
      reply_buff[1] = count; 
    }
    elyCmdSendReply(reply_buff, hdr.reply_addr);
  }
  else {
    elyNLFreeBuffer(elyNLFromFW(buffer));
  }

}

static void set_block(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  uint8_t hdr_ext = (hdr.reply ? 4 : 2);
  uint8_t bank = buffer[hdr_ext];
  uint8_t addr = buffer[hdr_ext+1];
  uint8_t count = hdr.length - (hdr.reply ? 2 : 0) - (hdr.crc ? 2 : 0) - 2;
  uint8_t * reply_buff = elyFWGetBuffer();
  if (reply_buff == NULL) {
    /* TODO signal a buffer overflow here */
    elyNLFreeBuffer(elyNLFromFW(buffer));
    return;
  }
    
  
  if (bank == 0 || bank > 4 || 
      (addr >= RegCoreMAX - count + 1 && addr < 0x80) ||
      (addr >= RegNLMAX - count + 1 && addr < 0xC0) ||
      (addr >= RegDLLMAX - count + 1)) {
    if (hdr.reply) {
      write_failure(reply_buff, hdr);
      elyCmdSendReply(reply_buff, hdr.reply_addr);
    }
    elyNLFreeBuffer(elyNLFromFW(buffer));
    return;
  }
  
  memcpy(reply_buff + 4, buffer + hdr_ext + 2, count);
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  elyRegSetBlock(bank, reply_buff + 4, addr, count);
  
  if (hdr.reply) {
    /* Send a SUCCESS message */
    write_success(reply_buff, hdr);
    elyCmdSendReply(reply_buff, hdr.reply_addr);
  }
  else {
    elyFWFreeBuffer(elyNLFromFW(reply_buff));
  }
}

static void get_tx_freq(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  get_u32(RegTXFreqLsb, hdr, TX_BAND_MAX, TX_BAND_MIN);

}

static void set_tx_freq(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  set_u32(RegTXFreqLsb, buffer, hdr, TX_BAND_MAX, TX_BAND_MIN);
  
  elyNLFreeBuffer(elyNLFromFW(buffer));

  /* Tell the RF thread to re-configure */
  elyRFCfgMarkDirty(RFTxFreqUpdated);
      
  /* Signal the event */
  elyEventSignal(EvtTXFreqChange);
  
}

static void get_rx_freq(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  get_u32(RegRXFreqLsb, hdr, RX_BAND_MAX, RX_BAND_MIN);

}

static void set_rx_freq(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  set_u32(RegRXFreqLsb, buffer, hdr, RX_BAND_MAX, RX_BAND_MIN);
  
  elyNLFreeBuffer(elyNLFromFW(buffer));

  /* Tell the RF thread to re-configure */
  elyRFCfgMarkDirty(RFRxFreqUpdated);
      
  /* Signal the event */
  elyEventSignal(EvtRXFreqChange);
  
}

static void get_tx_br(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  get_u32(RegTXBRLsb, hdr, TX_BR_MAX, TX_BR_MIN);

}

static void set_tx_br(uint8_t* buffer, elysium_cmd_hdr_t hdr) {

  set_u32(RegTXBRLsb, buffer, hdr, TX_BR_MAX, TX_BR_MIN);
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  /* Tell the RF thread to re-configure */
  elyRFCfgMarkDirty(RFTxBRUpdated);
      
  /* Signal the event */
  elyEventSignal(EvtTXBRChange);

}

static void get_rx_br(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  get_u32(RegRXBRLsb, hdr, RX_BR_MAX, RX_BR_MIN);
  
}

static void set_rx_br(uint8_t* buffer, elysium_cmd_hdr_t hdr) {

  set_u32(RegRXBRLsb, buffer, hdr, RX_BR_MAX, RX_BR_MIN);
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  /* Tell the RF thread to re-configure */
  elyRFCfgMarkDirty(RFRxBRUpdated);
      
  /* Signal the event */
  elyEventSignal(EvtRXBRChange);
  
}

static void get_tx_dev(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  get_u32(RegTXDevLsb, hdr, TX_DEV_MAX, TX_DEV_MIN);

}

static void set_tx_dev(uint8_t* buffer, elysium_cmd_hdr_t hdr) {

  set_u32(RegTXDevLsb, buffer, hdr, TX_DEV_MAX, TX_DEV_MIN);
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  /* Tell the RF thread to re-configure */
  elyRFCfgMarkDirty(RFTxDevUpdated);
      
  /* Signal the event */
  elyEventSignal(EvtTXDevChange);
  
}

static void get_rx_dev(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  get_u32(RegRXDevLsb, hdr, RX_DEV_MAX, RX_DEV_MIN);
  
}

static void set_rx_dev(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  set_u32(RegRXDevLsb, buffer, hdr, RX_DEV_MAX, RX_DEV_MIN);
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  /* Tell the RF thread to re-configure */
  elyRFCfgMarkDirty(RFRxDevUpdated);
      
  /* Signal the event */
  elyEventSignal(EvtRXDevChange);

}

static void get_tx_pow(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  if (hdr.reply) {
    uint8_t * reply_buff = elyFWGetBuffer();
    if (reply_buff == NULL) {
      /* TODO signal a buffer overflow here */
      return;
    }
    
    reply_buff[0] = hdr.opcode;
    reply_buff[2] = bank0p[RegSrcAddrMsb];
    reply_buff[3] = bank0p[RegSrcAddrLsb];
    reply_buff[4] = bank0p[RegOutputPower];
    if (hdr.crc) {
      reply_buff[1] = 3;
      crcGenX25(buffer, 7);
    }
    else {
      reply_buff[1] = 1;
    }
    elyCmdSendReply(reply_buff, hdr.reply_addr);
  }

}

static void set_tx_pow(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  uint8_t hdr_ext = (hdr.reply ? 4 : 2);
  
  if (buffer[hdr_ext] > TX_POW_MAX) {
    if (hdr.reply) {
      /* Send a FAILURE message */
      gen_failure(hdr);
    }
    else {
      elyErrorSignal(ErrCmdFailure);
    }
    elyNLFreeBuffer(elyNLFromFW(buffer));
    return;
  }
  
  bank0w[RegOutputPower] = buffer[hdr_ext];
  
  /* Tell the RF thread to re-configure */
  elyRFCfgMarkDirty(RFTxPowerUpdated);
      
  if (hdr.reply) {
    /* Send a SUCCESS message */
    gen_success(hdr);
  }
  
  elyNLFreeBuffer(elyNLFromFW(buffer));

}

static void get_baud(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  get_u32(RegUARTBaudLsb, hdr, UART_BAUD_MAX, UART_BAUD_MIN);

}

static void set_baud(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  set_u32(RegUARTBaudLsb, buffer, hdr, UART_BAUD_MAX, UART_BAUD_MIN);
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  /* Tell the UART thread to reconfigure */
  elyUARTCfgMarkDirty();
      
}

static void reload_config(uint8_t * buffer, elysium_cmd_hdr_t hdr) {
  uint8_t hdr_ext = (hdr.reply ? 4 : 2);
  uint8_t bank = buffer[hdr_ext];
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  if (bank == 0 || bank > 4) {
    
    if (hdr.reply) {
      /* Send a FAILURE message */
      gen_failure(hdr);
    }
    else {
      elyErrorSignal(ErrCmdFailure);
    }
    return;
    
  }
  
  elyRegGetBlock(bank, bank0w, 0, RegCoreMAX);
  elyRegGetBlock(bank, bank0w + 0x80, 0x80, RegNLMAX - 0x80);
  elyRegGetBlock(bank, bank0w + 0xC0, 0xC0, RegDLLMAX - 0xC0);
  
  /* Set new active bank */
  bank0w[RegActiveBank] = bank;
  
  active_buffer = NULL; /* have to null before reset */
  
  /* Reboot the Elysium to re-assert config */
  /* TODO go through all the threads and make sure this really happens */
  PMMCTL0 = PMMPW | SVSHE | PMMSWPOR; /* SW POR */
}

static void channel_sub(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  uint8_t payload_len = hdr.length - ((hdr.crc ? 2 : 0) + (hdr.reply ? 2 : 0));
  uint8_t hdr_ext = (hdr.reply ? 4 : 2);
  uint32_t interval = 0;
  
  for (int i = hdr_ext + 4; i < hdr_ext + payload_len; i++) {
    if ((buffer[i] < 0x40) || 
        (buffer[i] >= ChanCoreMAX && buffer[i] < 0x60) ||
        (buffer[i] >= ChanNLMAX && buffer[i] < 0x70) ||
        (buffer[i] >= ChanDLLMAX)) {
      if (hdr.reply) {
        /* Send a FAILURE message */
        gen_failure(hdr);
      }
      else {
        elyErrorSignal(ErrCmdFailure);
      }
      elyNLFreeBuffer(elyNLFromFW(buffer));
      return;
    }
  }
  
  for (int i = hdr_ext; i < hdr_ext + 4; i++) {
    interval |= ((uint32_t)(buffer[hdr_ext]) << (8 * (3-i)));
  }
  
  /* coerce interval to an acceptable value */
  interval = (interval / 100) * 100;
  
  /* Subscribe to channels */
  elyChanSubscribe(buffer + hdr_ext + 4, payload_len-4, interval);
  
  if (hdr.reply) {
    /* send a SUCCESS message */
    gen_success(hdr);
  }
  
  elyNLFreeBuffer(elyNLFromFW(buffer));

}

static void channel_unsub(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  uint8_t payload_len = hdr.length - ((hdr.crc ? 2 : 0) + (hdr.reply ? 2 : 0));
  uint8_t hdr_ext = (hdr.reply ? 4 : 2);
  
  for (int i = hdr_ext; i < hdr_ext + payload_len; i++) {
    if (buffer[i] < 0x40 || buffer[i] > 0x7F) {
      if (hdr.reply) {
        /* Send a FAILURE message */
        gen_failure(hdr);
      }
      else {
        elyErrorSignal(ErrCmdFailure);
      }
      elyNLFreeBuffer(elyNLFromFW(buffer));
      return;
    }
  }
  
  elyChanUnsubscribe(buffer + hdr_ext, payload_len);
  
  if (hdr.reply) {
    /* send a SUCCESS message */
    gen_success(hdr);
  }
  elyNLFreeBuffer(elyNLFromFW(buffer));

}

static void log_chan(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  uint8_t payload_len = hdr.length - ((hdr.crc ? 2 : 0) + (hdr.reply ? 2 : 0));
  uint8_t hdr_ext = (hdr.reply ? 4 : 2);
  uint32_t interval = 0;
  
  for (int i = hdr_ext + 4; i < hdr_ext + payload_len; i++) {
    if ((buffer[i] < 0x40) || 
        (buffer[i] >= ChanCoreMAX && buffer[i] < 0x60) ||
        (buffer[i] >= ChanNLMAX && buffer[i] < 0x70) ||
        (buffer[i] >= ChanDLLMAX)) {
      if (hdr.reply) {
        /* Send a FAILURE message */
        gen_failure(hdr);
      }
      else {
        elyErrorSignal(ErrCmdFailure);
      }
      elyNLFreeBuffer(elyNLFromFW(buffer));
      return;
    }
  }
  
  for (int i = hdr_ext; i < hdr_ext + 4; i++) {
    interval |= ((uint32_t)(buffer[hdr_ext]) << (8 * (3-i)));
  }
  
  /* coerce interval to an acceptable value */
  interval = (interval / 100) * 100;
  
  elyChanLog(buffer + hdr_ext + 4, payload_len - 4, interval);
  
  if (hdr.reply) {
    /* send a SUCCESS message */
    gen_success(hdr);
  }
  
  elyNLFreeBuffer(elyNLFromFW(buffer));

}

static void unlog_chan(uint8_t* buffer, elysium_cmd_hdr_t hdr) {

  uint8_t payload_len = hdr.length - ((hdr.crc ? 2 : 0) + (hdr.reply ? 2 : 0));
  uint8_t hdr_ext = (hdr.reply ? 4 : 2);
  
  for (int i = hdr_ext; i < hdr.length + hdr_ext - (hdr.crc ? 2 : 0); i++) {
    if (buffer[i] < 0x40 || buffer[i] > 0x7F) {
      if (hdr.reply) {
        /* Send a FAILURE message */
        gen_failure(hdr);
      }
      else {
        elyErrorSignal(ErrCmdFailure);
      }
      elyNLFreeBuffer(elyNLFromFW(buffer));
      return;
    }
  }
  
  elyChanUnlog(buffer + hdr_ext, payload_len);
  
  if (hdr.reply) {
    /* send a SUCCESS message */
    gen_success(hdr);
  }
  
  elyNLFreeBuffer(elyNLFromFW(buffer));

}

static void get_chan(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  if (hdr.reply) {
    if ((buffer[4] < 0x40) || 
        (buffer[4] >= ChanCoreMAX && buffer[4] < 0x60) ||
        (buffer[4] >= ChanNLMAX && buffer[4] < 0x70) ||
        (buffer[4] >= ChanDLLMAX)) {
        /* Send a FAILURE message */
        gen_failure(hdr);
        elyNLFreeBuffer(elyNLFromFW(buffer));
        return;
    }
    
    uint8_t * reply_buff = elyFWGetBuffer();
    if (reply_buff == NULL) {
      /* TODO signal a buffer overflow here */
      elyNLFreeBuffer(elyNLFromFW(buffer));
      return;
    }
    
    reply_buff[0] = hdr.opcode;
    reply_buff[2] = bank0p[RegSrcAddrMsb];
    reply_buff[3] = bank0p[RegSrcAddrLsb];
    
    /* Returns the size of the channel */
    size_t n = elyChanGetValue(&reply_buff[4], buffer[4]);
    
    elyNLFreeBuffer(elyNLFromFW(buffer));
    
    if (hdr.crc) {
      reply_buff[1] = n + 3;
      crcGenX25(reply_buff, 7+n);
    }
    else {
      reply_buff[1] = n + 1;
    }
    
    elyCmdSendReply(reply_buff, hdr.reply_addr);
  }
  else {
    elyNLFreeBuffer(elyNLFromFW(buffer));
  }

}

static void reset_chan(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  elyChanReset();
  
  if (hdr.reply) {
    /* send a SUCCESS message */
    gen_success(hdr);
  }
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
}

static void event_sub(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  uint8_t payload_len = hdr.length - ((hdr.crc ? 2 : 0) + (hdr.reply ? 2 : 0));
  uint8_t hdr_ext = (hdr.reply ? 4 : 2);
  
  for (int i = hdr_ext; i < hdr_ext + payload_len; i++) {
    if ((buffer[i] < 0xC0) || 
        (buffer[i] >= EvtCoreMAX && buffer[i] < 0xE0) ||
        (buffer[i] >= EvtNLMAX && buffer[i] < 0xF0) ||
        (buffer[i] >= EvtDLLMAX)) {
      if (hdr.reply) {
        /* Send a FAILURE message */
        gen_failure(hdr);
      }
      else {
        elyErrorSignal(ErrCmdFailure);
      }
      elyNLFreeBuffer(elyNLFromFW(buffer));
      return;
    }
  }
  
  uint16_t addr;
  if (hdr.reply) {
    addr = ((buffer[2] << 8) | (buffer[3]));
  }
  else {
    addr = ((bank0p[RegEventDefaultAddrMsb] << 8) | (bank0p[RegEventDefaultAddrLsb]));
  }
  
  /* ENH batch these together by pushing the loop into the Event code */
  for (int i = hdr_ext; i < hdr_ext + payload_len; i++) {
      elyEventSubscribe(buffer[i], addr);
  }
  
  if (hdr.reply) {
    /* send a SUCCESS message */
    gen_success(hdr);
  }
  
  elyNLFreeBuffer(elyNLFromFW(buffer));

}

static void event_unsub(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  uint8_t payload_len = hdr.length - ((hdr.crc ? 2 : 0) + (hdr.reply ? 2 : 0));
  uint8_t hdr_ext = (hdr.reply ? 4 : 2);
  
  for (int i = hdr_ext; i < hdr_ext + payload_len; i++) {
    if (buffer[i] < 0xC0) {
      if (hdr.reply) {
        /* Send a FAILURE message */
        gen_failure(hdr);
      }
      else {
        elyErrorSignal(ErrCmdFailure);
      }
      elyNLFreeBuffer(elyNLFromFW(buffer));
      return;
    }
  }
  
  for (int i = hdr_ext; i < hdr_ext + payload_len; i++) {
    elyEventUnsubscribe(buffer[i]);
  }
  
  if (hdr.reply) {
    /* send a SUCCESS message */
    gen_success(hdr);
  }
  
  elyNLFreeBuffer(elyNLFromFW(buffer));

}

static void log_event(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  uint8_t payload_len = hdr.length - ((hdr.crc ? 2 : 0) + (hdr.reply ? 2 : 0));
  uint8_t hdr_ext = (hdr.reply ? 4 : 2);
  
  for (int i = hdr_ext; i < hdr_ext + payload_len; i++) {
    if ((buffer[i] < 0xC0) || 
        (buffer[i] >= EvtCoreMAX && buffer[i] < 0xE0) ||
        (buffer[i] >= EvtNLMAX && buffer[i] < 0xF0) ||
        (buffer[i] >= EvtDLLMAX)) {
      if (hdr.reply) {
        /* Send a FAILURE message */
        gen_failure(hdr);
      }
      else {
        elyErrorSignal(ErrCmdFailure);
      }
      elyNLFreeBuffer(elyNLFromFW(buffer));
      return;
    }
  }
  
  for (int i = hdr_ext; i < hdr_ext + payload_len; i++) {
    elyEventLog(buffer[i]);
  }
  
  if (hdr.reply) {
    /* send a SUCCESS message */
    gen_success(hdr);
  }
  
  elyNLFreeBuffer(elyNLFromFW(buffer));

}

static void unlog_event(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  uint8_t payload_len = hdr.length - ((hdr.crc ? 2 : 0) + (hdr.reply ? 2 : 0));
  uint8_t hdr_ext = (hdr.reply ? 4 : 2);
  
  for (int i = hdr_ext; i < hdr_ext + payload_len; i++) {
    /* TODO no magic numbers */
    if (buffer[i] < 0xC0) {
      if (hdr.reply) {
        /* Send a FAILURE message */
        gen_failure(hdr);
      }
      else {
        elyErrorSignal(ErrCmdFailure);
      }
      elyNLFreeBuffer(elyNLFromFW(buffer));
      return;
    }
  }
  
  for (int i = hdr_ext; i < hdr_ext + payload_len; i++) {
    elyEventUnlog(buffer[i]);
  }
  
  if (hdr.reply) {
    /* send a SUCCESS message */
    gen_success(hdr);
  }
  
  elyNLFreeBuffer(elyNLFromFW(buffer));

}

static void reset_event(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  elyEventReset();
  
  if (hdr.reply) {
    /* send a SUCCESS message */
    gen_success(hdr);
  }
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
}

static void set_time(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  uint8_t hdr_ext = (hdr.reply ? 4 : 2);
  
  /* TODO this should probably be atomic */
  bank0w[RegMissionTimeMsb] = buffer[hdr_ext];
  bank0w[RegMissionTimeHmb] = buffer[hdr_ext+1];
  bank0w[RegMissionTimeLmb] = buffer[hdr_ext+2];
  bank0w[RegMissionTimeLsb] = buffer[hdr_ext+3];
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
      
  if (hdr.reply) {
    uint8_t * reply_buff = elyFWGetBuffer();
    if (reply_buff == NULL) {
      /* TODO signal a buffer overflow here */
      return;
    }
    
    reply_buff[0] = hdr.opcode;
    reply_buff[2] = bank0p[RegSrcAddrMsb];
    reply_buff[3] = bank0p[RegSrcAddrLsb];
    reply_buff[1] = hdr.length - 2;
    if (hdr.crc) {
      crcGenX25(reply_buff, reply_buff[1]+4);
    }
    /* TODO handle failure */
    elyCmdSendReply(reply_buff, hdr.reply_addr);
  }

}

static void get_time(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
    
  if (hdr.reply) {
    uint8_t * reply_buff = elyFWGetBuffer();
    if (reply_buff == NULL) {
      /* TODO signal a buffer overflow here */
      return;
    }
    
    reply_buff[0] = hdr.opcode;
    reply_buff[2] = bank0p[RegSrcAddrMsb];
    reply_buff[3] = bank0p[RegSrcAddrLsb];
    reply_buff[1] = hdr.length + 2;
    /* ENH this could be more efficient */
    for (int i = 0; i < 4; i++) {
      reply_buff[i + 4] = bank0p[RegMissionTimeMsb - i];
    }
    if (hdr.crc) {
      crcGenX25(reply_buff, reply_buff[1]+4);
    }
    elyCmdSendReply(reply_buff, hdr.reply_addr);
  }

}

static void get_err(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  if (hdr.reply) {
    uint8_t * reply_buff = elyFWGetBuffer();
    if (reply_buff == NULL) {
      /* TODO signal a buffer overflow here */
      elyNLFreeBuffer(elyNLFromFW(buffer));
      return;
    }
    
    reply_buff[0] = hdr.opcode;
    reply_buff[2] = bank0p[RegSrcAddrMsb];
    reply_buff[3] = bank0p[RegSrcAddrLsb];
    reply_buff[4] = bank0p[RegErrRptLvl];
    if (hdr.crc) {
      reply_buff[1] = 3;
      crcGenX25(reply_buff, 7);
    }
    else {
      reply_buff[1] = 1;
    }
    
    elyCmdSendReply(reply_buff, hdr.reply_addr);
  }
  
  elyNLFreeBuffer(elyNLFromFW(buffer));

}

static void set_err(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  uint8_t hdr_ext = (hdr.reply ? 4 : 2);
  uint8_t err = buffer[hdr_ext];
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  if (err > ELY_ALL_ERRORS) {
    if (hdr.reply) {
      /* Send a FAILURE message */
      gen_failure(hdr);
    }
    else {
      elyErrorSignal(ErrCmdFailure);
    }
    return;
  }
  
  bank0w[RegErrRptLvl] = err;
  
  chSysLock();
  elyErrorSetRptLvlS(err);
  chSysUnlock();
      
  if (hdr.reply) {
    /* Send a SUCCESS message */
    gen_success(hdr);
  }

}

static void get_log(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
    
  if (hdr.reply) {
    uint8_t * reply_buff = elyFWGetBuffer();
    if (reply_buff == NULL) {
      /* TODO signal a buffer overflow here */
      elyNLFreeBuffer(elyNLFromFW(buffer));
      return;
    }
    
    reply_buff[0] = hdr.opcode;
    reply_buff[2] = bank0p[RegSrcAddrMsb];
    reply_buff[3] = bank0p[RegSrcAddrLsb];
    reply_buff[4] = bank0p[RegErrLogLvl];
    if (hdr.crc) {
      reply_buff[1] = 3;
      crcGenX25(reply_buff, 7);
    }
    else {
      reply_buff[1] = 1;
    }
    
    elyCmdSendReply(reply_buff, hdr.reply_addr);
  }

}

static void set_log(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  uint8_t hdr_ext = (hdr.reply ? 4 : 2);
  uint8_t err = buffer[hdr_ext];
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  if (err > ELY_ALL_ERRORS) {
    if (hdr.reply) {
      /* Send a FAILURE message */
      gen_failure(hdr);
    }
    else {
      elyErrorSignal(ErrCmdFailure);
    }
    return;
  }
  
  bank0w[RegErrLogLvl] = err;
  
  chSysLock();
  elyErrorSetLogLvlS(err);
  chSysUnlock();
      
  if (hdr.reply) {
    /* Send a SUCCESS message */
    gen_success(hdr);
  }

}

static void upload_fw_reply_cb(uint8_t * buff) {
  
  chSysLockFromISR();
    write_success(buff, stored_hdr);
    elyCmdSendReplyI(buff, stored_hdr.reply_addr);
  chSysUnlockFromISR();
}

static void upload_fw_noreply_cb(uint8_t * buff) {
  chSysLockFromISR();
  elyFWFreeBufferI(elyNLFromFW(buff));
  chSysUnlockFromISR();
}

static void upload_fw(uint8_t * buffer, elysium_cmd_hdr_t hdr) {
  uint8_t hdr_ext = (hdr.reply ? 4 : 2);
  uint32_t address = 0;
  
  /* ENH this could be more efficient */
  for (int i = 0; i < 4; i++) {
    address |= (uint32_t)(buffer[i + hdr_ext]) << (8 * (3-i));
  }
  
  if (address < 0x4400 || address > 0x13FFF) {
    if (hdr.reply) {
      /* Send a FAILURE message */
      gen_failure(hdr);
    }
    else {
      elyErrorSignal(ErrCmdFailure);
    }
    elyNLFreeBuffer(elyNLFromFW(buffer));
    return;
  }
  
  uint8_t * reply_buff = elyFWGetBuffer();
  if (reply_buff == NULL) {
    /* TODO signal a buffer overflow here */
    elyNLFreeBuffer(elyNLFromFW(buffer));
    return;
  }
  
  fram_req_t * req;
  
  /* TODO timeout for safety */
  chSysLock();
  elyFramGetRequestTimeoutS(&req, TIME_INFINITE);
  chSysUnlock();
  req->size = hdr.length - (hdr.reply ? 2: 0) - (hdr.crc ? 2: 0) - 4;
  
  memcpy(reply_buff, buffer + hdr_ext + 4, req->size);
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  stored_hdr = hdr;
  
  /* Fill in the request */
  req->address = (address - 0x4400) + FRAM_FW_BASE;
  req->read = 0;
  req->buffer = reply_buff;
  req->callback = (hdr.reply ? upload_fw_reply_cb : upload_fw_noreply_cb);
  
  /* Post the request */
  chSysLock();
  elyFramPostRequestS(req);
  chSysUnlock();
}

static void verify_fw_cb(uint8_t * buffer) {
  (void)(buffer);
  
  uint16_t result = crcStop();
  
  chSysLockFromISR();
  uint8_t * reply_buff = elyFWGetBufferI();
  if (reply_buff == NULL) {
    /* TODO signal a buffer overflow here */
    return;
  }
  
  if (result == verify_crc) {
    write_success(reply_buff, verify_stored_hdr);
    elyCmdSendReplyI(reply_buff, stored_hdr.reply_addr);
  }
  else {
    write_failure(reply_buff, verify_stored_hdr);
    elyCmdSendReplyI(reply_buff, stored_hdr.reply_addr);
  }
  
  chSysUnlockFromISR();
}

static void verify_fw(uint8_t * buffer, elysium_cmd_hdr_t hdr) {
  if (hdr.reply) {
    /* Extract expected CRC */
    verify_crc = ((buffer[4] << 8) | (buffer[5]));
    
    elyNLFreeBuffer(elyNLFromFW(buffer));
    
    /* Set up the CRC module */
    volatile uint8_t * crc_addr = crcStart();
    
    fram_req_t * req;
    
    verify_stored_hdr = hdr;
    
    /* TODO timeout for safety */
    chSysLock();
    elyFramGetRequestTimeoutS(&req, TIME_INFINITE);
    chSysUnlock();
    
    /* Fill in the request */
    req->address = FRAM_FW_BASE; 
    req->read = 1;
    req->special = 1; /* read to a constant register address */
    req->size = FRAM_FW_SIZE;
    req->buffer = (uint8_t *)(crc_addr); /* crc input register */
    req->callback = verify_fw_cb;
    
    /* Post the request */
    chSysLock();
    elyFramPostRequestS(req);
    chSysUnlock();
  }
  else {
    elyNLFreeBuffer(elyNLFromFW(buffer));
  }
}

static void cancel_fw_eeprom_cb(uint8_t * buffer) {
  (void)(buffer);
  chSysLockFromISR();
  chBSemSignalI(&cancel_sem);
  chSysUnlockFromISR();
}

#if 0
static void cancel_fw_reply_cb(uint8_t * buffer) {
  (void)(buffer);
  chSysLockFromISR();
  gen_success(stored_buff, stored_buff[0] & 0x80);
  elyCmdSendReplyI(stored_buff);
  chSysUnlockFromISR();

}

static void cancel_fw_noreply_cb(uint8_t * buffer) {
  (void)(buffer);
  chSysLockFromISR();
  elyNLFreeBufferI(elyNLPack(stored_buff));
  chSysUnlockFromISR();
}
#endif

static void cancel_fw(uint8_t * buffer, elysium_cmd_hdr_t hdr) {
  static uint8_t fill_value = 0x00;
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  fram_req_t * req;
  
#if 0
  /* TODO timeout for safety */
  elyFramGetRequest(&req);
  
  
  /* Fill in the request */
  req->address = FRAM_FW_BASE; 
  req->read = 0;
  req->special = 1; /* write a constant value */
  req->size = FRAM_FW_SIZE;
  req->buffer = &fill_value; 
  if (hdr.reply) {
    req->callback = cancel_fw_reply_cb;
  }
  else {
    req->callback = cancel_fw_noreply_cb;
  }
  
  lookbehind = 4;
  
  /* Post the request */
  /* TODO handle failure */
  chSysLock();
  elyFramPostRequestS(req);
  chSysUnlock();
#else
  for (size_t i = 0; i < FRAM_FW_SIZE/128; i++) {
    chSysLock();
    /* TODO timeout for safety */
    elyFramGetRequestTimeoutS(&req, TIME_INFINITE);
    chSysUnlock();
    
    /* Fill in the request */
    req->address = FRAM_FW_BASE + (i * 128); 
    req->read = 0;
    req->special = 1; /* write a constant value */
    req->size = 128;
    req->buffer = &fill_value; 
    req->callback = cancel_fw_eeprom_cb;
    
    /* Post the request */
    /* TODO handle failure */
    chSysLock();
    elyFramPostRequestS(req);
    chSysUnlock();
    
    /* TODO timeout for safety */
    chBSemWaitTimeout(&cancel_sem, TIME_INFINITE);
  }

  if (hdr.reply) {
    /* TODO admit the possibility of failure */
    gen_success(hdr);
  }
#endif

  
}

static void install_fw(uint8_t * buffer, elysium_cmd_hdr_t hdr) {
  (void)(hdr);
  (void)(buffer);
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
  
  /* ENH you should make verification un-inhibit this command */
  if (hdr.reply) {
    gen_success(hdr);
  }
  else {
    elyNLFreeBuffer(elyNLFromFW(buffer));
  }
  
  /* Mark the bootloader as active and reset */
  bootloader = 1;
  PMMCTL0 = PMMPW | SVSHE | PMMSWPOR; /* SW POR */
  
}

static void store_telem_reply_cb(uint8_t * buff) {
  
  write_success(buff, telem_stored_hdr); /* crc */
  chSysLockFromISR();
  elyCmdSendReplyI(buff, telem_stored_hdr.reply_addr);
  chSysUnlockFromISR();
  
}

static void store_telem_noreply_cb(uint8_t * buff) {
  
  chSysLockFromISR();
  elyFWFreeBufferI(elyNLFromFW(buff));
  chSysUnlockFromISR();
  
}

static void store_telem(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  uint8_t hdr_ext = (hdr.reply ? 4 : 2);
  uint8_t payload_len = hdr.length - ((hdr.crc ? 2 : 0) + (hdr.reply ? 2 : 0));
  
  uint8_t * reply_buff = elyFWGetBuffer();
  if (reply_buff == NULL) {
    /* TODO signal a buffer overflow here */
    elyNLFreeBuffer(elyNLFromFW(buffer));
    return;
  }
  
  memcpy(reply_buff + 5, buffer + hdr_ext, payload_len);
  memset(reply_buff + 5 + payload_len, 0, 251 - payload_len);
  telem_stored_hdr = hdr;
  
  elyNLFreeBuffer(elyNLFromFW(buffer));

  chSysLock();
  elyTelemPostBufferS(reply_buff, 
      (hdr.reply ? store_telem_reply_cb : store_telem_noreply_cb));
  chSysUnlock();
  
  /* Reply is posted from callback if needed */
}

static void get_telem(uint8_t* buffer, elysium_cmd_hdr_t hdr) {
  uint8_t hdr_ext = (hdr.reply ? 4 : 2);
  uint8_t payload_len = hdr.length - ((hdr.crc ? 2 : 0) + (hdr.reply ? 2 : 0));
  
  telem_cfg_t config;
  /* TODO no more duration */
  
  if (payload_len == 4 || payload_len == 12) {
    config.use_index = true;
    config.index_start = buffer[hdr_ext+2];
    config.index_end = buffer[hdr_ext+3];
  }
  else {
    config.use_index = false;
  }
  
  if (payload_len > 4) {
    config.use_timestamp = true;
    if (config.use_index) {
      for (int i = 0; i < 4; i++) {
        config.timestamp_start |= (buffer[hdr_ext + 7 - i] << (8 * i));
        config.timestamp_end |= (buffer[hdr_ext + 11 - 1] << (8 * i));
      }
    }
    else {
      for (int i = 0; i < 4; i++) {
        config.timestamp_start |= (buffer[hdr_ext + 5 - i] << (8 * i));
        config.timestamp_end |= (buffer[hdr_ext + 9 - 1] << (8 * i));
      }
    }
  }
  
  elyNLFreeBuffer(elyNLFromFW(buffer));
    
  chSysLock();
  elyTelemUpdateConfigS(config);
  chSysUnlock();
  
  if (hdr.reply) {
    /* send a SUCCESS message */
    gen_success(hdr);
  }
}

static const cmdhandler_t handlers[CMD_MAX] = {
  reset,
  get_gpo,
  set_gpo,
  get_active_bank,
  get_regs,
  set_regs,
  get_block,
  set_block,
  get_tx_freq,
  set_tx_freq,
  get_rx_freq,
  set_rx_freq,
  get_tx_br,
  set_tx_br,
  get_rx_br,
  set_rx_br,
  get_tx_dev,
  set_tx_dev,
  get_rx_dev,
  set_rx_dev,
  get_tx_pow,
  set_tx_pow,
  get_baud,
  set_baud,
  reload_config,
  channel_sub,
  channel_unsub,
  log_chan,
  unlog_chan,
  get_chan,
  reset_chan,
  event_sub,
  event_unsub,
  log_event,
  unlog_event,
  reset_event,
  set_time,
  get_time,
  get_err,
  set_err,
  get_log,
  set_log,
  upload_fw,
  verify_fw,
  cancel_fw,
  install_fw,
  store_telem,
  get_telem
};

/* ENH more clever ways to do this relying on non-portable constructs */
elysium_cmd_hdr_t elyCmdParse(uint8_t * fw_buffer) {
  elysium_cmd_hdr_t hdr;
  hdr.crc = (fw_buffer[0] >> 7);
  hdr.reply = ((fw_buffer[0] >> 6) & 0x01);
  hdr.opcode = (fw_buffer[0] & 0x3F);
  hdr.length = fw_buffer[1];
  if (hdr.reply) {
    hdr.reply_addr = (((uint16_t)(fw_buffer[2]) << 8) | (fw_buffer[3]));
  }
  return hdr;
}

bool elyCmdValidate(elysium_cmd_hdr_t hdr, uint8_t * buff) {
  
  /* Check opcode */
  if (!(hdr.opcode < CMD_MAX)) {
    elyErrorSignal(ErrInvalidOpcode);
    return false;
  }
  
  /* Check return address */
  if (hdr.reply) {
    if (buff[2] == bank0p[RegSrcAddrMsb] && buff[3] == bank0p[RegSrcAddrLsb]) {
      /* TODO maybe make this an error */
      return false;
    }
  }
  
  /* Check length */
  uint8_t payload_len = hdr.length - ((hdr.crc ? 2 : 0) + (hdr.reply ? 2 : 0));
  switch(hdr.opcode) {
    case CmdReset:
    case CmdGetGPO:
    case CmdGetActiveBank:
    case CmdGetTXFreq:
    case CmdGetRXFreq:
    case CmdGetTXRate:
    case CmdGetRXRate:
    case CmdGetTXDev:
    case CmdGetRXDev:
    case CmdGetTXPow:
    case CmdGetBaud:
    case CmdResetChan:
    case CmdResetEvent:
    case CmdGetTime:
    case CmdGetErr:
    case CmdGetLog:
    case CmdCancelFW:
    case CmdInstallFW:
      if (payload_len != 0) {
        elyErrorSignal(ErrInvalidLength);
        return false;
      }
      break;

    case CmdSetGPO:
    case CmdSetTXPow:
    case CmdReloadConfig:
    case CmdGetChan:
    case CmdSetErr:
    case CmdSetLog:
      if (payload_len != 1) {
        elyErrorSignal(ErrInvalidLength);
        return false;
      }
      break;
      
    case CmdVerifyFW: /* 2 */
      if (payload_len != 2) {
        elyErrorSignal(ErrInvalidLength);
        return false;
      }
      break;
      
    case CmdGetBlock: /* 3 */
      if (payload_len != 3) {
        elyErrorSignal(ErrInvalidLength);
        return false;
      }
      break;

    case CmdSetTXFreq:/* 4 */
    case CmdSetRXFreq:/* 4 */
    case CmdSetTXRate:/* 4 */
    case CmdSetRXRate:/* 4 */
    case CmdSetTXDev: /* 4 */
    case CmdSetRXDev: /* 4 */
    case CmdSetBaud:  /* 4 */
    case CmdSetTime:/* 4 */
      if (payload_len != 4) {
        elyErrorSignal(ErrInvalidLength);
        return false;
      }
      break;
      
    case CmdGetTelem: /* 2 + 2 + 8 */
      if (payload_len != 2 && payload_len != 4 && 
          payload_len != 10 && payload_len != 12) {
        elyErrorSignal(ErrInvalidLength);
        return false;
      }
      break;

    case CmdGetRegs: /* 8-bit aligned */
    case CmdChannelUnsub: /* 8-bit aligned */
    case CmdUnlogChan: /* 8-bit aligend */
    case CmdEventSub: /* 8-bit aligned */
    case CmdEventUnsub:
    case CmdLogEvent: /* 8-bit aligned */
    case CmdUnlogEvent:
    case CmdStoreTelem: /* 8-bit aligned */
      if (payload_len > 251) {
        elyErrorSignal(ErrInvalidLength);
        return false;
      }
      break;
      
    case CmdSetRegs: /* 1 + 16-bit aligned */
      if (payload_len < 1 || !(payload_len & 0x01) || payload_len > 251) {
        elyErrorSignal(ErrInvalidLength);
        return false;
      }
      break;
      
    case CmdSetBlock: /* 2 + 8-bit aligned */
      if (payload_len < 2 || payload_len > 251) {
        elyErrorSignal(ErrInvalidLength);
        return false;
      }
      break;
      
    case CmdChannelSub: /* 4 + 8-bit aligned */
    case CmdLogChan: /* 4 + 8-bit aligned */
    case CmdUploadFW: /* 4 + 8-bit aligned */
      if (payload_len < 4 || payload_len > 251) {
        elyErrorSignal(ErrInvalidLength);
        return false;
      }
      break;
      
    default:
      /* Shouldn't happen */
      chDbgAssert(false, "invalid opcode in length check");
  }
  
  /* Check CRC */
  if (hdr.crc) {
    if (!crcCheckX25(buff, hdr.length + 2)) {
      elyErrorSignal(ErrFCSError);
      return false;
    }
  }
  
  return true;
  
}

void elyCmdDispatch(elysium_cmd_hdr_t hdr, uint8_t * buff) {
  /* Dispatch */
  handlers[hdr.opcode](buff, hdr);
}

void elyCmdSendReply(uint8_t * buff, uint16_t dest_addr) {
  chSysLock();
  elyCmdSendReplyI(buff, dest_addr);
  chSysUnlock();
}

void elyCmdSendReplyI(uint8_t * buff, uint16_t dest_addr) {
  uint8_t length = buff[1];
  uint8_t * buffer = elyNLFromFW(buff);
  
  elyNLSetHeader(buffer, length+4, dest_addr);
  switch (elyNLGetDest(buffer, dest_addr)) {
    case ELY_DEST_RF:
      if (MSG_OK != elyRFPostI(buffer)) {
        /* We should never hit this because everyone allocates the same amount */
        chDbgAssert(false, "internal buffer overflows should be impossible");
        /* If we hit it anyway, free the buffer and weep */
        elyFWFreeBuffer(buffer);
      }
      break;
    case ELY_DEST_UART:
      if (MSG_OK != elyUARTPostI(buffer)) {
        /* We should never hit this because everyone allocates the same amount */
        chDbgAssert(false, "internal buffer overflows should be impossible");
        /* If we hit it anyway, free the buffer and weep */
        elyFWFreeBuffer(buffer);
      }
      break;
    default:
      chDbgAssert(false, "shouldn't happen");
      break;
  }
}

THD_WORKING_AREA(waCmdThd, 256);
THD_FUNCTION(CmdThd, arg) {
  (void)arg;
  
  while (true) {
    if (NULL == active_buffer) {
      chMBFetch(&main_mbox, (msg_t *)(&active_buffer), TIME_INFINITE);
    }
    
    uint8_t * cmd_buffer = elyNLToFW(active_buffer);
    elysium_cmd_hdr_t hdr;
    
    hdr = elyCmdParse(cmd_buffer);
    if (elyCmdValidate(hdr, cmd_buffer)) {
      elyCmdDispatch(hdr, cmd_buffer);
    }
    else {
      /* Invalid command - free the buffer (guaranteed to be NL) */
      elyNLFreeBuffer(active_buffer);
    }
    
    /* null the buffer so we get a new one */
    active_buffer = NULL; 
  }
}
