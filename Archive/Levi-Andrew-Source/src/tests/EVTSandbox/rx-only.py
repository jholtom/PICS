#!/usr/bin/env python

import serial
import random
import time
from spp import *
import threading

s = serial.Serial('/dev/ttyACM1', 115200, timeout=0)
o = open('received.txt', 'wb')
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
              readable = map(hex, bytearray(rx_payload))
              o.write(str(readable) + '\n')
              o.flush()
    print "Done reading..."


read = threading.Thread(name="reader-thread", target=read_from_port, args=(s,))
read.setDaemon(True)

read.start()

raw_input()
print "Exiting...."

done = True
read.join()
