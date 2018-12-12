#!/usr/bin/env python

import argparse

parser = argparse.ArgumentParser()
parser.add_argument('module', type=str)
parser.add_argument('indexes', type=int, nargs='*')

ns = parser.parse_args()

import importlib
testmod = importlib.import_module(ns.module)
if (ns.indexes):
  for i in ns.indexes:
    testmod.functions[i]()
else:
  for func in testmod.functions:
    func()


