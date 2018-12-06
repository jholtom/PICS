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

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * Setup for the EXP430FR5969 LaunchPad board
 */

/*
 * Board identifier.
 */
#define BOARD_ELYSIUM
#define BOARD_NAME "Elysium Radio"

/*
 * IO lines assignments.
 */
#define LINE_SS_CONFIG_B                          PAL_LINE(IOPORT0, 2U)
#define LINE_SS_DATA_B                            PAL_LINE(IOPORT0, 3U)
#define LINE_SX1212_IRQ_0                         PAL_LINE(IOPORT1, 3U)
#define LINE_SX1212_IRQ_1                         PAL_LINE(IOPORT2, 15U)
#define LINE_SX1212_PLL_LOCK                      PAL_LINE(IOPORT2, 11U)
#define LINE_SX1278_SS_B                          PAL_LINE(IOPORT2, 3U)
#define LINE_SX1212_RESET                         PAL_LINE(IOPORT2, 8U)
#define LINE_SX1278_RESET_B                       PAL_LINE(IOPORT2, 6U)
#define LINE_SX1278_DIO0                          PAL_LINE(IOPORT2, 7U)
#define LINE_SX1278_DIO1                          PAL_LINE(IOPORT2, 12U)
#define LINE_SX1278_DIO2                          PAL_LINE(IOPORT2, 13U)
#define LINE_SX1278_DIO3                          PAL_LINE(IOPORT2, 14U)
#define LINE_SX1278_DIO4                          PAL_LINE(IOPORT1, 15U)
#define LINE_SX1278_DIO5                          PAL_LINE(IOPORT1, 11U)
#define LINE_PA_PC_EN                             PAL_LINE(IOPORT2, 4U)
#define LINE_PA_PGOOD                             PAL_LINE(IOPORT1, 10U)

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the MSP430X Family Users Guide for details.
 */
/*
 * Port A setup:
 * 
 * P1.0 - PA thermistor, bottom   (alternate 3, analog input)
 * P1.1 - LNA voltage sense       (alternate 3, analog input)
 * P1.2 - PA thermistor, top      (alternate 3, analog input)
 * P1.3 - SX1212 IRQ_0            (input pulldown rising edge interrupt)
 * P1.4 - SX1212 DATA (unused)    (input pullup)
 * P1.5 - SX1212 CLKOUT           (alternate 1, timer input)
 * P1.6 - FRAM SDA                (alternate 2)
 * P1.7 - FRAM SCL                (alternate 2)
 * P2.0 - Adamant UART TX         (alternate 2)
 * P2.1 - Adamant UART RX         (alternate 2)
 * P2.2 - PA PGOOD                (input pulldown falling edge interrupt)
 * P2.3 - SX1278 DIO5             (input pulldown rising edge interrupt) TODO sometimes timer input
 * P2.4 - Transceiver SCK         (alternate 2)
 * P2.5 - Transceiver MOSI        (alternate 2)
 * P2.6 - Transceiver MISO        (alternate 2)
 * P2.7 - SX1278 DIO4             (input pulldown rising edge interrupt)
 */
#define VAL_IOPORT1_OUT   0x0010
#define VAL_IOPORT1_DIR   0x3000
#define VAL_IOPORT1_REN   0x8C18
#define VAL_IOPORT1_SEL0  0x0027
#define VAL_IOPORT1_SEL1  0x73C7

/*
 * Port B setup:
 * 
 * P3.0 - PA power sense          (alternate 3, analog input)
 * P3.1 - PA power reference      (alternate 3, analog input)
 * P3.2 - SX1278 TX indicator     (input pulldown falling edge interrupt)
 * P3.3 - SX1278 SS_B             (output high)
 * P3.4 - PA power enable         (output low)
 * P3.5 - LNA power enable        (output low)
 * P3.6 - SX1278 RESET_B          (input pullup) TODO when writing SX1278 driver, make this better
 * P3.7 - SX1278 DIO0             (input pulldown rising edge interrupt)
 * P4.0 - SX1212 RESET            (input pulldown) TODO when writing SX1212 driver, make this better
 * P4.1 - 2.5V voltage sense      (alternate 3, analog input)
 * P4.2 - N/C                     (input pullup)
 * P4.3 - SX1212 PLL Lock         (input pulldown rising edge interrupt)
 * P4.4 - SX1278 DIO1             (input pulldown rising edge interrupt) TODO maybe sometimes timer input?
 * P4.5 - SX1278 DIO2             (input pulldown rising edge interrupt)
 * P4.6 - SX1278 DIO3             (input pulldown rising edge interrupt)
 * P4.7 - SX1212 IRQ_1            (input pulldown rising edge interrupt)
 */
#define VAL_IOPORT2_OUT   0x0448
#define VAL_IOPORT2_DIR   0x0038
#define VAL_IOPORT2_REN   0xFE84
#define VAL_IOPORT2_SEL0  0x0203
#define VAL_IOPORT2_SEL1  0x0203

/*
 * Port J setup:
 * 
 * PJ.0 - N/C                     (input pullup)
 * PJ.1 - N/C                     (input pullup)
 * PJ.2 - SX1212 Config SS_B      (output high)
 * PJ.3 - SX1212 Data SS_B        (output high)
 * PJ.4 - LFXIN                   (alternate 1)
 * PJ.5 - LFXOUT                  (alternate 1)
 * PJ.6 - HFXIN                   (alternate 1)
 * PJ.7 - HFXOUT                  (alternate 1)
 */
#define VAL_IOPORT0_OUT   0x000F
#define VAL_IOPORT0_DIR   0x000C
#define VAL_IOPORT0_REN   0x0003
#define VAL_IOPORT0_SEL0  0x00F0
#define VAL_IOPORT0_SEL1  0x0000
#define VAL_IOPORT0_IES   0x0000
#define VAL_IOPORT0_IE    0x0000

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */
