/*
  ChibiOS - Copyright (C) 2016 Andrew Wygle aka awygle
  ChibiOS - Copyright (C) 2018 Reed Koser aka bob_twinkles

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
 * POSIX simulator driver configuratino
 */

#define POSIX_MCUCONF

/*
 * SERIAL driver system settings.
 */
#define USE_POSIX_SERIAL1          FALSE
#define USE_POSIX_SERIAL2          FALSE

/*
 * SPI driver system settings.
 */
#define POSIX_SPI_USE_SPI1         TRUE

/*
 * SX1212 driver system settings.
 */
#define SX1212_IRQ_0   LINE_SX1212_IRQ_0
#define SX1212_IRQ_1   LINE_SX1212_IRQ_1
#define SX1212_PLL_LOCK LINE_SX1212_PLL_LOCK

/*
 * UART driver system settings
 */
#define POSIX_UART_USE_UART1        TRUE

/*
 * GPT driver system settings
 */
#define POSIX_GPT_USE_GPT1          TRUE

/*
 * I2C driver system settings.
 */
#define POSIX_I2C_USE_I2C1          TRUE

#endif
