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

    conf::Config config;
    config("usage: g1g [OPTION...] WORDLIST VOCAB_INIT VOCAB_OUTNAME\n")
      ('h', "help", "", "", "display help")
      ('u', "cutoff=INT", "arg", "0", "Cutoff value for each iteration")
      ('c', "candidates=INT", "arg", "25000", "Number of subwords to consider for removal per iteration")
      ('r', "removals=INT", "arg", "500", "Number of removals per iteration")
      ('v', "vocab-size=INT", "arg must", "", "Target vocabulary size (stopping criterion)")
      ('t', "temp-vocabs=INT", "arg", "0", "Write out intermediate vocabularies for #V mod INT == 0")
      ('f', "forward-backward", "", "", "Use Forward-backward segmentation instead of Viterbi");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 3) config.print_help(stderr, 1);

    flt_type one_char_min_lp = -25.0;
    string wordlist_fname = config.arguments[0];
    string vocab_fname = config.arguments[1];
    string out_vocab_fname = config.arguments[2];
    float cutoff_value = config["cutoff"].get_float();
    int n_candidates_per_iter = config["candidates"].get_int();
    int removals_per_iter = config["removals"].get_int();
    int target_vocab_size = config["vocab-size"].get_int();
    int temp_vocab_interval = config["temp-vocabs"].get_int();
    bool enable_forward_backward = config["forward-backward"].specified;

    cerr << "parameters, wordlist: " << wordlist_fname << endl;
    cerr << "parameters, initial vocabulary: " << vocab_fname << endl;
    cerr << "parameters, cutoff: " << setprecision(15) << cutoff_value << endl;
    cerr << "parameters, candidates per iteration: " << n_candidates_per_iter << endl;
    cerr << "parameters, removals per iteration: " << removals_per_iter << endl;
    cerr << "parameters, target vocab size: " << target_vocab_size << endl;
    if (temp_vocab_interval > 0)
        cerr << "parameters, write temp vocabs: " << temp_vocab_interval << endl;
    else
        cerr << "parameters, write temp vocabs: NO" << endl;
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
    for (auto it = vocab.cbegin(); it != vocab.end(); ++it) {
        if (it->first.length() == 1)
            all_chars[it->first] = 0.0;
    }

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
    flt_type cost = gg.resegment_words(words, vocab, freqs);
    cerr << "cost: " << cost << endl;

    flt_type temp_cutoff = 1.0;
    while (true) {
        gg.cutoff(freqs, temp_cutoff);
        cerr << "\tcutoff: " << temp_cutoff << "\t" << "vocabulary size: " << freqs.size() << endl;
        vocab = freqs;
        Unigrams::freqs_to_logprobs(vocab);
        assert_single_chars(vocab, all_chars, one_char_min_lp);
        temp_cutoff += 1.0;
        if (temp_cutoff > (flt_type)cutoff_value) break;
        cost = gg.resegment_words(words, vocab, freqs);
        cerr << "cost: " << cost << endl;
    }

    cerr << "Removing subwords one by one" << endl;
    int itern = 1;
    while (true) {

        cerr << "iteration " << itern << endl;

        cerr << "collecting candidate subwords for removal" << endl;
        set<string> candidates;
        if ((int)vocab.size()-n_candidates_per_iter < target_vocab_size) n_candidates_per_iter = (int)vocab.size()-target_vocab_size;
        gg.init_candidates_by_usage(words, vocab, candidates, n_candidates_per_iter/3);
        gg.init_candidates_by_random(vocab, candidates, (n_candidates_per_iter-candidates.size())/3);
        gg.init_candidates(vocab, candidates, n_candidates_per_iter-candidates.size());

        cerr << "ranking candidate subwords (" << candidates.size() << ")" << endl;
        vector<pair<string, flt_type> > removal_scores;
        cost = gg.rank_candidates(words, vocab, candidates, freqs, removal_scores);

        cerr << "starting cost before removing subwords one by one: " << cost << endl;

        // Remove subwords one by one
        unsigned int n_removals = 0;
        for (unsigned int i=0; i<removal_scores.size(); i++) {

            if (removal_scores[i].first.length() == 1) continue;
            // Score most probably went to zero already
            if (vocab.find(removal_scores[i].first) == vocab.end()) continue;
            if (freqs.find(removal_scores[i].first) == freqs.end()) continue;

            vocab.erase(removal_scores[i].first);
            freqs.erase(removal_scores[i].first);
            n_removals++;

            if (temp_vocab_interval > 0 && vocab.size() % temp_vocab_interval == 0) {
                vocab = freqs;
                Unigrams::freqs_to_logprobs(vocab);
                assert_single_chars(vocab, all_chars, one_char_min_lp);
                ostringstream vocabfname;
                vocabfname << "iteration_" << itern << "_" << vocab.size() << ".vocab";
                Unigrams::write_vocab(vocabfname.str(), vocab);
            }

            if (n_removals >= removals_per_iter) break;
            if (vocab.size() <= target_vocab_size) break;
        }

        int n_cutoff = Unigrams::cutoff(freqs, cutoff_value);
        vocab = freqs;
        Unigrams::freqs_to_logprobs(vocab);
        assert_single_chars(vocab, all_chars, one_char_min_lp);
        cost = gg.iterate(words, vocab, 2);
        assert_single_chars(vocab, all_chars, one_char_min_lp);

        cerr << "subwords removed in this iteration: " << n_removals << endl;
        cerr << "subwords removed with cutoff this iteration: " << n_cutoff << endl;
        cerr << "current vocabulary size: " << vocab.size() << endl;
        cerr << "likelihood after the removals: " << cost << endl;

        itern++;

        if (vocab.size() <= target_vocab_size) {
            cerr << "stopping by min_vocab_size." << endl;
            break;
        }
    }

    exit(1);
}
