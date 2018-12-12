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

print "Verifying CRC of uploaded image"
verify_crc = crc_x25(ih.tobinstr(start=FW_BASE, size=FW_SIZE))
r = run_and_read(ser, CmdVerifyFW, verify_crc, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdVerifyFW.opcode, 3, elysium_apid)
assert ord(payload[0]) == 1, "VerifyFW didn't return success"

print "Installing firmware"
r = run_and_read(ser, CmdInstallFW, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdInstallFW.opcode, 3, elysium_apid, '\x01')
