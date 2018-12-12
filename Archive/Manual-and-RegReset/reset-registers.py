#!/usr/bin/env python

from ely_util import *
from cmds import *

f = open('registers.bin', 'rb')
defaults = map(ord, list(f.read()))
f.close()

print "[0] Initializing banks to default values"
for bank in range(1, 5):
    r = run_and_read(ser, CmdSetBlock, bank, 0, defaults[:0x56], False, True, test_apid)
    (opcode, length, source, payload) = extract_and_assert(r, CmdSetBlock.opcode, 1, 1)
    assert ord(payload[0]) == 1, "FAILURE - SetBlock didn't return success"
    r = run_and_read(ser, CmdSetBlock, bank, 0x80, defaults[0x80:0x8a], False, True, test_apid)
    (opcode, length, source, payload) = extract_and_assert(r, CmdSetBlock.opcode, 1, 1)
    assert ord(payload[0]) == 1, "FAILURE - SetBlock didn't return success"
    r = run_and_read(ser, CmdSetBlock, bank, 0xC0, defaults[0xC0:0xD5], False, True, test_apid)
    (opcode, length, source, payload) = extract_and_assert(r, CmdSetBlock.opcode, 1, 1)
    assert ord(payload[0]) == 1, "FAILURE - SetBlock didn't return success"
    
print "> SUCCESS"
