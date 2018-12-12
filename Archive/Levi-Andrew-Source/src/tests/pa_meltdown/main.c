/*
    Copyright (C) 2016 Andrew Wygle aka awygle

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

#include "hal.h"
#include "ch.h"
#include <string.h>
#include <stdio.h>

#include "sx1278.h"
#include "hal_adc.h"

static ADCConfig adc_config = {
  255 /* dma_index */
};

static ADCConversionGroup adc_group = {
  false, /* circular */
  2, /* num_channels */
  NULL, /* end_cb */
  NULL, /* error_cb */
  {
    0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
  }, /* channels */
  MSP430X_ADC_RES_12BIT, /* resolution */
  MSP430X_ADC_SHT_32, /* rate */
  MSP430X_ADC_VSS_VREF_BUF, /* ref */
  MSP430X_REF_2V5 /* vref_src */
};

typedef int64_t fixed_q36_8;

fixed_q36_8 convert_ntc(ADCConversionGroup *grpp, adcsample_t sample) {
  const int32_t c3 = -10; /* Q0.31 */
  const int32_t c2 = 66515; /* Q0.31 */
  const int16_t c1 = -2841; /* Q0.15 */
  const int32_t c0 = 28750; /* Q7.8 */
  int64_t y;
  
  /* Get converted ADC value */
  adcMSP430XAdjustResult(grpp, sample);
  
  /* Convert value */
  /* NOTE: This is a lot of fixed point math. The regression line I'm using 
   * comes from LibreOffice, plotting Temperature vs. DN over the -40..+85 C
   * range. The equation is -4.6411...x^3+3.0974...x^2-0.0867...x+112.3073
   * We solve it using Horner's rule in fixed point math as follows */
  y = c3 * sample; /* Q0.31 * Q12.0 => Q12.31 */
  y += c2; /* Q12.31 + Q0.31 => OK */
  y *= sample; /* Q12.31 * Q12.0 => Q24.31 */
  y = (y >> 16); /* Q24.31 >> 16 => Q24.15 */
  y += c1; /* Q24.15 + Q0.15 => OK */
  y *= sample; /* Q24.15 * Q12.0 => Q36.15 */
  y = (y >> 7); /* Q36.15 >> 7 => Q36.8 */
  y += c0; /* Q36.8 + Q7.8 => OK */
  
  /* Now y is our result in Q36.8 format. We return it using a typedef to
   * make that clear. */
  return y;
}

static fixed_q36_8 temps[2];
static adcsample_t samples[2];
char out_string[128];
const char * fmt_string = "%s NTC Temp: %d C\r\n";
const char * pow_string = "PA powered.\r\n";
const char * unpow_string = "PA unpowered.\r\n";
const char * err_string = "\r\nERROR! PA in unexpected state - likely PGOOD event\r\n";
const char * therm_string = "\r\nERROR! PA overheating - powering off now\r\n";

static void save_bacon(void) {
  palClearLine(LINE_PA_PC_EN);
}

/*
 * Thread 2.
 */
THD_WORKING_AREA(waThread2, 2048);
THD_FUNCTION(Thread2, arg) {

  (void)arg;

  /*
   * Activate the serial driver 0 using the driver default configuration.
   */
  sdStart(&SD1, NULL);
  
  /* Activate the ADC driver 1 using its config */
  adcStart(&ADCD1, &adc_config);

  /*chnWrite(&SD1, (const uint8_t *)start_msg, strlen(start_msg));*/
  
  uint8_t in;
  bool powered = false;
  while (true) {
    
    in = sdGetTimeout(&SD1, TIME_IMMEDIATE);
    
    if (in == 'p') {
      if (powered) {
        /* Disable "save my bacon" PGOOD falling edge interrupt */
        osalSysLock();
        palLineDisableEventI(LINE_PA_PGOOD);
        osalSysUnlock();
        /* Power off */
        palClearLine(LINE_PA_PC_EN);
        powered = false;
        chnWrite(&SD1, (const uint8_t *)unpow_string, strlen(unpow_string));
      }
      else {
        /* Set up "save my bacon" PGOOD falling edge interrupt */
        osalSysLock();
        palLineEnableEventI(LINE_PA_PGOOD, PAL_EVENT_MODE_FALLING_EDGE, save_bacon);
        osalSysUnlock();
        /* Power on */
        palSetLine(LINE_PA_PC_EN);
        powered = true;
        chnWrite(&SD1, (const uint8_t *)pow_string, strlen(pow_string));
      }
    }
      
    /* Read the NTC thermistor value */
    /*chnWrite(&SD1, (const uint8_t *)test_1_msg, strlen(test_1_msg));*/
    adcStartConversion(&ADCD1, &adc_group, samples, 1);
    while (ADCD1.state == ADC_ACTIVE) ;
    
    for (int i = 0; i < 2; i++) {
      temps[i] = convert_ntc(&adc_group, samples[i]);
      if ((temps[i] >> 8) > 60) {
        /* Disable "save my bacon" PGOOD falling edge interrupt */
        osalSysLock();
        palLineDisableEventI(LINE_PA_PGOOD);
        osalSysUnlock();
        /* Power off */
        palClearLine(LINE_PA_PC_EN);
        powered = false;
        chnWrite(&SD1, (const uint8_t *)therm_string, strlen(therm_string));
      }
    }
    
    sniprintf(out_string, 128, fmt_string, "Bottom", temps[0] >> 8);
    chnWrite(&SD1, (const uint8_t *)out_string, strlen(out_string));
    sniprintf(out_string, 128, fmt_string, "Top", temps[1] >> 8);
    chnWrite(&SD1, (const uint8_t *)out_string, strlen(out_string));
    
    if (powered != (palReadLine(LINE_PA_PC_EN) == PAL_HIGH)) {
      chnWrite(&SD1, (const uint8_t *)err_string, strlen(err_string));
      powered = (palReadLine(LINE_PA_PC_EN) == PAL_HIGH);
    }
    
    chThdSleepMilliseconds(500);
  }
}

/*
 * Threads static table, one entry per thread. The number of entries must
 * match NIL_CFG_NUM_THREADS.
 */
THD_TABLE_BEGIN
  THD_TABLE_ENTRY(waThread2, "hello", Thread2, NULL)
THD_TABLE_END

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  WDTCTL = WDTPW | WDTHOLD;
  

  halInit();
  chSysInit();
  dmaInit();
  
  /* This is now the idle thread loop, you may perform here a low priority
     task but you must never try to sleep or wait in this loop. Note that
     this tasks runs at the lowest priority level so any instruction added
     here will be executed after all other tasks have been started.*/
  while (true) {
  }
}
