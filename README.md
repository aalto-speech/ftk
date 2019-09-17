### Factor Toolkit

This toolkit contains programs for segmenting strings and training segmentation models.
It has been developed primarily for learning units for speech recognition,
but can be used for other purposes as well. This is not intended for less-resourced scenarios;
in that case you are likely better off with a Morfessor Baseline model.
It is a good idea to start with Morfessor Baseline anyway before experimenting with these algorithms.
With these models the maximum subword length and the vocabulary size can be optimized
to obtain the best results.

Subword vocabulary trained with unigram statistics
has been evaluated with respect to cross-entropy and LVCSR accuracy. Phrase models and bigram 
models are more experimental at the moment. g1g-sents may be adapted for 
other segmentation purposes as well.

The segmentation model is a unigram or bigram model.
Parameters are trained from text with Expectation-Maximization similarly as for multigrams.
The model training starts from a large vocabulary which is pruned to a suitable size.
Vocabulary selection is greedy in the sense that in each iteration strings which have the
least effect on total data likelihood, are removed.

#### Compiling executables

Requirements
* Provided Makefile works with a GCC compiler which has C++11 support, i.e. GCC 4.6 or newer. Should work on MinGW as well.
* Zlib libraries and headers. On linux systems these are often included in packages `zlib1g` and `zlib1g-dev`.
* Scripts under the `scripts` folder require python3
* Running unit tests requires boost unit test framework libraries and headers.
On linux systems these are often included in packages `libboost-test` and `libboost-test-dev` packages.
The unit tests may be disabled by setting `NO_UNIT_TESTS=1` in Makefile.local file

These commands should work in most cases  
`cp Makefile.local.example Makefile.local`  
`make`  
The compilation parameters are easiest changed by modifying Makefile.local file which is included by the main Makefile.

#### File formats

Plaintext files are expected and most of the executables support gzip compressed files as well.
Currently only utf-8 and 1-byte character encodings like ISO Latin are supported.
If the utf-8 switch is not defined, the files are expected to be coded by a 1-byte encoding.
For the Python scripts, the encoding defaults to utf-8.
Please convert the input to utf-8 if the character set is not presentable in ISO Latin.

#### Programs

* `substrings`: gets all substrings from a set of strings
* `strscore`: scores strings with an ARPA letter n-gram
* `segtext`: segments text with a trained model
* `segposts`: computes segmentation boundary posterior probabilities using unigram or bigram model
* `iterate`: iterates unigram/multigram Expectation-Maximization without pruning over a word list
* `iterate-sents`: iterates unigram/multigram Expectation-Maximization without pruning over a text corpus
* `g1g`: trains a subword unigram model from word list
* `g1g-sents`: trains a phrase unigram model from a text corpus
* `llh`: computes log likelihoods given a model
* `counts`: gets fractional unigram and bigram counts
* `cmsfg`: constructs a multi string factor graph containing all segmentations of words, suitable for 2gram EM.
* `iterate12`: iterates bigram/bi-multigram Expectation-Maximization initialized with unigram stats
* `g2gr`: trains a subword bigram model, simple pruning
* `g2g`: trains a subword bigram model, more accurate pruning, Forward-backward recommended
* `g2gkn`: train a subword bigram model, more accurate pruning with Kneser-Ney smoothing, Viterbi recommended

#### Example usage for a subword unigram model

wordcounts.txt should contain words with frequencies in the format:  
~~~~
COUNT	WORD  
...  
~~~~

1. Find substrings up to some suitable length defined by -l  
    `substrings -8 -l 15 wordcounts.txt substrings.txt`

2. Initialize substrings with initial probabilities
 * Using a character n-gram model  
    `scripts/wlist2srilm.py --encoding utf-8 wordcounts.txt srilm_corpus.txt`  
    `ngram-count -text srilm_corpus.txt -text-has-weights -order 8 -lm srilm_gt_8g.arpa`  
    `strscore -8 srilm_gt_8g.arpa substrings.txt substrings.scored.txt`

 * Alternatively initialize substrings with 0-gram scores  
    `flatinit.py --encoding utf-8 substrings.txt substrings.scored.txt`

3. Iterate forward-backward a few times to get a local maximum for the initial vocabulary  
    `iterate -8 -i 5 -f wordcounts.txt substrings.scored.txt initial.vocab`

4. Prune the vocabulary with cutoff + greedy likelihood pruning
It may be a good idea to experiment with the parameters. The following should work reasonably:  
    `g1g -8 -l 100000,250,50000,100 -v 20000 -t 5000 wordcounts.txt initial.vocab final.vocab`  
If you set the target vocabulary size (-v) lower so you may try out different vocabulary sizes
For utf-8 encoded data use the utf-8 switch with every executable as in the example.

Unigram phrase model is trained similarly, just iterate-sents and g1g-sents executables are used.

Training corpus could be formatted like this without counts (depending how you want to model the word boundaries):  
~~~~
_the_first_sentence
_the_second_sentence
~~~~

More care is needed in initializing the phrase model.
For instance all substrings from most common words and most common word bigrams could be used as a starting point.

#### License

The software is licensed under the BSD-3 license, for more details see the `LICENSE` file.  

For academic purposes, please cite the following:
~~~~
Matti Varjokallio and Mikko Kurimo
A Toolkit For Efficient Learning of Lexical Units for Speech Recognition
in Proceedings of The 9th edition of the Language Resources and Evaluation Conference (LREC 2014)
26-31 May, 2014
Reykjavik, Iceland.
~~~~

or a publication analyzing the unigram model performance
~~~~
Matti Varjokallio, Mikko Kurimo and Sami Virpioja
Learning a Subword Vocabulary Based on Unigram Likelihood
in Proceedings of the IEEE Workshop on Automatic Speech Recognition and Understanding (ASRU2013)
8-12 December, 2013
Olomouc, Czech Republic
~~~~
