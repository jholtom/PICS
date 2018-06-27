.PHONY: all directories

################################################################################
# file lists
################################################################################
PROJECT_ROOT = ../
CHIBIOS = ../../ChibiOS-RT/
CHIBIOS_CONTRIB = ../../ChibiOS-Contrib/
IDIRS :=                                                                      \
  ./overrides                                                                 \
  $(PROJECT_ROOT)/                                                            \
  $(PROJECT_ROOT)/board/b/                                                    \
  $(PROJECT_ROOT)ccsds/                                                       \
  $(PROJECT_ROOT)core/                                                        \
  $(PROJECT_ROOT)slip/                                                        \
  $(PROJECT_ROOT)spp/                                                         \
  $(CHIBIOS)os/common/oslib/include/                                          \
  $(CHIBIOS)os/hal/include                                                    \
  $(CHIBIOS)os/hal/osal/nil                                                   \
  $(CHIBIOS)os/license                                                        \
  $(CHIBIOS)os/nil/include                                                    \
  $(CHIBIOS_CONTRIB)os/common/ports/MSP430X/                                  \
  $(CHIBIOS_CONTRIB)os/hal/ports/MSP430X/                                     \
  $(CHIBIOS_CONTRIB)os/common/ports/MSP430X/compilers/GCC/                    \
  $(CHIBIOS_CONTRIB)os/ex/Semtech/                                            \

SRCS_NAMES := \
  ccsds/sdlp.c \
  core/registers.c \
  spp/spp.c \

#  core/cmds.c \
#  core/rf.c \
#  core/main.c \
#  core/fram.c \

FUZZ_SRCS := \
  FuzzDriver.cpp \
  SimTodo.cpp

# util/init.c

SRCS := $(addprefix ../,$(SRCS_NAMES))
OBJECTS := $(patsubst $(PROJECT_ROOT)%.c,build/%.o,$(SRCS))

FUZZ_OBJECTS := $(patsubst %.cpp,build/fuzz/%.o,$(FUZZ_SRCS))
################################################################################
# flags
################################################################################

SANATIZE_FLAGS := -fsanitize=fuzzer,address

CFLAGS ?=
# CFLAGS += -include ./overrides/compiler_globals.h
CFLAGS += $(SANATIZE_FLAGS)
CFLAGS += -g -O1
CFLAGS += $(addprefix -I ,$(IDIRS))
CFLAGS += -D__MSP430FR5969__
CFLAGS += -D__FUZZ__
CC := clang

LDFLAGS ?=
LDFLAGS += $(SANATIZE_FLAGS)

################################################################################
# ChibiOS Imports
################################################################################

# Startup files.
include $(CHIBIOS_CONTRIB)/os/common/startup/MSP430X/compilers/GCC/mk/startup_msp430fr5xxx.mk
# HAL-OSAL files
include $(CHIBIOS)/os/hal/hal.mk
# Other files
include $(CHIBIOS_CONTRIB)/os/ex/Semtech/sx1278.mk
include $(CHIBIOS_CONTRIB)/os/ex/Semtech/sx1212.mk

################################################################################
# Build rules
################################################################################
all: directories fuzz_binary

directories:
	mkdir -p build/ccsds
	mkdir -p build/spp
	mkdir -p build/core
	mkdir -p build/fuzz
	mkdir -p build/util

fuzz_binary: $(OBJECTS) $(CHIBI_OBJECTS) $(FUZZ_OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

build/%.o: $(PROJECT_ROOT)%.c
	$(CC) -c $(CFLAGS) $< -o $@

build/fuzz/%.o: ./%.cpp
	$(CC) -c $(CFLAGS) -x c++ -std=c++11 $< -o $@
