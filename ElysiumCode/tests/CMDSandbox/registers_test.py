#!/usr/bin/env python

from all_cmds import *

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

r = run_and_read(ser, CmdGetActiveBank, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetActiveBank.opcode, 3, 1)
value = ord(payload[0])
assert value == 2, "value error in response"
print "Active bank is now %d" % value

print "Setting active bank to 1"
run(ser, CmdReloadConfig, 1, True, True, test_apid)
print "Waiting for 5 seconds for Elysium restart..."
time.sleep(5)
print "Okay, done waiting."

r = run_and_read(ser, CmdGetActiveBank, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetActiveBank.opcode, 3, 1)
value = ord(payload[0])
assert value == 1, "value error in response"
print "Active bank is now %d" % value

print "Setting active bank to 9"
r = run_and_read(ser, CmdReloadConfig, 9, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdReloadConfig.opcode, 2, 1)
assert len(payload) == 2, "ReloadConfig returned something other than failure"

r = run_and_read(ser, CmdGetActiveBank, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetActiveBank.opcode, 3, 1)
value = ord(payload[0])
assert value == 1, "value error in response"
print "Active bank is still %d" % value
  
print "Reading the low middle bytes of all the RF parameters"
addresses = [x for x in range(1, 0x1F, 4)]
expected = [87, 80, 19, 195, 131, 187, 18, 97]
r = run_and_read(ser, CmdGetRegs, 0, addresses, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetRegs.opcode, 8, 1)
print "The bytes are %s" % str(map(ord, payload))

print "Writing the low middle bytes of all the RF parameters to 0xFF in bank 1"
written = [255, 255, 255, 255, 255, 255, 255, 255]
r = run_and_read(ser, CmdSetRegs, 1, addresses, written, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetRegs.opcode, 3, 1)
assert ord(payload[0]) == 1, "Setregs didn't return success"

print "Reading the TX frequency by block"
r = run_and_read(ser, CmdGetBlock, 0, 1, 4, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetBlock.opcode, 6, 1)
print "The bytes are %s" % str(map(ord, payload))

print "Writing the TX frequency to 0xFF in bank 1"
r = run_and_read(ser, CmdSetBlock, 1, 1, [0xFF, 0xFF, 0xFF, 0xFF], False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetBlock.opcode, 1, 1)
assert ord(payload[0]) == 1, "SetBlock didn't return success"



