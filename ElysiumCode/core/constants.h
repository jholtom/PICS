
#ifndef _ELYSIUM_CONSTANTS_H_
#define _ELYSIUM_CONSTANTS_H_

#include "nl_constants.h"
#include "dll_constants.h"

/* True constants */
#define UART_BAUD_MAX 1000000UL
#define UART_BAUD_MIN 1UL

#define TX_DEV_MIN 600UL
#define TX_DEV_MAX 200000UL

#define TX_BR_MIN 1200UL
#define TX_BR_MAX 300000UL

#define RX_BR_MIN 780UL
#define RX_BR_MAX 150000UL

#define TEMP_MIN -80
#define TEMP_MAX 125

#define RX_DEV_MIN 33000UL
#define RX_DEV_MAX 200000UL

/* Derived constants */
#define TX_BR_LSB (TX_BR & 0xFF)
#define TX_BR_LMB ((TX_BR >> 8) & 0xFF)
#define TX_BR_HMB ((TX_BR >> 16) & 0xFF)
#define TX_BR_MSB (TX_BR >> 24)

#define RX_BR_LSB (RX_BR & 0xFF)
#define RX_BR_LMB ((RX_BR >> 8) & 0xFF)
#define RX_BR_HMB ((RX_BR >> 16) & 0xFF)
#define RX_BR_MSB (RX_BR >> 24)

#define TX_DEV_LSB (TX_DEV & 0xFF)
#define TX_DEV_LMB ((TX_DEV >> 8) & 0xFF)
#define TX_DEV_HMB ((TX_DEV >> 16) & 0xFF)
#define TX_DEV_MSB (TX_DEV >> 24)

#define RX_DEV_LSB (RX_DEV & 0xFF)
#define RX_DEV_LMB ((RX_DEV >> 8) & 0xFF)
#define RX_DEV_HMB ((RX_DEV >> 16) & 0xFF)
#define RX_DEV_MSB (RX_DEV >> 24)

#define TX_SYNC_LSB (TX_SYNC & 0xFF)
#define TX_SYNC_LMB ((TX_SYNC >> 8) & 0xFF)
#define TX_SYNC_HMB ((TX_SYNC >> 16) & 0xFF)
#define TX_SYNC_MSB (TX_SYNC >> 24)

#define RX_SYNC_LSB (RX_SYNC & 0xFF)
#define RX_SYNC_LMB ((RX_SYNC >> 8) & 0xFF)
#define RX_SYNC_HMB ((RX_SYNC >> 16) & 0xFF)
#define RX_SYNC_MSB (RX_SYNC >> 24)

#define TX_FILTER_DEFAULT ((FILTER_DEFAULT >> 4) & 0x03)
#define RX_FILTER_DEFAULT (FILTER_DEFAULT & 0x0F)

#define RF_PARAMS_LSB (RF_PARAMS_DEFAULT & 0xFF)
#define RF_PARAMS_MSB (RF_PARAMS_DEFAULT >> 8)

#define UART_BAUD_LSB (UART_BAUD & 0xFF)
#define UART_BAUD_LMB ((UART_BAUD >> 8) & 0xFF)
#define UART_BAUD_HMB ((UART_BAUD >> 16) & 0xFF)
#define UART_BAUD_MSB (UART_BAUD >> 24)

#define TX_BR_MAX_LSB (TX_BR_MAX & 0xFF)
#define TX_BR_MAX_LMB ((TX_BR_MAX >> 8) & 0xFF)
#define TX_BR_MAX_HMB ((TX_BR_MAX >> 16) & 0xFF)
#define TX_BR_MAX_MSB (TX_BR_MAX >> 24)
#define TX_BR_MIN_LSB (TX_BR_MIN & 0xFF)
#define TX_BR_MIN_LMB ((TX_BR_MIN >> 8) & 0xFF)
#define TX_BR_MIN_HMB ((TX_BR_MIN >> 16) & 0xFF)
#define TX_BR_MIN_MSB (TX_BR_MIN >> 24)

#define RX_BR_MAX_LSB (RX_BR_MAX & 0xFF)
#define RX_BR_MAX_LMB ((RX_BR_MAX >> 8) & 0xFF)
#define RX_BR_MAX_HMB ((RX_BR_MAX >> 16) & 0xFF)
#define RX_BR_MAX_MSB (RX_BR_MAX >> 24)
#define RX_BR_MIN_LSB (RX_BR_MIN & 0xFF)
#define RX_BR_MIN_LMB ((RX_BR_MIN >> 8) & 0xFF)
#define RX_BR_MIN_HMB ((RX_BR_MIN >> 16) & 0xFF)
#define RX_BR_MIN_MSB (RX_BR_MIN >> 24)

