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
#include "GreedyUnigrams.hh"

using namespace std;


void assert_single_chars(map<string, flt_type> &vocab,
                         const map<string, flt_type> &chars,
                         flt_type val)
{
    for (auto it = chars.cbegin(); it != chars.cend(); ++it)
        if (vocab.find(it->first) == vocab.end())
            vocab[it->first] = val;
}


void collect_trans_stats(const map<pair<string,string>, flt_type> &transitions,
                         const map<string, flt_type> &words,
                         map<string, FactorGraph*> &fg_words,
                         map<pair<string,string>, flt_type> &trans_stats,
                         map<string, flt_type> &trans_normalizers)
{
    trans_stats.clear();
    trans_normalizers.clear();
    for (auto it = fg_words.begin(); it != fg_words.end(); ++it) {
        map<pair<string,string>, flt_type> stats;
        forward_backward(transitions, *it->second, stats);
        for (auto statit = stats.cbegin(); statit != stats.cend(); ++statit) {
            trans_stats[statit->first] += words.at(it->first) * statit->second;
            trans_normalizers[statit->first.first] += words.at(it->first) * statit->second;
        }
    }

}


void normalize(map<pair<string,string>, flt_type> &trans_stats,
               map<string, flt_type> &trans_normalizers)
{
    auto iter = trans_stats.begin();
    while (iter != trans_stats.end()) {
        iter->second /= trans_normalizers[iter->first.first];
        if (iter->second <= 0.0 || std::isnan(iter->second)) trans_stats.erase(iter++);
        else {
            iter->second = log(iter->second);
            ++iter;
        }
    }

}


void write_transitions(const map<pair<string,string>, flt_type> &transitions,
                       const string &filename)
{
    ofstream transfile(filename);
    if (!transfile) return;

    for (auto it = transitions.begin(); it != transitions.end(); ++it)
        transfile << it->first.first << " " << it->first.second << " " << it->second << endl;

    transfile.close();
}


flt_type bigram_cost(const map<pair<string,string>, flt_type> &transitions,
                     const map<pair<string,string>, flt_type> &trans_stats)
{
    flt_type total = 0.0;
    flt_type tmp = 0.0;
    for (auto iter = transitions.cbegin(); iter != transitions.cend(); ++iter) {
        tmp = iter->second * trans_stats.at(iter->first);
        if (!std::isnan(tmp)) total += tmp;
    }
    return total;
}


int main(int argc, char* argv[]) {

    float cutoff_value = 0.0;
    int target_vocab_size = 50000;
    flt_type one_char_min_lp = -25.0;
    string vocab_fname;
    string wordlist_fname;

    // Popt documentation:
    // http://linux.die.net/man/3/popt
    // http://privatemisc.blogspot.fi/2012/12/popt-basic-example.html
    poptContext pc;
    struct poptOption po[] = {
        {"cutoff", 'u', POPT_ARG_FLOAT, &cutoff_value, 11001, NULL, "Cutoff value for each iteration"},
        {"vocab_size", 'g', POPT_ARG_INT, &target_vocab_size, 11007, NULL, "Target vocabulary size (stopping criterion)"},
        POPT_AUTOHELP
        {NULL}
    };

    pc = poptGetContext(NULL, argc, (const char **)argv, po, 0);
    poptSetOtherOptionHelp(pc, "[INITIAL VOCABULARY] [WORDLIST]");

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

    cerr << "parameters, initial vocabulary: " << vocab_fname << endl;
    cerr << "parameters, wordlist: " << wordlist_fname << endl;
    cerr << "parameters, cutoff: " << setprecision(15) << cutoff_value << endl;
    cerr << "parameters, target vocab size: " << target_vocab_size << endl;

    int maxlen, word_maxlen;
    map<string, flt_type> all_chars;
    map<string, flt_type> vocab;
    map<string, flt_type> freqs;
    map<string, flt_type> words;

    cerr << "Reading vocabulary " << vocab_fname << endl;
    int retval = read_vocab(vocab_fname, vocab, maxlen);
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
    retval = read_vocab(wordlist_fname, words, word_maxlen);
    if (retval < 0) {
        cerr << "something went wrong reading word list" << endl;
        exit(0);
    }
    cerr << "\t" << "wordlist size: " << words.size() << endl;
    cerr << "\t" << "maximum word length: " << word_maxlen << endl;

    GreedyUnigrams gg;
    gg.set_segmentation_method(forward_backward);

    cerr << "Initial cutoff" << endl;
    gg.resegment_words(words, vocab, freqs);
    flt_type densum = gg.get_sum(freqs);
    flt_type cost = gg.get_cost(freqs, densum);
    cerr << "cost: " << cost << endl;

    gg.cutoff(freqs, (flt_type)cutoff_value);
    cerr << "\tcutoff: " << cutoff_value << "\t" << "vocabulary size: " << freqs.size() << endl;
    vocab = freqs;
    densum = gg.get_sum(vocab);
    gg.freqs_to_logprobs(vocab, densum);
    assert_single_chars(vocab, all_chars, one_char_min_lp);

    StringSet<flt_type> ss_vocab(vocab);
    map<string, FactorGraph*> fg_words;
    string start_end_symbol("*");
    map<pair<string,string>, flt_type> transitions;
    map<pair<string,string>, flt_type> trans_stats;
    map<string, flt_type> trans_normalizers;
    vocab[start_end_symbol] = 0.0;

    // Initial segmentation using unigram model
    for (auto it=words.cbegin(); it != words.cend(); ++it) {
        FactorGraph *fg = new FactorGraph(it->first, start_end_symbol, ss_vocab);
        fg_words[it->first] = fg;
        map<pair<string,string>, flt_type> stats;
        forward_backward(vocab, *fg, stats);
        for (auto statit = stats.cbegin(); statit != stats.cend(); ++statit) {
            trans_stats[statit->first] += it->second * statit->second;
            trans_normalizers[statit->first.first] += it->second * statit->second;
        }
    }
    cerr << "amount of transitions: " << trans_stats.size() << endl;
    transitions = trans_stats;
    normalize(transitions, trans_normalizers);
    cost = bigram_cost(transitions, trans_stats);
    cerr << "cost: " << cost << endl;

    // Re-estimate using bigram stats
    for (int i=0; i<50; i++) {
        collect_trans_stats(transitions, words, fg_words, trans_stats, trans_normalizers);
        transitions = trans_stats;
        normalize(transitions, trans_normalizers);
        cost = bigram_cost(transitions, trans_stats);
        cerr << "cost: " << cost << endl;
    }

    // Viterbi segmentations
    for (auto it=fg_words.begin(); it != fg_words.cend(); ++it) {
        std::vector<std::string> best_path;
        viterbi(transitions, *it->second, best_path);
        for (int i=0; i<best_path.size(); i++) {
            cout << best_path[i];
            if (i<best_path.size()-1) cout << " ";
        }
        cout << endl;
    }
    write_transitions(transitions, "transitions.out");

    // Clean up
    for (auto it = fg_words.begin(); it != fg_words.cend(); ++it)
        delete it->second;

    exit(1);
}

