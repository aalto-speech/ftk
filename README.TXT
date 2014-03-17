Factor Toolkit

This toolkit contains programs for segmenting strings and training segmentation models.
It has been developed primarily for learning units for speech recognition,
but can be used for other purposes as well. Subword vocabulary trained with unigram statistics
has been evaluated with respect to entropy and LVCSR accuracy. Phrase models and bigram 
statistics are more experimental at the moment. g1g-sents may be adapted for 
general segmentation purposes.

The segmentation model is a unigram or bigram model.
Parameters are trained from text with Expectation-Maximization similarly as for multigrams.
The model training starts from a large vocabulary which is pruned to a suitable size.
Vocabulary selection is greedy in the sense that in each iteration strings which have the
least effect on total data likelihood, are removed.

Licence: BSD-3
There is a patent on bi-multigrams for class information which may or may not apply for commercial use.

Citations for publications:
Matti Varjokallio and Mikko Kurimo: "A Toolkit For Efficient Learning of Lexical Units for Speech Recognition", in the proceedings of The 9th edition of the Language Resources and Evaluation Conference (LREC 2014), 26-31 May, Reykjavik, Iceland. (to appear)

results with unigram statistics
Matti Varjokallio, Mikko Kurimo and Sami Virpioja: "Learning a Subword Vocabulary Based on Unigram Likelihood", In IEEE Workshop on Automatic Speech Recognition and Understanding, ASRU2013.

Implementation: C++11, GCC 4.6 or newer recommended.
        EM layer: FactorEncoder, FactorGraph, MSFG
        Model selection layer: Unigrams, Bigrams
        Current executables roughly cover all the functionality
        but the idea is to write new ones as needed

Dependencies: CPPUnit needed for running unit tests, may be commented out in Makefile
              Unit tests mainly for Expectation-Maximization and related data structures

Character encoding: core functionality supports all normal fixed-length and UTF8-style encodings.
        Initialization programs substrings and strscore currently only 1-byte encoding

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

wordcounts.txt should contain words with frequencies in the format:
COUNT	WORD
...

1) Get substrings up to some suitable length defined by -l
substrings -l 17 wordcounts.txt substrings.txt

2a) Initialize substrings with a letter n-gram model
cat substrings.txt | wlist2srilm.py > srilm.txt
ngram-count -text srilm.txt -text-has-weights -order 8 -lm srilm_gt_8g.arpa
strscore srilm_gt_8g.arpa substrings.txt substrings.scored.txt

2b) Initialize substrings with 0-gram scores
flatinit.py substrings.txt > substrings.scored.txt

3) Iterate forward-backward a few times to get a local maximum for the initial vocabulary
iterate -i 5 -f wordcounts.txt substrings.scored.txt initial.vocab

4) Prune the vocabulary with cutoff + greedy likelihood pruning
It may be a good idea to experiment with the parameters. The following should work reasonably:
g1g -l 100000,250,50000,100 -v 20000 -t 5000 wordcounts.txt initial.vocab final.vocab
Set target vocabulary size (-v) lower so you may try out different vocabulary sizes

