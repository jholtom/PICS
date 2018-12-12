#!/usr/bin/env python

from all_cmds import *

# RF
payload = '\xff'*1024
apid = 0x03ff

pkt = stuff(packetize(payload, apid, tc=False))

ser.write(pkt)

