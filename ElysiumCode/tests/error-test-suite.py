#!/usr/bin/env python

import random
import spp
import slip
import math
import itertools
import serial
import sys
import time

from cmds import *
from errs import *
from ely_util import *

functions = []

# List:
#   Errors are reported:
#       Register Value Clipping
#       Invalid Opcode
#       FCS Error
#       Unreported Command Failure
#       Subscription Overwritten (?)
#       UART Error (?)
#   Errors reported based on mask:
#       All valid priorities
#       Can set multiple reported priorities at once

def test_0_init():
  pass

functions.append(test_0_init)

def test_1_reporting():
  print "[1] Errors reports are sent and formatted properly"
  # Enable reporting of all errors regardless of priority
  r = run_and_read(ser, CmdGetErr, False, True, test_apid)
  (opcode, length, source, payload) = extract_and_assert(r, CmdGetErr.opcode, 1, 1)
  mask = ord(payload)
  run_and_read(ser, CmdSetErr, 0x1F, False, True, test_apid)

#       Register Value Clipping
  r = run_and_read(ser, CmdSetRegs, 1, [0x31], [0xFF], True, True, test_apid)
  try:
      (opcode, length, source, payload) = extract_and_assert(r, ErrRegClip, 0, 1)
  except:
      print readify(r)
      raise
  r = blocking_read(ser) # receive the valid reply which we do get
  
#       Invalid Opcode
  r = header(0xFF, 0, False, True, 1)
  ser.write(slip.stuff(spp.packetize(r, elysium_apid)))
  r = blocking_read(ser)
  try:
      (opcode, length, source, payload) = extract_and_assert(r, ErrInvalidOpcode, 0, 1)
  except:
      print readify(r)
      raise
  
#       FCS Error
  pkt = spp.packetize(CmdGetGPO(True, True, test_apid), elysium_apid)
  pkt = pkt[:-2] + chr(random.getrandbits(8)) + chr(random.getrandbits(8))
  ser.write(slip.stuff(pkt))
  exception = False
  r = blocking_read(ser)
  try:
      (opcode, length, source, payload) = extract_and_assert(r, ErrFCSError, 0, 1)
  except:
      print readify(r)
      raise
  
#       Unreported Command Failure
  r = run_and_read(ser, CmdSetTXFreq, 0, True, False, test_apid)
  try:
      (opcode, length, source, payload) = extract_and_assert(r, ErrCmdFailure, 0, 1)
  except:
      print readify(r)
      raise
  
#       UART Error (?)
#       Subscription Overwritten (?)

  # Restore error settings
  run_and_read(ser, CmdSetErr, mask, False, True, test_apid)
  
  print "> SUCCESS"

functions.append(test_1_reporting)

