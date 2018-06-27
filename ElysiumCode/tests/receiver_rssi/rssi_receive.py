#!/usr/bin/env python

import serial
import struct

s = serial.Serial('/dev/ttyACM1', 38400)

while True:
    raw_input("Press Enter to read RSSI")
    
    s.write('l');

    rssi = struct.unpack('b', s.read())[0]
    
    print "Received an RSSI reading of %d" % rssi
