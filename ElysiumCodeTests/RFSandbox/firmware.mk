# List of all the firmware files to link into this test
FWSRC := ${PROJECT_ROOT}/core/uart.c \
				 ${PROJECT_ROOT}/core/main.c \
				 ${PROJECT_ROOT}/core/mych.c \
				 ${PROJECT_ROOT}/core/cmds.c \
				 ${PROJECT_ROOT}/core/registers.c \
				 ${PROJECT_ROOT}/slip/slip_uart_dll.c \
				 ${PROJECT_ROOT}/crc/crc_x25.c \
				 ${PROJECT_ROOT}/ccsds/crc_sdlp.c \
				 ${PROJECT_ROOT}/ccsds/sdlp.c \
				 ${PROJECT_ROOT}/core/rf.c \
				 ${PROJECT_ROOT}/spp/spp.c

# Required include directories
FWINC := ${PROJECT_ROOT}/core/ \
				 ${PROJECT_ROOT}/slip/ \
				 ${PROJECT_ROOT}/spp/ \
				 ${PROJECT_ROOT}/ccsds/ \
				 ${PROJECT_ROOT}/crc/
	
