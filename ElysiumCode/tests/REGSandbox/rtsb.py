#!/usr/bin/env python

import random
import spp
import slip
import math
import itertools
import serial
import sys
from cmd import *

# List:
#   Active bank changes after Reload
#   Get and Set Registers commands match for at least 2 changes on Core, NL, and DLL registers
#   Get and Set Block commands match for at least 2 changes on Core, NL, and DLL registers
#   Get Registers command for Bank 0 is accepted
#   Set Registers command for Bank 0 is rejected
#   Get and Set Registers commands for invalid Banks are rejected
#   Get Registers command results match properly before and after ReloadConfig
#   ReloadConfig causes UART and RF to reconfigure with new values
#   ReloadConfig causes proper resets of memory pools and mailboxes, including changing NLMaxLength
#   New firmware can be uploaded, verified, and installed
#   Uploaded data with invalid address is rejected
#   Cancel and Install firmware commands correctly return SUCCESS

# Serial port setup
ser = serial.Serial('/dev/ttyACM1', 115200, timeout=5)

test_apid = 2
elysium_apid = 1

# Utility functions
def blocking_read(ser):
    try:
        result = ''
        while True:
            result = ser.read_until(terminator='\xc0')
            return result
    except:
        e = sys.exc_info()[0] # last exception
        print e
        raise
      
def readify(buff):
  return str(map(hex, bytearray(buff)))
  
def run(ser, cmd, *args):
    ser.write(slip.stuff(spp.packetize(cmd(*args), elysium_apid)))
    
def run_and_read(ser, cmd, *args):
    ser.write(slip.stuff(spp.packetize(cmd(*args), elysium_apid)))
    return blocking_read(ser)

def extract_and_assert(reply, opcode, length, source, payload = None):
  (_opcode, _length, _source, _payload) = extract(spp.unpacketize(slip.unstuff(reply))[3])
  try:
    assert _opcode == opcode, "FAILURE - opcode error in response"
    assert _length == length, "FAILURE - length error in response"
    assert _source == source, "FAILURE - APID error in response"
    if payload:
        assert _payload == payload, "FAILURE - payload error in response"
  except:
    print "Expected opcode is %s, received %s" % (str(opcode), str(_opcode))
    print "Expected length is %s, received %s" % (str(length), str(_length))
    print "Expected source is %s, received %s" % (str(source), str(_source))
    if payload:
      print "Expected payload is %s, received %s" % (payload, payload)
    raise  
  return (_opcode, _length, _source, _payload)

def check_crc(buffer):
    buff = spp.unpacketize(slip.unstuff(buffer))[3]
    crc = crc_x25(buff[:-2])
    assert crc >> 8 == ord(buff[-2]), "FAILURE - CRC error"
    assert crc & 0xFF == ord(buff[-1]), "FAILURE - CRC error"
    
f = open('registers.bin', 'rb')
defaults = map(ord, list(f.read()))
f.close()

print "[0] Initializing banks to default values"
for bank in range(1, 5):
    r = run_and_read(ser, CmdSetBlock, bank, 0, defaults[:0x56], False, True, test_apid)
    (opcode, length, source, payload) = extract_and_assert(r, CmdSetBlock.opcode, 1, 1)
    assert ord(payload[0]) == 1, "FAILURE - SetBlock didn't return success"
    r = run_and_read(ser, CmdSetBlock, bank, 0x80, defaults[0x80:0x8a], False, True, test_apid)
    (opcode, length, source, payload) = extract_and_assert(r, CmdSetBlock.opcode, 1, 1)
    assert ord(payload[0]) == 1, "FAILURE - SetBlock didn't return success"
    r = run_and_read(ser, CmdSetBlock, bank, 0xC0, defaults[0xC0:0xD5], False, True, test_apid)
    (opcode, length, source, payload) = extract_and_assert(r, CmdSetBlock.opcode, 1, 1)
    assert ord(payload[0]) == 1, "FAILURE - SetBlock didn't return success"
    
print "> SUCCESS"

