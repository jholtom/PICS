#!/usr/bin/env python

from ely_util import *
from cmds import *
import time
import sys
import threading
from spp import *
from slip import *

done = False

# Chan
def read_from_port(ser):
    result = ''
    while not done:
        c = ser.read() 
        if c:
          result += c
          if c == '\xc0':
              rx_payload = unpacketize(unstuff(result))[3]
              # Do something with rx_payload here
              print readify(rx_payload)
              result = ''
  
# RF
def test_main(ser):
  global done
  payload = '\x00'*1000
  apid = 0x0002
  
  pkt = slip.stuff(spp.packetize(payload, apid, tc=False))
  
  r = run(ser, CmdSetTXFreq, 903000000, False, False, test_apid)
  time.sleep(1)
  r = run(ser, CmdChannelSub, 1, [0x41], False, False, test_apid)
  time.sleep(1)
    
  for value in range(60, 131):
    r = run(ser, CmdSetTXPow, value, False, False, test_apid)
    time.sleep(1)
  
    print "configged"
    time.sleep(10)
    print "Commanded %d" % value
    for i in range(5):
      print i
      ser.write(pkt)
      time.sleep(20)
    time.sleep(10)
    
  done = True
  

read = threading.Thread(name="reader-thread", target=read_from_port, args=(ser,))
read.setDaemon(True)
write = threading.Thread(name="writer-thread", target=test_main, args=(ser,))
write.setDaemon(True)

print "Sending now..."

read.start()
write.start()

read.join()
write.join()

time.sleep(2)
