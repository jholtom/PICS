#!/usr/bin/env python

from all_cmds import *

# Registers
print "Reading the low middle bytes of all the RF parameters"
addresses = [x for x in range(1, 0x1F, 4)]
expected = [87, 80, 19, 195, 131, 187, 18, 97]
r = run_and_read(ser, CmdGetRegs, 0, addresses, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetRegs.opcode, 8, 1)
print "The bytes are %s" % str(map(ord, payload))

print "Writing the low middle bytes of all the RF parameters to 0x55 in bank 1"
written = [0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55]
r = run_and_read(ser, CmdSetRegs, 1, addresses, written, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetRegs.opcode, 3, 1)
assert ord(payload[0]) == 1, "Setregs didn't return success"



