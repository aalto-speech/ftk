#include <algorithm>
#include <iostream>

#include "FactorEncoder.hh"
#include "GreedyUnigrams.hh"

using namespace std;


void
GreedyUnigrams::resegment_words(const map<string, double> &words,
                                const map<string, double> &vocab,
                                map<string, double> &new_freqs)
{
    new_freqs.clear();
    MorphSet morphset_vocab(vocab);

    for (auto worditer = words.cbegin(); worditer != words.cend(); ++worditer) {

        map<string, double> stats;
        segf(morphset_vocab, worditer->first, stats);

        if (stats.size() == 0) {
            cerr << "warning, no segmentation for word: " << worditer->first << endl;
            exit(0);
        }

        // Update statistics
        for (auto it = stats.begin(); it != stats.end(); ++it)
            new_freqs[it->first] += worditer->second * it->second;
    }
}


void
GreedyUnigrams::resegment_words_w_diff(const map<string, double> &words,
                                       const map<string, double> &vocab,
                                       map<string, double> &new_freqs,
                                       map<string, map<string, double> > &diffs)
{
    new_freqs.clear();
    MorphSet morphset_vocab(vocab);
    MorphSet hypo_vocab(vocab);

    for (auto worditer = words.cbegin(); worditer != words.cend(); ++worditer) {

        map<string, double> stats;
        segf(morphset_vocab, worditer->first, stats);

        if (stats.size() == 0) {
            cerr << "warning, no segmentation for word: " << worditer->first << endl;
            exit(0);
        }

        // Update statistics
        for (auto it = stats.cbegin(); it != stats.cend(); ++it)
            new_freqs[it->first] += worditer->second * it->second;

        // Hypothesize what the segmentation would be if some subword didn't exist
        for (auto hypoiter = stats.cbegin(); hypoiter != stats.cend(); ++hypoiter) {

            // If wanting to hypothesize removal of this subword
            if (diffs.find(hypoiter->first) != diffs.end()) {

                double stored_value = hypo_vocab.remove(hypoiter->first);
                map<string, double> hypo_stats;

                segf(hypo_vocab, worditer->first, hypo_stats);

                if (hypo_stats.size() == 0) {
                    cerr << "warning, no hypo segmentation for word: " << worditer->first << endl;
                    exit(0);
                }

                for (auto it = stats.cbegin(); it != stats.cend(); ++it)
                    diffs[hypoiter->first][it->first] -= worditer->second * it->second;
                for (auto it = hypo_stats.cbegin(); it != hypo_stats.cend(); ++it)
                    diffs[hypoiter->first][it->first] += worditer->second * it->second;

                hypo_vocab.add(hypoiter->first, stored_value);
            }
        }
    }
}


double
GreedyUnigrams::get_sum(const map<string, double> &freqs)
{
    double total = 0.0;
    for (auto iter = freqs.cbegin(); iter != freqs.cend(); ++iter)
        total += iter->second;
    return total;
}


double
GreedyUnigrams::get_sum(const map<string, double> &freqs,
                        const map<string, double> &freq_diffs)
{
    double total = 0.0;
    for (auto iter = freqs.cbegin(); iter != freqs.cend(); ++iter)
        total += iter->second;
    for (auto iter = freq_diffs.cbegin(); iter != freq_diffs.cend(); ++iter)
        total += iter->second;
    return total;
}


double
GreedyUnigrams::get_cost(const map<string, double> &freqs,
                         double densum)
{
    double total = 0.0;
    double tmp = 0.0;
    densum = log2(densum);
    for (auto iter = freqs.cbegin(); iter != freqs.cend(); ++iter) {
        tmp = iter->second * (log2(iter->second)-densum);
        if (!std::isnan(tmp)) total += tmp;
    }
    return total;
}


double
GreedyUnigrams::get_cost(const map<string, double> &freqs,
                         const map<string, double> &freq_diffs,
                         double densum)
{
    double total = 0.0;
    double tmp = 0.0;
    densum = log2(densum);
    for (auto iter = freqs.cbegin(); iter != freqs.cend(); ++iter) {
        tmp = iter->second;
        if (freq_diffs.find(iter->first) != freq_diffs.end())
            tmp += freq_diffs.at(iter->first);
        tmp = tmp * (log2(tmp)-densum);
        if (!std::isnan(tmp)) total += tmp;
    }
    return total;
}


void
GreedyUnigrams::apply_freq_diffs(map<string, double> &freqs,
                                 const map<string, double> &freq_diffs)
{
    for (auto iter = freq_diffs.cbegin(); iter != freq_diffs.cend(); ++iter)
        freqs[iter->first] += iter->second;

    // http://stackoverflow.com/questions/8234779/how-to-remove-from-a-map-while-iterating-it
    auto iter = freqs.begin();
    while (iter != freqs.end()) {
        if (iter->second <= 0.0) freqs.erase(iter++);
        else ++iter;
    }
}


void
GreedyUnigrams::apply_backpointer_changes(map<string, map<string, double> > &backpointers,
                                          const map<string, map<string, double> > &bps_to_remove,
                                          const map<string, map<string, double> > &bps_to_add)
{
    for (auto switer = bps_to_remove.cbegin(); switer != bps_to_remove.cend(); ++switer)
        for (auto worditer = switer->second.cbegin(); worditer != switer->second.cend(); ++worditer)
            backpointers[switer->first].erase(worditer->first);
    for (auto switer = bps_to_add.cbegin(); switer != bps_to_add.cend(); ++switer)
        for (auto worditer = switer->second.cbegin(); worditer != switer->second.cend(); ++worditer)
            backpointers[switer->first][worditer->first] = worditer->second;
}


