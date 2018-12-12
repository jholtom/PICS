#!/usr/bin/env python

from all_cmds import *
from spp import *
from slip import *
import intelhex

FW_BASE = 0x4400
FW_SIZE = 0xFC00
BLOCK_SIZE = 128

ih = intelhex.IntelHex('reg.hex')
ih.padding = 0x00

print "Cancelling FW upload to ensure consistent state"
r = run_and_read(ser, CmdCancelFW, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdCancelFW.opcode, 3, elysium_apid)
assert ord(payload[0]) == 1, "CancelFW didn't return success"


for addr in range(FW_BASE, FW_BASE+FW_SIZE, BLOCK_SIZE):
    thunk = ih.tobinstr(start=addr, size=BLOCK_SIZE)
    # don't upload wastes of time
    if all(v == '\x00' for v in thunk):
        continue
    print "Uploading firmware for address " + hex(addr)
    r = run_and_read(ser, CmdUploadFW, addr, thunk, True, True, test_apid)
    check_crc(r)
    (opcode, length, source, payload) = extract_and_assert(r, CmdUploadFW.opcode, 3, elysium_apid)
    assert ord(payload[0]) == 1, "UploadFW didn't return success"
   
