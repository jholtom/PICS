#!/usr/bin/env python

import random
import spp
import slip
import math
import itertools
import serial
import sys
import time

from cmds import *
from ely_util import *

functions = []

# List:
#   Receive >4 packets
#   Receive minimum size packet (7 bytes)
#   Receive maximum size packet (4096 bytes)
#   Receive packet containing timestamp
#   Receive packet containing p-field
#   Reject malformed packets
#     > max length
#     > specified length (header + framing mismatch)
#     malformed header (fails elyNLValidate)
#   Recover from rejecting invalid packets
#   Receive packet with >2 consecutive escape sequences, in all permutations
#   Receive packet with no escape sequences
#   Receive packet wile transmitting
#   Transmit >4 packets
#   Transmit minimum size packet (7 bytes)
#   Transmit maximum size packet (2048 bytes)
#   Transmit packet containing timestamp
#   Transmit packet containing p-field
#   Transmit packet with >2 consecutive escape sequences, in all permutations
#   Transmit packet with no escape sequences
#   Transmit packet wile receiving

#   Change config
#   Change config while receiving
#   Reset while transmitting and successfully retransmit

log = open("test.log", "w")

def test_0_init():
    pass

functions.append(test_0_init)

def test_1_nominal():
    print "[1] Receive > 4 packets. Transmit > 4 packets."
    for i in range(0, 8):
      pkt = rand_packet(apid = elysium_apid, tc = False, length = None, maxlength=2036)
      ser.write(slip.stuff(pkt))
      result = slip.unstuff(blocking_read(ser))
      try:
        assert ord(result[0]) & ~0x10 == ord(pkt[0]), "FAILURE - sent and received packets not equal"
        assert result[1:] == pkt[1:], "FAILURE - sent and received packets not equal"
      except:
        print "result is: " + readify(result)
        print "pkt is: " + readify(pkt)
        raise
      print i
    print "> SUCCESS\n" 

functions.append(test_1_nominal)

def test_2_min():
    print "[2] Receive 7-byte packet. Transmit 7-byte packet."
    pkt = rand_packet(apid = elysium_apid, tc = False, length = 1, timestamp = 1, pfield = 1)
    ser.write(slip.stuff(pkt))
    result = slip.unstuff(blocking_read(ser))
    try:
      assert ord(result[0]) & ~0x10 == ord(pkt[0]), "FAILURE - sent and received packets not equal"
      assert result[1:] == pkt[1:], "FAILURE - sent and received packets not equal"
    except:
      print "result is: " + readify(result)
      print "pkt is: " + readify(pkt)
      raise
    print "> SUCCESS\n"
    
functions.append(test_2_min)

def test_3_max():
    print "[3] Receive 2048-byte packet. Transmit 2048-byte packet. Receive packet containing timestamp. Transmit packet containing timestamp. Receive packet containing p-field. Transmit packet containing p-field."
    #pkt = rand_packet(apid = elysium_apid, tc = False, length = 2037, timestamp = 1, pfield = 1) # TODO fix this in more fully featured sandboxes
    pkt = rand_packet(apid = elysium_apid, tc = False, timestamp = 0, pfield = 0, length = 2042) # TODO fix this in more fully featured sandboxes
    ser.write(slip.stuff(pkt))
    result = slip.unstuff(blocking_read(ser))
    try:
      assert ord(result[0]) & ~0x10 == ord(pkt[0]), "FAILURE - sent and received packets not equal"
      assert result[1:] == pkt[1:], "FAILURE - sent and received packets not equal"
    except:
      print "result is: " + readify(result)
      print "pkt is: " + readify(pkt)
      raise
    print "> SUCCESS\n"
    
functions.append(test_3_max)

def test_4_long():
    print "[4] Reject malformed packets - > max length. Recover from invalid packet rejections."
    # Start by turning off event reporting
    r = run_and_read(ser, CmdGetErr, False, True, test_apid)
    (opcode, length, source, payload) = extract_and_assert(r, CmdGetErr.opcode, 1, 1)
    mask = ord(payload)
    run_and_read(ser, CmdSetErr, 0, False, True, test_apid)
    
    pkt = rand_packet(apid = elysium_apid, tc = False, length = 4096) 
    ser.write(slip.stuff(pkt))
    tmp = blocking_read(ser)
    result = slip.unstuff(tmp)
    try:
      assert result == '', "FAILURE - long packet not rejected"
    except:
      print "result is: " + readify(result)
      print "pkt is: " + readify(pkt)
      raise
      
    pkt = rand_packet(apid = elysium_apid, tc = False, length = None, maxlength=2036)
    ser.write(slip.stuff(pkt))
    result = slip.unstuff(blocking_read(ser))
    try:
      assert ord(result[0]) & ~0x10 == ord(pkt[0]), "FAILURE - state machine did not recover"
      assert result[1:] == pkt[1:], "FAILURE - state machine did not recover"
    except:
      print "result is: " + readify(result)
      print "pkt is: " + readify(pkt)
      raise
  
    # Restore error settings
    run_and_read(ser, CmdSetErr, mask, False, True, test_apid)
    
    print "> SUCCESS\n"
    
