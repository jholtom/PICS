#!/usr/bin/env python

from all_cmds import *

# Errors
r = run_and_read(ser, CmdGetErr, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetErr.opcode, 1, 1)
value = struct.unpack('!B', payload)[0]
print "Current error mask is %s" % hex(value)

print "Setting error mask to %s" % hex(value ^ 0x1F)
run(ser, CmdSetErr, value ^ 0x1F, True)

r = run_and_read(ser, CmdGetErr, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetErr.opcode, 3, 1)
newval = struct.unpack('!B', payload[:1])[0]
print newval
assert newval == value ^ 0x1F, "SetErr not successful"
print "Now error mask is %s" % hex(newval)

print "Setting error mask to %s" % hex(0xFF)
r = run_and_read(ser, CmdSetErr, 0xFF, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetErr.opcode, 0, 1)

r = run_and_read(ser, CmdGetErr, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetErr.opcode, 3, 1)
value = struct.unpack('!B', payload[:1])[0]
print "Error Mask is still %s" % hex(value)

r = run_and_read(ser, CmdGetLog, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetLog.opcode, 1, 1)
value = struct.unpack('!B', payload)[0]
print "Current error logging mask is %s" % hex(value)

print "Setting error logging mask to %s" % hex(value ^ 0x1F)
run(ser, CmdSetLog, value ^ 0x1F, True)

r = run_and_read(ser, CmdGetLog, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetLog.opcode, 3, 1)
newval = struct.unpack('!B', payload[:1])[0]
print newval
assert newval == value ^ 0x1F, "SetLog not successful"
print "Now error logging mask is %s" % hex(newval)

print "Setting error mask to %s" % hex(0xFF)
r = run_and_read(ser, CmdSetLog, 0xFF, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetLog.opcode, 0, 1)

r = run_and_read(ser, CmdGetLog, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetLog.opcode, 3, 1)
value = struct.unpack('!B', payload[:1])[0]
print "Error Logging Mask is still %s" % hex(value)

