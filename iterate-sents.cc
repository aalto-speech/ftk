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
#include "FactorEncoder.hh"
#include "Unigrams.hh"

using namespace std;


void assert_single_chars(map<string, flt_type> &vocab,
                         const map<string, flt_type> &chars,
                         flt_type val)
{
    for (auto it = chars.cbegin(); it != chars.cend(); ++it)
        if (vocab.find(it->first) == vocab.end())
            vocab[it->first] = val;
}


int main(int argc, char* argv[]) {

    flt_type one_char_min_lp = -25.0;

    conf::Config config;
    config("usage: iterate-sents [OPTION...] CORPUS IN_VOCAB OUT_VOCAB\n")
      ('h', "help", "", "", "display help")
      ('i', "iterations=INT", "arg", "5", "Number of iterations")
      ('f', "forward-backward", "", "", "Use Forward-backward segmentation instead of Viterbi")
      ('t', "temp-vocabs", "", "", "Write out vocabulary after each iteration");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 3) config.print_help(stderr, 1);

    int num_iterations = config["iterations"].get_int();
    bool enable_forward_backward = config["forward-backward"].specified;
    bool write_temp_vocabs = config["temp-vocabs"].get_int();
    string training_fname = config.arguments[0];
    string vocab_in_fname = config.arguments[1];
    string vocab_out_fname = config.arguments[2];

    cerr << "parameters, training data: " << training_fname << endl;
    cerr << "parameters, initial vocabulary: " << vocab_in_fname << endl;
    cerr << "parameters, final vocabulary: " << vocab_out_fname << endl;
    cerr << "parameters, use forward-backward: " << enable_forward_backward << endl;
    cerr << "parameters, number of iterations: " << num_iterations << endl;
    cerr << "parameters, write vocabulary after each iteration: " << write_temp_vocabs << endl;

    int maxlen;
    map<string, flt_type> all_chars;
    map<string, flt_type> vocab;
    map<string, flt_type> freqs;
    vector<string> sents;

    cerr << "Reading vocabulary " << vocab_in_fname << endl;
    int retval = Unigrams::read_vocab(vocab_in_fname, vocab, maxlen);
    if (retval < 0) {
        cerr << "something went wrong reading vocabulary" << endl;
        exit(0);
    }
    cerr << "\t" << "size: " << vocab.size() << endl;
    cerr << "\t" << "maximum string length: " << maxlen << endl;
    for (auto it = vocab.cbegin(); it != vocab.end(); ++it)
        if (it->first.length() == 1)
            all_chars[it->first] = 0.0;

    Unigrams gg;
    if (enable_forward_backward)
        gg.set_segmentation_method(forward_backward);
    else
        gg.set_segmentation_method(viterbi);

    cerr << "Reading training corpus " << training_fname << endl;
    Unigrams::read_sents(training_fname, sents);

    cerr << "iterating.." << endl;
    for (int i=0; i<num_iterations; i++) {
        flt_type cost = gg.resegment_sents(sents, vocab, freqs);
        cerr << "cost: " << cost << endl;
        vocab = freqs;
        Unigrams::freqs_to_logprobs(vocab);
        assert_single_chars(vocab, all_chars, one_char_min_lp);

        if (write_temp_vocabs) {
            ostringstream tempfname;
            tempfname << vocab_out_fname << ".iter" << i+1;
            Unigrams::write_vocab(tempfname.str(), vocab);
        }
    }

    Unigrams::write_vocab(vocab_out_fname, vocab);

    exit(1);
}

