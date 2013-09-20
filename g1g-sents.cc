#include <cmath>
#include <cstdlib>
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
    int target_vocab_size = 50000;
    bool enable_forward_backward = false;
    flt_type one_char_min_lp = -25.0;
    string vocab_fname;
    string corpus_fname;

    // Popt documentation:
    // http://linux.die.net/man/3/popt
    // http://privatemisc.blogspot.fi/2012/12/popt-basic-example.html
    poptContext pc;
    struct poptOption po[] = {
        {"cutoff", 'u', POPT_ARG_FLOAT, &cutoff_value, 11001, NULL, "Cutoff value for each iteration"},
        {"candidates", 'c', POPT_ARG_INT, &n_candidates_per_iter, 11002, NULL, "Number of candidate subwords to try to remove per iteration"},
        {"max_removals", 'a', POPT_ARG_INT, &max_removals_per_iter, 11003, NULL, "Maximum number of removals per iteration"},
        {"min_removals", 'i', POPT_ARG_INT, &min_removals_per_iter, 11004, NULL, "Minimum number of removals per iteration (stopping criterion)"},
        {"vocab_size", 'g', POPT_ARG_INT, &target_vocab_size, 11007, NULL, "Target vocabulary size (stopping criterion)"},
        {"forward_backward", 'f', POPT_ARG_NONE, &enable_forward_backward, 11007, "Use Forward-backward segmentation instead of Viterbi", NULL},
        POPT_AUTOHELP
        {NULL}
    };

    pc = poptGetContext(NULL, argc, (const char **)argv, po, 0);
    poptSetOtherOptionHelp(pc, "[INITIAL VOCABULARY] [TRAINING CORPUS]");

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
        corpus_fname.assign((char*)poptGetArg(pc));
    else {
        cerr << "Training corpus not set" << endl;
        exit(1);
    }

    cerr << "parameters, initial vocabulary: " << vocab_fname << endl;
    cerr << "parameters, training corpus: " << corpus_fname << endl;
    cerr << "parameters, cutoff: " << setprecision(15) << cutoff_value << endl;
    cerr << "parameters, candidates per iteration: " << n_candidates_per_iter << endl;
    cerr << "parameters, removals per iteration: " << max_removals_per_iter << endl;
    cerr << "below 100k.." << endl;
    cerr << "parameters, removals per iteration: 500" << endl;
    cerr << "parameters, min removals per iteration: " << min_removals_per_iter << endl;
    cerr << "parameters, target vocab size: " << target_vocab_size << endl;
    cerr << "parameters, use forward-backward: " << enable_forward_backward << endl;

    int maxlen;
    map<string, flt_type> all_chars;
    map<string, flt_type> vocab;
    map<string, flt_type> freqs;
    vector<string> sents;

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

    Unigrams gg;
    if (enable_forward_backward)
        gg.set_segmentation_method(forward_backward);
    else
        gg.set_segmentation_method(viterbi);

    cerr << "Reading training corpus " << corpus_fname << endl;
    Unigrams::read_sents(corpus_fname, sents);

    cerr << "Iterating cutoff" << endl;
    flt_type temp_cutoff = 0.0;
    flt_type cost = 0.0;
    while (true) {

        if (temp_cutoff >= cutoff_value) break;

        cost = gg.resegment_sents(sents, vocab, freqs);
        cerr << "cost: " << cost << endl;
        vocab = freqs;
        Unigrams::freqs_to_logprobs(vocab);
        assert_single_chars(vocab, all_chars, one_char_min_lp);

        ostringstream tempfname;
        tempfname << "cutoff." << temp_cutoff << ".vocab";
        Unigrams::write_vocab(tempfname.str(), vocab);

        Unigrams::cutoff(freqs, temp_cutoff);
        cerr << "\tcutoff: " << temp_cutoff << "\t" << "vocabulary size: " << freqs.size() << endl;
        vocab = freqs;
        Unigrams::freqs_to_logprobs(vocab);
        assert_single_chars(vocab, all_chars, one_char_min_lp);

        if (vocab.size() <= target_vocab_size) break;
        temp_cutoff += 1.0;
    }

    cerr << "Pruning factors" << endl;
    int itern = 1;
    while (true) {

        cerr << "iteration " << itern << endl;

        cerr << "candidates for removal: " << n_candidates_per_iter << endl;
        set<string> candidates;
        gg.init_candidates(n_candidates_per_iter, vocab, candidates);

        cerr << "ranking candidate factors" << endl;
        vector<pair<string, flt_type> > removal_scores;
        cost = gg.rank_candidates(sents, vocab, candidates, freqs, removal_scores);
        cerr << "cost: " << cost << endl;

        // Perform removals one by one
        unsigned int n_removals = 0;
        for (unsigned int i=0; i<removal_scores.size(); i++) {

            if (removal_scores[i].first.length() == 1) continue;
            // Score most probably went to zero already
            if (vocab.find(removal_scores[i].first) == vocab.end()) continue;
            if (freqs.find(removal_scores[i].first) == freqs.end()) continue;

            vocab.erase(removal_scores[i].first);
            freqs.erase(removal_scores[i].first);
            n_removals++;

            if (vocab.size() % 5000 == 0) {
                vocab = freqs;
                Unigrams::freqs_to_logprobs(vocab);
                assert_single_chars(vocab, all_chars, one_char_min_lp);
                ostringstream vocabfname;
                vocabfname << "iter" << itern << "_" << vocab.size() << ".vocab";
                Unigrams::write_vocab(vocabfname.str(), vocab);
            }

            if (n_removals >= max_removals_per_iter) break;
            if (vocab.size() <= target_vocab_size) break;
        }

        int n_cutoff = Unigrams::cutoff(freqs, cutoff_value);
        vocab = freqs;
        Unigrams::freqs_to_logprobs(vocab);
        assert_single_chars(vocab, all_chars, one_char_min_lp);
        cost = gg.iterate(sents, vocab, 2);
        assert_single_chars(vocab, all_chars, one_char_min_lp);

        cerr << "factors removed in this iteration: " << n_removals << endl;
        cerr << "factors removed with cutoff this iteration: " << n_cutoff << endl;
        cerr << "current vocabulary size: " << vocab.size() << endl;
        cerr << "likelihood after the removals: " << cost << endl;

        ostringstream vocabfname;
        vocabfname << "iter" << itern << ".vocab";
        Unigrams::write_vocab(vocabfname.str(), vocab);

        itern++;

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

