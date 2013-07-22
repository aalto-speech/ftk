#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <iostream>
#include <sstream>

#include "FactorEncoder.hh"
#include "Unigrams.hh"

using namespace std;


int
Unigrams::read_vocab(string fname,
                     map<string, flt_type> &vocab,
                     int &maxlen)
{
    ifstream vocabfile(fname);
    if (!vocabfile) return -1;

    string line;
    flt_type count;
    maxlen = -1;
    while (getline(vocabfile, line)) {
        stringstream ss(line);
        string word;
        ss >> count;
        ss >> word;
        vocab[word] = count;
        maxlen = max(maxlen, int(word.length()));
    }
    vocabfile.close();

    return vocab.size();
}


int
Unigrams::write_vocab(string fname,
                      const map<string, flt_type> &vocab)
{
    ofstream vocabfile(fname);
    if (!vocabfile) return -1;

    vector<pair<string, flt_type> > sorted_vocab;
    sort_vocab(vocab, sorted_vocab);
    for (unsigned int i=0; i<sorted_vocab.size(); i++)
        vocabfile << sorted_vocab[i].second << " " << sorted_vocab[i].first << endl;
    vocabfile.close();

    return vocab.size();
}


bool descending_sort(pair<string, flt_type> i,pair<string, flt_type> j) { return (i.second > j.second); }
bool ascending_sort(pair<string, flt_type> i,pair<string, flt_type> j) { return (i.second < j.second); }

void
Unigrams::sort_vocab(const map<string, flt_type> &vocab,
                     vector<pair<string, flt_type> > &sorted_vocab,
                     bool descending)
{
    sorted_vocab.clear();
    for (auto it = vocab.cbegin(); it != vocab.cend(); it++) {
        pair<string, flt_type> curr_pair(it->first, it->second);
        sorted_vocab.push_back(curr_pair);
    }
    if (descending)
        sort(sorted_vocab.begin(), sorted_vocab.end(), descending_sort);
    else
        sort(sorted_vocab.begin(), sorted_vocab.end(), ascending_sort);
}


flt_type
Unigrams::resegment_words(const map<string, flt_type> &words,
                          const map<string, flt_type> &vocab,
                          map<string, flt_type> &new_freqs)
{
    new_freqs.clear();
    StringSet<flt_type> stringset_vocab(vocab);
    flt_type ll = 0.0;

    for (auto worditer = words.cbegin(); worditer != words.cend(); ++worditer) {

        map<string, flt_type> stats;
        ll += worditer->second * segf(stringset_vocab, worditer->first, stats);

        if (stats.size() == 0) {
            cerr << "warning, no segmentation for word: " << worditer->first << endl;
            exit(0);
        }

        // Update statistics
        for (auto it = stats.begin(); it != stats.end(); ++it)
            new_freqs[it->first] += worditer->second * it->second;
    }

    return ll;
}


