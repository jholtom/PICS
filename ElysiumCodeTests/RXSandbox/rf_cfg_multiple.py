#!/usr/bin/env python

from all_cmds import *

# RF
txfreq = 451500000
rxfreq = 434000000
txrate = 4800
rxrate = 25000

for i in range(0, 10):
    txfreq -= 10
    rxfreq -= 10
    txrate -= 10
    rxrate -= 10
    print "Setting TX Freq to %d" % (txfreq)
    print "Setting RX Freq to %d" % (rxfreq)
    print "Setting TX Rate to %d" % (txrate)
    print "Setting RX Rate to %d" % (rxrate)
    
    run(ser, CmdSetTXFreq, txfreq, True)
    run(ser, CmdSetRXFreq, rxfreq, True)
    run(ser, CmdSetTXRate, txrate, True)
    run(ser, CmdSetRXRate, rxrate, True)

r = run_and_read(ser, CmdGetTXFreq, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXFreq.opcode, 4, 1)
txfreq = struct.unpack('!I', payload)[0]
print "Now TX Freq is %d" % txfreq

r = run_and_read(ser, CmdGetRXFreq, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetRXFreq.opcode, 4, 1)
rxfreq = struct.unpack('!I', payload)[0]
print "Now RX Freq is %d" % rxfreq

r = run_and_read(ser, CmdGetTXRate, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXRate.opcode, 4, 1)
txrate = struct.unpack('!I', payload)[0]
print "Now TX Rate is %d" % txrate

r = run_and_read(ser, CmdGetRXRate, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetRXRate.opcode, 4, 1)
rxrate = struct.unpack('!I', payload)[0]
print "Now RX Rate is %d" % rxrate

