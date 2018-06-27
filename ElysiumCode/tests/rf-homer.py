#!/usr/bin/env python

import serial
import random
import time
from spp import *
from slip import *
import threading

from ely_util import *

s = serial.Serial('/dev/ttyUSB1', 115200, timeout=0)

hi = open('iliad.txt', 'rb')
ho = open('iliad_transported.txt', 'wb')
rhso = open('stuffed_out', 'wb')
done = False

def read_from_port(ser, ho):
    result = ''
    while not done:
        c = ser.read() 
        if c:
          result += c
          if c == '\xc0':
              rx_payload = unpacketize(unstuff(result))[3]
              # Do something with rx_payload here
              ho.write(rx_payload)
              ho.flush()
              result = ''
    c = ser.read() 
    if c:
      result += c
      if c == '\xc0':
          rx_payload = unpacketize(unstuff(result))[3]
          # Do something with rx_payload here
          ho.write(rx_payload)
          ho.flush()
          result = ''
    c = ser.read() 
    if c:
      result += c
      if c == '\xc0':
          rx_payload = unpacketize(unstuff(result))[3]
          # Do something with rx_payload here
          ho.write(rx_payload)
          ho.flush()
          result = ''
    print "Done reading..."


def test_main(ser, hi):
    global done
    i = 0
    j = 0
    while not done:
        apid = random.getrandbits(11)
        timestamp = False
        length = random.getrandbits(12)
        
        if (length > 2036):
            length = 2036
        if (length == 0):
            length = 1
        
        # Create payload here
        payload = hi.read(length)
        
        if len(payload) < length:
            length = len(payload)
            done = True
        
        packet = packetize(payload, apid, tc=False, timestamp=timestamp)
            
        stuffed_packet = stuff(packet)
        
        s.write(stuffed_packet)
        
        rhso.write(stuffed_packet + '\n')
        rhso.flush()
        
        i += 1
        if i > 31:
            i = 0
            time.sleep(10)
            j += 1
            if j > 3:
                done = True
        
        
    print "Done writing..."

    

read = threading.Thread(name="reader-thread", target=read_from_port, args=(s,ho))
read.setDaemon(True)
write = threading.Thread(name="writer-thread", target=test_main, args=(s,hi))
write.setDaemon(True)

print "Sending now..."

read.start()
write.start()

read.join()
write.join()

time.sleep(2)