void
Unigrams::resegment_words_w_diff(const map<string, flt_type> &words,
                                 const map<string, flt_type> &vocab,
                                 map<string, flt_type> &new_freqs,
                                 map<string, map<string, flt_type> > &diffs)
{
    new_freqs.clear();
    StringSet<flt_type> morphset_vocab(vocab);
    StringSet<flt_type> hypo_vocab(vocab);

    for (auto worditer = words.cbegin(); worditer != words.cend(); ++worditer) {

        map<string, flt_type> stats;
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

                flt_type stored_value = hypo_vocab.remove(hypoiter->first);
                map<string, flt_type> hypo_stats;

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


flt_type
Unigrams::get_sum(const map<string, flt_type> &freqs)
{
    flt_type total = 0.0;
    for (auto iter = freqs.cbegin(); iter != freqs.cend(); ++iter)
        total += iter->second;
    return total;
}


flt_type
Unigrams::get_sum(const map<string, flt_type> &freqs,
                  const map<string, flt_type> &freq_diffs)
{
    flt_type total = 0.0;
    for (auto iter = freqs.cbegin(); iter != freqs.cend(); ++iter)
        total += iter->second;
    for (auto iter = freq_diffs.cbegin(); iter != freq_diffs.cend(); ++iter)
        total += iter->second;
    return total;
}


flt_type
Unigrams::get_cost(const map<string, flt_type> &freqs,
                   flt_type densum)
{
    flt_type total = 0.0;
    flt_type tmp = 0.0;
    densum = log(densum);
    for (auto iter = freqs.cbegin(); iter != freqs.cend(); ++iter) {
        tmp = iter->second * (log(iter->second)-densum);
        if (!std::isnan(tmp)) total += tmp;
    }
    return total;
}


flt_type
Unigrams::get_cost(const map<string, flt_type> &freqs,
                   const map<string, flt_type> &freq_diffs,
                   flt_type densum)
{
    flt_type total = 0.0;
    flt_type tmp = 0.0;
    densum = log(densum);
    for (auto iter = freqs.cbegin(); iter != freqs.cend(); ++iter) {
        tmp = iter->second;
        if (freq_diffs.find(iter->first) != freq_diffs.end())
            tmp += freq_diffs.at(iter->first);
        tmp = tmp * (log(tmp)-densum);
        if (!std::isnan(tmp)) total += tmp;
    }
    return total;
}


void
Unigrams::apply_freq_diffs(map<string, flt_type> &freqs,
                           const map<string, flt_type> &freq_diffs)
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
Unigrams::apply_backpointer_changes(map<string, map<string, flt_type> > &backpointers,
                                    const map<string, map<string, flt_type> > &bps_to_remove,
                                    const map<string, map<string, flt_type> > &bps_to_add)
{
    for (auto switer = bps_to_remove.cbegin(); switer != bps_to_remove.cend(); ++switer)
        for (auto worditer = switer->second.cbegin(); worditer != switer->second.cend(); ++worditer)
            backpointers[switer->first].erase(worditer->first);
    for (auto switer = bps_to_add.cbegin(); switer != bps_to_add.cend(); ++switer)
        for (auto worditer = switer->second.cbegin(); worditer != switer->second.cend(); ++worditer)
            backpointers[switer->first][worditer->first] = worditer->second;
}


void
Unigrams::freqs_to_logprobs(map<string, flt_type> &vocab,
                            flt_type densum)
{
    densum = log(densum);
    for (auto iter = vocab.begin(); iter != vocab.end(); ++iter)
        iter->second = (log(iter->second)-densum);
}


int
Unigrams::cutoff(map<string, flt_type> &vocab,
                 flt_type limit)
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
int
Unigrams::init_removal_candidates(int n_candidates,
                                  const map<string, flt_type> &words,
                                  const map<string, flt_type> &vocab,
                                  map<string, map<string, flt_type> > &diffs)
{
    map<string, flt_type> new_morph_freqs;
    resegment_words(words, vocab, new_morph_freqs);

    vector<pair<string, flt_type> > sorted_vocab;
    sort_vocab(new_morph_freqs, sorted_vocab, false);

    int selected_candidates = 0;
    for (auto it = sorted_vocab.cbegin(); it != sorted_vocab.cend(); ++it) {
        const string &subword = it->first;
        if (subword.length() < 2) continue;
        map<string, flt_type> emptymap;
        diffs[subword] = emptymap;
        selected_candidates++;
        if (selected_candidates >= n_candidates) break;
    }

    return selected_candidates;
}


// Select n_candidates number of subwords in the vocabulary as removal candidates
// by random
int
Unigrams::init_removal_candidates_by_random(int n_candidates,
                                            const map<string, flt_type> &words,
                                            const map<string, flt_type> &vocab,
                                            map<string, map<string, flt_type> > &diffs)
{
    vector<string> shuffled_vocab;
    for (auto it = vocab.cbegin(); it != vocab.cend(); ++it)
        shuffled_vocab.push_back(it->first);
    random_shuffle(shuffled_vocab.begin(), shuffled_vocab.end());

    int selected_candidates = 0;
    for (auto it = shuffled_vocab.begin(); it != shuffled_vocab.end(); ++it) {
        if (it->length() < 2) continue;
        map<string, flt_type> emptymap;
        diffs[*it] = emptymap;
        selected_candidates++;
        if (selected_candidates >= n_candidates) break;
    }

    return selected_candidates;
}


bool rank_desc_sort(pair<string, flt_type> i,pair<string, flt_type> j) { return (i.second > j.second); }

// Perform each of the removals (independent of others in the list) to get
// initial order for the removals
void
Unigrams::rank_removal_candidates(const map<string, flt_type> &words,
                                  const map<string, flt_type> &vocab,
                                  map<string, map<string, flt_type> > &diffs,
                                  map<string, flt_type> &new_morph_freqs,
                                  vector<pair<string, flt_type> > &removal_scores)
{
    new_morph_freqs.clear();
    removal_scores.clear();

    resegment_words_w_diff(words, vocab, new_morph_freqs, diffs);
    flt_type densum = get_sum(new_morph_freqs);
    flt_type cost = get_cost(new_morph_freqs, densum);

    for (auto iter = diffs.begin(); iter != diffs.end(); ++iter) {
        flt_type hypo_densum = get_sum(new_morph_freqs, iter->second);
        flt_type hypo_cost = get_cost(new_morph_freqs, iter->second, hypo_densum);
        pair<string, flt_type> removal_score = make_pair(iter->first, hypo_cost-cost);
        removal_scores.push_back(removal_score);
    }

    sort(removal_scores.begin(), removal_scores.end(), rank_desc_sort);
}


void
Unigrams::get_backpointers(const map<string, flt_type> &words,
                           const map<string, flt_type> &vocab,
                           map<string, map<string, flt_type> > &backpointers)
{
    backpointers.clear();
    StringSet<flt_type> stringset_vocab(vocab);

    for (auto worditer = words.cbegin(); worditer != words.cend(); ++worditer) {

        map<string, flt_type> stats;
        segf(stringset_vocab, worditer->first, stats);

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
Unigrams::hypo_removal(StringSet<flt_type> &vocab,
                       const string &subword,
                       const map<string, map<string, flt_type> > &backpointers,
                       map<string, map<string, flt_type> > &backpointers_to_remove,
                       map<string, map<string, flt_type> > &backpointers_to_add,
                       map<string, flt_type> &freq_diffs)
{
    backpointers_to_remove.clear();
    backpointers_to_add.clear();
    freq_diffs.clear();

    for (auto worditer = backpointers.at(subword).cbegin(); worditer != backpointers.at(subword).cend(); ++worditer) {

        map<string, flt_type> stats;
        segf(vocab, worditer->first, stats);

        map<string, flt_type> hypo_stats;
        flt_type stored_value = vocab.remove(subword);
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
