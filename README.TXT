Factor Toolkit

This toolkit contains programs for segmenting strings and training segmentation models.
It has been developed primarily for learning units for speech recognition,
but can be used for other purposes as well. Subword vocabulary trained with unigram statistics
has been evaluated with respect to entropy and LVCSR accuracy. Phrase models and bigram 
statistics are more experimental at the moment.

Licence: Currently only for Aalto internal use. 
Author: Matti Varjokallio.
        Contains code modified from other Aalto projects like AaltoASR by Teemu Hirsimäki and others,
        including ArpaReader, LM and conf are directly from AaltoASR
Implementation: C++11, GCC 4.6 or newer recommended. 
Dependencies: CPPUnit needed for running unit tests, may be commented out in Makefile

PROGRAMS:
substrings: get all substrings from a set of strings
strscore: score strings with a ARPA letter n-gram
fe: segment text with a trained model
segposts: compute segmentation boundary posterior probabilities using unigram or bigram model
iterate: iterate unigram Expectation-Maximization without pruning over a word list
iterate-sents: iterate unigram Expectation-Maximization without pruning over a text corpus
g1g: train a subword unigram model from word list
g1g-sents: train a phrase unigram model from a text corpus
ll: compute log likelihoods given a model
counts: get fractional unigram and bigram counts
cmsfg: construct a multi string factor graph containing all segmentations of words, suitable for 2gram EM.
g2g: train a subword bigram model over a multi string factor graph
g2gr: train a subword bigram model over a multi string factor graph

EXAMPLE USAGE (subword unigram model):
substrings -l 17 wordcounts.txt substrings.txt
cat substrings.txt | wlist2srilm.py > srilm.txt
ngram-count -text srilm.txt -text-has-weights -order 8 -lm srilm_gt_8g.arpa
strscore srilm_gt_8g.arpa substrings.txt substrings.scored.txt
iterate -i 5 -f wordcounts.txt substrings.scored.txt initial.vocab
g1g -u 50 -c 25000 -r 500 -v 20000 wordcounts.txt initial.vocab final.vocab  (-f for forward-backward)
