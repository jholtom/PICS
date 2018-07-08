
#include "nl.h"
#include "registers.h"
#include "errors.h"
#include "rf.h"
#include "uart.h"
#include "main.h"

size_t PERSIST elyNLMaxLen = elyNLDefaultMaxLen;
static uint8_t PERSIST mpool_storage[elyNLTotalBuffer];

uint8_t PERSIST packets_received;
uint16_t PERSIST packets_sent = 1;
uint8_t PERSIST packets_relayed;
static size_t sh_len;

void * nl_allocator(size_t size, unsigned align) {
  (void)(align);
  /* Provides memory blocks for the pool */
  static size_t PERSIST curr_index = 0;
  
  if (curr_index + size > elyNLTotalBuffer) {
    return NULL;
  }

  void * result = mpool_storage + curr_index;
  
  curr_index += size;
  return result;
}

static PERSIST MEMORYPOOL_DECL(main_mpool, elyNLDefaultMaxLen, nl_allocator);

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

uint8_t elyNLClampReg(uint8_t addr, uint8_t value) {
  chDbgAssert(addr >= 0x80 && addr < RegNLMAX, "invalid addr");
  switch(addr) {
    case RegNLMaxPktLengthLsb:
      value = clamp(value, 7, 0xFF);
      break;
    case RegNLMaxPktLengthMsb:
      value = clamp(value, 0, 0x10);
      break;
    case RegNLGroundAPIDMsb:
      value = clamp(value, 0, 0x07);
      break;
    case RegNLPktNameMsb:
      value = clamp(value, 0, 0x3F);
      break;
    case RegNLPVNErrLvl:
    case RegNLPktLenghtLvl:
      value = clamp_err(value);
      break;
    default:
      /* All other registers have ranges equal to their data type's */
      break;
  }
  return value;
}

void elyNLChangeMaxLengthS(size_t len) {
  chDbgCheckClassS();
  /* We assume the register changes have already been made */
  /* Clear out the memory pool */
  while (chPoolAllocI(&main_mpool) != NULL) ;
  /* Free all of the storage */
  for (size_t i = 0; i < elyNLTotalBuffer; i += elyNLMaxLen) {
    chPoolFreeI(&main_mpool, mpool_storage + i);
  }
  /* Change the size */
  elyNLMaxLen = len;
  main_mpool.object_size = elyNLMaxLen;
}

uint8_t * elyNLGetBuffer() {
  return chPoolAlloc(&main_mpool);
}

void elyNLFreeBuffer(uint8_t * buffer) {
  chPoolFree(&main_mpool, buffer);
}

uint8_t * elyNLGetBufferI() {
  chDbgCheckClassI();
  return chPoolAllocI(&main_mpool);
}

void elyNLFreeBufferI(uint8_t * buffer) {
  chDbgCheckClassI();
  chPoolFreeI(&main_mpool, buffer);
}

/* This function is constrained to only touch the header */
bool elyNLValidate(const uint8_t * buffer) {
  /* Validation for Space Packet Protocol means checking the following:
   * Packet Version Number - 000
   * Packet Length - <= MaxPktLength */
  if (buffer[0] & 0xE0) {
    /* Raise PVN Mismatch Error */
    elyErrorSignal(ErrNLPVNMismatch);
    return false;
  }
  
  /* ENH nasty pointer math makes this faster/better/more atomic */
  chSysLock();
  if ( elyNLGetLength(buffer) > 
      ((uint16_t)(bank0p[RegNLMaxPktLengthMsb] << 8) | (uint16_t)(bank0p[RegNLMaxPktLengthLsb])) ) {
    chSysUnlock();
    /* Raise Packet Length Mismatch error */
    elyErrorSignal(ErrNLPacketLengthMismatch);
    return false;
  }
  chSysUnlock();
  return true;
}

