#!/usr/bin/env python

import sys
from PyQt4 import QtXml

if __name__ == "__main__":
  num_places = 1000
  places_per_row = 100

  print "<workflow>"
  start_x = 500
  start_y = 500
  pos = (start_x, start_y)
  offset = 100
  max_x = start_x + 50 * offset
  for p in xrange(num_places):
    x,y = pos
    print "\t<place name=\"%d\" pos=\"(%f, %f)\"/>" % (p, x, y)
    if x + offset > max_x:
      x = start_x
      y = y + offset
    else:
      x = x + offset
    pos = (x,y)
  print "</workflow>"
