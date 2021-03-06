#!/usr/bin/env python

import random
import spp
import slip
import math
import itertools
import serial
import sys
import time

from ely_util import *
from cmds import *

functions = []

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

def test_0_init():
  f = open('registers.bin', 'rb')
  defaults = map(ord, list(f.read()))
  f.close()
  
  print "[0] Initializing banks to default values"
  # Start by turning off error reporting
  r = run_and_read(ser, CmdGetErr, False, True, test_apid)
  (opcode, length, source, payload) = extract_and_assert(r, CmdGetErr.opcode, 1, 1)
  mask = ord(payload)
  run_and_read(ser, CmdSetErr, 0, False, True, test_apid)
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
      
  # Restore error settings
  run_and_read(ser, CmdSetErr, mask, False, True, test_apid)
  print "> SUCCESS"

functions.append(test_0_init)
  
def test_1_get_set():
  print "[1] Get and Set Registers commands match for at least 2 changes on Core, NL, and DLL registers"
  for bank in range(1, 5):
    addresses = [0x01, 0x05, 0x84, 0xc8]
    r = run_and_read(ser, CmdGetRegs, bank, addresses, False, True, test_apid)
    try:
        (opcode, length, source, payload) = extract_and_assert(r, CmdGetRegs.opcode, 4, 1)
    except:
        print readify(r)
        raise
    initial = map(ord, payload)
    
    print "Retrieved %s" % readify(initial)
    
    written = map(lambda x: x-1, initial)
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
    
    print "Retrieved %s" % readify(result)
    
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
  
functions.append(test_1_get_set)

def test_2_block():
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
    
      print "Retrieved %s" % readify(initial)
    
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
  
functions.append(test_2_block)

def test_3_banks():
  print "[3] Get Register and Get Block commands for bank 0 are accepted. Set Register and Set Block commands for bank 0 are rejected. Get and Set Registers and Bank commands for invalid banks are rejected."
  addresses = range(0, 4)
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
  
  r = run_and_read(ser, CmdSetRegs, 7, addresses, initial, True, True, test_apid)
  try:
      check_crc(r)
      (opcode, length, source, payload) = extract_and_assert(r, CmdSetRegs.opcode, 2, 1)
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
  
functions.append(test_3_banks)

def test_4_reload():
  print "[4] Active bank changes after Reload. Get Registers command results match properly before and after ReloadConfig."
  r = run_and_read(ser, CmdGetBlock, 0, 0, 4, False, True, test_apid)
  try:
    (opcode, length, source, payload) = extract_and_assert(r, CmdGetBlock.opcode, 4, 1)
  except:
    print "GetBlock returned %s" % (readify(r))
    raise
  initial = map(ord, payload)
  
  r = run_and_read(ser, CmdGetActiveBank, False, True, test_apid)
  try:
    (opcode, length, source, payload) = extract_and_assert(r, CmdGetActiveBank.opcode, 1, 1)
    value = ord(payload[0])
    assert value in range(1, 5), "FAILURE - value error in GetActiveBank response"
  except:
    print r
    print "GetActiveBank returned %d" % value
    raise
  
  written = map(lambda x: x+1, initial)
  oldbank = value
  newbank = (value + 1)
  if newbank > 4:
    newbank = 1
  try:
      print written
      print newbank
      r = run_and_read(ser, CmdSetBlock, newbank, 0, written, True, True, test_apid)
      check_crc(r)
      (opcode, length, source, payload) = extract_and_assert(r, CmdSetBlock.opcode, 3, 1)
      assert ord(payload[0]) == 1, "FAILURE - SetBlock for bank %d didn't return SUCCESS" % newbank
  except:
      print "SetBlock returned %s" % (readify(r))
      raise
  
  run(ser, CmdReloadConfig, newbank)
  time.sleep(25)
  ser.flushInput()
  ser.flushOutput()
  
  r = run_and_read(ser, CmdGetActiveBank, False, True, test_apid)
  try:
    (opcode, length, source, payload) = extract_and_assert(r, CmdGetActiveBank.opcode, 1, 1)
    value = ord(payload[0])
    assert value == newbank, "FAILURE - ReloadConfig didn't successfully change active bank"
  except:
    print readify(r)
    print "GetActiveBank returned %d" % value
    raise
  
  r = run_and_read(ser, CmdGetBlock, 0, 0, 4, False, True, test_apid)
  try:
    (opcode, length, source, payload) = extract_and_assert(r, CmdGetBlock.opcode, 4, 1)
    final = map(ord, payload)
    assert final == written, "FAILURE - Bank 0 values after ReloadConfig don't match Bank %d values before ReloadConfig" % newbank
  except:
    print "GetBlock returned %s" % (readify(r))
    raise
  
  r = run_and_read(ser, CmdSetBlock, newbank, 0, initial, True, True, test_apid)
  check_crc(r)
  (opcode, length, source, payload) = extract_and_assert(r, CmdSetBlock.opcode, 3, 1)
  try:
      assert ord(payload[0]) == 1, "FAILURE - SetBlock for bank %d didn't return SUCCESS" % newbank
  except:
      print "SetBlock returned %s" % (readify(r))
      raise
  
  run(ser, CmdReloadConfig, oldbank)
  time.sleep(25)
  ser.flushInput()
  ser.flushOutput()
  
  try:
    r = run_and_read(ser, CmdGetActiveBank, False, True, test_apid)
    (opcode, length, source, payload) = extract_and_assert(r, CmdGetActiveBank.opcode, 1, 1)
    value = ord(payload[0])
    assert value == oldbank, "FAILURE - ReloadConfig didn't successfully change active bank"
  except:
    print readify(r)
    print "GetActiveBank returned %d" % value
    raise
  
  r = run_and_read(ser, CmdGetBlock, 0, 0, 4, False, True, test_apid)
  try:
    (opcode, length, source, payload) = extract_and_assert(r, CmdGetBlock.opcode, 4, 1)
    final = map(ord, payload)
    assert final == initial, "FAILURE - Bank 0 values after ReloadConfig don't match Bank %d values before ReloadConfig" % newbank
  except:
    print "GetBlock returned %s" % (readify(r))
    raise
  
  print "> SUCCESS"

functions.append(test_4_reload)
  
#   Reset command resets processor
#   Further commands are possible after reset
#   ReloadConfig to invalid bank is rejected
#   ReloadConfig causes UART and RF to reconfigure with new values
#   ReloadConfig causes proper resets of memory pools and mailboxes, including changing NLMaxLength
#   Get and Set Mission Time commands agree to within (X) seconds at least twice (two different epochs)
#   New firmware can be uploaded, verified, and installed
#   Uploaded data with invalid address is rejected
#   Invalid command formats cause otherwise correct packets to be rejected
#   Cancel and Install firmware commands correctly return SUCCESS


if __name__ == "__main__":
  for func in functions:
    func()
