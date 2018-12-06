
#ifndef _ELYSIUM_CFG_H_
#define _ELYSIUM_CFG_H_

#define A 0
#define B 1

#define ELY_REVISION B

#define ELY_DISCRETE_PA_CTL TRUE

/* 1 Mbps - maximum rate for SX1212 */
#define SPI_BR 1000000UL

/* 115200 bps */
#define TX_BR 115200UL

/* 55555 bps */
#define RX_BR 55555UL

/* 28.8 kHz */
#define TX_DEV 28800UL

/* 80 kHz */
#define RX_DEV 80000UL

/* Configuration for sync words dependent on FEC type */

#define FEC_RS 1

#define FEC_TURBO 2

#define FEC_TYPE FEC_RS

#define TX_SYNC_RS 0x1ACFFC1DUL
#define TX_SYNC_TURBO 0x034776C7272895B0UL

#define RX_SYNC 0x8AD8BB2AUL

/* No Gaussian, 100 kHz */
#define FILTER_DEFAULT 0xC0

#define RF_PARAMS_DEFAULT 0x0001U

/* 115200 baud */
#define UART_BAUD 115200

/* 33 dBm */
#define TX_POW_MAX 130
#define TX_POW_MIN ((15.8 - FIXED_ATTEN)*5)

#define TX_BAND_MIN 865000000UL
#define TX_BAND_MAX 955000000UL

#define RX_BAND_MIN 457000000UL
#define RX_BAND_MAX 471000000UL

/* TODO: This will change on a new revision of the board */
#define FIXED_ATTEN 4

#define UART_TIMEOUT_MS 1000

#define MAIN_BUF_LEN 8

#include "constants.h"

#endif

