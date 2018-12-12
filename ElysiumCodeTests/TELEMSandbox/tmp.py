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
#  Fill partial packets.
#  Retrieval (no filter).
#  Retrieval (index filter).
#  Retrieval (timestamp filter).
#  Retrieval (both filters).
#  Non-overlap with Reg, FW.
#  Store while retrieving.
#  UART while retrieving.
#  RF while retrieving.

def test_1_storage():
  print "[1] Storage"
  reply = True
  crc = random.getrandbits(1)
  payload = (bytearray(random.getrandbits(8) for _ in xrange(251)))
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
    
  print crc
  print "> SUCCESS"
  
functions.append(test_1_storage)

if __name__ == "__main__":
  for func in functions:
    func()
