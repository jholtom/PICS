#!/usr/bin/env python

import random
import spp
import slip
import math
import itertools
import serial
import sys
import time

from ely_util import *
from cmds import *

functions = []

# List:
#   Active bank changes after Reload
#   Get and Set Registers commands match for at least 2 changes on Core, NL, and DLL registers
#   Get and Set Block commands match for at least 2 changes on Core, NL, and DLL registers
#   Get Registers command for Bank 0 is accepted
#   Set Registers command for Bank 0 is rejected
#   Get and Set Registers commands for invalid Banks are rejected
#   Get Registers command results match properly before and after ReloadConfig
#   ReloadConfig causes UART and RF to reconfigure with new values
#   ReloadConfig causes proper resets of memory pools and mailboxes, including changing NLMaxLength
#   New firmware can be uploaded, verified, and installed
#   Uploaded data with invalid address is rejected
#   Cancel and Install firmware commands correctly return SUCCESS

bank = 1
addresses = [0x01, 0x05, 0x84, 0xc8]
r = run_and_read(ser, CmdGetRegs, bank, addresses, False, True, test_apid)
try:
    (opcode, length, source, payload) = extract_and_assert(r, CmdGetRegs.opcode, 4, 1)
except:
    print readify(r)
    raise
initial = map(ord, payload)

print "Retrieved %s" % readify(initial)

