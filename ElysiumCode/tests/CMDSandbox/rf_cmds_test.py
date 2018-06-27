#!/usr/bin/env python

from all_cmds import *

# RF
r = run_and_read(ser, CmdGetTXFreq, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXFreq.opcode, 4, 1)
value = struct.unpack('!I', payload)[0]
print "TX Freq is %d" % value

print "Setting TX Freq to %d" % (value - 10)
run(ser, CmdSetTXFreq, value - 10, True)

r = run_and_read(ser, CmdGetTXFreq, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXFreq.opcode, 6, 1)
newval = struct.unpack('!I', payload[:4])[0]
assert newval == value - 10, "SetTXFreq not successful"
print "Now TX Freq is %d" % newval

print "Setting TX Freq to %d" % 0xFFFFFFFF
r = run_and_read(ser, CmdSetTXFreq, 0xFFFFFFFF, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetTXFreq.opcode, 0, 1)
assert payload == '', "SetTXFreq didn't return success"

r = run_and_read(ser, CmdGetTXFreq, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXFreq.opcode, 6, 1)
value = struct.unpack('!I', payload[:4])[0]
print "TX Freq is still %d" % value

r = run_and_read(ser, CmdGetRXFreq, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetRXFreq.opcode, 4, 1)
value = struct.unpack('!I', payload)[0]
print "RX Freq is %d" % value

print "Setting RX Freq to %d" % (value - 10)
run(ser, CmdSetRXFreq, value - 10, True)

r = run_and_read(ser, CmdGetRXFreq, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetRXFreq.opcode, 6, 1)
newval = struct.unpack('!I', payload[:4])[0]
assert newval == value - 10, "SetRXFreq not successful"
print "Now RX Freq is %d" % newval

print "Setting RX Freq to %d" % 0xFFFFFFFF
r = run_and_read(ser, CmdSetRXFreq, 0xFFFFFFFF, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetRXFreq.opcode, 0, 1)
assert payload == '', "SetRXFreq didn't return success"

r = run_and_read(ser, CmdGetRXFreq, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetRXFreq.opcode, 6, 1)
value = struct.unpack('!I', payload[:4])[0]
print "RX Freq is still %d" % value

r = run_and_read(ser, CmdGetTXRate, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXRate.opcode, 4, 1)
value = struct.unpack('!I', payload)[0]
print "TX Rate is %d" % value

print "Setting TX Rate to %d" % (value - 10)
run(ser, CmdSetTXRate, value - 10, True)

r = run_and_read(ser, CmdGetTXRate, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXRate.opcode, 6, 1)
newval = struct.unpack('!I', payload[:4])[0]
assert newval == value - 10, "SetTXRate not successful"
print "Now TX Rate is %d" % newval

print "Setting TX Rate to %d" % 0xFFFFFFFF
r = run_and_read(ser, CmdSetTXRate, 0xFFFFFFFF, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetTXRate.opcode, 0, 1)
assert payload == '', "SetTXRate didn't return success"

r = run_and_read(ser, CmdGetTXRate, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXRate.opcode, 6, 1)
value = struct.unpack('!I', payload[:4])[0]
print "TX Rate is still %d" % value

r = run_and_read(ser, CmdGetRXRate, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetRXRate.opcode, 4, 1)
value = struct.unpack('!I', payload)[0]
print "RX Rate is %d" % value

print "Setting RX Rate to %d" % (value - 10)
run(ser, CmdSetRXRate, value - 10, True)

r = run_and_read(ser, CmdGetRXRate, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetRXRate.opcode, 6, 1)
newval = struct.unpack('!I', payload[:4])[0]
assert newval == value - 10, "SetRXRate not successful"
print "Now RX Rate is %d" % newval

print "Setting RX Rate to %d" % 0xFFFFFFFF
r = run_and_read(ser, CmdSetRXRate, 0xFFFFFFFF, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetRXRate.opcode, 0, 1)
assert payload == '', "SetRXRate didn't return success"

