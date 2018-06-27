#!/usr/bin/env python

import serial
import random
import time
from spp import *
import threading

from ely_util import *

done = False

def read_from_port(ser):
    out = open('extra-garbage.txt', 'wb')
    result = ''
    while not done:
        c = ser.read() 
        if c:
          result += c
          if c == '\xc0':
              rx_payload = unpacketize(unstuff(result))[3]
              out.write(rx_payload)
              out.flush()
              result = ''
    print "Done reading..."


def test_main(ser):
    while not done:
      for i in range(0, 512):
        length = random.getrandbits(16)
        
        payload = bytearray(random.getrandbits(8) for _ in xrange(size))
        ser.write(payload)
        
        time.sleep(2)
        
      print "Wrote 512 blocks of garbage."
      success = True
      for i in range(0, 3):
        print "Attempt " + str(i) + " to send and receive packets"
            
        success = True
        for i in range(0, 8):
          pkt = rand_packet(apid = elysium_apid, tc = False, length = None, maxlength=2036)
          ser.write(slip.stuff(pkt))
          result = slip.unstuff(blocking_read(ser))
          try:
            assert ord(result[0]) & ~0x10 == ord(pkt[0]), "FAILURE - sent and received packets not equal"
            assert result[1:] == pkt[1:], "FAILURE - sent and received packets not equal"
          except:
            print "> FAILURE"
            success = False
            break
        if success:
          print "> SUCCESS\n" 
          break;
        
      if not success:
        print "Failed to recover"
        done = True
    print "Done writing..."

    

read = threading.Thread(name="reader-thread", target=read_from_port, args=(ser,))
read.setDaemon(True)
write = threading.Thread(name="writer-thread", target=test_main, args=(ser,))
write.setDaemon(True)

read.start()
write.start()

raw_input()
print "Exiting...."

done = True
read.join()
write.join()
