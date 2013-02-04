#!/usr/bin/python

import sys
from collections import OrderedDict


ngramfname = sys.argv[1]
ngramfile = open(ngramfname)

curr_n = -1
ngrams = OrderedDict()

for line in ngramfile:
    line = line.strip()
    if line.startswith(r'\data'): continue
    if line.startswith(r'\end'): continue
    if line.startswith('ngram'): continue
    if not line: continue
    if line.startswith('\\'):
        vals = line.split('-')
        curr_n  = int(vals[0][1:len(vals[0])])
        continue

    vals = line.split()
    prev_cost = 0
    curr_cost  = float(vals[0])
    curr_str = ""
    for i in range(curr_n):
        curr_str += vals[1+i]
        if i == curr_n-2:
            prev_cost = ngrams[curr_str]
    ngrams[curr_str] = prev_cost + curr_cost


for ngram in ngrams.keys():
    print "%.4f %s" % (ngrams[ngram], ngram)

