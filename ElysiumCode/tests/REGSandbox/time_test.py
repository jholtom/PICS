#!/usr/bin/env python

from all_cmds import *

# Time
r = run_and_read(ser, CmdGetTime, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTime.opcode, 4, 1)
value = struct.unpack('!i', payload)[0]
print "Current mission time is %d" % value

print "Setting mission time to %d" % (value + 1000)
run(ser, CmdSetTime, value + 1000, True)

r = run_and_read(ser, CmdGetTime, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTime.opcode, 6, 1)
newval = struct.unpack('!i', payload[:4])[0]
assert newval == value + 1000, "SetTime not successful"
print "Now Mission Time is %d" % newval

print "Setting Mission Time to %d" % 0x3FFFFFFF
r = run_and_read(ser, CmdSetTime, 0x3FFFFFFF, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetTime.opcode, 4, 1)

r = run_and_read(ser, CmdGetTime, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTime.opcode, 6, 1)
value = struct.unpack('!i', payload[:4])[0]
print "Mission Time is now %d" % value

