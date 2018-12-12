#!/usr/bin/env python

import serial
import random
import time
from spp import *
import threading

s = serial.Serial('/dev/ttyACM1', 115200, timeout=0)
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
                
              # Do something with rx_payload here
              result = ''
    print "Done reading..."


def test_main(ser):
    global done
    i = 0
    while not done:
        if i == 64:
            done = True
            continue
        i += 1
        apid = random.getrandbits(11)
        pfield = random.getrandbits(1)
        timestamp = random.getrandbits(1)
        length = random.getrandbits(10)
        
        if (length > 2036):
            length = 2036
        if (length == 0):
            length = 1
        
        # Create payload here
        payload = str(bytearray(random.getrandbits(8) for _ in xrange(length)))
        
        packet = packetize(payload, apid, timestamp, pfield)
    
        readable = map(hex, bytearray(packet))
        uso.write(str(readable) + '\n')
        uso.flush()
            
        stuffed_packet = stuff(packet)
        
        readable = map(hex, bytearray(stuffed_packet))
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
