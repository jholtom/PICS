#!/usr/bin/env python

from ely_util import *
from cmds import *
import time
import sys

# RF
payload = '\x00'*1000
apid = 0x0002

pkt = slip.stuff(spp.packetize(payload, apid, tc=False))

r = run_and_read(ser, CmdSetTXFreq, 903000000, False, True, test_apid)
(opcode, length, source, payload) = extract_and_assert(r, CmdSetTXFreq.opcode, 1, 1)
try:
  assert ord(payload[0]) == 1, "%s didn't return success" % CmdSetTXFreq
except:
  print "%s returned %s" % (CmdSetTXFreq, readify(payload))
  raise
  
#for value in range(60, 131):
for value in [115]:
  r = run_and_read(ser, CmdSetTXPow, value, False, True, test_apid)
  (opcode, length, source, payload) = extract_and_assert(r, CmdSetTXPow.opcode, 1, 1)
  try:
    assert ord(payload[0]) == 1, "%s didn't return success" % CmdSetTXPow
  except:
    print "%s returned %s" % (CmdSetTXPow, readify(payload))
    raise

  time.sleep(10)
  print "Register %d" % value
  ser.write(pkt)
  

