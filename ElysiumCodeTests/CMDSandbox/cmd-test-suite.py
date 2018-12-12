#!/usr/bin/env python

import random
import spp
import slip
import math
import itertools
import serial
import sys

from cmds import *
from ely_util import *

functions = []

# List:
#   Reset command resets processor
#   Further commands are possible after reset
#   Set and Get GPO commands match for at least 3 toggles
#   Setting GPO out of bounds is rejected
#   Active bank changes after Reload
#   Get and Set Registers commands match for at least 2 changes on Core, NL, and DLL registers
#   Get and Set Block commands match for at least 2 changes on Core, NL, and DLL registers
#   Get Registers command for Bank 0 is accepted
#   Set Registers command for Bank 0 is rejected
#   Get and Set Registers commands for invalid Banks are rejected
#   Get and Set commands match for at least 2 changes (TXFreq through Baud)
#   Set commands correctly reject invalid values (TXFreq through Baud)
#   Get Registers command results match properly before and after ReloadConfig
#   ReloadConfig causes UART and RF to reconfigure with new values
#   ReloadConfig causes proper resets of memory pools and mailboxes, including changing NLMaxLength
#   Get and Set Mission Time commands agree to within (X) seconds at least twice (two different epochs)
#   New firmware can be uploaded, verified, and installed
#   Uploaded data with invalid address is rejected
#   Invalid CRCs cause otherwise correct packets to be rejected
#   Invalid headers cause otherwise correct packets to be rejected
#   Invalid command formats cause otherwise correct packets to be rejected
#   All Set commands correctly return SUCCESS
#   Cancel and Install firmware commands correctly return SUCCESS

# Test suite

def test_0_init():
  pass

functions.append(test_0_init)

def test_1_gpo():
  print "[1] Set and Get GPO commands match for at least 3 toggles"
  r = run_and_read(ser, CmdGetGPO, False, True, test_apid)
  print(readify(r))
  print(readify(slip.unstuff(r)))
  (opcode, length, source, payload) = extract_and_assert(r, CmdGetGPO.opcode, 1, 1)
  value = ord(payload[0])
  try:
    assert value == 1 or value == 0, "FAILURE - GetGPO returned invalid value"
  except:
    print "GetGPO returned %d" % value
    raise
  
  for i in range(0, 3):
    run(ser, CmdSetGPO, value ^ 1, True)
    
    r = run_and_read(ser, CmdGetGPO, True, True, test_apid)
    check_crc(r)
    (opcode, length, source, payload) = extract_and_assert(r, CmdGetGPO.opcode, 3, 1)
    try:
      assert ord(payload[0]) == value ^ 1, "FAILURE - SetGPO failed to change value"
    except:
      print "GetGPO returned %d" % value
      raise
    value = ord(payload[0])
  print "> SUCCESS"
  
functions.append(test_1_gpo)

def test_2_gpo_invalid():
  print "[2] Setting GPO out of bounds is rejected"
  r = run_and_read(ser, CmdGetGPO, False, True, test_apid)
  (opcode, length, source, payload) = extract_and_assert(r, CmdGetGPO.opcode, 1, 1)
  value = ord(payload[0])
  try:
    assert value == 1 or value == 0, "FAILURE - GetGPO returned invalid value"
  except:
    print "GetGPO returned %d" % value
    raise
  
  r = run_and_read(ser, CmdSetGPO, 7, True, True, test_apid)
  check_crc(r)
  (opcode, length, source, payload) = extract_and_assert(r, CmdSetGPO.opcode, 2, 1)
  try:
    assert len(payload) == 2, "FAILURE - invalid SetGPO returned other than failure"
  except:
    print "SetGPO returned %s" & readify(payload)
    raise
  
  r = run_and_read(ser, CmdGetGPO, True, True, test_apid)
  check_crc(r)
  (opcode, length, source, payload) = extract_and_assert(r, CmdGetGPO.opcode, 3, 1)
  try:
    assert ord(payload[0]) == value, "FAILURE - invalid SetGPO command modified GPO state"
  except:
    print "New GPO state is %d" % ord(payload[0])
    raise
  print "> SUCCESS\n" 
  
functions.append(test_2_gpo_invalid)

def test_3_get_set():
  print "[3] Get and Set commands match for at least 2 changes (TXFreq through Baud). All Set commands correctly return SUCCESS. Set commands correctly reject invalid values."
  
  for (getcmd, setcmd) in [
      (CmdGetTXFreq, CmdSetTXFreq), (CmdGetRXFreq, CmdSetRXFreq), 
      (CmdGetTXRate, CmdSetTXRate), (CmdGetRXRate, CmdSetRXRate), 
      (CmdGetTXDev, CmdSetTXDev), (CmdGetRXDev, CmdSetRXDev), 
      (CmdGetBaud, CmdSetBaud)]:
    r = run_and_read(ser, getcmd, False, True, test_apid)
    (opcode, length, source, payload) = extract_and_assert(r, getcmd.opcode, 4, 1)
    value = struct.unpack('!I', payload)[0]
    
    written = value - 10
    run(ser, setcmd, value - 10, True)
    
    r = run_and_read(ser, getcmd, True, True, test_apid)
    check_crc(r)
    (opcode, length, source, payload) = extract_and_assert(r, getcmd.opcode, 6, 1)
    newval = struct.unpack('!I', payload[:4])[0]
    try:
      assert newval == value - 10, "FAILURE - %s did not successfully change value" % setcmd
    except:
      print "%s returned %d, expected %d" % (getcmd, newval, value-10)
      raise
    
    r = run_and_read(ser, setcmd, 0xffffffff, True, True, test_apid)
    check_crc(r)
    (opcode, length, source, payload) = extract_and_assert(r, setcmd.opcode, 2, 1)
      
    r = run_and_read(ser, getcmd, False, True, test_apid)
    (opcode, length, source, payload) = extract_and_assert(r, getcmd.opcode, 4, 1)
    newval = struct.unpack('!I', payload)[0]
    try:
      assert newval == value - 10, "FAILURE - invalid %s changed value" % setcmd
    except:
      print "%s returned %d, expected %d" % (getcmd, newval, value-10)
      raise
    
    r = run_and_read(ser, setcmd, value, False, True, test_apid)
    (opcode, length, source, payload) = extract_and_assert(r, setcmd.opcode, 1, 1)
    try:
      assert ord(payload[0]) == 1, "%s didn't return success" % setcmd
    except:
      print "%s returned %s" % (setcmd, readify(payload))
      raise
    
    r = run_and_read(ser, getcmd, True, True, test_apid)
    check_crc(r)
    (opcode, length, source, payload) = extract_and_assert(r, getcmd.opcode, 6, 1)
    newval = struct.unpack('!I', payload[:4])[0]
    try:
      assert newval == value, "FAILURE - %s did not successfully reset value" % setcmd
    except:
      print "%s returned %d, expected %d" % (getcmd, newval, value-10)
      raise
    
