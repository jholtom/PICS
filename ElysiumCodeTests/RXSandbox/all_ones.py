#!/usr/bin/env python

from all_cmds import *
import random

# RF
payload = str(bytearray([0xff]) * 1500)
#payload += str(bytearray([random.getrandbits(8)]))
apid = 0x03ff

pkt = stuff(packetize(payload, apid, tc=False))

readable = map(hex, bytearray(pkt))

print readable

ser.write(pkt)

