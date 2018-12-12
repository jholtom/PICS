#!/usr/bin/env python

from all_cmds import *

# Channels
print "Subscribing odd-numbered channels at 10 second intervals"
chans = range(0x41, 0x50, 2)
r = run_and_read(ser, CmdChannelSub, 10000, chans, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdChannelSub.opcode, 3, 1)
res = ord(payload[0])
assert res == 1, "ChannelSub not successful"

print "Unsubscribing odd-numbered channels"
r = run(ser, CmdChannelUnsub, chans, True)

print "Logging even-numbered channels at 30 second intervals"
chans = range(0x40, 0x50, 2)
r = run_and_read(ser, CmdLogChan, 30000, chans, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdLogChan.opcode, 1, 1)
res = ord(payload[0])
assert res == 1, "ChannelSub not successful"

print "Resetting channels"
r = run_and_read(ser, CmdResetChan, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdResetChan.opcode, 3, 1)

print "Unlogging even-numbered channels"
r = run_and_read(ser, CmdUnlogChan, chans, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdUnlogChan.opcode, 3, 1)
res = ord(payload[0])
assert res == 1, "ChannelSub not successful"

print "Getting channel value for ChanPATemp"
r = run_and_read(ser, CmdGetChan, 0x4D, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetChan.opcode, 4, 1)
(id, val) = struct.unpack('!bb', payload[:2])
assert id == 0x4D, "Incorrect channel ID returned"
print "Channel value is %d" % val