#define TX_DEV_MAX_LSB (TX_DEV_MAX & 0xFF)
#define TX_DEV_MAX_LMB ((TX_DEV_MAX >> 8) & 0xFF)
#define TX_DEV_MAX_HMB ((TX_DEV_MAX >> 16) & 0xFF)
#define TX_DEV_MAX_MSB (TX_DEV_MAX >> 24)
#define TX_DEV_MIN_LSB (TX_DEV_MIN & 0xFF)
#define TX_DEV_MIN_LMB ((TX_DEV_MIN >> 8) & 0xFF)
#define TX_DEV_MIN_HMB ((TX_DEV_MIN >> 16) & 0xFF)
#define TX_DEV_MIN_MSB (TX_DEV_MIN >> 24)

#define RX_DEV_MAX_LSB (RX_DEV_MAX & 0xFF)
#define RX_DEV_MAX_LMB ((RX_DEV_MAX >> 8) & 0xFF)
#define RX_DEV_MAX_HMB ((RX_DEV_MAX >> 16) & 0xFF)
#define RX_DEV_MAX_MSB (RX_DEV_MAX >> 24)
#define RX_DEV_MIN_LSB (RX_DEV_MIN & 0xFF)
#define RX_DEV_MIN_LMB ((RX_DEV_MIN >> 8) & 0xFF)
#define RX_DEV_MIN_HMB ((RX_DEV_MIN >> 16) & 0xFF)
#define RX_DEV_MIN_MSB (RX_DEV_MIN >> 24)

#define TX_BAND_MAX_LSB (TX_BAND_MAX & 0xFF)
#define TX_BAND_MAX_LMB ((TX_BAND_MAX >> 8) & 0xFF)
#define TX_BAND_MAX_HMB ((TX_BAND_MAX >> 16) & 0xFF)
#define TX_BAND_MAX_MSB (TX_BAND_MAX >> 24)
#define TX_BAND_MIN_LSB (TX_BAND_MIN & 0xFF)
#define TX_BAND_MIN_LMB ((TX_BAND_MIN >> 8) & 0xFF)
#define TX_BAND_MIN_HMB ((TX_BAND_MIN >> 16) & 0xFF)
#define TX_BAND_MIN_MSB (TX_BAND_MIN >> 24)
#define TX_BAND_MID ((TX_BAND_MIN + TX_BAND_MAX) / 2)
#define TX_BAND_MID_LSB (TX_BAND_MID & 0xFF)
#define TX_BAND_MID_LMB ((TX_BAND_MID >> 8) & 0xFF)
#define TX_BAND_MID_HMB ((TX_BAND_MID >> 16) & 0xFF)
#define TX_BAND_MID_MSB (TX_BAND_MID >> 24)

#define RX_BAND_MIN_LSB (RX_BAND_MIN & 0xFF)
#define RX_BAND_MIN_LMB ((RX_BAND_MIN >> 8) & 0xFF)
#define RX_BAND_MIN_HMB ((RX_BAND_MIN >> 16) & 0xFF)
#define RX_BAND_MIN_MSB (RX_BAND_MIN >> 24)
#define RX_BAND_MAX_LSB (RX_BAND_MAX & 0xFF)
#define RX_BAND_MAX_LMB ((RX_BAND_MAX >> 8) & 0xFF)
#define RX_BAND_MAX_HMB ((RX_BAND_MAX >> 16) & 0xFF)
#define RX_BAND_MAX_MSB (RX_BAND_MAX >> 24)
#define RX_BAND_MID ((RX_BAND_MIN + RX_BAND_MAX) / 2)
#define RX_BAND_MID_LSB (RX_BAND_MID & 0xFF)
#define RX_BAND_MID_LMB ((RX_BAND_MID >> 8) & 0xFF)
#define RX_BAND_MID_HMB ((RX_BAND_MID >> 16) & 0xFF)
#define RX_BAND_MID_MSB (RX_BAND_MID >> 24)

#define UART_BAUD_MAX_LSB (UART_BAUD_MAX & 0xFF)
#define UART_BAUD_MAX_LMB ((UART_BAUD_MAX >> 8) & 0xFF)
#define UART_BAUD_MAX_HMB ((UART_BAUD_MAX >> 16) & 0xFF)
#define UART_BAUD_MAX_MSB (UART_BAUD_MAX >> 24)

#define ELY_SRCADDR_LSB (ELY_SRCADDR & 0xFF)
#define ELY_SRCADDR_MSB ((ELY_SRCADDR >> 8) & 0xFF)

#endif