functions.append(test_4_long)

def test_5_len():
    print "[5] Reject malformed packets - > specified length. Recover from invalid packet rejections."
    # Start by turning off event reporting
    r = run_and_read(ser, CmdGetErr, False, True, test_apid)
    (opcode, length, source, payload) = extract_and_assert(r, CmdGetErr.opcode, 1, 1)
    mask = ord(payload)
    run_and_read(ser, CmdSetErr, 0, False, True, test_apid)
    
    pkt = rand_packet(apid = elysium_apid, tc = False, length = None, maxlength=2036)
    pkt = pkt[:len(pkt)/2] + str(random.getrandbits(8) for _ in xrange(16)) + pkt[len(pkt)/2:]
    ser.write(slip.stuff(pkt))
    tmp = blocking_read(ser)
    result = slip.unstuff(tmp)
    try:
      assert result == '', "FAILURE - invalid packet not rejected"
    except:
      print "result is: " + readify(result)
      print "pkt is: " + readify(pkt)
      raise
    pkt = rand_packet(apid = elysium_apid, tc = False, maxlength=2036)
    ser.write(slip.stuff(pkt))
    result = slip.unstuff(blocking_read(ser))
    try:
      assert ord(result[0]) & ~0x10 == ord(pkt[0]), "FAILURE - state machine did not recover"
      assert result[1:] == pkt[1:], "FAILURE - state machine did not recover"
    except:
      print "result is: " + readify(result)
      print "pkt is: " + readify(pkt)
      raise
  
    # Restore error settings
    run_and_read(ser, CmdSetErr, mask, False, True, test_apid)
    
    print "> SUCCESS\n"
    
functions.append(test_5_len)

def test_6_escape():
    print "[6] Receive packet with > 2 consecutive escape sequences. Transmit packet with > 2 consecutive escape sequences."
    for arg in itertools.product('\xc0\xdb', repeat=4):
      arg = ''.join(arg)
      pkt = spp.packetize(arg, elysium_apid, tc = False)
      ser.write(slip.stuff(pkt))
      result = slip.unstuff(blocking_read(ser))
      try:
        assert ord(result[0]) & ~0x10 == ord(pkt[0]), "FAILURE - sent and received multi-escape packets not equal"
        assert result[1:] == pkt[1:], "FAILURE - sent and received multi-escape packets not equal"
      except:
        print "result is: " + readify(result)
        print "pkt is: " + readify(pkt)
        raise
    print "> SUCCESS\n"
    
functions.append(test_6_escape)

def test_7_noescape():
    print "[7] Receive packet with no escape sequences. Transmit packet with no escape sequences."
    length = random.randint(1, 2036)
    arg = ''
    while len(arg) < length:
      c = '\xc0'
      while c in ['\xc0', '\xdb']:
        c = chr(random.getrandbits(8))
      arg += c
    pkt = spp.packetize(arg, elysium_apid, tc = False)
    ser.write(slip.stuff(pkt))
    result = slip.unstuff(blocking_read(ser))
    try:
      assert ord(result[0]) & ~0x10 == ord(pkt[0]), "FAILURE - sent and received no-escape packets not equal"
      assert result[1:] == pkt[1:], "FAILURE - sent and received no-escape packets not equal"
    except:
      print "result is: " + readify(result)
      print "pkt is: " + readify(pkt)
      raise
    print "> SUCCESS\n"
    
functions.append(test_7_noescape)

def test_8_short():
    print "[8] Reject malformed packets - < min length. Recover from invalid packet rejections."
    # Start by turning off event reporting
    r = run_and_read(ser, CmdGetErr, False, True, test_apid)
    (opcode, length, source, payload) = extract_and_assert(r, CmdGetErr.opcode, 1, 1)
    mask = ord(payload)
    run_and_read(ser, CmdSetErr, 0, False, True, test_apid)
    
    pkt = str(bytearray(random.getrandbits(8) for _ in xrange(6)))
    ser.write(slip.stuff(pkt))
    tmp = blocking_read(ser)
    result = slip.unstuff(tmp)
    try:
      assert result == '', "FAILURE - short packet not rejected"
    except:
      print "result is: " + readify(result)
      print "pkt is: " + readify(pkt)
      raise
  
    # Restore error settings
    run_and_read(ser, CmdSetErr, mask, False, True, test_apid)
    
    pkt = rand_packet(apid = elysium_apid, tc = False, maxlength=2036)
    ser.write(slip.stuff(pkt))
    result = slip.unstuff(blocking_read(ser))
    try:
      assert ord(result[0]) & ~0x10 == ord(pkt[0]), "FAILURE - state machine did not recover"
      assert result[1:] == pkt[1:], "FAILURE - state machine did not recover"
    except:
      print "result is: " + readify(result)
      print "pkt is: " + readify(pkt)
      raise
  
    print "> SUCCESS\n"

functions.append(test_8_short)

if __name__ == "__main__":
    for func in functions:
        func()

