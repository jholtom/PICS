#!/usr/bin/env python

import struct
import time

# PVN 0
# Packet Type 1
# SHF dual random (timestamp, pfield)
# APID random
# Sequence Flags 11
# Sequence Count incrementing from 0 to 16383
# Length random 0-65535
# if pcode it's 0x2C
# if timestamp, unix time

def stuff(s):
    return s.replace('\xDB', '\xDB\xDD').replace('\xC0', '\xDB\xDC') + '\xc0'

def unstuff(s):
    return s.replace('\xC0', '').replace('\xDB\xDC', '\xC0').replace('\xDB\xDD', '\xDB')

def unpacketize(payload, pfield=False):
    (header0, header1, length) = struct.unpack('!HHH', payload[:6])
    if header0 & 0x0800:
        # timestamp
        if pfield:
            # also pfield
            (pfield, timestamp) = struct.unpack('!BL', payload[6:11])
            payload = payload[11:]
        else:
            (timestamp) = struct.unpack('!L', payload[6:10])
            payload = payload[10:]
    else:
        payload = payload[6:]
        
    apid = header0 & 0x03FF
    seqno = header1 & 0x3FFF
    # TODO check the length
    # TODO return the pfield
    return (apid, seqno, length, payload)
    
def packetize(payload, apid, tc=True, timestamp=False, pfield=False):
    if not hasattr(packetize, "i"):
        packetize.i = 1
    length = len(payload)
    if timestamp:
        length += 4
        if pfield:
            length += 1
    
    
    header0 = apid
    if tc:
        header0 |= 0x1000
    header1 = 0xC000 | (packetize.i+1)
    header2 = length - 1
    if timestamp:
        header0 |= 0x0800
        timestamp = int(time.time())
        if pfield:
            pfield = 0x2C
        
    
    if timestamp:
        if pfield:
            packet = struct.pack('!HHHBL%ds' % (length - 5), header0, header1, header2, pfield, timestamp, payload)
        else:
            packet = struct.pack('!HHHL%ds' % (length - 4), header0, header1, header2, timestamp, payload)
    else:
        packet = struct.pack('!HHH%ds' % length, header0, header1, header2, payload)
        
    return packet
        
