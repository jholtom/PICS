#!/usr/bin/env python

def stuff(s):
    return s.replace('\xDB', '\xDB\xDD').replace('\xC0', '\xDB\xDC') + '\xc0'

def unstuff(s):
    return s.replace('\xC0', '').replace('\xDB\xDC', '\xC0').replace('\xDB\xDD', '\xDB')

