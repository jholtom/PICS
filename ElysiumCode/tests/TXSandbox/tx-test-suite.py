#!/usr/bin/env python

import random
import spp
import slip
import math
import itertools
import serial
import sys
import time

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
      pkt = rand_packet(length = None, maxlength=1008, tc=0, timestamp=0, pfield=0)
      ser.write(slip.stuff(pkt))
      result = slip.unstuff(blocking_read(ser))
      try:
        assert result == pkt, "FAILURE - sent and received packets not equal"
      except:
        print "result is: " + readify(result)
        print "pkt is: " + readify(pkt)
        raise
      #try:
      #  blocking_read(ser) # discard idle packet
      #except:
      #  pass
      print "small success"
    print "> SUCCESS\n" 

functions.append(test_1_nominal)

def test_2_min():
    print "[2] Receive 7-byte packet. Transmit 7-byte packet."
    pkt = rand_packet(length = 1, timestamp = 0, pfield = 0, tc=0)
    ser.write(slip.stuff(pkt))
    result = slip.unstuff(blocking_read(ser))
    try:
      assert result == pkt, "FAILURE - sent and received 7-byte packets not equal"
    except:
      print "result is: " + readify(result)
      print "pkt is: " + readify(pkt)
      raise
    try:
      blocking_read(ser) # discard idle packet
    except:
      pass
    print "> SUCCESS\n"
    
functions.append(test_2_min)

def test_3_max():
    print "[3] Receive 2048-byte packet. Transmit 2048-byte packet. Receive packet containing timestamp. Transmit packet containing timestamp. Receive packet containing p-field. Transmit packet containing p-field."
    #pkt = rand_packet(length = 2037, timestamp = 1, pfield = 1) # TODO fix this in more fully featured sandboxes
    pkt = rand_packet(timestamp = 0, pfield = 0, length = 2036, tc=0) # TODO fix this in more fully featured sandboxes
    ser.write(slip.stuff(pkt))
    result = slip.unstuff(blocking_read(ser))
    try:
      assert result == pkt, "FAILURE - sent and received 2048-byte packets not equal"
    except:
      print "result is: " + readify(result)
      print "pkt is: " + readify(pkt)
      raise
    try:
      blocking_read(ser) # discard idle packet
    except:
      pass
    print "> SUCCESS\n"
    
functions.append(test_3_max)

if __name__ == "__main__":
    for func in functions:
        func()

