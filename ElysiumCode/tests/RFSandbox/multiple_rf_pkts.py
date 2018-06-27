#!/usr/bin/env python

from all_cmds import *

o = open('sent.txt', 'wb')
# RF
payload = '\xDB'*1024
apid = 0x03FE

pkt = stuff(packetize(payload, apid, tc=False))

readable = map(hex, bytearray(pkt))
o.write(str(readable) + '\n')
o.flush

ser.write(pkt)

time.sleep(1)

payload = '\x24'*512
apid = 0x0001

pkt = stuff(packetize(payload, apid, tc=False))

readable = map(hex, bytearray(pkt))
o.write(str(readable) + '\n')
o.flush

ser.write(pkt)

