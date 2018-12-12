#!/usr/bin/env python

import serial
import random
import time
from spp import *
import threading

s = serial.Serial('/dev/ttyACM2', 115200, timeout=0)
uso = open('unstuffed_out', 'wb')
so = open('stuffed_out', 'wb')
si = open('stuffed_in', 'wb')
usi = open('unstuffed_in', 'wb')
done = False

def read_from_port(ser):
    result = ''
    while not done:
        c = ser.read() 
        if c:
          result += c
          if c == '\xc0':
              readable = map(hex, bytearray(result))
              si.write(str(readable) + '\n')
              si.flush()

              unstuffed_result = unstuff(result)
              readable = map(hex, bytearray(unstuffed_result))
              usi.write(str(readable) + '\n')
              usi.flush()

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
        payload = '\xc0\xc0\xdb\xdb\xc0\xdb\xdb\xc0' * 200
        
        packet = packetize(payload, apid, timestamp)
            
        readable = map(hex, bytearray(packet))
        uso.write(str(readable) + '\n')
        uso.flush()

        stuffed_packet = stuff(packet)
        
        readable = map(hex, bytearray(stuffed_packet))
        readable.extend(['0xc0'])
        so.write(str(readable) + '\n')
        so.flush()

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
