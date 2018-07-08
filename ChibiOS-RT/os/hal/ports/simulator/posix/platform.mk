# List of all the Win32 platform files.
PLATFORMSRC = ${CHIBIOS}/os/hal/ports/simulator/posix/hal_lld.c \
              ${CHIBIOS}/os/hal/ports/simulator/posix/fuzz/fuzz_core.c \
              ${CHIBIOS}/os/hal/ports/simulator/posix/hal_gpt_lld.c \
              ${CHIBIOS}/os/hal/ports/simulator/posix/hal_i2c_lld.c \
              ${CHIBIOS}/os/hal/ports/simulator/posix/hal_spi_lld.c \
              ${CHIBIOS}/os/hal/ports/simulator/posix/hal_serial_lld.c \
              ${CHIBIOS}/os/hal/ports/simulator/posix/hal_uart_lld.c \
              ${CHIBIOS}/os/hal/ports/simulator/console.c \
              ${CHIBIOS}/os/hal/ports/simulator/hal_pal_lld.c \
              ${CHIBIOS}/os/hal/ports/simulator/hal_st_lld.c

# Required include directories
PLATFORMINC = ${CHIBIOS}/os/hal/ports/simulator/posix \
              ${CHIBIOS}/os/hal/ports/simulator/posix/fuzz \
              ${CHIBIOS}/os/hal/ports/simulator
