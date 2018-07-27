#include <msp430.h>
#include <stdint.h>

extern volatile uint8_t bootloader;

__attribute__((optimize(0))) __attribute__((section(".info"))) void _bootloader_func(void) {
  if (bootloader == 0) {
    __crt0_start();
  }
  /* pins */
  P1OUT = 0xD0;
  P1DIR = 0x00;
  P1REN = 0xD8;
  P1SEL0 = 0x27;
  P1SEL1 = 0xC7;
  PM5CTL0 &= ~LOCKLPM5;

  UCB0BRW = 20;
  UCB0I2CSA = 0x51;
  UCB0CTLW0 = 0xF92;
  UCB0TXBUF = 0x04;
  while (!(UCB0IFG & UCTXIFG0)) ;
  UCB0TXBUF = 0x00;
  while (!(UCB0IFG & UCTXIFG0)) ;
  UCB0CTLW0 = 0x0F82;
  uint32_t addr = 0x4400;
  while (addr < 0x14000) {
    while (!(UCB0IFG & UCRXIFG0)) ;
    *((uint8_t *)(addr)) = UCB0RXBUF;
    addr++;
    WDTCTL = WDTPW | WDTCNTCL;
  }
  bootloader = 0;

  PMMCTL0 = PMMPW | SVSHE | PMMSWPOR; /* SW POR */
}

__attribute__((section(".bootloader"))) uint16_t _bootloader_start = &_bootloader_func;

