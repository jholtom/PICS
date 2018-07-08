/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

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
 * Board identifier.
 */
#define BOARD_FUZZ_SIM
#define BOARD_NAME "Elysium Radio fuzzer"

/*
 * IO lines assignments.
 */
#define LINE_SS_CONFIG_B                          PAL_LINE(IOPORT1, 0U)
#define LINE_SS_DATA_B                            PAL_LINE(IOPORT1, 1U)
#define LINE_SX1212_IRQ_0                         PAL_LINE(IOPORT1, 2U)
#define LINE_SX1212_IRQ_1                         PAL_LINE(IOPORT1, 3U)
#define LINE_SX1212_PLL_LOCK                      PAL_LINE(IOPORT1, 4U)
#define LINE_SX1278_SS_B                          PAL_LINE(IOPORT1, 5U)
#define LINE_SX1212_RESET                         PAL_LINE(IOPORT1, 6U)
#define LINE_SX1278_RESET_B                       PAL_LINE(IOPORT1, 7U)
#define LINE_SX1278_DIO0                          PAL_LINE(IOPORT1, 8U)
#define LINE_SX1278_DIO1                          PAL_LINE(IOPORT1, 9U)
#define LINE_SX1278_DIO2                          PAL_LINE(IOPORT1, 10U)
#define LINE_SX1278_DIO3                          PAL_LINE(IOPORT1, 11U)
#define LINE_SX1278_DIO4                          PAL_LINE(IOPORT1, 12U)
#define LINE_SX1278_DIO5                          PAL_NOLINE
#define LINE_PA_PC_EN                             PAL_LINE(IOPORT1, 13U)
#define LINE_PA_PGOOD                             PAL_LINE(IOPORT1, 14U)
#define LINE_GPO                                  PAL_LINE(IOPORT1, 15U)

/*
 * Random utilities
 */
#define BIT0 (1 << 0)
#define BIT1 (1 << 1)
#define BIT2 (1 << 2)
#define BIT3 (1 << 3)
#define BIT4 (1 << 4)
#define BIT5 (1 << 5)
#define BIT6 (1 << 6)
#define BIT7 (1 << 7)

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
