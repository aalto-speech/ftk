#!/usr/bin/python

import os
import sys
import codecs
from optparse import OptionParser


if __name__ == "__main__":

    usage = "usage: %prog [options] [input] [output]"
    description = "Convert list of word counts for letter n-gram training."
    parser = OptionParser(usage=usage, description=description)
    parser.add_option('--utf-8', dest='utf8', action='store_true', default=False,
                      help='Input is in Utf-8 format')

    (options, args) = parser.parse_args()
    infile = None
    outfile = None

    infile = sys.stdin
    outfile = sys.stdout
    if len(args):
        if not len(args) == 2:
            print >>sys.stderr, "Please define either no input/output files (stdin and stdout will be used) or both."
            sys.exit(0)
        infile = open(args[0])
        outfile = open(args[1], 'w')

    if options.utf8:
        infile = codecs.getreader('utf-8')(infile)
        outfile = codecs.getwriter('utf-8')(outfile)

    for line in infile:
        line = line.strip()
        vals = line.split()
        letters = list(vals[1])

        outfile.write("%s <s>" % vals[0])
        for letter in letters:
            outfile.write(" ")
            outfile.write(letter)
        outfile.write(" </s>%s" % os.linesep)

