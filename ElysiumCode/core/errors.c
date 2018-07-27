
#include "errors.h"
#include "registers.h"

/* TODO this needs to be fixed... badly */
event_mask_t PERSIST logged_events = 0;
event_mask_t PERSIST reported_events = 0;

/* TODO these should come from register constants, when we have them */
/* ENH same as registers - this can be sparse if we use the linker */
elysium_err_lvl_t error_priorities[64] = {
  WARNING | ERROR, /* PAOvertemp - handled specially */
  WARNING | ERROR, /* HSOvertemp - handled specially */
  WARNING | ERROR, /* MCUOvertemp - handled specially */
  ERROR, /* GroundCommFault */
  ERROR, /* SCCommFault */
  CRITICAL, /* LNAOvercurrent - fixed */
  CRITICAL, /* PAOvercurrent - fixed */
  CRITICAL, /* MCUUndervolt - fixed */
  WARNING, /* RegClip */
  WARNING, /* InvalidOpcode */
  WARNING, /* InvalidLength */
  WARNING, /* FCS Error */
  WARNING, /* CmdFailure */
  INFO, /* SubOverwrite */
  ERROR, /* Reset */
  WARNING, /* UART error */
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  NL_ERR_DEFAULTS,
  DLL_ERR_DEFAULTS
};

static thread_t * error_thd;

eventmask_t mask_from_error(uint8_t error) {
  return (1 << (error & 0x3F));
}

void elyErrorSignal(uint8_t error) {
  chEvtSignal(error_thd, mask_from_error(error));
}

void elyErrorSignalI(uint8_t error) {
  chDbgCheckClassI();
  chEvtSignalI(error_thd, mask_from_error(error));
}

void elyErrorSetRptLvlS(uint8_t lvl) {
  chDbgCheckClassS();
  chDbgAssert(lvl < ELY_ALL_ERRORS, "invalid reporting mask");

  logged_events = 0;
  reported_events = 0;

  /* update the mask of enabled events */
  for (int i = 0x80; i < ErrCoreMAX; i++) {
    if (error_priorities[i] & bank0p[RegErrRptLvl]) {
      reported_events |= mask_from_error(i);
    }
  }
  for (int i = 0xA0; i < ErrNLMAX; i++) {
    if (error_priorities[i] & bank0p[RegErrRptLvl]) {
      reported_events |= mask_from_error(i);
    }
  }
  for (int i = 0xB0; i < ErrNLMAX; i++) {
    if (error_priorities[i] & bank0p[RegErrRptLvl]) {
      reported_events |= mask_from_error(i);
    }
  }

  /* Awaken the thread */
  chSchReadyI(error_thd, MSG_RESET);
}

void elyErrorSetLogLvlS(uint8_t lvl) {
  chDbgCheckClassS();
  chDbgAssert(lvl < ELY_ALL_ERRORS, "invalid reporting mask");

  /* update the mask of enabled events */
  for (int i = 0x80; i < ErrCoreMAX; i++) {
    if (error_priorities[i] & bank0p[RegErrLogLvl]) {
      logged_events |= mask_from_error(i);
    }
  }
  for (int i = 0xA0; i < ErrNLMAX; i++) {
    if (error_priorities[i] & bank0p[RegErrLogLvl]) {
      logged_events |= mask_from_error(i);
    }
  }
  for (int i = 0xB0; i < ErrNLMAX; i++) {
    if (error_priorities[i] & bank0p[RegErrLogLvl]) {
      logged_events |= mask_from_error(i);
    }
  }


  /* Awaken the thread */
  chSchReadyI(error_thd, MSG_RESET);
}

static elysium_errs_t get_max_error(event_mask_t mask) {
  chDbgAssert(mask != 0, "shouldn't be called with nothing set");
  /* TODO faster, more magic algorithms exist */
  for (int i = 0; i < 64; i++) {
    if ((mask >> i) == 0) {
      return (i-1);
    }
  }
  return 63;
}

THD_WORKING_AREA(waEvtThd, 128);
THD_FUNCTION(EvtThd, arg) {
  (void)arg;
  msg_t msg;
  event_mask_t result;
  elysium_errs_t err;
  elysium_err_lvl_t lvl;

  error_thd = chThdGetSelfX();

  while (true) {
    result = chEvtWaitAnyTimeout(logged_events | reported_events, TIME_INFINITE);
    while (result > 0) {
      /* Get the highest index error */
      err = get_max_error(result);

      /* Get the error priority level */
      switch (err) {
        case ErrPAOvertemp:
          /* TODO special case */
        case ErrHSOvertemp:
          /* TODO special case */
        case ErrMCUOvertemp:
          /* TODO special case */
        case ErrGroundCommFault:
          lvl = bank0p[RegGFLvl];
          break;
        case ErrSCCommFault:
          lvl = bank0p[RegSCCommLvl];
          break;
        case ErrLNAOvercurrent:
          lvl = CRITICAL;
          break;
        case ErrPAOvercurrent:
          lvl = CRITICAL;
          break;
        case ErrMCUUndervolt:
          lvl = CRITICAL;
          break;
        case ErrRegClip:
          lvl = bank0p[RegRegErrLvl];
          break;
        case ErrInvalidOpcode:
          lvl = bank0p[RegOpErrLvl];
          break;
        case ErrInvalidLength:
          lvl = bank0p[RegLengthErrLvl];
          break;
        case ErrFCSError:
          lvl = bank0p[RegFCSLvl];
          break;
        case ErrCmdFailure:
          lvl = bank0p[RegNRErrLvl];
          break;
        case ErrSubOverwrite:
          lvl = bank0p[RegSubOverLvl];
          break;
        case ErrReset:
          lvl = bank0p[RegResetErrLvl];
          break;
        case ErrUARTError:
          lvl = bank0p[RegUARTErrLvl];
          break;
        default:
          chDbgAssert(false, "shouldn't happen");
      }

      /* Test the level against the mask */
      if (bank0p[RegErrRptLvl] & lvl) {
        /* TODO report the error */
      }
      if (bank0p[RegErrLogLvl] & lvl) {
        /* TODO log the error */
      }

      /* Remove the error from the mask */
      result &= ~(1 << err);

    }
    /* If the loop never runs it means the masks have been updated */
  }
}
