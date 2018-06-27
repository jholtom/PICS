
.section ".bootloader", "a"
__msp430_resetvec_hook:
.word _bootloader_start

.section ".info", "a"
.global __bootloader_start
_bootloader_start:
  mov.w &bootloader, r4 ; read bootloader flag
  cmp #1, r4 ; is r4 one?
  jz _main ; if so, continue
  call #__crt0_start ; we continue by jumping to the TI crt0 - this can't move
_main:
  ; now we're in the bootloader proper

  ; disable the watchdog
  mov.w #0x5A84, &WDTCTL

  ; initialize the clocks
  mov.w #0xA500, &CSCTL0
  mov.w #0x0006, &CSCTL1
  mov.w #0x0033, &CSCTL2
  mov.w #0x0033, &CSCTL3
  mov.b #0xFF, &CSCTL0_H

  ; dma
  mov.w #0x0012, &DMACTL0
  mov.w #0xFBFF, &DMA0SZ
  mov.w #0x4400, &DMA0DA
  mov.w #0x064C, &DMA0SA
  mov.w #0x0CD0, &DMA0CTL

  ; initialize the I2C module
  ; master mode, i2c mode, smclk (8 MHz DCO), transmit, start
  mov.w #0x0F93, &UCB0CTLW0
  ; defaults are fine for UCB0CTLW1
  ; BR of 20 gives 400 kHz
  mov.w #10, &UCB0BRW
  ; STATW is read-only
  ; not using TBCNT
  ; 7-bit slave address for upper half
  mov.w #0x51, &UCB0I2CSA

  ; initialize the pins
  mov.b #0xD0, &PAOUT_L
  mov.b #0x00, &PADIR_L
  mov.b #0xD8, &PAREN_L
  mov.b #0x27, &PASEL0_L
  mov.b #0xC7, &PASEL1_L
  bic #0x0001, &PM5CTL0

  ; enable i2c
  bic #0x0001, &UCB0CTLW0
  ; wait until TXIFG triggers
  addr_loop_1:
    bit.w #0x0002, &UCB0IFG
    jz addr_loop_1
  ; transmit base address
  mov.b #0x04, &UCB0TXBUF
  ; wait until TXIFG triggers
  addr_loop_2:
    bit.w #0x0002, &UCB0IFG
    jz addr_loop_2
  mov.b #0x00, &UCB0TXBUF
  ; wait until TXIFG triggers
  addr_loop_3:
    bit.w #0x0002, &UCB0IFG
    jz addr_loop_3
  mov.b #0x00, &UCB0TXBUF

  ; restart as receiver
  mov.w #0X0F82, &UCB0CTLW0
  ; initialize address register
  ;mov.a #0x4400, r4 
  ;main_loop:
  ;  rx_loop:
  ;    ; wait until RXIFG triggers
  ;    bit.w #0x0001, &UCB0IFG
  ;    jz rx_loop
  ;  ; move value from RXBUF to appropriate memory slot
  ;  mov.b &UCB0RXBUF_L, 0(r4)
  ;  incx.a r4 ; next address
  ;  cmpx.a #0x13FFF, r4 ; are we done?
  ;  jne main_loop ; if not, do it again

  dma_loop:
    ; wait until DMAIFG triggers
    bit.w #0x0008, &DMA0CTL
    jz dma_loop

  ; nack/stop
  bis #0x0004, &UCB0CTLW0

  ; last byte
  mov.b &UCB0RXBUF_L, 0(r4)

  stop_loop:
    bit.w #0x004, &UCB0CTLW0
    jnz stop_loop

  ; done programming - clear the bootloader flag
  mov.b #0, &bootloader
  ; reset
  mov.w #0xA548, &PMMCTL0
