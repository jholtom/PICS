#!/usr/bin/env python

from all_cmds import *

# GPOs
r = run_and_read(ser, CmdGetGPO, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetGPO.opcode, 1, 1)
value = ord(payload[0])
assert value == 1 or value == 0, "value error in response"
print "GPO is %d" % value

print "Setting GPO to %d" % (value ^ 1)
run(ser, CmdSetGPO, value ^ 1, True)

r = run_and_read(ser, CmdGetGPO, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetGPO.opcode, 3, 1)
assert ord(payload[0]) == value ^ 1, "SetGPO not successful"
value = ord(payload[0])
print "Now GPO is %d" % value

print "Setting GPO to %d" % (value ^ 1)
r = run_and_read(ser, CmdSetGPO, value ^ 1, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetGPO.opcode, 1, 1)
assert ord(payload[0]) == 1, "SetGPO didn't return success"

r = run_and_read(ser, CmdGetGPO, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetGPO.opcode, 3, 1)
assert ord(payload[0]) == value ^ 1, "SetGPO not successful"
print "Now GPO is %d again" % ord(payload[0])

print "Setting GPO to 7"
r = run_and_read(ser, CmdSetGPO, 7, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetGPO.opcode, 2, 1)
assert len(payload) == 2, "SetGPO returned success with invalid value"

r = run_and_read(ser, CmdGetGPO, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetGPO.opcode, 3, 1)
assert ord(payload[0]) == value ^ 1, "SetGPO not successful"
print "GPO is still %d" % ord(payload[0])