static void nl_route(uint8_t * buffer, 
    msg_t (*pass_func)(uint8_t *), 
    msg_t (*fw_func)(uint8_t *),
    msg_t (*loop_func)(uint8_t *)) {
  /* Routing rules:
   * If (TC AND APID == Elysium) FW
   * If (TM AND APID == Elysium) Undefined
   * Else Passthrough */
  
  /* Run Network Layer validation checks */
  if (!elyNLValidate(buffer)) {
    elyNLFreeBuffer(buffer);
    return;
  }
  
  /* TODO better constants */
  uint16_t apid = ( ((buffer[0] & 0x07) << 8) | (buffer[1]) );
  uint16_t elysium_apid = ( ((bank0p[RegSrcAddrMsb] << 8) | 
        (bank0p[RegSrcAddrLsb])) );
  bool tc = buffer[0] & 0x10;
  
  if (apid == elysium_apid) {
    if (tc) {
      if (MSG_OK != fw_func(buffer)) {
        /* We should never hit this because everyone allocates from elyNLMaxLen */
        chDbgAssert(false, "internal buffer overflows should be impossible");
        /* If we hit it anyway, free the buffer and weep */
        elyNLFreeBuffer(buffer);
      }
      return;
    }
    else {
#if CH_DBG_ENABLE_ASSERTS
      buffer[0] |= 0x10; /* set apid to free properly */
      if (MSG_OK != loop_func(buffer)) {
        /* We should never hit this because everyone allocates from elyNLMaxLen */
        chDbgAssert(false, "internal buffer overflows should be impossible");
        /* If we hit it anyway, free the buffer and weep */
        elyNLFreeBuffer(buffer);
      }
#else
      /* Reject invalid packet - free it. */
      /* TODO create an error class for Invalid Address */
      elyNLFreeBuffer(buffer);
#endif
      return;
    }
  }
  
  if (MSG_OK != pass_func(buffer)) {
    /* We should never hit this because everyone allocates from elyNLMaxLen */
    chDbgAssert(false, "internal buffer overflows should be impossible");
    /* If we hit it anyway, free the buffer and weep */
    elyNLFreeBuffer(buffer);
  }
  
  return;
}

static inline msg_t uart_pass(uint8_t * buffer) {
  return elyUARTPost(buffer, TIME_INFINITE);
}

static inline msg_t rf_pass(uint8_t * buffer) {
  return elyRFPost(buffer, TIME_INFINITE);
}

static inline msg_t rf_pass_i(uint8_t * buffer) {
  return elyRFPostI(buffer);
}

static inline msg_t uart_pass_i(uint8_t * buffer) {
  return elyUARTPostI(buffer);
}

static inline msg_t fw(uint8_t * buffer) {
  return elyMainMBPost(buffer, TIME_INFINITE);
}

static inline msg_t fw_i(uint8_t * buffer) {
  return elyMainMBPostI(buffer);
}

void elyNLRouteUART(uint8_t * buffer) {
  nl_route(buffer, rf_pass, fw, uart_pass);
}

void elyNLRouteRF(uint8_t * buffer) {
  nl_route(buffer, uart_pass, fw, rf_pass);
}

void elyNLRouteRFI(uint8_t * buffer) {
  nl_route(buffer, uart_pass_i, fw_i, rf_pass_i);
}

elysium_destinations_t elyNLGetDest(uint8_t * buffer, uint16_t dest_addr) {
  /* buffer ignored for SPP */
  (void)(buffer);
  
  uint16_t ground_apid = ( ((bank0p[RegNLGroundAPIDMsb] << 8) | 
        (bank0p[RegNLGroundAPIDLsb])) );
  
  if (dest_addr == ground_apid) {
    return ELY_DEST_RF;
  }
  
  return ELY_DEST_UART;
}