#   Errors reported based on mask:
#       All valid priorities
#       Can set multiple reported priorities at once
def test_2_priority():
  print "[2] Errors reports are sent based on priority correctly"
  # Save error settings and active bank
  r = run_and_read(ser, CmdGetErr, False, True, test_apid)
  (opcode, length, source, payload) = extract_and_assert(r, CmdGetErr.opcode, 1, 1)
  mask = ord(payload)
  r = run_and_read(ser, CmdGetActiveBank, False, True, test_apid)
  try:
    (opcode, length, source, payload) = extract_and_assert(r, CmdGetActiveBank.opcode, 1, 1)
    value = ord(payload[0])
    assert value in range(1, 5), "FAILURE - value error in GetActiveBank response"
  except:
    print r
    print "GetActiveBank returned %d" % value
    raise
  
  # DEBUG
  run_and_read(ser, CmdSetRegs, 1, [0x30], [0x01], True, True, test_apid)
  run(ser, CmdReloadConfig, 1)
  time.sleep(5)
  ser.flushInput()
  ser.flushOutput()
  run_and_read(ser, CmdSetErr, 0x01, False, True, test_apid)
  r = header(0xFF, 0, False, True, 1)
  ser.write(slip.stuff(spp.packetize(r, elysium_apid)))
  r = blocking_read(ser)
  try:
      (opcode, length, source, payload) = extract_and_assert(r, ErrInvalidOpcode, 0, 1)
  except:
      print readify(r)
      raise
  run_and_read(ser, CmdSetErr, 0x1E, False, True, test_apid)
  r = header(0xFF, 0, False, True, 1)
  ser.write(slip.stuff(spp.packetize(r, elysium_apid)))
  r = blocking_read(ser)
  try:
      assert r == ''
  except:
      print readify(r)
      raise
  
  # INFO
  run_and_read(ser, CmdSetRegs, 1, [0x30], [0x02], True, True, test_apid)
  run(ser, CmdReloadConfig, 1)
  time.sleep(5)
  ser.flushInput()
  ser.flushOutput()
  run_and_read(ser, CmdSetErr, 0x02, False, True, test_apid)
  r = header(0xFF, 0, False, True, 1)
  ser.write(slip.stuff(spp.packetize(r, elysium_apid)))
  r = blocking_read(ser)
  try:
      (opcode, length, source, payload) = extract_and_assert(r, ErrInvalidOpcode, 0, 1)
  except:
      print readify(r)
      raise
  run_and_read(ser, CmdSetErr, 0x1D, False, True, test_apid)
  r = header(0xFF, 0, False, True, 1)
  ser.write(slip.stuff(spp.packetize(r, elysium_apid)))
  r = blocking_read(ser)
  try:
      assert r == ''
  except:
      print readify(r)
      raise
  
  # WARNING
  run_and_read(ser, CmdSetRegs, 1, [0x30], [0x04], True, True, test_apid)
  run(ser, CmdReloadConfig, 1)
  time.sleep(5)
  ser.flushInput()
  ser.flushOutput()
  run_and_read(ser, CmdSetErr, 0x04, False, True, test_apid)
  r = header(0xFF, 0, False, True, 1)
  ser.write(slip.stuff(spp.packetize(r, elysium_apid)))
  r = blocking_read(ser)
  try:
      (opcode, length, source, payload) = extract_and_assert(r, ErrInvalidOpcode, 0, 1)
  except:
      print readify(r)
      raise
  run_and_read(ser, CmdSetErr, 0x1B, False, True, test_apid)
  r = header(0xFF, 0, False, True, 1)
  ser.write(slip.stuff(spp.packetize(r, elysium_apid)))
  r = blocking_read(ser)
  try:
      assert r == ''
  except:
      print readify(r)
      raise
  
  # ERROR
  run_and_read(ser, CmdSetRegs, 1, [0x30], [0x08], True, True, test_apid)
  run(ser, CmdReloadConfig, 1)
  time.sleep(5)
  ser.flushInput()
  ser.flushOutput()
  run_and_read(ser, CmdSetErr, 0x08, False, True, test_apid)
  r = header(0xFF, 0, False, True, 1)
  ser.write(slip.stuff(spp.packetize(r, elysium_apid)))
  r = blocking_read(ser)
  try:
      (opcode, length, source, payload) = extract_and_assert(r, ErrInvalidOpcode, 0, 1)
  except:
      print readify(r)
      raise
  run_and_read(ser, CmdSetErr, 0x17, False, True, test_apid)
  r = header(0xFF, 0, False, True, 1)
  ser.write(slip.stuff(spp.packetize(r, elysium_apid)))
  r = blocking_read(ser)
  try:
      assert r == ''
  except:
      print readify(r)
      raise
  
  ## CRITICAL
  #run_and_read(ser, CmdSetRegs, 1, [0x30], [0x10], True, True, test_apid)
  #run(ser, CmdReloadConfig, 1)
  #time.sleep(5)
  #ser.flushInput()
  #ser.flushOutput()
  #run_and_read(ser, CmdSetErr, 0x10, False, True, test_apid)
  #r = header(0xFF, 0, False, True, 1)
  #ser.write(slip.stuff(spp.packetize(r, elysium_apid)))
  #r = blocking_read(ser)
  #try:
  #    (opcode, length, source, payload) = extract_and_assert(r, ErrInvalidOpcode, 0, 1)
  #except:
  #    print readify(r)
  #    raise
  #run_and_read(ser, CmdSetErr, 0x0F, False, True, test_apid)
  #r = header(0xFF, 0, False, True, 1)
  #ser.write(slip.stuff(spp.packetize(r, elysium_apid)))
  #r = blocking_read(ser)
  #try:
  #    assert r == ''
  #except:
  #    print readify(r)
  #    raise
  
  # Restore error settings
  run_and_read(ser, CmdSetErr, mask, False, True, test_apid)
  
  print "> SUCCESS"
  
functions.append(test_2_priority)

if __name__ == "__main__":
  for func in functions:
    func()
