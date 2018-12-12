#!/usr/bin/env python

import serial
import sys
from spp import *
from slip import *
from cmds import *
import crcmod

crc_x25 = crcmod.predefined.mkPredefinedCrcFun('x-25')

ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=5)

test_apid = 2
elysium_apid = 1

def blocking_read(ser):
    try:
        result = ''
        while True:
            c = ser.read()
            result += c
            if c == '\xc0':
                return result
    except:
        e = sys.exc_info()[0] # last exception
        print e
        return result
    
def run(ser, cmd, *args):
    ser.write(stuff(packetize(cmd(*args), elysium_apid)))
    
def run_and_read(ser, cmd, *args):
    ser.write(stuff(packetize(cmd(*args), elysium_apid)))
    return blocking_read(ser)

def extract_and_assert(reply, opcode, length, source, payload = None):
    (_opcode, _length, _source, _payload) = extract(unpacketize(unstuff(reply))[3])
    assert _opcode == opcode, "opcode error in response"
    assert _length == length, "length error in response"
    assert _source == source, "APID error in response"
    if payload:
        assert _payload == payload, "payload error in response"
    return (_opcode, _length, _source, _payload)

def check_crc(buffer):
    buff = unpacketize(unstuff(buffer))[3]
    crc = crc_x25(buff[:-2])
    assert crc >> 8 == ord(buff[-2]), "CRC error"
    assert crc & 0xFF == ord(buff[-1]), "CRC error"
    
if __name__ == '__main__':
    from gpo_test import *
    
    from registers_test import *