bool is_fw_buf(uint8_t * buffer) {
  /* Freeing rules:
   * If (TM AND APID == Elysium) FW Reply
   * If (TC AND APID == Elysium) Coding error
   * Else NL packet */
  
  /* TODO better constants */
  uint16_t apid = ( ((buffer[0] & 0x07) << 8) | (buffer[1]) );
  uint16_t elysium_apid = ( ((bank0p[RegSrcAddrMsb] << 8) | 
        (bank0p[RegSrcAddrLsb])) );
  bool tc = buffer[0] & 0x10;
  
  if (apid == elysium_apid) {
    if (!tc) {
      return true;
    }
    else {
      /* Fall through to NL to allow loopback */
    }
  }
  
  return false;
}
  
void elyNLFreeBufferChecked(uint8_t * buffer) {
  if (is_fw_buf(buffer)) {
      elyFWFreeBuffer(buffer);
  }
  else {
    elyNLFreeBuffer(buffer);
  }
}

void elyNLFreeBufferCheckedI(uint8_t * buffer) {
  if (is_fw_buf(buffer)) {
      elyFWFreeBufferI(buffer);
  }
  else {
    elyNLFreeBufferI(buffer);
  }
}

void elyNLInit(void) {
  uint8_t options = bank0p[RegNLOptions];
  if (options & 0x02) { /* Timestamp */
    if (options & 0x01) { /* P-field */
      /* 5-byte Secondary Header + 6-byte Primary Header */
      sh_len = 5;
    }
    else { /* No P-field */
      /* 4-byte Secondary Header + 6-byte Primary Header */
      sh_len = 4;
    }
  }
  
  sh_len = 0;
}

void elyNLSetHeader(uint8_t * buffer, uint16_t length, uint16_t dest_addr) {
  /* dest_addr not used for SPP */
  (void)(dest_addr);
  
  /* Set APID */
  uint16_t elysium_apid = ( ((bank0p[RegSrcAddrMsb] << 8) | 
        (bank0p[RegSrcAddrLsb])) );
  buffer[0] = (elysium_apid >> 8);
  buffer[1] = (elysium_apid & 0xFF);
  
  length = (length + sh_len - 1);
  buffer[4] = (length >> 8);
  buffer[5] = (length & 0xFF);
  
  uint8_t options = bank0p[RegNLOptions];
  
  if (options & 0x04) {
    chSysLock();
    buffer[2] = bank0p[RegNLPktNameMsb];
    buffer[3] = bank0p[RegNLPktNameLsb];
    chSysUnlock();
  }
  else {
    buffer[2] = (packets_sent >> 8) | 0xC0;
    buffer[3] = (packets_sent & 0xFF);
  }
  
  packets_sent = (packets_sent + 1) & 0x3FFF;
  
  /* insert timestamps here too if required */
  if (options & 0x02) { /* Timestamp */
    buffer[0] |= 0x08; /* Sec Hdr Flag */
    if (options & 0x01) { /* P-field */
      buffer[6] = SPP_PFIELD;
      chSysLock();
      buffer[7] = bank0p[RegMissionTimeMsb];
      buffer[8] = bank0p[RegMissionTimeHmb];
      buffer[9] = bank0p[RegMissionTimeLmb];
      buffer[10] = bank0p[RegMissionTimeLsb];
      chSysUnlock();
      
    }
    else { /* No P-field */
      /* 4-byte Secondary Header + 6-byte Primary Header */
      chSysLock();
      buffer[6] = bank0p[RegMissionTimeMsb];
      buffer[7] = bank0p[RegMissionTimeHmb];
      buffer[8] = bank0p[RegMissionTimeLmb];
      buffer[9] = bank0p[RegMissionTimeLsb];
      chSysUnlock();
    }
  }
  
  return;
}

size_t elyNLGetLength(const uint8_t * buffer) {
  return ((buffer[4] << 8) | (buffer[5])) + elyNLHeaderLen + 1;
}

size_t elyNLGetPayloadLength(const uint8_t * buffer) {
  return ((buffer[4] << 8) | (buffer[5])) - sh_len + 1;
}

uint8_t * elyNLToFW(uint8_t * buffer) {
  return buffer + sh_len + 6;
}

uint8_t * elyNLFromFW(uint8_t * buffer) {
  return buffer - sh_len - 6;
}
