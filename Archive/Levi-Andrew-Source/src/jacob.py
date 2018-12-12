#!/usr/bin/env python2

import random
import spp
import slip
import math
import itertools
import serial
import sys
import time

from cmds import *
from ely_util import *

#SET WHETHER WE ARE PICA OR B HERE
PICA = False
PICB = True

#txf = 903800000 #Pic-A is 903.8MHz, Pic-B is 903.4MHz
#rxf = 459200000 #Pic-A is 459.2MHz, Pic-B is 459.0MHz
txf = 903400000 #Pic-A is 903.8MHz, Pic-B is 903.4MHz
rxf = 459000000 #Pic-A is 459.2MHz, Pic-B is 459.0MHz

txrate = 115200
rxrate = 115200

txdev = txrate/4 #1/4 our bitrate (300kbps, aka 300000bps)
rxdev = rxrate/2 #1/2 our ground station bitrate

txpow = 59 #10 + 0.2x = 33dBm = 2W
#txpow = 115 #10 + 0.2x = 33dBm = 2W

if PICA:
    srcaddr = 1009
elif PICB:
    srcaddr = 1109

bank = 1
addr = [0x20, #FilterParams
        0x27, #FaultResponse
        0xC1, #TFLength LSB
        0xC0, #TFLength MSB
        0x86, #SPP Options
        0x83, #GroundAPID MSB
        0x82] #GroundAPID LSB
values = [242, #values = 11 11 0010 - Middle bits are Gaussian filter, 01 for 1.0 BT, 10 for 0.5 BT, and 11 for 0.3BT (BT = Bandwidth Time Product) Last 4 are RX_BW, single-sided.
          0, #Disable all Fault Responses
          0x04, #TFLength of 1024
          0x00, #^^^
          0x00, #SPP Options are 0, no options
          0x03, #GroundAPID is 820 - A, 920 -B
          0x98] #PIC-B is 0x98, A is 0x34
r = run_and_read(ser, CmdGetRegs, bank, addr, False, True, test_apid)
try:
    (opcode, length, source, payload) = extract_and_assert(r, CmdGetRegs.opcode, len(values), 1)
except:
    print readify(r)
    raise
initial = map(ord, payload)
print "Current Register Values in bank %d: %s" % (bank, readify(initial))
print "Configuring Filter Params register in bank %d" % bank
r = run_and_read(ser, CmdSetRegs, bank, addr, values, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetRegs.opcode, 3, 1)
assert ord(payload[0]) == 1, "FAILURE - SetRegs didn't return success"

r = run_and_read(ser, CmdGetRegs, bank, addr, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetRegs.opcode, len(values), 1)
result = map(ord, payload)
try:
   assert result == values, "FAILURE - SetRegs didn't successfully change registers"
except:
   print result
   print values
   print "GetRegs returned %s, expected %s" % (readify(result), readify(written))
   raise

bankone = 1
run(ser, CmdReloadConfig, bankone)
time.sleep(5)
ser.flushInput()
ser.flushOutput()

# TxFreq
r = run_and_read(ser, CmdGetTXFreq, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXFreq.opcode, 6, 1)
curtxf = struct.unpack('!I', payload[:4])[0]
print "Current TX Freq is: %d" % curtxf

print "Setting TX Freq to: %d" % txf
r = run_and_read(ser, CmdSetTXFreq, txf, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetTXFreq.opcode, 3, 1)

r = run_and_read(ser, CmdGetTXFreq, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXFreq.opcode, 4, 1)
newval = struct.unpack('!I', payload)[0]
try:
    assert newval == txf, "FAILURE - TX Freq was not set"
except:
    print "%s returned %d, expected %d" % (CmdGetTXFreq, newval, txf)
    raise

# RxFreq
r = run_and_read(ser, CmdGetRXFreq, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetRXFreq.opcode, 6, 1)
currxf = struct.unpack('!I', payload[:4])[0]
print "Current RX Freq is: %d Hz" % currxf

print "Setting RX Freq to: %d Hz" % rxf
r = run_and_read(ser, CmdSetRXFreq, rxf, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetRXFreq.opcode, 3, 1)

r = run_and_read(ser, CmdGetRXFreq, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetRXFreq.opcode, 4, 1)
newval = struct.unpack('!I', payload)[0]
try:
    assert newval == rxf, "FAILURE - RX Freq was not set"
except:
    print "%s returned %d, expected %d" % (CmdGetRXFreq, newval, rxf)
    raise

