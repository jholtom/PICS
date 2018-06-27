
#ifndef _ELYSIUM_CFG_H_
#define _ELYSIUM_CFG_H_

#define A 0
#define B 1

#define ELY_REVISION B

#define ELY_DISCRETE_PA_CTL TRUE

/* 1 Mbps - maximum rate for SX1212 */
#define SPI_BR 1000000UL

/* 4800 bps */
#define TX_BR 4800UL

/* 25000 bps */
#define RX_BR 25000UL

/* 5 kHz */
#define TX_DEV 5000UL

/* 50 kHz */
#define RX_DEV 50000UL

#define TX_SYNC 0x97E983EAUL

#define RX_SYNC 0x8AD8BB2AUL

/* No Gaussian, 100 kHz */
#define FILTER_DEFAULT 0xC0

#define RF_PARAMS_DEFAULT 0x0001U

/* 115200 baud */
#define UART_BAUD 115200

/* 33 dBm */
#define TX_POW_MAX 130
#define TX_POW_MIN ((15.8 - FIXED_ATTEN)*5)

/* TODO REV A CFG */
#define TX_BAND_MIN 865000000UL
#define TX_BAND_MAX 955000000UL

#define RX_BAND_MIN 457000000UL
#define RX_BAND_MAX 471000000UL

#define FIXED_ATTEN 4

#define UART_TIMEOUT_MS 1000

#define MAIN_BUF_LEN 8

#include "constants.h"

#endif

