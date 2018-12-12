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
#  Storage.
#  Retrieval (no filter).
#  Retrieval (index filter).
#  Retrieval (timestamp filter).
#  Retrieval (both filters).
#  Fill partial packets.
#  Non-overlap with Reg, FW.
#  Store while retrieving.
#  UART while retrieving.
#  RF while retrieving.

def test_0_stub():
    pass

def test_1_storage():
  print "[1] Storage. Retrieval (no filter)."
  reply = False
  payloads = []
  for index in range(0, 256):
    crc = random.getrandbits(1)
    reply = not reply
    payload = (bytearray(random.getrandbits(8) for _ in xrange(251)))
    payloads.append(payload)
    print readify(payload)
    if reply:
      print "Reply"
      if crc:
        length = 3
      else:
        length = 1
      r = run_and_read(ser, CmdStoreTelem, payload, crc, reply, test_apid)
      try:
        (opcode, length, source, payload) = extract_and_assert(r, CmdStoreTelem.opcode, length, 1)
      except:
        print readify(payload)
        print readify(r)
        print crc
        raise
    else:
      print "No Reply"
      run(ser, CmdStoreTelem, payload, crc, False, test_apid)
      time.sleep(1)
  print "> SUCCESS"
  
  time.sleep(5)
  
  r = run(ser, CmdGetTelem, True)
  for index in range(0, 256):
    value = blocking_read(ser)
    print readify(value)
    (apid, seqno, length, telem) = spp.unpacketize(slip.unstuff(value))
    print(length)
    assert length == 255
    assert apid == elysium_apid
    if telem[5:] != payloads[index]:
        print index
        print readify(telem)
        print readify(payloads[index])
        raise

functions.append(test_0_stub)
functions.append(test_1_storage)

if __name__ == "__main__":
  for func in functions:
    func()
