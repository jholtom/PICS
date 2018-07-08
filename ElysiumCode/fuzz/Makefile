.PHONY: all directories
##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT = $(XOPT)
endif
USE_OPT += -m32 -DCH_POSIX

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT = 
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -fno-rtti
endif

# Enable this if you want the linker to remove unused code and data.
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# Linker extra options here.
ifeq ($(USE_LDOPT),)
  USE_LDOPT = 
endif

# Enable this if you want link time optimizations (LTO)
ifeq ($(USE_LTO),)
  USE_LTO = no
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

# If enabled, this option makes the build process faster by not compiling
# modules not used in the current configuration.
ifeq ($(USE_SMART_BUILD),)
  USE_SMART_BUILD = no
endif

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

USE_COPT += -g -O1
USE_COPT += $(addprefix -I ,$(IDIRS))
# USE_COPT += -D__MSP430FR5969__
USE_COPT += -D__FUZZ__

USE_OPT += -Wimplicit-fallthrough=1

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, sources and paths
#

# Define project name here
PROJECT = fuzz
ELYSIUM_ROOT = ..
FUZZ_ROOT = .

# Imported source files and paths
CHIBIOS = ../../ChibiOS-RT
CHIBIOS_CONTRIB = ../../ChibiOS-Contrib
# Startup files.
# HAL-OSAL files (optional).
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/hal/ports/simulator/posix/platform.mk
include $(CHIBIOS)/os/hal/osal/nil/osal.mk
# RTOS files (optional).
include $(CHIBIOS)/os/nil/nil.mk
include $(CHIBIOS)/os/common/ports/SIMIA32/compilers/GCC/port.mk
# Semtech chip drivers
include $(CHIBIOS_CONTRIB)/os/ex/Semtech/sx1212.mk
include $(CHIBIOS_CONTRIB)/os/ex/Semtech/sx1278.mk
# Board config
include $(FUZZ_ROOT)/board/board.mk
# Other files (optional).
#include $(CHIBIOS)/test/rt/test.mk
#include $(CHIBIOS)/os/hal/lib/streams/streams.mk
#include $(CHIBIOS)/os/various/shell/shell.mk
# Firmware files
FWSRC := ${ELYSIUM_ROOT}/core/uart.c \
				 ${ELYSIUM_ROOT}/core/main.c \
				 ${ELYSIUM_ROOT}/core/mych.c \
				 ${ELYSIUM_ROOT}/core/cmds.c \
				 ${ELYSIUM_ROOT}/core/registers.c \
				 ${ELYSIUM_ROOT}/core/stubs/errors_stub.c \
				 ${ELYSIUM_ROOT}/core/stubs/events_stub.c \
				 ${ELYSIUM_ROOT}/core/stubs/channels_stub.c \
				 ${ELYSIUM_ROOT}/core/stubs/telem_stub.c \
				 ${ELYSIUM_ROOT}/slip/slip_uart_dll.c \
				 ${ELYSIUM_ROOT}/core/crc_x25.c \
				 ${ELYSIUM_ROOT}/ccsds/crc_sdlp.c \
				 ${ELYSIUM_ROOT}/ccsds/sdlp.c \
				 ${ELYSIUM_ROOT}/core/fram.c \
				 ${ELYSIUM_ROOT}/core/rf.c \
				 ${ELYSIUM_ROOT}/spp/spp.c

# Required include directories
FWINC := ${ELYSIUM_ROOT}/core/ \
				 ${ELYSIUM_ROOT}/slip/ \
				 ${ELYSIUM_ROOT}/spp/ \
				 ${ELYSIUM_ROOT}/ccsds/


# C sources here.
CSRC = $(STARTUPSRC) \
       $(KERNSRC) \
       $(PORTSRC) \
       $(OSALSRC) \
       $(HALSRC) \
       $(PLATFORMSRC) \
       $(BOARDSRC) \
       $(TESTSRC) \
       $(STREAMSSRC) \
       $(SHELLSRC) \
       $(FWSRC) \
       $(SX1212SRC) \
       $(SX1278SRC) \
       fuzz_main.c

# C++ sources here.
CPPSRC =

# List ASM source files here
ASMSRC =
ASMXSRC = $(STARTUPASM) $(PORTASM) $(OSALASM)

INCDIR = $(CHIBIOS)/os/license \
         $(STARTUPINC) $(KERNINC) $(PORTINC) $(OSALINC) \
         $(HALINC) $(PLATFORMINC) $(BOARDINC) $(TESTINC) \
         $(STREAMSINC) $(SHELLINC) $(FWINC) $(SX1212INC) \
         $(SX1278INC)

# GCOV files.
GCOVSRC = $(KERNSRC)

#
# Project, sources and paths
##############################################################################

##############################################################################
# Compiler settings
#

#TRGT = powerpc-eabi-
TRGT = 
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
# Enable loading with g++ only if you need C++ runtime support.
# NOTE: You can use C++ even without C++ support if you are careful. C++
#       runtime support makes code size explode.
LD   = $(TRGT)gcc
#LD   = $(TRGT)g++
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
AR   = $(TRGT)ar
OD   = $(TRGT)objdump
SZ   = $(TRGT)size
BIN  = $(CP) -O binary
COV  = gcov

# Define C warning options here
CWARN = -Wall -Wextra -Wundef -Wstrict-prototypes

# Define C++ warning options here
CPPWARN = -Wall -Wextra -Wundef

#
# Compiler settings
##############################################################################

##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1
UDEFS = -DSIMULATOR $(XDEFS)

# Define ASM defines here
UADEFS =

# List all user directories here
UINCDIR =

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS = -lgcov

#
# End of user defines
##############################################################################

RULESPATH = $(CHIBIOS)/os/common/startup/SIMIA32/compilers/GCC
include $(RULESPATH)/rules.mk

misra:
	@lint-nt -v -w3 $(DEFS) pclint/co-gcc.lnt pclint/au-misra3.lnt pclint/waivers.lnt $(IINCDIR) $(KERNSRC)