#  r = run_and_read(ser, CmdGetTXPow, False, True, test_apid)
#  (opcode, length, source, payload) = extract_and_assert(r, CmdGetTXPow.opcode, 1, 1)
#  value = struct.unpack('!b', payload)[0]
#  
#  written = value - 10
#  run(ser, CmdSetTXPow, value - 10, True)
#  
#  r = run_and_read(ser, CmdGetTXPow, True, True, test_apid)
#  check_crc(r)
#  (opcode, length, source, payload) = extract_and_assert(r, CmdGetTXPow.opcode, 3, 1)
#  newval = struct.unpack('!b', payload[:1])[0]
#  try:
#    assert newval == value - 10, "FAILURE - %s did not successfully change value" % CmdSetTXPow
#  except:
#    print "%s returned %d, expected %d" % (CmdGetTXPow, newval, value-10)
#    raise
#    
#  r = run_and_read(ser, CmdSetTXPow, 127, True, True, test_apid)
#  check_crc(r)
#  (opcode, length, source, payload) = extract_and_assert(r, CmdSetTXPow.opcode, 2, 1)
#    
#  r = run_and_read(ser, CmdGetTXPow, False, True, test_apid)
#  (opcode, length, source, payload) = extract_and_assert(r, CmdGetTXPow.opcode, 1, 1)
#  newval = struct.unpack('!b', payload)[0]
#  try:
#    assert newval == value - 10, "FAILURE - invalid %s changed value" % CmdGetTXPow
#  except:
#    print "%s returned %d, expected %d" % (CmdGetTXPow, newval, value-10)
#    raise
#    
#  r = run_and_read(ser, CmdSetTXPow, value, False, True, test_apid)
#  (opcode, length, source, payload) = extract_and_assert(r, CmdSetTXPow.opcode, 1, 1)
#  try:
#    assert ord(payload[0]) == 1, "%s didn't return success" % CmdSetTXPow
#  except:
#    print "%s returned %s" % (CmdSetTXPow, readify(payload))
#    raise
#  
#  r = run_and_read(ser, CmdGetTXPow, True, True, test_apid)
#  check_crc(r)
#  (opcode, length, source, payload) = extract_and_assert(r, CmdGetTXPow.opcode, 3, 1)
#  newval = struct.unpack('!b', payload[:1])[0]
#  try:
#    assert newval == value, "FAILURE - %s did not successfully reset value" % CmdSetTXPow
#  except:
#    print "%s returned %d, expected %d" % (CmdGetTXPow, newval, value-10)
#    raise
    
  print "> SUCCESS"

functions.append(test_3_get_set)

#   Reset command resets processor
#   Further commands are possible after reset
#   Get and Set Mission Time commands agree to within (X) seconds at least twice (two different epochs)
#   Invalid command formats cause otherwise correct packets to be rejected

def test_4_invalid():
  print "[4] Invalid CRCs cause otherwise correct packets to be rejected. Invalid headers cause otherwise correct packets to be rejected."
  # Start by turning off event reporting
  r = run_and_read(ser, CmdGetErr, False, True, test_apid)
  (opcode, length, source, payload) = extract_and_assert(r, CmdGetErr.opcode, 1, 1)
  mask = ord(payload)
  run_and_read(ser, CmdSetErr, 0, False, True, test_apid)
    
  pkt = spp.packetize(CmdGetGPO(True, True, test_apid), elysium_apid)
  pkt = pkt[:-2] + str(random.getrandbits(8)) + str(random.getrandbits(8))
  ser.write(slip.stuff(pkt))
  exception = False
  r = blocking_read(ser)
  try:
    assert r == '', "FAILURE - Elysium responded to command packet with invalid CRC"
  except:
    print "Returned %s" % readify(r)
    raise
  
  pkt = spp.packetize(CmdGetGPO(True, True, test_apid), elysium_apid)
  pkt = chr(ord(pkt[0]) | 0xE0) + pkt[1:]
  ser.write(slip.stuff(pkt))
  exception = False
  r = blocking_read(ser)
  try:
    assert r == '', "FAILURE - Elysium responded to packet with invalid Packet Version Number"
  except:
    print "Returned %s" % readify(r)
    raise
  
  # Restore error settings
  run_and_read(ser, CmdSetErr, mask, False, True, test_apid)
  
  print "> SUCCESS"
  
functions.append(test_4_invalid)

if __name__ == "__main__":
  for func in functions:
    func()
