#!/usr/bin/python

import sys
import math


def read_bo_model(infname):

    curr_n = -1
    ngrams = dict()
    ngramfile = open(infname)
    separator = ' '

    for line in ngramfile:
        line = line.strip()
        if line.startswith(r'\data'): continue
        if line.startswith(r'\interpolated'):
            print >>sys.stderr, "Model in interpolated format."
            exit()
        if line.startswith(r'\end'): continue
        if line.startswith('ngram'): continue
        if not line: continue
        if line.startswith('\\'):
            vals = line.split('-')
            curr_n  = int(vals[0][1:len(vals[0])])
            continue

        if '\t' in line: separator = '\t'
        vals = line.partition(separator)
        logp  = float(vals[0])
        backoff = None
        ngram = vals[2]
        vals = ngram.split()
        if len(vals) > curr_n:
            backoff = vals[len(vals)-1]
            ngram = ngram.replace(backoff, "")
            backoff = float(backoff)
        ngram = ngram.strip()
        ngrams[ngram] = (logp, backoff)

    ngramfile.close()
    return ngrams


def convert_lst2str(lst):
    curr_ngram = ""
    for i in range(len(lst)):
        if i > 0: curr_ngram += " "
        curr_ngram += lst[i]
    return curr_ngram


def get_score(letters, ngrams):

    if len(letters) == 0:
        return 0.0

    score = 0.0

    try:

        for i in range(len(letters)):

            curr_ngram_lst = letters[0:i+1]
            curr_context_lst = letters[0:i]

            while True:
                curr_ngram = convert_lst2str(curr_ngram_lst)
                if ngrams.has_key(curr_ngram):
                    score += ngrams[curr_ngram][0]
                    break
                # Backoff
                else:
                    curr_context = convert_lst2str(curr_context_lst)
                    if ngrams.has_key(curr_context):
                        logp, backoff = ngrams[curr_context]
                        if backoff:
                            score += backoff
                    curr_ngram_lst.pop(0)
                    curr_context_lst.pop(0)

    except:

        print "error scoring: %s" % letters

    return score


if  __name__ =='__main__':

    if len(sys.argv) < 2:
        exit()

    ngrams = read_bo_model(sys.argv[1])

    strfile = sys.stdin
    if len(sys.argv) > 2:
        strfname = sys.argv[2]
        strfile = open(strfname)

    for line in strfile:
        line = line.strip()
        if len(line.split()) == 2:
            line = line.split()[1]
        letters = list(line)
        score = get_score(letters, ngrams)
        score *= math.log(10) # Convert from log10 (ARPA default) to ln
        print "%f %s" % (score, line)
