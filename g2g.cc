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


int main(int argc, char* argv[]) {

    float cutoff_value = 0.0;
    int n_candidates_per_iter = 5000;
    int removals_per_iter = 5000;
    int target_vocab_size = 30000;
    flt_type one_char_min_lp = -25.0;
    string initial_transitions_fname;
    string wordlist_fname;
    string msfg_fname;
    string transition_fname;

    // Popt documentation:
    // http://linux.die.net/man/3/popt
    // http://privatemisc.blogspot.fi/2012/12/popt-basic-example.html
    poptContext pc;
    struct poptOption po[] = {
        {"candidates", 'c', POPT_ARG_INT, &n_candidates_per_iter, 11002, NULL, "Number of candidate subwords to try to remove per iteration"},
        {"removals", 'r', POPT_ARG_INT, &removals_per_iter, 11003, NULL, "Number of removals per iteration"},
        {"vocab_size", 'g', POPT_ARG_INT, &target_vocab_size, 11007, NULL, "Target vocabulary size (stopping criterion)"},
        POPT_AUTOHELP
        {NULL}
    };

    pc = poptGetContext(NULL, argc, (const char **)argv, po, 0);
    poptSetOtherOptionHelp(pc, "[INITIAL TRANSITIONS] [WORDLIST] [MSFG_IN] [TRANSITIONS]");

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
        initial_transitions_fname.assign((char*)poptGetArg(pc));
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
        cerr << "Input MSFG file not set" << endl;
        exit(1);
    }

    if (poptPeekArg(pc) != NULL)
        transition_fname.assign((char*)poptGetArg(pc));
    else {
        cerr << "Transition file not set" << endl;
        exit(1);
    }

    cerr << "parameters, initial transitions: " << initial_transitions_fname << endl;
    cerr << "parameters, wordlist: " << wordlist_fname << endl;
    cerr << "parameters, msfg: " << msfg_fname << endl;
    cerr << "parameters, transitions: " << transition_fname << endl;
    cerr << "parameters, candidates per iteration: " << n_candidates_per_iter << endl;
    cerr << "parameters, removals per iteration: " << removals_per_iter << endl;
    cerr << "parameters, target vocab size: " << target_vocab_size << endl;

    int maxlen, word_maxlen;
    string start_end_symbol("*");
    map<string, flt_type> all_chars;
    map<string, flt_type> freqs;
    map<string, flt_type> words;
    MultiStringFactorGraph msfg(start_end_symbol);
    transitions_t transitions;
    transitions_t trans_stats;
    map<string, flt_type> unigram_stats;

    cerr << "Reading initial transitions " << initial_transitions_fname << endl;
    int retval = Bigrams::read_transitions(transitions, initial_transitions_fname);
    if (retval < 0) {
        cerr << "something went wrong reading transitions" << endl;
        exit(0);
    }

    cerr << "Reading word list " << wordlist_fname << endl;
    retval = Unigrams::read_vocab(wordlist_fname, words, word_maxlen);
    if (retval < 0) {
        cerr << "something went wrong reading word list" << endl;
        exit(0);
    }
    cerr << "\t" << "wordlist size: " << words.size() << endl;
    cerr << "\t" << "maximum word length: " << word_maxlen << endl;

    cerr << "Reading msfg " << msfg_fname << endl;
    msfg.read(msfg_fname);

    if (transitions.size() < msfg.factor_node_map.size()) {
        vector<string> to_remove;
        cerr << "Pruning " << msfg.factor_node_map.size()-transitions.size() << " unused transitions from msfg." << endl;
        for (auto it = msfg.factor_node_map.begin(); it != msfg.factor_node_map.end(); ++it)
            if (transitions.find(it->first) == transitions.end())
                to_remove.push_back(it->first);
        for (auto it = to_remove.begin(); it != to_remove.end(); ++it)
            msfg.remove_arcs(*it);
    }

    assign_scores(transitions, msfg);

    std::cerr << std::setprecision(15);
    int iteration = 1;
    while (true) {

        cerr << "Iteration " << iteration << endl;

        flt_type lp = Bigrams::collect_trans_stats(words, msfg, trans_stats, unigram_stats);
        Bigrams::copy_transitions(trans_stats, transitions);
        Bigrams::normalize(transitions);

        cerr << "\tbigram cost: " << lp << endl;
        cerr << "\tamount of transitions: " << Bigrams::transition_count(transitions) << endl;
        cerr << "\tvocab size: " << transitions.size() << endl;

        // Get removal candidates based on unigram stats
        cerr << "\tinitializing removals .." << endl;
        map<string, flt_type> candidates;
        Bigrams::init_removal_candidates(n_candidates_per_iter, unigram_stats, candidates);

        // Score all candidates
        cerr << "\tranking removals .." << endl;
        Bigrams::rank_removal_candidates(words, msfg, unigram_stats, transitions, candidates);

        // Remove least significant subwords
        vector<pair<string, flt_type> > sorted_scores;
        Unigrams::sort_vocab(candidates, sorted_scores, true);
        vector<string> to_remove;
        for (auto it = sorted_scores.begin(); it != sorted_scores.end(); ++it) {
            to_remove.push_back(it->first);
            if (iteration == 1) {
                if ((to_remove.size() >= transitions.size() % 1000) && (to_remove.size() > 0)) break;
            }
            else if (to_remove.size() >= removals_per_iter) break;
        }
        Bigrams::remove_transitions(to_remove, transitions);
        for (auto it = to_remove.begin(); it != to_remove.end(); ++it)
            msfg.remove_arcs(*it);

        // Write temp transitions
        ostringstream transitions_temp;
        transitions_temp << "transitions.iter" << iteration << ".bz2";
        cerr << "\twriting to: " << transitions_temp.str() << endl;
        Bigrams::write_transitions(transitions, transitions_temp.str());

        if  (transitions.size() <= target_vocab_size) break;
        iteration++;
    }

    // Write transitions
    Bigrams::write_transitions(transitions, transition_fname);

    exit(1);
}
