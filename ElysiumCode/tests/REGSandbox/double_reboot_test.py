#!/usr/bin/env python

from all_cmds import *

f = open('registers.bin', 'rb')
defaults = map(ord, list(f.read()))
f.close()

# Registers
r = run_and_read(ser, CmdGetActiveBank, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetActiveBank.opcode, 1, 1)
value = ord(payload[0])
assert value in range(1, 5), "value error in response"
print "Active bank is %d" % value

print "Setting active bank to 2"
run(ser, CmdReloadConfig, 2)
print "Waiting for 5 seconds for Elysium restart..."
time.sleep(5)
print "Okay, done waiting."


