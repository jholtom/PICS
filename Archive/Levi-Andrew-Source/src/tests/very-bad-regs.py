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
  defaults = [random.randint(0, 255) for i in xrange(256)]
  defaults[0x22] = 0
  defaults[0x23] = 0xC2
  defaults[0x24] = 1
  defaults[0x25] = 0
  defaults[0x21] = 59
  defaults[0x50] = 1
  defaults[0x51] = 0
  defaults[0x86] = 0
  f.close()
  
  print "[0] Initializing banks to RANDOM values"
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
      
  # load a random bank
  run(ser, CmdReloadConfig, random.randint(1, 4))
  time.sleep(25)
  ser.flushInput()
  ser.flushOutput()
  print "> SUCCESS"

functions.append(test_0_init)
  
def test_1_nominal():
    print "[1] Receive a packets. Transmit a packets."
    for i in range(0, 1):
      pkt = rand_packet(apid = elysium_apid, tc = False, length = None, maxlength=2036)
      ser.write(slip.stuff(pkt))
      result = slip.unstuff(blocking_read(ser))
      try:
        assert ord(result[0]) & ~0x10 == ord(pkt[0]), "FAILURE - sent and received packets not equal"
        assert result[1:] == pkt[1:], "FAILURE - sent and received packets not equal"
      except:
        print "result is: " + readify(result)
        print "pkt is: " + readify(pkt)
        raise
      print i
    print "> SUCCESS\n" 

functions.append(test_1_nominal)


if __name__ == "__main__":
  for func in functions:
    func()
