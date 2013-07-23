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
    int n_candidates_per_iter = 5000;
    int max_removals_per_iter = 5000;
    int min_removals_per_iter = 0;
    float threshold = -25.0;
    float threshold_decrease = 25.0;
    int target_vocab_size = 50000;
    bool enable_forward_backward = false;
    bool random_candidates = false;
    flt_type one_char_min_lp = -25.0;
    string vocab_fname;
    string wordlist_fname;

    // Popt documentation:
    // http://linux.die.net/man/3/popt
    // http://privatemisc.blogspot.fi/2012/12/popt-basic-example.html
    poptContext pc;
    struct poptOption po[] = {
        {"cutoff", 'u', POPT_ARG_FLOAT, &cutoff_value, 11001, NULL, "Cutoff value for each iteration"},
        {"candidates", 'c', POPT_ARG_INT, &n_candidates_per_iter, 11002, NULL, "Number of candidate subwords to try to remove per iteration"},
        {"max_removals", 'a', POPT_ARG_INT, &max_removals_per_iter, 11003, NULL, "Maximum number of removals per iteration"},
        {"min_removals", 'i', POPT_ARG_INT, &min_removals_per_iter, 11004, NULL, "Minimum number of removals per iteration (stopping criterion)"},
        {"threshold", 't', POPT_ARG_FLOAT, &threshold, 11005, NULL, "Likelihood threshold for removals"},
        {"threshold_decrease", 'd', POPT_ARG_FLOAT, &threshold_decrease, 11006, NULL, "Threshold decrease between iterations"},
        {"vocab_size", 'g', POPT_ARG_INT, &target_vocab_size, 11007, NULL, "Target vocabulary size (stopping criterion)"},
        {"forward_backward", 'f', POPT_ARG_NONE, &enable_forward_backward, 11007, "Use Forward-backward segmentation instead of Viterbi", NULL},
        {"random_init", 'r', POPT_ARG_NONE, &random_candidates, 11007, "Select removal candidates by random", NULL},
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
    cerr << "parameters, candidates per iteration: " << n_candidates_per_iter << endl;
    cerr << "parameters, removals per iteration: " << max_removals_per_iter << endl;
    cerr << "parameters, threshold: " << threshold << endl;
    cerr << "parameters, threshold decrease per iteration: " << threshold_decrease << endl;
    cerr << "parameters, min removals per iteration: " << min_removals_per_iter << endl;
    cerr << "parameters, target vocab size: " << target_vocab_size << endl;
    cerr << "parameters, use forward-backward: " << enable_forward_backward << endl;
    cerr << "parameters, random candidates: " << random_candidates << endl;

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

    Unigrams gg;
    if (enable_forward_backward)
        gg.set_segmentation_method(forward_backward);
    else
        gg.set_segmentation_method(viterbi);

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

    cerr << "Removing subwords one by one" << endl;
    int itern = 1;
    while (true) {

        cerr << "iteration " << itern << endl;

        cerr << "collecting candidate subwords for removal" << endl;
        map<string, map<string, flt_type> > diffs;
        if ((int)vocab.size()-n_candidates_per_iter < target_vocab_size) n_candidates_per_iter = (int)vocab.size()-target_vocab_size;
        if (random_candidates)
            gg.init_removal_candidates_by_random(n_candidates_per_iter, words, vocab, diffs);
        else
            gg.init_removal_candidates(n_candidates_per_iter, words, vocab, diffs);

        cerr << "ranking candidate subwords" << endl;
        vector<pair<string, flt_type> > removal_scores;
        gg.rank_removal_candidates(words, vocab, diffs, freqs, removal_scores);

        // Perform removals one by one if likelihood change above threshold
        flt_type curr_densum = gg.get_sum(freqs);
        flt_type curr_cost = gg.get_cost(freqs, curr_densum);
        map<string, map<string, flt_type> > backpointers;
        gg.get_backpointers(words, vocab, backpointers);

        cerr << "starting cost before removing subwords one by one: " << curr_cost << endl;

        unsigned int n_removals = 0;
        for (unsigned int i=0; i<removal_scores.size(); i++) {

            if (removal_scores[i].first.length() == 1) continue;
            // Score most probably went to zero already
            if (vocab.find(removal_scores[i].first) == vocab.end()) continue;

            cout << removal_scores[i].first << "\t" << "expected ll diff: " << removal_scores[i].second << endl;

            map<string, flt_type> freq_diffs;
            map<string, map<string, flt_type> > backpointers_to_remove;
            map<string, map<string, flt_type> > backpointers_to_add;
            StringSet stringset_vocab(vocab);
            gg.hypo_removal(stringset_vocab, removal_scores[i].first, backpointers,
                            backpointers_to_remove, backpointers_to_add, freq_diffs);

            flt_type hypo_densum = gg.get_sum(freqs, freq_diffs);
            flt_type hypo_cost = gg.get_cost(freqs, freq_diffs, hypo_densum);

            cout << removal_scores[i].first << "\t" << "change in likelihood: " << (hypo_cost-curr_cost);
            if ((hypo_cost-curr_cost) < threshold) {
                cout << " was below threshold " << threshold << endl;
                continue;
            }
            cout << " removed, was above threshold " << threshold << endl;

            gg.apply_freq_diffs(freqs, freq_diffs);
            freqs.erase(removal_scores[i].first);
            gg.apply_backpointer_changes(backpointers, backpointers_to_remove, backpointers_to_add);
            backpointers.erase(removal_scores[i].first);

            curr_densum = hypo_densum;
            curr_cost = hypo_cost;
            vocab = freqs;
            gg.freqs_to_logprobs(vocab, hypo_densum);
            assert_single_chars(vocab, all_chars, one_char_min_lp);

            n_removals++;

            if (vocab.size() % 5000 == 0) {
                ostringstream vocabfname;
                vocabfname << "iter" << itern << "_" << vocab.size() << ".vocab";
                Unigrams::write_vocab(vocabfname.str(), vocab);
            }

            if (n_removals >= max_removals_per_iter) break;
            if (vocab.size() <= target_vocab_size) break;
        }

        int n_cutoff = gg.cutoff(freqs, cutoff_value);
        flt_type co_densum = gg.get_sum(freqs);
        vocab = freqs;
        gg.freqs_to_logprobs(vocab, co_densum);
        assert_single_chars(vocab, all_chars, one_char_min_lp);
        gg.resegment_words(words, vocab, freqs);
        curr_densum = gg.get_sum(freqs);
        curr_cost = gg.get_cost(freqs, curr_densum);

        cerr << "subwords removed in this iteration: " << n_removals << endl;
        cerr << "subwords removed with cutoff this iteration: " << n_cutoff << endl;
        cerr << "current vocabulary size: " << vocab.size() << endl;
        cerr << "likelihood after the removals: " << curr_cost << endl;

        ostringstream vocabfname;
        vocabfname << "iter" << itern << ".vocab";
        Unigrams::write_vocab(vocabfname.str(), vocab);

        itern++;
        threshold -= threshold_decrease;

        if (n_removals < min_removals_per_iter) {
            cerr << "stopping by min_removals_per_iter." << endl;
            break;
        }

        if (vocab.size() <= target_vocab_size) {
            cerr << "stopping by min_vocab_size." << endl;
            break;
        }
    }

    exit(1);
}