# TxDev
r = run_and_read(ser, CmdGetTXDev, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXDev.opcode, 6, 1)
curtxdev = struct.unpack('!I', payload[:4])[0]
print "Current TX Deviation is: %d Hz" % curtxdev

print "Setting TX Deviation to: %d Hz" % txdev
r = run_and_read(ser, CmdSetTXDev,txdev, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetTXDev.opcode, 3, 1)

r = run_and_read(ser, CmdGetTXDev, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXDev.opcode, 4, 1)
newval = struct.unpack('!I', payload)[0]
try:
    assert newval == txdev, "FAILURE - TX Deviation was not set"
except:
    print "%s returned %d, expected %d" % (CmdGetTXDev, newval, txdev)
    raise

# RxDev 
r = run_and_read(ser, CmdGetRXDev, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetRXDev.opcode, 6, 1)
currxdev = struct.unpack('!I', payload[:4])[0]
print "Current RX Deviation is: %d Hz" % currxdev

print "Setting RX Deviation to: %d Hz" % rxdev
r = run_and_read(ser, CmdSetRXDev,rxdev, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetRXDev.opcode, 3, 1)

r = run_and_read(ser, CmdGetRXDev, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetRXDev.opcode, 4, 1)
newval = struct.unpack('!I', payload)[0]
try:
    assert newval == rxdev, "FAILURE - RX Deviation was not set"
except:
    print "%s returned %d, expected %d" % (CmdGetRXDev, newval, rxdev)
    raise

# Tx BitRate
r = run_and_read(ser, CmdGetTXRate, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXRate.opcode, 6, 1)
curtxbr = struct.unpack('!I', payload[:4])[0]
print "Current TX Bitrate is: %d bps" % curtxbr

print "Setting TX Bitrate to: %d bps" % txrate 
r = run_and_read(ser, CmdSetTXRate, txrate, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetTXRate.opcode, 3, 1)

r = run_and_read(ser, CmdGetTXRate, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXRate.opcode, 4, 1)
newval = struct.unpack('!I', payload)[0]
try:
    assert newval == txrate, "FAILURE - TX Rate was not set"
except:
    print "%s returned %d, expected %d" % (CmdGetTXRate, newval, rxdev)
    raise

# Rx BitRate
r = run_and_read(ser, CmdGetRXRate, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetRXRate.opcode, 6, 1)
currxbr = struct.unpack('!I', payload[:4])[0]
print "Current RX Bitrate is: %d bps" % currxbr

print "Setting RX Bitrate to: %d bps" % rxrate
r = run_and_read(ser, CmdSetRXRate, rxrate, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetRXRate.opcode, 3, 1)

r = run_and_read(ser, CmdGetRXRate, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetRXRate.opcode, 4, 1)
newval = struct.unpack('!I', payload)[0]
try:
    assert newval == rxrate, "FAILURE - RX Rate was not set"
except:
    print "%s returned %d, expected %d" % (CmdGetRXRate, newval, rxrate)
    raise


# Set TX Power
r = run_and_read(ser, CmdGetTXPow, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXPow.opcode, 1, 1)
curtxpow = struct.unpack('!b', payload)[0]
print "Current TX Power is: %d dBm" % curtxpow

print "Setting TX Power to: %d dBm" % txpow
r = run_and_read(ser, CmdSetTXPow, txpow, True, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetTXPow.opcode, 3, 1)

r = run_and_read(ser, CmdGetTXPow, True, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTXPow.opcode, 3, 1)
newval = struct.unpack('!b', payload[:1])[0]
try:
    assert newval == txpow, "FAILURE - TX Power was not set"
except:
    print "%s returned %d, expected %d" % (CmdGetTXPow, newval, txpow)
    raise


# Set Mission Time (UNIX Epoch)
# CmdSetTime
missiontime = int(time.time())

r = run_and_read(ser, CmdGetTime, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdGetTime.opcode, 4, 1)
curtime = struct.unpack('!I', payload[:4])[0]
print "Current Time is %d seconds since the UNIX epoch" % curtime

print "Setting Time to: %d seconds since the UNIX epoch" % missiontime 
r = run_and_read(ser, CmdSetTime, missiontime, True, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetTime.opcode, 6, 1)

# Read RFConfig (0x6F)
bankrf = 0
addrrf = [0x6F,0x70]
r = run_and_read(ser, CmdGetRegs, bankrf, addrrf, False, True, test_apid)
try:
    (opcode, length, source, payload) = extract_and_assert(r, CmdGetRegs.opcode, 2, 1)
except:
    print readify(r)
    raise
rfconfig = map(ord, payload)
print "Curent RFConfig Values: %s" % readify(rfconfig)


