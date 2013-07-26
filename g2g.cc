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
    int n_candidates_per_iter = 5000;
    int max_removals_per_iter = 5000;
    int target_vocab_size = 30000;
    flt_type one_char_min_lp = -25.0;
    string vocab_fname;
    string wordlist_fname;
    string msfg_fname;
    string transition_fname;

    // Popt documentation:
    // http://linux.die.net/man/3/popt
    // http://privatemisc.blogspot.fi/2012/12/popt-basic-example.html
    poptContext pc;
    struct poptOption po[] = {
        {"candidates", 'c', POPT_ARG_INT, &n_candidates_per_iter, 11002, NULL, "Number of candidate subwords to try to remove per iteration"},
        {"max_removals", 'a', POPT_ARG_INT, &max_removals_per_iter, 11003, NULL, "Maximum number of removals per iteration"},
        {"cutoff", 'u', POPT_ARG_FLOAT, &cutoff_value, 11001, NULL, "Cutoff value for each iteration"},
        {"vocab_size", 'g', POPT_ARG_INT, &target_vocab_size, 11007, NULL, "Target vocabulary size (stopping criterion)"},
        POPT_AUTOHELP
        {NULL}
    };

    pc = poptGetContext(NULL, argc, (const char **)argv, po, 0);
    poptSetOtherOptionHelp(pc, "[INITIAL VOCABULARY] [WORDLIST] [MSFG_IN] [TRANSITIONS]");

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
        cerr << "Input MSFG file not set" << endl;
        exit(1);
    }

    if (poptPeekArg(pc) != NULL)
        transition_fname.assign((char*)poptGetArg(pc));
    else {
        cerr << "Transition file not set" << endl;
        exit(1);
    }

    cerr << "parameters, initial vocabulary: " << vocab_fname << endl;
    cerr << "parameters, wordlist: " << wordlist_fname << endl;
    cerr << "parameters, msfg: " << msfg_fname << endl;
    cerr << "parameters, transitions: " << transition_fname << endl;
    cerr << "parameters, candidates per iteration: " << n_candidates_per_iter << endl;
    cerr << "parameters, removals per iteration: " << max_removals_per_iter << endl;
    cerr << "parameters, cutoff: " << setprecision(15) << cutoff_value << endl;
    cerr << "parameters, target vocab size: " << target_vocab_size << endl;

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

    string start_end_symbol("*");
    StringSet ss_vocab(vocab);
    map<string, FactorGraph*> fg_words;
    MultiStringFactorGraph msfg(start_end_symbol);
    msfg.read(msfg_fname);
    transitions_t transitions;
    transitions_t trans_stats;
    map<string, flt_type> unigram_stats;
    vocab[start_end_symbol] = 0.0;
    flt_type lp = 0.0;

    // Initial segmentation using unigram model
    assign_scores(vocab, msfg);
    lp = Bigrams::collect_trans_stats(words, msfg, trans_stats, unigram_stats);

    // Unigram cost with word end markers
    densum = ug.get_sum(unigram_stats);
    cost = ug.get_cost(unigram_stats, densum);
    cerr << "unigram cost with word end symbols: " << cost << endl;

    // Initial bigram cost
    transitions = trans_stats;
    Bigrams::normalize(transitions);
    cerr << "bigram cost: " << lp << endl;
    cerr << "\tamount of transitions: " << Bigrams::transition_count(transitions) << endl;
    cerr << "\tvocab size: " << unigram_stats.size() << endl;
    int co_removed = Bigrams::cutoff(unigram_stats, cutoff_value, transitions, msfg);
    cerr << "\tremoved by cutoff: " << co_removed << endl;

    assign_scores(transitions, msfg);

    // Re-estimate using bigram stats
    int iteration = 1;
    while (true) {

        cerr << "Iteration " << iteration << endl;

        flt_type lp = Bigrams::collect_trans_stats(words, msfg, trans_stats, unigram_stats);
        Bigrams::copy_transitions(trans_stats, transitions);
        Bigrams::normalize(transitions);

        cerr << "\tbigram cost: " << lp << endl;
        cerr << "\tamount of transitions: " << Bigrams::transition_count(transitions) << endl;
        cerr << "\tvocab size: " << unigram_stats.size() << endl;

        // Get removal candidates based on unigram stats
        cerr << "\titializing removals .." << endl;
        vector<pair<string, flt_type> > sorted_stats;
        Unigrams::sort_vocab(unigram_stats, sorted_stats, false);
        map<string, flt_type> candidates;
        for (auto it = sorted_stats.begin(); it != sorted_stats.end(); ++it) {
            if (it->first.length() < 2) continue;
            candidates[it->first] = 0.0;
            if (candidates.size() >= n_candidates_per_iter) break;
        }

        // Score all candidates
        cerr << "\tranking removals .." << endl;
        map<string, set<string> > backpointers;
        Bigrams::get_backpointers(msfg, backpointers, 1);
        transitions_t reverse;
        Bigrams::reverse_transitions(transitions, reverse);
        int cidx = 0;
        for (auto it = candidates.begin(); it != candidates.end(); ++it) {
            transitions_t changes;
            set<string> words_to_resegment = backpointers.at(it->first);
            flt_type orig_score = likelihood(words, words_to_resegment, msfg);
            flt_type context_score = Bigrams::remove_string(reverse, it->first,
                                                            unigram_stats, transitions, changes);
            flt_type hypo_score = likelihood(words, words_to_resegment, msfg);
            it->second = hypo_score-orig_score + context_score;
            Bigrams::restore_string(transitions, changes);
            if (cidx % 1000 == 0) cout << "\tcandidate " << cidx << endl;
            cidx++;
        }

        // Remove least significant subwords
        vector<pair<string, flt_type> > sorted_scores;
        Unigrams::sort_vocab(candidates, sorted_scores, true);
        vector<string> to_remove;
        for (auto it = sorted_scores.begin(); it != sorted_scores.end(); ++it) {
            to_remove.push_back(it->first);
            if (to_remove.size() >= max_removals_per_iter) break;
        }
        Bigrams::remove_transitions(to_remove, transitions);
        for (int i=0; i<to_remove.size(); i++)
            msfg.remove_arcs(to_remove[i]);

        // Write temp transitions
        ostringstream transitions_temp;
        transitions_temp << "transitions.iter" << iteration << ".bz2";
        cerr << "\twriting to: " << transitions_temp.str() << endl;
        Bigrams::write_transitions(transitions, transitions_temp.str());

        if  (unigram_stats.size() <= target_vocab_size) break;
        iteration++;
    }

    // Write transitions
    Bigrams::write_transitions(transitions, transition_fname);

    exit(1);
}
