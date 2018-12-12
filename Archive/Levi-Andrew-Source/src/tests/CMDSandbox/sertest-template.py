#!/usr/bin/env python

import serial
import random
import time
from spp import *
import threading

s = serial.Serial('/dev/ttyACM1', 115200, timeout=0)
done = False

def read_from_port(ser):
    result = ''
    while not done:
        c = ser.read() 
        if c:
          result += c
          if c == '\xc0':
              rx_payload = unpacketize(unstuff(result))[3]
              # Do something with rx_payload here
              result = ''
    print "Done reading..."


def test_main(ser):
    while not done:
        apid = random.getrandbits(11)
        timestamp = random.getrandbits(1)
        length = random.getrandbits(12)
        
        if (length > 2036):
            length = 2036
        
        # Create payload here
        
        packet = packetize(payload, apid, timestamp)
            
        stuffed_packet = stuff(packet)
        
        s.write(stuffed_packet)
        
        time.sleep(2)
        
    print "Done writing..."

    

read = threading.Thread(name="reader-thread", target=read_from_port, args=(s,))
read.setDaemon(True)
write = threading.Thread(name="writer-thread", target=test_main, args=(s,))
write.setDaemon(True)

read.start()
write.start()

raw_input()
print "Exiting...."

done = True
read.join()
write.join()
