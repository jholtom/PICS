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
    0, 2, 0, 0, 0, 0, 0, 0,
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
const char * unpow_string = "\r\nPGOOD ERROR - PA unpowered.\r\n";
const char * err_string = "\r\nERROR! A timeout or reset occurred - packet not sent";
const char * therm_string = "\r\nWARNING! PA overheating!\r\n";
const char * sending_string = "\r\nSending packet...\r\n";
const char * sent_string = "\r\nPacket sent!\r\n";

static void save_bacon(void) {
  palClearLine(LINE_PA_PC_EN);
  
  chnWrite(&SD0, (const uint8_t *)unpow_string, strlen(unpow_string));

}

static SPIConfig SPIDA1_config = {
  NULL, /* callback */
  LINE_SX1278_SS_B, /* slave select line */
  25000, /* data rate */
  MSP430X_SPI_BO_MSB, /* bit order */
  MSP430X_SPI_DS_EIGHT, /* data size */
  0 /* SPI mode */
};

static sx1278_packet_config_t packet_config = {
  Fixed, /* format */
  0, /* whitening */
  0, /* manchester */
  0, /* CRCs */
  0, /* preamble polarity */
  0, /* addressing */
  0, /* length byte (ignored) */
};

static SX1278Config config = {
  &SPIDA1,
  &SPIDA1_config,
  9600, /* bitrate */
  434e6, /* frequency */
  4800, /* frequency deviation - m = 1 */
  -4.2, /* power output */
  350, /* preamble length */
  0x5EA6C11D, /* sync word */
  {
    0,
    1
  }, /* IRQs */
  {
    LINE_SX1278_DIO0,
    LINE_SX1278_DIO1,
    LINE_SX1278_DIO2,
    LINE_SX1278_DIO3,
    LINE_SX1278_DIO4,
    LINE_SX1278_DIO5,
  }, /* DIO mapping */
  LINE_SX1278_RESET_B, /* reset line */
  &packet_config, 
  0,
  2047,
  Sleep
};

static SX1278Driver SX1278D1;
static uint8_t packet[2047] = { 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1, 0X16, 0xF1, 0X95, 0X09, 0XBB, 0X09, 0XFE, 0X04, 0X5D, 0X7E, 0XBF, 0X9C, 0XF4, 0X61, 0XEB, 0X8C, 0X43, 0X38, 0X1A, 0X60, 0X29, 0XEE, 0XEA, 0X47, 0X9C, 0X01, 0X17, 0X48, 0X2E, 0X8B, 0XDD, 0X1C, 0XCE, 0X9E, 0XBB, 0XB6, 0X3A, 0X3A, 0XF5, 0XD7, 0X2E, 0X88, 0XA9, 0XC4, 0XD2, 0XA8, 0XC4, 0XAA, 0XEC, 0XD2, 0X20, 0X29, 0X85, 0XAF, 0X3C, 0X4C, 0XB4, 0X2D, 0XD1, 0X25, 0XC8, 0XC6, 0XA9, 0XE1 };
static msg_t err_val = MSG_OK;

/*
 * Thread 2.
 */
THD_WORKING_AREA(waThread2, 2048);
THD_FUNCTION(Thread2, arg) {

  (void)arg;

  /*
   * Activate the serial driver 0 using the driver default configuration.
   */
  sdStart(&SD0, NULL);
  
  /* Activate the ADC driver 1 using its config */
  adcStart(&ADCD1, &adc_config);

  /* Initialize the SX1212 driver */
  sx1278ObjectInit(&SX1278D1);
  
  /* Start the SX1212 driver */
  sx1278Start(&SX1278D1, &config);
  
  /*chnWrite(&SD0, (const uint8_t *)start_msg, strlen(start_msg));*/
  uint8_t in;
  while (true) {
    
    in = sdGetTimeout(&SD0, TIME_IMMEDIATE);
    
    if (in == 'p') {
      chnWrite(&SD0, (const uint8_t *)sending_string, strlen(sending_string));
      
      /* Set up "save my bacon" PGOOD falling edge interrupt */
      osalSysLock();
      palLineEnableEventI(LINE_PA_PGOOD, PAL_EVENT_MODE_FALLING_EDGE, save_bacon);
      osalSysUnlock();
      /* Power on */
      palSetLine(LINE_PA_PC_EN);
      /* Wait for PGOOD to rise */
      chThdSleepMilliseconds(5);
      while ( !palReadLine(LINE_PA_PGOOD) ) ;
      /* Transmit */
      err_val = sx1278Send(&SX1278D1, packet, sizeof(packet), TIME_IMMEDIATE);
      /* Disable "save my bacon" PGOOD falling edge interrupt */
      osalSysLock();
      palLineDisableEventI(LINE_PA_PGOOD);
      osalSysUnlock();
      /* Power off */
      palClearLine(LINE_PA_PC_EN);

      if (err_val == MSG_OK) {
        chnWrite(&SD0, (const uint8_t *)sent_string, strlen(sent_string));
      }
      else {
        chnWrite(&SD0, (const uint8_t *)err_string, strlen(err_string));
      }
      
    }
      
    /* Read the NTC thermistor value */
    /*chnWrite(&SD0, (const uint8_t *)test_1_msg, strlen(test_1_msg));*/
    adcStartConversion(&ADCD1, &adc_group, samples, 1);
    while (ADCD1.state == ADC_ACTIVE) ;
    
    for (int i = 0; i < 2; i++) {
      temps[i] = convert_ntc(&adc_group, samples[i]);
      if ((temps[i] >> 8) > 60) {
        /* Report alarming result */
        chnWrite(&SD0, (const uint8_t *)therm_string, strlen(therm_string));
      }
    }
    
    sniprintf(out_string, 128, fmt_string, "Bottom", temps[0] >> 8);
    chnWrite(&SD0, (const uint8_t *)out_string, strlen(out_string));
    sniprintf(out_string, 128, fmt_string, "Top", temps[1] >> 8);
    chnWrite(&SD0, (const uint8_t *)out_string, strlen(out_string));
    
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
