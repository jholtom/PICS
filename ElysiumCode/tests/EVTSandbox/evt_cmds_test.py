#!/usr/bin/env python

from all_cmds import *

# Events
print "Subscribing odd-numbered events"
evts = range(0xC1, 0xD2, 2)
r = run_and_read(ser, CmdEventSub, evts, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdEventSub.opcode, 3, 1)
res = ord(payload[0])
assert res == 1, "EventSub not successful"

r = run_and_read(ser, CmdGetTXFreq, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXFreq.opcode, 4, 1)
value = struct.unpack('!I', payload)[0]
print "TX Freq is %d" % value

print "Setting TX Freq to %d to generate an event" % (value - 10)
run(ser, CmdSetTXFreq, value - 10, True)

r = run_and_read(ser, CmdGetTXFreq, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXFreq.opcode, 4, 1)
value = struct.unpack('!I', payload)[0]
print "TX Freq is now %d" % value

print "Resetting events"
r = run_and_read(ser, CmdResetEvent, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdResetEvent.opcode, 3, 1)

print "Unsubscribing odd-numbered events"
r = run(ser, CmdEventUnsub, evts, True)

print "Logging even-numbered events"
evts = range(0xC0, 0xD2, 2)
r = run_and_read(ser, CmdLogEvent, evts, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdLogEvent.opcode, 1, 1)
res = ord(payload[0])
assert res == 1, "EventLog not successful"

print "Unlogging even-numbered events"
r = run_and_read(ser, CmdUnlogEvent, evts, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdUnlogEvent.opcode, 3, 1)
res = ord(payload[0])
assert res == 1, "EventSub not successful"

