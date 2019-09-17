#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Convert list of word counts for letter n-gram training.')
    parser.add_argument('WORDCOUNTS', action="store", help='File containing the corpus word counts')
    parser.add_argument('SRILM_CORPUS', action="store", help='Training corpus for SRILM ngram-count')
    parser.add_argument('-e', '--encoding', dest='encoding', action='store', default="utf-8",
                        help='Used character encoding, DEFAULT: utf-8')
    args = parser.parse_args()

    if args.encoding == "utf-16" or args.encoding == "utf-32":
        raise Exception(
            "Currently only utf-8 and one byte character encodings are supported. Please convert the file to utf-8 first.")
    print("%s: using %s character encoding" % (parser.prog, args.encoding), file=sys.stderr)

    wordCountFile = open(args.WORDCOUNTS, encoding=args.encoding)
    srilmCorpusFile = open(args.SRILM_CORPUS, "w", encoding=args.encoding)

    for line in wordCountFile:
        line = line.strip()
        tokens = line.split()
        characters = list(tokens[1])
        print("%s <s> %s </s>" % (tokens[0], " ".join(characters)), file=srilmCorpusFile)
    srilmCorpusFile.close()
