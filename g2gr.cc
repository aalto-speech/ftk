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
#include "FactorEncoder.hh"
#include "Unigrams.hh"
#include "Bigrams.hh"

using namespace std;


int main(int argc, char* argv[]) {

    conf::Config config;
    config("usage: g2gr [OPTION...] WORDLIST TRANSITIONS_INIT MSFG_IN TRANSITIONS_OUT\n")
      ('h', "help", "", "", "display help")
      ('r', "removals=INT", "arg", "500", "Number of removals per iteration")
      ('v', "vocab-size=INT", "arg must", "30000", "Target vocabulary size (stopping criterion)");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 4) config.print_help(stderr, 1);

    int n_candidates_per_iter = config["candidates"].get_int();
    int removals_per_iter = config["removals"].get_int();
    int target_vocab_size = config["vocab-size"].get_int();
    string wordlist_fname = config.arguments[0];
    string initial_transitions_fname = config.arguments[1];
    string msfg_fname = config.arguments[2];
    string transition_fname = config.arguments[3];

    cerr << "parameters, initial transitions: " << initial_transitions_fname << endl;
    cerr << "parameters, wordlist: " << wordlist_fname << endl;
    cerr << "parameters, msfg: " << msfg_fname << endl;
    cerr << "parameters, transitions: " << transition_fname << endl;
    cerr << "parameters, removals per iteration: " << removals_per_iter << endl;
    cerr << "parameters, target vocab size: " << target_vocab_size << endl;
    cerr << "parameters, floor lp: " << FLOOR_LP << endl;

    int maxlen, word_maxlen;
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

        // Write temp transitions
        ostringstream transitions_temp;
        transitions_temp << "transitions.iter" << iteration << ".bz2";
        cerr << "\twriting to: " << transitions_temp.str() << endl;
        Bigrams::write_transitions(transitions, transitions_temp.str());

        cerr << "\tinitializing removals .." << endl;
        map<string, flt_type> removals;
        if (iteration == 1 && transitions.size() % 1000 != 0) {
            int first_iter_removals = transitions.size() % 1000;
            Bigrams::init_removal_candidates(first_iter_removals, unigram_stats, removals);
        }
        else
            Bigrams::init_removal_candidates(removals_per_iter, unigram_stats, removals);

        vector<string> to_remove;
        for (auto it = removals.begin(); it != removals.end(); ++it) {
            to_remove.push_back(it->first);
        }
        Bigrams::remove_transitions(to_remove, transitions);
        for (auto it = to_remove.begin(); it != to_remove.end(); ++it)
            msfg.remove_arcs(*it);

        if  (transitions.size() <= target_vocab_size) break;
        iteration++;
    }

    // Write transitions
    Bigrams::write_transitions(transitions, transition_fname);

    exit(1);
}