print "[1] Get and Set Registers commands match for at least 2 changes on Core, NL, and DLL registers"
for bank in range(1, 5):
  addresses = [0x01, 0x05, 0x84, 0xc7]
  r = run_and_read(ser, CmdGetRegs, bank, addresses, False, True, test_apid)
  (opcode, length, source, payload) = extract_and_assert(r, CmdGetRegs.opcode, 4, 1)
  initial = map(ord, payload)
  
  written = [0xff, 0xff, 0xff, 0xfe]
  r = run_and_read(ser, CmdSetRegs, bank, addresses, written, True, True, test_apid)
  check_crc(r)
  (opcode, length, source, payload) = extract_and_assert(r, CmdSetRegs.opcode, 3, 1)
  assert ord(payload[0]) == 1, "FAILURE - SetRegs didn't return success"
  
  r = run_and_read(ser, CmdGetRegs, bank, addresses, False, True, test_apid)
  (opcode, length, source, payload) = extract_and_assert(r, CmdGetRegs.opcode, 4, 1)
  result = map(ord, payload)
  try:
    assert result == written, "FAILURE - SetRegs didn't successfully change registers"
  except:
    print result
    print written
    print "GetRegs returned %s, expected %s" % (readify(result), readify(written))
    raise
  
  r = run_and_read(ser, CmdSetRegs, bank, addresses, initial, False, True, test_apid)
  (opcode, length, source, payload) = extract_and_assert(r, CmdSetRegs.opcode, 1, 1)
  assert ord(payload[0]) == 1, "FAILURE - SetRegs didn't return success"

  r = run_and_read(ser, CmdGetRegs, bank, addresses, True, True, test_apid)
  check_crc(r)
  (opcode, length, source, payload) = extract_and_assert(r, CmdGetRegs.opcode, 6, 1)
  result = map(ord, payload[:-2])
  try:
    assert result == initial, "FAILURE - SetRegs didn't successfully reset registers"
  except:
    print "GetRegs returned %s, expected %s" % (readify(result), readify(initial))
    raise
  
print "> SUCCESS"

print "[2] Get and Set Block commands match for at least 2 changes on Core, NL, and DLL registers"
for bank in range(1, 5):
  source = [(0x00, 4), (0x04, 4), (0x84, 2), (0xc9, 2)]
  for (addr, size) in source:
    r = run_and_read(ser, CmdGetBlock, bank, addr, size, False, True, test_apid)
    try:
      (opcode, length, source, payload) = extract_and_assert(r, CmdGetBlock.opcode, size, 1)
    except:
      print "GetBlock returned %s" % (readify(r))
      raise
    initial = map(ord, payload)
  
    written = map(lambda x: x+1, initial)
    r = run_and_read(ser, CmdSetBlock, bank, addr, written, True, True, test_apid)
    check_crc(r)
    (opcode, length, source, payload) = extract_and_assert(r, CmdSetBlock.opcode, 3, 1)
    assert ord(payload[0]) == 1, "FAILURE - SetBlock didn't return success"
  
    r = run_and_read(ser, CmdGetBlock, bank, addr, size, False, True, test_apid)
    try:
      (opcode, length, source, payload) = extract_and_assert(r, CmdGetBlock.opcode, size, 1)
      result = map(ord, payload)
      assert result == written, "FAILURE - SetBlock didn't successfully change registers"
    except:
      print r
      print written
      print "GetBlock returned %s, expected %s" % (readify(r), readify(written))
      raise
  
    r = run_and_read(ser, CmdSetBlock, bank, addr, initial, False, True, test_apid)
    (opcode, length, source, payload) = extract_and_assert(r, CmdSetBlock.opcode, 1, 1)
    assert ord(payload[0]) == 1, "FAILURE - SetBlock didn't return success"

    r = run_and_read(ser, CmdGetBlock, bank, addr, size, True, True, test_apid)
    check_crc(r)
    try:
      (opcode, length, source, payload) = extract_and_assert(r, CmdGetBlock.opcode, size + 2, 1)
      result = map(ord, payload[:-2])
      assert result == initial, "FAILURE - SetBlock didn't successfully reset registers"
    except:
      print "GetBlock returned %s, expected %s" % (readify(r), readify(initial))
      raise
  
