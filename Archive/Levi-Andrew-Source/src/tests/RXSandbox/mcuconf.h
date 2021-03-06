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

#ifndef MCUCONF_H
#define MCUCONF_H

/*
 * MSP430X drivers configuration.
 * The following settings override the default settings present in
 * the various device driver implementation headers.
 * Note that the settings for each driver only have effect if the driver
 * is enabled in halconf.h.
 * 
 */

#define MSP430X_MCUCONF

/* HAL driver system settings */
#define MSP430X_ACLK_SRC MSP430X_VLOCLK
#define MSP430X_LFXTCLK_FREQ 0
#define MSP430X_HFXTCLK_FREQ 0
#define MSP430X_DCOCLK_FREQ 16000000
#define MSP430X_MCLK_DIV 1
#define MSP430X_SMCLK_DIV 1

/*
 * SERIAL driver system settings.
 */
#define MSP430X_SERIAL_USE_USART0         FALSE
#define MSP430X_SERIAL_USE_USART1         FALSE
#define MSP430X_SERIAL_USE_USART2         FALSE
#define MSP430X_SERIAL_USE_USART3         FALSE

/*
 * ST driver system settings.
 */
#define MSP430X_ST_CLK_SRC MSP430X_SMCLK_SRC
#define MSP430X_ST_TIMER_TYPE B
#define MSP430X_ST_TIMER_INDEX 0

/*
 * SPI driver system settings.
 */
#define MSP430X_SPI_USE_SPIA0 TRUE
#define MSP430X_SPI_EXCLUSIVE_DMA FALSE

/*
 * SX1212 driver system settings.
 */
#define SX1212_IRQ_0 LINE_SX1212_IRQ_0
#define SX1212_IRQ_1 LINE_SX1212_IRQ_1
#define SX1212_PLL_LOCK LINE_SX1212_PLL_LOCK

/*
 * UART driver system settings
 */
#define MSP430X_UART_USE_UARTA1 TRUE
#define MSP430X_UART_EXCLUSIVE_DMA TRUE
#define MSP430X_UARTA1_CLK_SRC MSP430X_SMCLK_SRC

/*
 * GPT driver system settings
 */
#define MSP430X_GPT_USE_TA1 TRUE
#define MSP430X_TA1_CLK_SRC MSP430X_ACLK_SRC

/*
 * I2C driver system settings.
 */
#define MSP430X_I2CB0_CLK_SRC MSP430X_SMCLK_SRC
#define MSP430X_I2C_USE_I2CB0 TRUE
#define MSP430X_I2C_EXCLUSIVE_DMA FALSE
#define MSP430X_I2C_10BIT FALSE

#endif /* _MCUCONF_H_ */
