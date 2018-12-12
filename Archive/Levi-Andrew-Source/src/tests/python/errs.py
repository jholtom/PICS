#!/usr/bin/env python

import struct
import crcmod
import itertools

from cmds import *

ErrPAOvertemp = 0x80
ErrHSOvertemp = 0x81
ErrMCUOvertemp = 0x82
ErrGroundCommFault = 0x83
ErrSCCommFault = 0x84
ErrLNAOvercurrent = 0x85
ErrPAOvercurrent = 0x86
ErrMCUUndervolt = 0x87
ErrRegClip = 0x88
ErrInvalidOpcode = 0x89
ErrInvalidLength = 0x8A
ErrFCSError = 0x8B
ErrCmdFailure = 0x8C
ErrSubOverwrite = 0x8D
ErrReset = 0x8E
ErrUARTError = 0x8F
ErrPAWarntemp = 0x90
ErrHSWarntemp = 0x91
ErrMCUWarntemp = 0x92
ErrNLPVNMismatch = 0xA0
ErrNLPacketLengthMismatch = 0xA1
ErrDLLFEC = 0xB0
ErrDLLFECF = 0xB1
ErrDLLMissedFrame = 0xB2
ErrDLLLockout = 0xB3
ErrDLLDoubleFrame = 0xB4
ErrDLLInvalidID = 0xB5
ErrDLLShortFrame = 0xB6
ErrDLLLongFrame = 0xB7
ErrDLLWait = 0xB8
