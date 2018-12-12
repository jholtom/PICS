#!/usr/bin/env python

import struct
import crcmod
import itertools

crc_x25 = crcmod.predefined.mkPredefinedCrcFun('x-25')

def header(opcode, length, crc, reply, address):
    if crc:
        opcode |= 0x80
        length += 2
    if reply:
        opcode |= 0x40
        length += 2
        r = struct.pack('!BBH', opcode, length, address)
    else:
        r = struct.pack('!BB', opcode, length)
    return r

def footer(packet, crc):
    if crc:
        return struct.pack('!H', crc_x25(packet))
    return ''

def my_zip(a, b):
    iters = [iter(a), iter(b)]
    return list(it.next() for it in itertools.cycle(iters))

def extract(reply):
    (opcode, length, source) = struct.unpack('!BBH', reply[:4])
    payload = reply[4:]
    return (opcode, length, source, payload)

def CmdReset(crc = False, reply = False, address = None):
    CmdReset.opcode = 0x00
    r = header(CmdReset.opcode, 0, crc, reply, address)
    r += footer(r, crc)
    return r
        
def CmdGetGPO(crc = False, reply = False, address = None):
    CmdGetGPO.opcode = 0x01
    r = header(CmdGetGPO.opcode, 0, crc, reply, address)
    r += footer(r, crc)
    return r

def CmdSetGPO(value, crc = False, reply = False, address = None):
    CmdSetGPO.opcode = 0x02
    opcode = CmdSetGPO.opcode
    length = 1
    r = header(CmdSetGPO.opcode, length, crc, reply, address)
    r += struct.pack('!B', value)
    r += footer(r, crc)
    return r

def CmdGetActiveBank(crc = False, reply = False, address = None):
    CmdGetActiveBank.opcode = 0x03
    r = header(CmdGetActiveBank.opcode, 0, crc, reply, address)
    r += footer(r, crc)
    return r

def CmdGetRegs(bank, addrs, crc = False, reply = False, address = None):
    CmdGetRegs.opcode = 0x04
    length = len(addrs) + 1
    r = header(CmdGetRegs.opcode, length, crc, reply, address)
    r += struct.pack('B%dB' % len(addrs), bank, *addrs)
    r += footer(r, crc)
    return r

def CmdSetRegs(bank, addrs, values, crc = False, reply = False, address = None):
    CmdSetRegs.opcode = 0x05
    payload = my_zip(addrs, values)
    length = len(payload) + 1
    r = header(CmdSetRegs.opcode, length, crc, reply, address)
    r += struct.pack('B%dB' % len(payload), bank, *payload)
    r += footer(r, crc)
    return r

def CmdGetBlock(bank, addr, count, crc = False, reply = False, address = None):
    CmdGetBlock.opcode = 0x06
    length = 3
    r = header(CmdGetBlock.opcode, length, crc, reply, address)
    r += struct.pack('BBB', bank, addr, count)
    r += footer(r, crc)
    return r

def CmdSetBlock(bank, addr, values, crc = False, reply = False, address = None):
    CmdSetBlock.opcode = 0x07
    length = len(values) + 2
    r = header(CmdSetBlock.opcode, length, crc, reply, address)
    r += struct.pack('BB%dB' % len(values), bank, addr, *values)
    r += footer(r, crc)
    return r

def CmdGetTXFreq(crc = False, reply = False, address = None):
    CmdGetTXFreq.opcode = 0x08
    length = 0
    r = header(CmdGetTXFreq.opcode, length, crc, reply, address)
    r += footer(r, crc)
    return r

def CmdSetTXFreq(freq, crc = False, reply = False, address = None):
    CmdSetTXFreq.opcode = 0x09
    length = 4
    r = header(CmdSetTXFreq.opcode, length, crc, reply, address)
    r += struct.pack('!I', freq)
    r += footer(r, crc)
    return r

def CmdGetRXFreq(crc = False, reply = False, address = None):
    CmdGetRXFreq.opcode = 0x0A
    length = 0
    r = header(CmdGetRXFreq.opcode, length, crc, reply, address)
    r += footer(r, crc)
    return r

def CmdSetRXFreq(freq, crc = False, reply = False, address = None):
    CmdSetRXFreq.opcode = 0x0B
    length = 4
    r = header(CmdSetRXFreq.opcode, length, crc, reply, address)
    r += struct.pack('!I', freq)
    r += footer(r, crc)
    return r

def CmdGetTXRate(crc = False, reply = False, address = None):
    CmdGetTXRate.opcode = 0x0C
    length = 0
    r = header(CmdGetTXRate.opcode, length, crc, reply, address)
    r += footer(r, crc)
    return r

def CmdSetTXRate(rate, crc = False, reply = False, address = None):
    CmdSetTXRate.opcode = 0x0D
    length = 4
    r = header(CmdSetTXRate.opcode, length, crc, reply, address)
    r += struct.pack('!I', rate)
    r += footer(r, crc)
    return r

