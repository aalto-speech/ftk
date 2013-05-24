#include <cmath>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>

#include "defs.hh"
#include "GreedyUnigrams.hh"
#include "FactorEncoder.hh"
#include "StringSet.hh"

using namespace std;


void
update_trans_stats(const transitions_t &collected_stats,
                   flt_type weight,
                   transitions_t &trans_stats,
                   map<string, flt_type> &trans_normalizers,
                   map<string, flt_type> &unigram_stats)
{
    for (auto srcit = collected_stats.cbegin(); srcit != collected_stats.cend(); ++srcit) {
        for (auto tgtit = srcit->second.cbegin(); tgtit != srcit->second.cend(); ++tgtit) {
            trans_stats[srcit->first][tgtit->first] += weight * tgtit->second;
            trans_normalizers[srcit->first] += weight * tgtit->second;
            unigram_stats[tgtit->first] += weight * tgtit->second;
        }
    }
}


void
normalize(transitions_t &trans_stats,
          map<string, flt_type> &trans_normalizers,
          flt_type min_cost = -200.0)
{
    for (auto srcit = trans_stats.begin(); srcit != trans_stats.end(); ++srcit) {
        auto tgtit = srcit->second.begin();
        while (tgtit != srcit->second.end()) {
            tgtit->second /= trans_normalizers[srcit->first];
            tgtit->second = log(tgtit->second);
            if (tgtit->second < min_cost || std::isnan(tgtit->second))
                srcit->second.erase(tgtit++);
            else
                ++tgtit;
        }
    }
}


void
collect_trans_stats(const transitions_t &transitions,
                    const map<string, flt_type> &words,
                    map<string, FactorGraph*> &fg_words,
                    transitions_t &trans_stats,
                    map<string, flt_type> &trans_normalizers,
                    map<string, flt_type> &unigram_stats,
                    bool fb=true)
{
    trans_stats.clear();
    trans_normalizers.clear();
    unigram_stats.clear();

    for (auto it = fg_words.begin(); it != fg_words.end(); ++it) {
        transitions_t word_stats;
        if (fb)
            forward_backward(transitions, *it->second, word_stats);
        else
            viterbi(transitions, *it->second, word_stats);
        update_trans_stats(word_stats, words.at(it->first), trans_stats, trans_normalizers, unigram_stats);
    }
}


int main(int argc, char* argv[]) {

    int maxlen, word_maxlen;
    flt_type densum;
    string vocab_fname, wordlist_fname;
    map<string, flt_type> vocab;
    map<string, flt_type> words;
    map<string, flt_type> all_chars;
    map<string, flt_type> freqs;

    cerr << "Reading vocabulary " << argv[1] << endl;
    read_vocab(std::string(argv[1]), vocab, maxlen);

    cerr << "Reading word list " << argv[2] << endl;
    read_vocab(std::string(argv[2]), words, word_maxlen);


    // 1-GRAM POSTERIORS AFTER 5 FB ITERATIONS
    cerr << "Optimizing 1-gram probs w Forward-Backward" << endl;
    GreedyUnigrams gg(forward_backward);
    for (int i=0; i<5; i++) {
        gg.resegment_words(words, vocab, freqs);
        densum = gg.get_sum(freqs);
        vocab = freqs;
        gg.freqs_to_logprobs(vocab, densum);
    }

    StringSet<double> ss_vocab(vocab);
    ofstream outpost("posteriors.1gram");
    for (auto wordit = words.cbegin(); wordit != words.cend(); ++wordit) {
        vector<string> best_path;
        map<string, flt_type> stats;
        vector<flt_type> post_scores;

        viterbi(ss_vocab, wordit->first, best_path, true);
        forward_backward(ss_vocab, wordit->first, stats, post_scores);

        outpost << wordit->first << ": ";
        for (int i=0; i<best_path.size(); i++)
            outpost << best_path[i] << " ";
        for (int i=0; i<post_scores.size(); i++)
            outpost << exp(post_scores[i]) << " ";
        outpost << endl;
    }
    outpost.close();

    // 2-GRAM POSTERIORS AFTER 5 FB ITERATIONS
    cerr << "Optimizing 2-gram probs w Forward-Backward" << endl;
    map<string, FactorGraph*> fg_words;
    string start_end_symbol("*");
    transitions_t transitions;
    transitions_t trans_stats;
    map<string, flt_type> trans_normalizers;
    map<string, flt_type> unigram_stats;
    vocab[start_end_symbol] = 0.0;

    // Initial segmentation using unigram model
    for (auto it = words.cbegin(); it != words.cend(); ++it) {
        FactorGraph *fg = new FactorGraph(it->first, start_end_symbol, ss_vocab);
        fg_words[it->first] = fg;
        transitions_t word_stats;
        forward_backward(vocab, *fg, word_stats);
        update_trans_stats(word_stats, it->second, trans_stats, trans_normalizers, unigram_stats);
    }

    for (int i=0; i<5; i++) {
        collect_trans_stats(transitions, words, fg_words, trans_stats, trans_normalizers, unigram_stats);
        transitions = trans_stats;
        normalize(transitions, trans_normalizers);
    }

    outpost.open("posteriors.2gram");
    for (auto wordit = words.cbegin(); wordit != words.cend(); ++wordit) {
        vector<string> best_path;
        transitions_t stats;
        vector<flt_type> post_scores;

        viterbi(transitions, *fg_words[wordit->first], best_path, true);
        forward_backward(transitions, *fg_words[wordit->first], stats, post_scores);

        outpost << wordit->first << ": ";
        for (int i=0; i<best_path.size(); i++)
            outpost << best_path[i] << " ";
        for (int i=0; i<post_scores.size(); i++)
            outpost << exp(post_scores[i]) << " ";
        outpost << endl;
    }

    // Clean up
    for (auto it = fg_words.begin(); it != fg_words.cend(); ++it)
        delete it->second;

}
