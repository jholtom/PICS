#!/usr/bin/env python

import fileinput
import math
    
for line in fileinput.input():
    if fileinput.isfirstline():
        pass
        
    fields = line.split(',')
    hz_low = float(fields[2])
    hz_step = float(fields[4])
    
    pow = 0
    
    for n, field in enumerate(fields[6:]):
        freq = hz_low + (n * hz_step)
        if freq >= 433.95e6 and freq <= 434e6:
            pow += 10 ** ( (float(field) - 30.0) / 10.0 )
    
    dB = 10.0 * math.log10(pow) + 30.0
    
    print ','.join(fields[:6]) + ', ' + str(dB)