def CmdGetRXRate(crc = False, reply = False, address = None):
    CmdGetRXRate.opcode = 0x0E
    length = 0
    r = header(CmdGetRXRate.opcode, length, crc, reply, address)
    r += footer(r, crc)
    return r

def CmdSetRXRate(rate, crc = False, reply = False, address = None):
    CmdSetRXRate.opcode = 0x0F
    length = 4
    r = header(CmdSetRXRate.opcode, length, crc, reply, address)
    r += struct.pack('!I', rate)
    r += footer(r, crc)
    return r

def CmdGetTXDev(crc = False, reply = False, address = None):
    CmdGetTXDev.opcode = 0x10
    length = 0
    r = header(CmdGetTXDev.opcode, length, crc, reply, address)
    r += footer(r, crc)
    return r

def CmdSetTXDev(dev, crc = False, reply = False, address = None):
    CmdSetTXDev.opcode = 0x11
    length = 4
    r = header(CmdSetTXDev.opcode, length, crc, reply, address)
    r += struct.pack('!I', dev)
    r += footer(r, crc)
    return r

def CmdGetRXDev(crc = False, reply = False, address = None):
    CmdGetRXDev.opcode = 0x12
    length = 0
    r = header(CmdGetRXDev.opcode, length, crc, reply, address)
    r += footer(r, crc)
    return r

def CmdSetRXDev(dev, crc = False, reply = False, address = None):
    CmdSetRXDev.opcode = 0x13
    length = 4
    r = header(CmdSetRXDev.opcode, length, crc, reply, address)
    r += struct.pack('!I', dev)
    r += footer(r, crc)
    return r

def CmdGetTXPow(crc = False, reply = False, address = None):
    CmdGetTXPow.opcode = 0x14
    length = 0
    r = header(CmdGetTXPow.opcode, length, crc, reply, address)
    r += footer(r, crc)
    return r

def CmdSetTXPow(pow, crc = False, reply = False, address = None):
    CmdSetTXPow.opcode = 0x15
    length = 1
    r = header(CmdSetTXPow.opcode, length, crc, reply, address)
    r += struct.pack('!b', pow)
    r += footer(r, crc)
    return r

def CmdGetBaud(crc = False, reply = False, address = None):
    CmdGetBaud.opcode = 0x16
    length = 0
    r = header(CmdGetBaud.opcode, length, crc, reply, address)
    r += footer(r, crc)
    return r

def CmdSetBaud(baud, crc = False, reply = False, address = None):
    CmdSetBaud.opcode = 0x17
    length = 4
    r = header(CmdSetBaud.opcode, length, crc, reply, address)
    r += struct.pack('!I', baud)
    r += footer(r, crc)
    return r

def CmdReloadConfig(bank, crc = False, reply = False, address = None):
    CmdReloadConfig.opcode = 0x18
    length = 1
    r = header(CmdReloadConfig.opcode, length, crc, reply, address)
    r += struct.pack('!B', bank)
    r += footer(r, crc)
    return r

def CmdChannelSub(interval, channels, crc = False, reply = False, address = None):
    CmdChannelSub.opcode = 0x19
    length = len(channels) + 4
    r = header(CmdChannelSub.opcode, length, crc, reply, address)
    r += struct.pack('!I%dB' % len(channels), interval, *channels)
    r += footer(r, crc)
    return r

def CmdChannelUnsub(channels, crc = False, reply = False, address = None):
    CmdChannelUnsub.opcode = 0x1A
    length = len(channels)
    r = header(CmdChannelUnsub.opcode, length, crc, reply, address)
    r += struct.pack('!%dB' % len(channels), *channels)
    r += footer(r, crc)
    return r

def CmdLogChan(interval, channels, crc = False, reply = False, address = None):
    CmdLogChan.opcode = 0x1B
    length = len(channels) + 4
    r = header(CmdLogChan.opcode, length, crc, reply, address)
    r += struct.pack('!I%dB' % len(channels), interval, *channels)
    r += footer(r, crc)
    return r

def CmdUnlogChan(channels, crc = False, reply = False, address = None):
    CmdUnlogChan.opcode = 0x1C
    length = len(channels)
    r = header(CmdUnlogChan.opcode, length, crc, reply, address)
    r += struct.pack('!%dB' % len(channels), *channels)
    r += footer(r, crc)
    return r

def CmdGetChan(channel, crc = False, reply = False, address = None):
    CmdGetChan.opcode = 0x1D
    length = 1
    r = header(CmdGetChan.opcode, length, crc, reply, address)
    r += struct.pack('!B', channel)
    r += footer(r, crc)
    return r

def CmdResetChan(crc = False, reply = False, address = None):
    CmdResetChan.opcode = 0x1E
    length = 0
    r = header(CmdResetChan.opcode, length, crc, reply, address)
    r += footer(r, crc)
    return r

def CmdEventSub(events, crc = False, reply = False, address = None):
    CmdEventSub.opcode = 0x1F
    length = len(events)
    r = header(CmdEventSub.opcode, length, crc, reply, address)
    r += struct.pack('!%dB' % len(events), *events)
    r += footer(r, crc)
    return r

