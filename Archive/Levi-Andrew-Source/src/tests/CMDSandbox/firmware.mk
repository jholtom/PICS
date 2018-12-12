# List of all the firmware files to link into this test
FWSRC := ${PROJECT_ROOT}/core/uart.c \
				 ${PROJECT_ROOT}/core/main.c \
				 ${PROJECT_ROOT}/core/mych.c \
				 ${PROJECT_ROOT}/core/cmds.c \
				 ${PROJECT_ROOT}/core/registers.c \
				 ${PROJECT_ROOT}/slip/slip_uart_dll.c \
				 ${PROJECT_ROOT}/core/stubs/fram_stub.c \
				 ${PROJECT_ROOT}/core/stubs/channels_stub.c \
				 ${PROJECT_ROOT}/core/stubs/errors_stub.c \
				 ${PROJECT_ROOT}/core/stubs/events_stub.c \
				 ${PROJECT_ROOT}/core/stubs/rf_stub.c \
				 ${PROJECT_ROOT}/core/stubs/rf_dll_stub.c \
				 ${PROJECT_ROOT}/core/stubs/telem_stub.c \
				 ${PROJECT_ROOT}/core/queues.c \
				 ${PROJECT_ROOT}/crc/crc_x25.c \
				 ${PROJECT_ROOT}/spp/spp.c

FWASM := ${PROJECT_ROOT}/core/bootloader/bootloader.S

# Required include directories
FWINC := ${PROJECT_ROOT}/core/ \
				 ${PROJECT_ROOT}/slip/ \
				 ${PROJECT_ROOT}/spp/ \
				 ${PROJECT_ROOT}/ccsds/ \
				 ${PROJECT_ROOT}/crc/
	