r = run_and_read(ser, CmdGetRXRate, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetRXRate.opcode, 6, 1)
value = struct.unpack('!I', payload[:4])[0]
print "RX Rate is still %d" % value

r = run_and_read(ser, CmdGetTXDev, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXDev.opcode, 4, 1)
value = struct.unpack('!I', payload)[0]
print "TX Dev is %d" % value

print "Setting TX Dev to %d" % (value - 10)
run(ser, CmdSetTXDev, value - 10, True)

r = run_and_read(ser, CmdGetTXDev, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXDev.opcode, 6, 1)
newval = struct.unpack('!I', payload[:4])[0]
assert newval == value - 10, "SetTXDev not successful"
print "Now TX Dev is %d" % newval

print "Setting TX Dev to %d" % 0xFFFFFFFF
r = run_and_read(ser, CmdSetTXDev, 0xFFFFFFFF, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetTXDev.opcode, 0, 1)
assert payload == '', "SetTXDev didn't return success"

r = run_and_read(ser, CmdGetTXDev, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXDev.opcode, 6, 1)
value = struct.unpack('!I', payload[:4])[0]
print "TX Dev is still %d" % value

r = run_and_read(ser, CmdGetRXDev, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetRXDev.opcode, 4, 1)
value = struct.unpack('!I', payload)[0]
print "RX Dev is %d" % value

print "Setting RX Dev to %d" % (value - 10)
run(ser, CmdSetRXDev, value - 10, True)

r = run_and_read(ser, CmdGetRXDev, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetRXDev.opcode, 6, 1)
newval = struct.unpack('!I', payload[:4])[0]
assert newval == value - 10, "SetRXDev not successful"
print "Now RX Dev is %d" % newval

print "Setting RX Dev to %d" % 0xFFFFFFFF
r = run_and_read(ser, CmdSetRXDev, 0xFFFFFFFF, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetRXDev.opcode, 0, 1)
assert payload == '', "SetRXDev didn't return success"

r = run_and_read(ser, CmdGetRXDev, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetRXDev.opcode, 6, 1)
value = struct.unpack('!I', payload[:4])[0]
print "RX Dev is still %d" % value

r = run_and_read(ser, CmdGetTXPow, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXPow.opcode, 1, 1)
value = struct.unpack('!b', payload)[0]
print "TX Pow is %d" % value

print "Setting TX Pow to %d" % (value - 10)
run(ser, CmdSetTXPow, value - 10, True)

r = run_and_read(ser, CmdGetTXPow, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXPow.opcode, 3, 1)
newval = struct.unpack('!b', payload[:1])[0]
assert newval == value - 10, "SetTXPow not successful"
print "Now TX Pow is %d" % newval

print "Setting TX Pow to %d" % 0x7F
r = run_and_read(ser, CmdSetTXPow, 0x7F, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetTXPow.opcode, 0, 1)
assert payload == '', "SetTXPow didn't return success"

r = run_and_read(ser, CmdGetTXPow, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXPow.opcode, 3, 1)
value = struct.unpack('!b', payload[:1])[0]
print "TX Pow is still %d" % value

r = run_and_read(ser, CmdGetBaud, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetBaud.opcode, 4, 1)
value = struct.unpack('!I', payload)[0]
print "Baud is %d" % value

print "Setting Baud to %d" % (value - 10)
run(ser, CmdSetBaud, value - 10, True)

r = run_and_read(ser, CmdGetBaud, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetBaud.opcode, 6, 1)
newval = struct.unpack('!I', payload[:4])[0]
assert newval == value - 10, "SetBaud not successful"
print "Now Baud is %d" % newval

print "Setting Baud to %d" % 0xFFFFFFFF
r = run_and_read(ser, CmdSetBaud, 0xFFFFFFFF, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetBaud.opcode, 0, 1)
assert payload == '', "SetBaud didn't return success"

r = run_and_read(ser, CmdGetBaud, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetBaud.opcode, 6, 1)
value = struct.unpack('!I', payload[:4])[0]
print "Baud is still %d" % value