void
GreedyUnigrams::freqs_to_logprobs(map<string, double> &vocab,
                                  double densum)
{
    densum = log2(densum);
    for (auto iter = vocab.begin(); iter != vocab.end(); ++iter)
        iter->second = (log2(iter->second)-densum);
}


int
GreedyUnigrams::cutoff(map<string, double> &vocab,
                       double limit)
{
    // http://stackoverflow.com/questions/8234779/how-to-remove-from-a-map-while-iterating-it
    int nremovals = 0;
    auto iter = vocab.begin();
    while (iter != vocab.end()) {
        if (iter->second <= limit && iter->first.length() > 1) {
            vocab.erase(iter++);
            nremovals++;
        }
        else ++iter;
    }
    return nremovals;
}


// Select n_candidates number of subwords in the vocabulary as removal candidates
// running from the least common subword
void
GreedyUnigrams::init_removal_candidates(int n_candidates,
                                        const map<string, double> &words,
                                        const map<string, double> &vocab,
                                        map<string, map<string, double> > &diffs)
{
    map<string, double> new_morph_freqs;
    resegment_words(words, vocab, new_morph_freqs);

    vector<pair<string, double> > sorted_vocab;
    sort_vocab(new_morph_freqs, sorted_vocab, false);

    for (int i=0; i<n_candidates; i++) {
        pair<string, double> &subword = sorted_vocab[i];
        map<string, double> emptymap;
        if (subword.first.length() > 1)
            diffs[subword.first] = emptymap;
    }
}


bool rank_desc_sort(pair<string, double> i,pair<string, double> j) { return (i.second > j.second); }

// Perform each of the removals (independent of others in the list) to get
// initial order for the removals
void
GreedyUnigrams::rank_removal_candidates(const map<string, double> &words,
                                        const map<string, double> &vocab,
                                        map<string, map<string, double> > &diffs,
                                        map<string, double> &new_morph_freqs,
                                        vector<pair<string, double> > &removal_scores)
{
    new_morph_freqs.clear();
    removal_scores.clear();

    resegment_words_w_diff(words, vocab, new_morph_freqs, diffs);
    double densum = get_sum(new_morph_freqs);
    double cost = get_cost(new_morph_freqs, densum);

    for (auto iter = diffs.begin(); iter != diffs.end(); ++iter) {
        double hypo_densum = get_sum(new_morph_freqs, iter->second);
        double hypo_cost = get_cost(new_morph_freqs, iter->second, hypo_densum);
        pair<string, double> removal_score = make_pair(iter->first, hypo_cost-cost);
        removal_scores.push_back(removal_score);
    }

    sort(removal_scores.begin(), removal_scores.end(), rank_desc_sort);
}


void
GreedyUnigrams::get_backpointers(const map<string, double> &words,
                                 const map<string, double> &vocab,
                                 map<string, map<string, double> > &backpointers)
{
    backpointers.clear();
    MorphSet morphset_vocab(vocab);

    for (auto worditer = words.cbegin(); worditer != words.cend(); ++worditer) {

        map<string, double> stats;
        segf(morphset_vocab, worditer->first, stats);

        if (stats.size() == 0) {
            cerr << "warning, no segmentation for word: " << worditer->first << endl;
            exit(0);
        }

        // Store backpointers
        for (auto it = stats.cbegin(); it != stats.cend(); ++it)
            backpointers[it->first][worditer->first] += worditer->second * it->second;
    }
}


// Hypothesizes removal and gives out updated freqs
void
GreedyUnigrams::hypo_removal(MorphSet &vocab,
                             const string &subword,
                             const map<string, map<string, double> > &backpointers,
                             map<string, map<string, double> > &backpointers_to_remove,
                             map<string, map<string, double> > &backpointers_to_add,
                             map<string, double> &freq_diffs)
{
    backpointers_to_remove.clear();
    backpointers_to_add.clear();
    freq_diffs.clear();

    for (auto worditer = backpointers.at(subword).cbegin(); worditer != backpointers.at(subword).cend(); ++worditer) {

        map<string, double> stats;
        segf(vocab, worditer->first, stats);

        map<string, double> hypo_stats;
        double stored_value = vocab.remove(subword);
        segf(vocab, worditer->first, hypo_stats);
        vocab.add(subword, stored_value);

        if (stats.size() == 0) {
            cerr << "warning, no segmentation for word: " << worditer->first << endl;
            exit(0);
        }

        if (hypo_stats.size() == 0) {
            cerr << "warning, no hypo segmentation for word: " << worditer->first << endl;
            exit(0);
        }

        // Collect frequency differences
        // Collect backpointer changes
        for (auto it = stats.cbegin(); it != stats.cend(); ++it) {
            freq_diffs[it->first] -= worditer->second * it->second;
            backpointers_to_remove[it->first][worditer->first] = worditer->second * it->second;
        }
        for (auto it = hypo_stats.cbegin(); it != hypo_stats.cend(); ++it) {
            freq_diffs[it->first] += worditer->second * it->second;
            backpointers_to_add[it->first][worditer->first] = worditer->second * it->second;
        }
    }
}
