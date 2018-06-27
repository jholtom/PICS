#!/usr/bin/env python

import serial
import random
import time
from spp import *
from slip import *
import threading

from ely_util import *

s = serial.Serial('/dev/ttyUSB0', 115200, timeout=0)
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
    while not done:
        apid = elysium_apid
        timestamp = 0
        
        # Create payload here
        payload = 'ab' #* 252
        
        packet = packetize(payload, apid, timestamp)
        
        readable = map(hex, bytearray(packet))
        uso.write(str(readable) + '\n')
        uso.flush()
            
        stuffed_packet = stuff(packet)
        
        readable = map(hex, bytearray(stuffed_packet))
        readable.extend(['0xc0'])
        so.write(str(readable) + '\n')
        print (str(readable) + '\n')
        so.flush()
        
        s.write(stuffed_packet)
        
        time.sleep(2)
        
        done = True
        
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
