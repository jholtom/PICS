
#include "telem.h"

/* guaranteed to be zero */
static PERSIST telem_cfg_t cfg;

static __attribute__((section(".persistent"))) uint8_t write_index = 0;
static __attribute__((section(".persistent"))) uint8_t read_index = 0;

/* TODO persistence? */
static uint8_t init_index;

static fram_req_t * request;
static thread_t * telem_thd;

static msg_t filter_index(uint8_t index, const telem_cfg_t * const cfg) {
  
  /* Apply index-based filter to calculate next valid index */
  if (cfg->use_index) {
    if (cfg->index_start < cfg->index_end) {
      if (index < cfg->index_start || index > cfg->index_end) {
        index = cfg->index_start;
      }
    }
    else { /* start > end - wraparound */
      if (index > cfg->index_end && index < cfg->index_start) {
        index = cfg->index_start;
      }
    }
  }
  
  return index;
}

static void telem_fram_cb(uint8_t * buffer) {
  
  chSysLockFromISR();
  
  buffer = elyNLFromFW(buffer);
  
  /* Update read_index per filters */
  read_index = filter_index(read_index, &cfg);
  
  /* Send the buffer */
  /* TODO the hardcoded 0 here for dest_addr is fine for SPP, not for others */
  elyNLSetHeader(buffer, 256, 0);
  if (MSG_OK != elyRFPostI(buffer)) {
    chDbgAssert(false, "internal buffer overruns should be impossible");
  }
  
  chSysUnlockFromISR();
}

/* S-class due to lack of mutexes */
void elyTelemUpdateConfigS(telem_cfg_t config) {
  
  chDbgCheckClassS();
  
  /* Autogenerated shallow copy */
  cfg = config;
  
  /* Set up indexes per filters */
  init_index = filter_index(write_index, &cfg);
  read_index = init_index;
    
  /* Signal the thread */
  chEvtSignalI(telem_thd, TelemConfigUpdated);
}

void elyTelemPostBufferS(uint8_t * buff, telemcallback_t cb) {
  
  chDbgCheckClassS();
  
  buff[0] = write_index;
  buff[1] = bank0p[RegMissionTimeMsb];
  buff[2] = bank0p[RegMissionTimeHmb];
  buff[3] = bank0p[RegMissionTimeLmb];
  buff[4] = bank0p[RegMissionTimeLsb];

  /* Get an FRAM request block */
  /* TODO timeout for safety */
  elyFramGetRequestTimeoutS(&request, TIME_INFINITE);
  
  /* Calculate the FRAM address based on the current index */
  request->address = FRAM_TELEM_BASE + (256U * write_index);
  write_index++;
  /* Fill in the rest of the request block */
  request->read = 0;
  request->size = 256;
  request->buffer = buff;
  request->callback = cb;
  
  /* Post the request */
  elyFramPostRequestS(request);
}

THD_WORKING_AREA(waTelemThd, 256);
THD_FUNCTION(TelemThd, arg) {
  (void)(arg);
  
  /* Store a thread pointer for later use */
  telem_thd = chThdGetSelfX();
  
  /* Event loop */
  while (true) {
    eventmask_t evt = chEvtWaitAnyTimeout(TelemConfigUpdated, TIME_INFINITE);
    chDbgAssert(TelemConfigUpdated == evt, "Woke up without an event!");
    
    do {
      /* Get the firmware buffer */
      chSysLock();
      uint8_t * buff = elyFWGetBufferTimeoutS(TIME_INFINITE);
      
      /* Get an FRAM request block */
      elyFramGetRequestTimeoutS(&request, TIME_INFINITE);
      chSysUnlock();
      
      /* Calculate the FRAM address based on the current index */
      request->address = FRAM_TELEM_BASE + (256U * read_index);
      read_index++;
      /* Fill in the rest of the request block */
      request->read = 1;
      request->size = 256;
      request->buffer = buff;
      request->callback = telem_fram_cb;
      
      /* Post the request */
      chSysLock();
      elyFramPostRequestS(request);
      chSysUnlock();
    } while (read_index != init_index);
    
  }
  
}
