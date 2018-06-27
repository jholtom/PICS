#!/usr/bin/env python

import spp
import slip
import serial
import cmds
import random

# Serial port setup
ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=5)

test_apid = 2
elysium_apid = 1

# Utility functions
def rand_packet(apid=None, timestamp=None, pfield=None, length=None, maxlength=None, tc=True):
  if apid is None:
    apid = random.getrandbits(11)
  if pfield is None:
    pfield = random.getrandbits(1)
  if timestamp is None:
    timestamp = random.getrandbits(1)
  if length is None:
    if maxlength is None:
      length = random.randint(1, 4085)
    else:
      length = random.randint(1, maxlength)
  
  # Create payload here
  payload = str(bytearray(random.getrandbits(8) for _ in xrange(length)))
  
  packet = spp.packetize(payload, apid, tc=tc, timestamp=timestamp, pfield=pfield)
  
  return packet

def blocking_read(ser):
    try:
        result = ''
        while True:
            result = ser.read_until(terminator='\xc0')
            return result
    except:
        e = sys.exc_info()[0] # last exception
        print e
        raise
      
def readify(buff):
  return str(map(hex, bytearray(buff)))
  
def run(ser, cmd, *args):
    ser.write(slip.stuff(spp.packetize(cmd(*args), elysium_apid)))
    
def run_and_read(ser, cmd, *args):
    ser.write(slip.stuff(spp.packetize(cmd(*args), elysium_apid)))
    return blocking_read(ser)

def extract_and_assert(reply, opcode, length, source, payload = None):
  (_opcode, _length, _source, _payload) = cmds.extract(spp.unpacketize(slip.unstuff(reply))[3])
  try:
    assert _opcode == opcode, "FAILURE - opcode error in response"
    assert _length == length, "FAILURE - length error in response"
    assert _source == source, "FAILURE - APID error in response"
    if payload:
        assert _payload == payload, "FAILURE - payload error in response"
  except:
    print "Expected opcode is %s, received %s" % (str(opcode), str(_opcode))
    print "Expected length is %s, received %s" % (str(length), str(_length))
    print "Expected source is %s, received %s" % (str(source), str(_source))
    if payload:
      print "Expected payload is %s, received %s" % (payload, payload)
    raise  
  return (_opcode, _length, _source, _payload)

def check_crc(buffer):
    buff = spp.unpacketize(slip.unstuff(buffer))[3]
    crc = cmds.crc_x25(buff[:-2])
    assert crc >> 8 == ord(buff[-2]), "FAILURE - CRC error"
    assert crc & 0xFF == ord(buff[-1]), "FAILURE - CRC error"
    
