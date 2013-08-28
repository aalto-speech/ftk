#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <popt.h>

#include "defs.hh"
#include "StringSet.hh"
#include "FactorEncoder.hh"
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

    float cutoff_value = 0.0;
    flt_type one_char_min_lp = -25.0;
    string vocab_fname;
    string wordlist_fname;
    string msfg_fname;

    // Popt documentation:
    // http://linux.die.net/man/3/popt
    // http://privatemisc.blogspot.fi/2012/12/popt-basic-example.html
    poptContext pc;
    struct poptOption po[] = {
        {"cutoff", 'u', POPT_ARG_FLOAT, &cutoff_value, 11001, NULL, "Cutoff value for initial segmentation"},
        POPT_AUTOHELP
        {NULL}
    };

    pc = poptGetContext(NULL, argc, (const char **)argv, po, 0);
    poptSetOtherOptionHelp(pc, "[INITIAL VOCABULARY] [WORDLIST] [MSFG_OUT]");

    int val;
    while ((val = poptGetNextOpt(pc)) >= 0)
        continue;

    // poptGetNextOpt returns -1 when the final argument has been parsed
    // otherwise an error occured
    if (val != -1) {
        switch (val) {
        case POPT_ERROR_NOARG:
            cerr << "Argument missing for an option" << endl;
            exit(1);
        case POPT_ERROR_BADOPT:
            cerr << "Option's argument could not be parsed" << endl;
            exit(1);
        case POPT_ERROR_BADNUMBER:
        case POPT_ERROR_OVERFLOW:
            cerr << "Option could not be converted to number" << endl;
            exit(1);
        default:
            cerr << "Unknown error in option processing" << endl;
            exit(1);
        }
    }

    // Handle ARG part of command line
    if (poptPeekArg(pc) != NULL)
        vocab_fname.assign((char*)poptGetArg(pc));
    else {
        cerr << "Initial vocabulary file not set" << endl;
        exit(1);
    }

    if (poptPeekArg(pc) != NULL)
        wordlist_fname.assign((char*)poptGetArg(pc));
    else {
        cerr << "Wordlist file not set" << endl;
        exit(1);
    }

    if (poptPeekArg(pc) != NULL)
        msfg_fname.assign((char*)poptGetArg(pc));
    else {
        cerr << "Output msfg file not set" << endl;
        exit(1);
    }

    cerr << "parameters, initial vocabulary: " << vocab_fname << endl;
    cerr << "parameters, wordlist: " << wordlist_fname << endl;
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
    ug.resegment_words(words, vocab, freqs);
    flt_type densum = ug.get_sum(freqs);
    flt_type cost = ug.get_cost(freqs, densum);
    cerr << "unigram cost without word end symbols: " << cost << endl;

    ug.cutoff(freqs, (flt_type)cutoff_value);
    cerr << "\tcutoff: " << cutoff_value << "\t" << "vocabulary size: " << freqs.size() << endl;
    vocab = freqs;
    densum = ug.get_sum(vocab);
    ug.freqs_to_logprobs(vocab, densum);
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
