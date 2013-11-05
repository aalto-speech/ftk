#!/usr/bin/python

import sys


for line in sys.stdin:
    line = line.strip()
    vals = line.split()
    letters = list(vals[1])

    sys.stdout.write(vals[0])
    sys.stdout.write(" <s>")
    for letter in letters:
        sys.stdout.write(" ")
        sys.stdout.write(letter)
    sys.stdout.write(" </s>\n")

