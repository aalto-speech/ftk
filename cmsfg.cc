#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "defs.hh"
#include "conf.hh"
#include "StringSet.hh"
#include "EM.hh"
#include "Unigrams.hh"
#include "Bigrams.hh"

using namespace std;


void
assert_single_chars(map<string, flt_type> &vocab,
                    const map<string, flt_type> &chars,
                    flt_type val)
{
    for (auto it = chars.cbegin(); it != chars.cend(); ++it)
        if (vocab.find(it->first) == vocab.end())
            vocab[it->first] = val;
}


int main(int argc, char* argv[]) {

    conf::Config config;
    config("usage: cmsfg [OPTION...] WORDLIST INIT_VOCAB MSFG_OUT\n")
      ('h', "help", "", "", "display help")
      ('u', "cutoff=INT", "arg", "0", "Cutoff value for initial segmentation");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 3) config.print_help(stderr, 1);

    flt_type one_char_min_lp = -25.0;
    flt_type cutoff_value = config["cutoff"].get_float();
    string wordlist_fname = config.arguments[0];
    string vocab_fname = config.arguments[1];
    string msfg_fname = config.arguments[2];

    cerr << "parameters, wordlist: " << wordlist_fname << endl;
    cerr << "parameters, initial vocabulary: " << vocab_fname << endl;
    cerr << "parameters, msfg to write: " << msfg_fname << endl;
    cerr << "parameters, cutoff: " << setprecision(15) << cutoff_value << endl;

    int maxlen, word_maxlen;
    map<string, flt_type> all_chars;
    map<string, flt_type> vocab;
    map<string, flt_type> words;
    map<string, flt_type> freqs;

    cerr << "Reading vocabulary " << vocab_fname << endl;
    int retval = Unigrams::read_vocab(vocab_fname, vocab, maxlen);
    if (retval < 0) {
        cerr << "something went wrong reading vocabulary" << endl;
        exit(0);
    }
    cerr << "\t" << "size: " << vocab.size() << endl;
    cerr << "\t" << "maximum string length: " << maxlen << endl;
    for (auto it = vocab.cbegin(); it != vocab.end(); ++it)
        if (it->first.length() == 1)
            all_chars[it->first] = 0.0;

    cerr << "Reading word list " << wordlist_fname << endl;
    retval = Unigrams::read_vocab(wordlist_fname, words, word_maxlen);
    if (retval < 0) {
        cerr << "something went wrong reading word list" << endl;
        exit(0);
    }
    cerr << "\t" << "wordlist size: " << words.size() << endl;
    cerr << "\t" << "maximum word length: " << word_maxlen << endl;

    Unigrams ug;
    ug.set_segmentation_method(forward_backward);

    cerr << "Initial cutoff" << endl;
    flt_type cost = ug.resegment_words(words, vocab, freqs);
    cerr << "unigram cost without word end symbols: " << cost << endl;

    ug.cutoff(freqs, (flt_type)cutoff_value);
    cerr << "\tcutoff: " << cutoff_value << "\t" << "vocabulary size: " << freqs.size() << endl;
    vocab = freqs;
    Unigrams::freqs_to_logprobs(vocab);
    assert_single_chars(vocab, all_chars, one_char_min_lp);

    StringSet ss_vocab(vocab);
    MultiStringFactorGraph msfg(start_end_symbol);
    vocab[start_end_symbol] = 0.0;

    int old_nodes = 0;
    int old_arcs = 0;
    int curr_word_idx = 0;
    for (auto it = words.cbegin(); it != words.cend(); ++it) {
        FactorGraph fg(it->first, start_end_symbol, ss_vocab);
        msfg.add(fg);
        old_nodes += fg.nodes.size();
        old_arcs += fg.arcs.size();
        curr_word_idx++;
        if (curr_word_idx % 10000 == 0) cerr << "... processing word " << curr_word_idx << endl;
        if (curr_word_idx % 100000 == 0) {
            stringstream tempfname;
            tempfname << "msfg." << curr_word_idx;
            cerr << "... writing to file " << tempfname.str() << endl;
            msfg.write(tempfname.str());
        }
    }

    msfg.write(msfg_fname);

    cerr << "factor graph strings: " << msfg.string_end_nodes.size() << endl;
    cerr << "factor graph nodes: " << msfg.nodes.size() << endl;
    cerr << "nodes for separate fgs: " << old_nodes << endl;
    cerr << "arcs for separate fgs: " << old_arcs << endl;

    exit(1);
}
