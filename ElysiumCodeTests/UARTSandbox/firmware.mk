# List of all the firmware files to link into this test
FWSRC := ${PROJECT_ROOT}/core/uart.c \
				 ${PROJECT_ROOT}/core/mych.c \
				 ${PROJECT_ROOT}/core/main.c \
				 ${PROJECT_ROOT}/core/registers.c \
				 ${PROJECT_ROOT}/core/stubs/cmds_stub.c \
				 ${PROJECT_ROOT}/core/stubs/rf_stub.c \
				 ${PROJECT_ROOT}/core/stubs/errors_stub.c \
				 ${PROJECT_ROOT}/slip/slip_uart_dll.c \
				 ${PROJECT_ROOT}/spp/spp.c

FWASM := ${PROJECT_ROOT}/core/bootloader/bootloader.S

# Required include directories
FWINC := ${PROJECT_ROOT}/core/ \
				 ${PROJECT_ROOT}/slip/ \
				 ${PROJECT_ROOT}/spp/ \
				 ${PROJECT_ROOT}/crc/ \
				 ${PROJECT_ROOT}/ccsds/ 
	