print "> SUCCESS"

#   Reset command resets processor
#   Further commands are possible after reset
#   Active bank changes after Reload
#   Get Registers command for Bank 0 is accepted
#   Set Registers command for Bank 0 is rejected
#   Get and Set Registers commands for invalid Banks are rejected
#   Get Registers command results match properly before and after ReloadConfig
#   ReloadConfig causes UART and RF to reconfigure with new values
#   ReloadConfig causes proper resets of memory pools and mailboxes, including changing NLMaxLength
#   Get and Set Mission Time commands agree to within (X) seconds at least twice (two different epochs)
#   New firmware can be uploaded, verified, and installed
#   Uploaded data with invalid address is rejected
#   Invalid command formats cause otherwise correct packets to be rejected
#   All Set commands correctly return SUCCESS
#   Cancel and Install firmware commands correctly return SUCCESS

print "[3] Get Register and Get Block commands for bank 0 are accepted. Set Register and Set Block commands for bank 0 are rejected. Get and Set Registers and Bank commands for invalid banks are rejected."
addresses = range(0, 5)
r = run_and_read(ser, CmdGetRegs, 0, addresses, False, True, test_apid)
try:
    (opcode, length, source, payload) = extract_and_assert(r, CmdGetRegs.opcode, 4, 1)
except:
    print r
    print "FAILURE - GetRegs for bank 0 returned %s" % readify(r)
    raise
initial = map(ord, payload)

r = run_and_read(ser, CmdSetRegs, 0, addresses, initial, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetRegs.opcode, 2, 1)
try:
    assert len(payload) == 2, "FAILURE - SetRegs for bank 0 didn't return FAILURE"
except:
    print "SetRegs returned %s" % readify(r)
    raise
  
r = run_and_read(ser, CmdGetBlock, 0, 0, 4, False, True, test_apid)
try:
  (opcode, length, source, payload) = extract_and_assert(r, CmdGetBlock.opcode, 4, 1)
except:
  print "GetBlock returned %s" % (readify(r))
  raise
initial = map(ord, payload)

r = run_and_read(ser, CmdSetBlock, 0, 0, initial, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetBlock.opcode, 2, 1)
try:
    assert len(payload) == 2, "FAILURE - SetBlock for bank 0 didn't return FAILURE"
except:
    print "SetBlock returned %s" % (readify(r))
    raise

r = run_and_read(ser, CmdGetRegs, 7, addresses, False, True, test_apid)
try:
    (opcode, length, source, payload) = extract_and_assert(r, CmdGetRegs.opcode, 0, 1)
except:
    print "FAILURE - GetRegs for bank 7 didn't return FAILURE (%s)" % readify(r)
    raise
initial = map(ord, payload)

r = run_and_read(ser, CmdSetRegs, 7, addresses, initial, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetRegs.opcode, 2, 1)
try:
    assert len(payload) == 2, "FAILURE - SetRegs for bank 7 didn't return FAILURE"
except:
    print "SetRegs returned %s" % readify(r)
    raise
  
r = run_and_read(ser, CmdGetBlock, 7, 0, 4, False, True, test_apid)
try:
  (opcode, length, source, payload) = extract_and_assert(r, CmdGetBlock.opcode, 0, 1)
except:
  print "FAILURE - GetBlock for bank 7 didn't return FAILURE (%s)" % (readify(r))
  raise
initial = map(ord, payload)

r = run_and_read(ser, CmdSetBlock, 7, 0, initial, True, True, test_apid)
check_crc(r)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetBlock.opcode, 2, 1)
try:
    assert len(payload) == 2, "FAILURE - SetBlock for bank 7 didn't return FAILURE"
except:
    print "SetBlock returned %s" % (readify(r))
    raise

print "> SUCCESS"


