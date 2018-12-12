#!/usr/bin/env python

from all_cmds import *

# RF
payload = '\x00'*1000
apid = 0x0002

pkt = stuff(packetize(payload, apid, tc=False))

ser.write(pkt)

