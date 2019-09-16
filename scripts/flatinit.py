#!/usr/bin/python

import sys
import math
from collections import OrderedDict

substrs = OrderedDict()

substrfname = sys.argv[1]
substrf = open(substrfname)

for line in substrf:
    line = line.strip()
    substr = line.split()[1]
    substrs[substr] = None

flatlp = 1.0/float(len(substrs))
flatlp = math.log(flatlp)
for substr in substrs:
    print "%.4f\t%s" % (flatlp, substr)