def CmdEventUnsub(events, crc = False, reply = False, address = None):
    CmdEventUnsub.opcode = 0x20
    length = len(events)
    r = header(CmdEventUnsub.opcode, length, crc, reply, address)
    r += struct.pack('!%dB' % len(events), *events)
    r += footer(r, crc)
    return r

def CmdLogEvent(events, crc = False, reply = False, address = None):
    CmdLogEvent.opcode = 0x21
    length = len(events)
    r = header(CmdLogEvent.opcode, length, crc, reply, address)
    r += struct.pack('!%dB' % len(events), *events)
    r += footer(r, crc)
    return r

def CmdUnlogEvent(events, crc = False, reply = False, address = None):
    CmdUnlogEvent.opcode = 0x22
    length = len(events)
    r = header(CmdUnlogEvent.opcode, length, crc, reply, address)
    r += struct.pack('!%dB' % len(events), *events)
    r += footer(r, crc)
    return r

def CmdResetEvent(crc = False, reply = False, address = None):
    CmdResetEvent.opcode = 0x23
    length = 0
    r = header(CmdResetEvent.opcode, length, crc, reply, address)
    r += footer(r, crc)
    return r

def CmdSetTime(time, crc = False, reply = False, address = None):
    CmdSetTime.opcode = 0x24
    length = 4
    r = header(CmdSetTime.opcode, length, crc, reply, address)
    r += struct.pack('!I', time)
    r += footer(r, crc)
    return r

def CmdGetTime(crc = False, reply = False, address = None):
    CmdGetTime.opcode = 0x25
    length = 0
    r = header(CmdGetTime.opcode, length, crc, reply, address)
    r += footer(r, crc)
    return r

def CmdGetErr(crc = False, reply = False, address = None):
    CmdGetErr.opcode = 0x26
    length = 0
    r = header(CmdGetErr.opcode, length, crc, reply, address)
    r += footer(r, crc)
    return r

def CmdSetErr(err, crc = False, reply = False, address = None):
    CmdSetErr.opcode = 0x27
    length = 1
    r = header(CmdSetErr.opcode, length, crc, reply, address)
    r += struct.pack('!B', err)
    r += footer(r, crc)
    return r

def CmdGetLog(crc = False, reply = False, address = None):
    CmdGetLog.opcode = 0x28
    length = 0
    r = header(CmdGetLog.opcode, length, crc, reply, address)
    r += footer(r, crc)
    return r

def CmdSetLog(log, crc = False, reply = False, address = None):
    CmdSetLog.opcode = 0x29
    length = 1
    r = header(CmdSetLog.opcode, length, crc, reply, address)
    r += struct.pack('!B', log)
    r += footer(r, crc)
    return r

def CmdUploadFW(addr, chunk, crc = False, reply = False, address = None):
    CmdUploadFW.opcode = 0x2A
    length = len(chunk) + 4
    r = header(CmdUploadFW.opcode, length, crc, reply, address)
    r += struct.pack('!I', addr)
    r += chunk
    r += footer(r, crc)
    return r

def CmdVerifyFW(chunk_crc, crc = False, reply = False, address = None):
    CmdVerifyFW.opcode = 0x2B
    length = 2
    r = header(CmdVerifyFW.opcode, length, crc, reply, address)
    r += struct.pack('!H', chunk_crc)
    r += footer(r, crc)
    return r

def CmdCancelFW(crc = False, reply = False, address = None):
    CmdCancelFW.opcode = 0x2C
    r = header(CmdCancelFW.opcode, 0, crc, reply, address)
    r += footer(r, crc)
    return r

def CmdInstallFW(crc = False, reply = False, address = None):
    CmdInstallFW.opcode = 0x2D
    r = header(CmdInstallFW.opcode, 0, crc, reply, address)
    r += footer(r, crc)
    return r

def CmdStoreTelem(chunk, crc = False, reply = False, address = None):
    CmdStoreTelem.opcode = 0x2E
    length = len(chunk)
    r = header(CmdStoreTelem.opcode, length, crc, reply, address)
    r += struct.pack('!%dB' % len(chunk), *chunk)
    r += footer(r, crc)
    return r

def CmdGetTelem(idx_start = None, idx_end = None, ts_start = None, ts_end = None, crc = False, reply = False, address = None):
    CmdGetTelem.opcode = 0x2F
    length = 2
    if idx_start and idx_end:
        length += 2
    if ts_start and ts_end:
        length += 8
    r = header(CmdGetTelem.opcode, length, crc, reply, address)
    
    if idx_start and idx_end and ts_start and ts_end:
        r += struct.pack('!BBii', idx_start, idx_end, ts_start, ts_end)
    elif ts_start and ts_end:
        r += struct.pack('!ii', ts_start, ts_end)
    elif idx_start and idx_end:
        r += struct.pack('!BB', idx_start, idx_end)

    r += footer(r, crc)
    return r

