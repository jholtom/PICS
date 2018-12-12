#!/usr/bin/env python

with open('msp_vectors.c', 'w') as f:
  print >>f, '#include <msp430.h>\n'
  for i in range(1, 56):
    print >>f, "__attribute__((interrupt(" + str(i) + ")))"
    print >>f, "void Vector" + str(i) + '(void) {\n'
    print >>f, "  while (1) {"
    print >>f, "  }"
    print >>f, "}\n"
  print >>f, "\n"

