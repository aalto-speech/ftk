#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "defs.hh"
#include "conf.hh"
#include "StringSet.hh"
#include "EM.hh"
#include "Unigrams.hh"

using namespace std;


int main(int argc, char* argv[]) {

    conf::Config config;
    config("usage: ll [OPTION...] WORDLIST VOCABULARY\n")
      ('h', "help", "", "", "display help")
      ('f', "forward-backward", "", "", "Use Forward-backward segmentation instead of Viterbi");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 2) config.print_help(stderr, 1);

    bool enable_forward_backward = config["forward-backward"].specified;
    string wordlist_fname = config.arguments[0];
    string vocab_in_fname = config.arguments[1];

    cerr << "parameters, wordlist: " << wordlist_fname << endl;
    cerr << "parameters, vocabulary: " << vocab_in_fname << endl;
    cerr << "parameters, use forward-backward: " << enable_forward_backward << endl;

    int maxlen, word_maxlen;
    map<string, flt_type> vocab;
    map<string, flt_type> freqs;
    map<string, flt_type> words;

    cerr << "Reading vocabulary " << vocab_in_fname << endl;
    int retval = Unigrams::read_vocab(vocab_in_fname, vocab, maxlen);
    if (retval < 0) {
        cerr << "something went wrong reading vocabulary" << endl;
        exit(0);
    }
    cerr << "\t" << "size: " << vocab.size() << endl;
    cerr << "\t" << "maximum string length: " << maxlen << endl;

    cerr << "Reading word list " << wordlist_fname << endl;
    retval = Unigrams::read_vocab(wordlist_fname, words, word_maxlen);
    if (retval < 0) {
        cerr << "something went wrong reading word list" << endl;
        exit(0);
    }
    cerr << "\t" << "wordlist size: " << words.size() << endl;
    cerr << "\t" << "maximum word length: " << word_maxlen << endl;

    Unigrams gg;
    if (enable_forward_backward)
        gg.set_segmentation_method(forward_backward);
    else
        gg.set_segmentation_method(viterbi);

    flt_type ll = gg.resegment_words(words, vocab, freqs);
    cerr << "cost: " << ll << endl;

    exit(1);
}
