#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import math
import argparse
from collections import OrderedDict

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Initializes substrings with a zero-gram probability.')
    parser.add_argument('SUBSTRINGS', action="store", help='Source file containing the substrings to be scored')
    parser.add_argument('SCORED_SUBSTRINGS', action="store", help='Target file including the zero-gram probabilities')
    parser.add_argument('-e', '--encoding', dest='encoding', action='store', default="utf-8",
                        help='Used character encoding, DEFAULT: utf-8')
    args = parser.parse_args()

    if args.encoding == "utf-16" or args.encoding == "utf-32":
        raise Exception(
            "Currently only utf-8 and one byte character encodings are supported. Please convert the file to utf-8 first.")
    print("%s: using %s character encoding" % (parser.prog, args.encoding), file=sys.stderr)

    substrings = OrderedDict()
    substrFile = open(args.SUBSTRINGS, encoding=args.encoding)
    for line in substrFile:
        line = line.strip()
        substring = line.split()[1]
        substrings[substring] = None

    scoredSubstrFile = open(args.SCORED_SUBSTRINGS, "w", encoding=args.encoding)
    log_prob = math.log(1.0 / float(len(substrings)))
    for substring in substrings:
        print("%.4f\t%s" % (log_prob, substring), file=scoredSubstrFile)
    scoredSubstrFile.close()
