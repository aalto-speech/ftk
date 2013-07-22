#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <popt.h>

#include "defs.hh"
#include "StringSet.hh"
#include "FactorEncoder.hh"
#include "Unigrams.hh"

using namespace std;


void remove_wbs(map<string, flt_type> &freqs,
                const string &wb_symbol)
{
    auto iter = freqs.begin();
    while (iter != freqs.end()) {
        if (iter->first.find(wb_symbol) != string::npos) {
            string tstr(iter->first);
            tstr.replace(tstr.find(wb_symbol), 1, "");
            freqs[tstr] += iter->second;
            freqs.erase(iter++);
        }
        else ++iter;
    }
}


void assert_single_chars(map<string, flt_type> &vocab,
                         const map<string, flt_type> &chars,
                         flt_type val)
{
    for (auto it = chars.cbegin(); it != chars.cend(); ++it)
        if (vocab.find(it->first) == vocab.end())
            vocab[it->first] = val;
}


int main(int argc, char* argv[]) {

    float cutoff_value = 0.0;
    bool enable_forward_backward = false;
    flt_type one_char_min_lp = -25.0;
    string wb_symbol("_");
    string vocab_fname;
    string wordlist_fname;
    string out_fname;

    // Popt documentation:
    // http://linux.die.net/man/3/popt
    // http://privatemisc.blogspot.fi/2012/12/popt-basic-example.html
    poptContext pc;
    struct poptOption po[] = {
        {"cutoff", 'u', POPT_ARG_FLOAT, &cutoff_value, 11001, NULL, "Cutoff value for each iteration"},
        {"forward_backward", 'f', POPT_ARG_NONE, &enable_forward_backward, 11007, "Use Forward-backward segmentation instead of Viterbi", NULL},
        POPT_AUTOHELP
        {NULL}
    };

    pc = poptGetContext(NULL, argc, (const char **)argv, po, 0);
    poptSetOtherOptionHelp(pc, "[INITIAL VOCABULARY] [WORDLIST] [OUT VOCABULARY]");

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
        out_fname.assign((char*)poptGetArg(pc));
    else {
        cerr << "Initialized vocabulary file not set" << endl;
        exit(1);
    }

    cerr << "parameters, initial vocabulary: " << vocab_fname << endl;
    cerr << "parameters, wordlist: " << wordlist_fname << endl;
    cerr << "parameters, out vocabulary: " << out_fname << endl;
    cerr << "parameters, cutoff: " << setprecision(15) << cutoff_value << endl;
    cerr << "parameters, use forward-backward: " << enable_forward_backward << endl;

    int maxlen, word_maxlen;
    map<string, flt_type> all_chars;
    map<string, flt_type> vocab;
    map<string, flt_type> freqs;
    map<string, flt_type> words;

    cerr << "Reading vocabulary " << vocab_fname << endl;
    int retval = Unigrams::read_vocab(vocab_fname, vocab, maxlen);
    if (retval < 0) {
        cerr << "something went wrong reading vocabulary" << endl;
        exit(0);
    }
    cerr << "\t" << "size: " << vocab.size() << endl;
    cerr << "\t" << "maximum string length: " << maxlen << endl;
    for (auto it = vocab.cbegin(); it != vocab.end(); ++it)
        if (it->first.length() == 1 && it->first != wb_symbol)
            all_chars[it->first] = 0.0;

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

    // Append word boundary symbol to the beginning of the words
    map<string, flt_type> temp_words;
    for (auto it = words.begin(); it != words.end(); ++it) {
        string key(it->first);
        temp_words[key.insert(0, wb_symbol)] += it->second;
    }
    words = temp_words;

    // Resegment and print cost
    gg.resegment_words(words, vocab, freqs);
    flt_type densum = gg.get_sum(freqs);
    flt_type cost = gg.get_cost(freqs, densum);
    cerr << "cost: " << cost << endl;

    // Remove word boundaries from vocabulary and words
    remove_wbs(freqs, wb_symbol);
    temp_words.clear();
    for (auto it = words.begin(); it != words.end(); ++it) {
        string key(it->first);
        temp_words[key.erase(0, 1)] += it->second;
    }
    words = temp_words;
    cerr << "\tremoved word break symbols\t" << "vocabulary size: " << freqs.size() << endl;

    // Cutoff
    gg.cutoff(freqs, (flt_type)cutoff_value);
    cerr << "\tcutoff: " << cutoff_value << "\t" << "vocabulary size: " << freqs.size() << endl;

    // Resegment without word boundaries
    vocab = freqs;
    densum = gg.get_sum(vocab);
    gg.freqs_to_logprobs(vocab, densum);
    assert_single_chars(vocab, all_chars, one_char_min_lp);

    gg.resegment_words(words, vocab, freqs);
    densum = gg.get_sum(freqs);
    cost = gg.get_cost(freqs, densum);
    cerr << "cost: " << cost << endl;

    vocab = freqs;
    gg.freqs_to_logprobs(vocab, densum);
    Unigrams::write_vocab(out_fname, vocab);

    exit(1);
}
