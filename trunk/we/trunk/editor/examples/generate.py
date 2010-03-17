#!/usr/bin/env python

import sys, math
from PyQt4 import QtXml

if __name__ == "__main__":
  num_places = 1000
  if len(sys.argv) > 1:
    num_places = int(sys.argv[1])
  places_per_row = int(math.sqrt(num_places))

  print "<workflow>"
  start_x = 50
  start_y = 50
  x, y = start_x, start_y
  offset = 75
  max_x = start_x + 50 * offset
  for p in xrange(num_places):
    print "\t<place name=\"%d\" pos=\"(%f, %f)\"/>" % (p, x, y)
    x += offset
    if x > max_x:
      x = start_x
      y = y + offset
    if p > 0:
      print "\t<edge from=\"%d\" to=\"%d\"/>" % (p-1, p)
  print "</workflow>"
