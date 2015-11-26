#include <algorithm>
#include <iomanip>
#include <cmath>

#include "Unigrams.hh"
#include "io.hh"
#include "str.hh"

using namespace std;


bool descending_sort(pair<string, flt_type> i,pair<string, flt_type> j)
{
    return (i.second > j.second);
}

bool ascending_sort(pair<string, flt_type> i,pair<string, flt_type> j)
{
    return (i.second < j.second);
}


int
Unigrams::read_vocab(string fname,
                     map<string, flt_type> &vocab,
                     int &maxlen,
                     bool utf8)
{
    SimpleFileInput vocabfile(fname);

    string line;
    flt_type count;
    maxlen = -1;
    while (vocabfile.getline(line)) {
        stringstream ss(line);
        string word;
        ss >> count;
        ss >> word;
        vocab[word] = count;

        maxlen = max(maxlen, (int)get_factor_length(word, utf8));
    }

    return vocab.size();
}


int
Unigrams::read_corpus(string fname,
                      map<string, flt_type> &vocab,
                      int &maxlen,
                      bool utf8)
{
    ifstream vocabfile(fname);
    if (!vocabfile) return -1;

    string line;
    maxlen = -1;
    while (getline(vocabfile, line)) {
        line = str::cleaned(line);
        vector<string> words = str::split(line, " \t", true);
        for (auto wit = words.begin(); wit != words.end(); ++wit)
            vocab[*wit] += 1.0;
    }
    vocabfile.close();

    for (auto wit = vocab.begin(); wit != vocab.end(); ++wit)
        maxlen = max(maxlen, (int)get_factor_length(wit->first, utf8));

    return vocab.size();
}


int
Unigrams::write_vocab(string fname,
                      const map<string, flt_type> &vocab,
                      bool count_style,
                      int num_decimals)
{
    SimpleFileOutput vocabfile(fname);

    vector<pair<string, flt_type> > sorted_vocab;
    sort_vocab(vocab, sorted_vocab);
    for (unsigned int i=0; i<sorted_vocab.size(); i++)
        if (count_style)
            vocabfile << sorted_vocab[i].first << "\t" << sorted_vocab[i].second << "\n";
        else
            vocabfile << sorted_vocab[i].second << " " << sorted_vocab[i].first << "\n";
    vocabfile.close();

    return vocab.size();
}


int
Unigrams::read_sents(string fname,
                     vector<string> &sents)
{
    sents.clear();
    SimpleFileInput sfi(fname);

    int lc = 0;
    string line;
    while (sfi.getline(line)) {
        trim(line, '\n');
        sents.push_back(line);
        lc++;
    }

    return lc;
}


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
Unigrams::iterate(const map<string, flt_type> &words,
                  map<string, flt_type> &vocab,
                  unsigned int iterations)
{
    map<string, flt_type> freqs;
    flt_type ll = 0.0;

    for (unsigned int i=0; i<iterations; i++) {
        ll = resegment_words(words, vocab, freqs);
        vocab = freqs;
        freqs_to_logprobs(vocab);
    }

    return ll;
}


flt_type
Unigrams::iterate(const vector<string> &sents,
                  map<string, flt_type> &vocab,
                  unsigned int iterations)
{
    map<string, flt_type> freqs;
    flt_type ll = 0.0;

    for (unsigned int i=0; i<iterations; i++) {
        ll = resegment_sents(sents, vocab, freqs);
        vocab = freqs;
        freqs_to_logprobs(vocab);
    }

    return ll;
}


flt_type
Unigrams::resegment_words(const map<string, flt_type> &words,
                          const map<string, flt_type> &vocab,
                          map<string, flt_type> &new_freqs,
                          set<string> special_words)
{
    StringSet stringset_vocab(vocab);
    return resegment_words(words, stringset_vocab, new_freqs, special_words);
}


flt_type
Unigrams::resegment_words(const map<string, flt_type> &words,
                          const StringSet &vocab,
                          map<string, flt_type> &new_freqs,
                          set<string> special_words)
{
    new_freqs.clear();
    flt_type ll = 0.0;

    for (auto worditer = words.cbegin(); worditer != words.cend(); ++worditer) {

        // Special symbols like <s> </s> etc. which should not be segmented
        if (special_words.size() > 0 && special_words.find(worditer->first) != special_words.end()) {
            new_freqs[worditer->first] += worditer->second;
            continue;
        }

        map<string, flt_type> stats;
        flt_type curr_ll = worditer->second * segf(vocab, worditer->first, stats, this->utf8);

        if (stats.size() == 0) {
            cerr << "warning, no segmentation for word: " << worditer->first << endl;
            continue;
        }

        ll += curr_ll;

        // Update statistics
        for (auto it = stats.begin(); it != stats.end(); ++it)
            new_freqs[it->first] += worditer->second * it->second;
    }

    return ll;
}


flt_type
Unigrams::resegment_sents(const vector<string> &sents,
                          const map<string, flt_type> &vocab,
                          map<string, flt_type> &new_freqs)
{
    StringSet stringset_vocab(vocab);
    return resegment_sents(sents, stringset_vocab, new_freqs);
}


flt_type
Unigrams::resegment_sents(const vector<string> &sents,
                          const StringSet &vocab,
                          map<string, flt_type> &new_freqs)
{
    new_freqs.clear();
    flt_type ll = 0.0;

    for (auto sent = sents.begin(); sent != sents.end(); ++sent) {

        map<string, flt_type> stats;
        flt_type curr_ll = segf(vocab, *sent, stats, this->utf8);

        if (stats.size() == 0) {
            cerr << "warning, no segmentation for sentence: " << *sent << endl;
            continue;
        }

        ll += curr_ll;

        // Update statistics
        for (auto it = stats.begin(); it != stats.end(); ++it)
            new_freqs[it->first] += it->second;
    }

    return ll;
}


flt_type
Unigrams::get_sum(const map<string, flt_type> &freqs)
{
    flt_type total = 0.0;
    for (auto iter = freqs.cbegin(); iter != freqs.cend(); ++iter) {
        if (std::isinf(iter->second) || std::isnan(iter->second)) continue;
        total += iter->second;
    }
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


void
Unigrams::freqs_to_logprobs(map<string, flt_type> &vocab,
                            flt_type min_lp)
{
    flt_type densum = Unigrams::get_sum(vocab);
    densum = log(densum);
    for (auto iter = vocab.begin(); iter != vocab.end(); ++iter) {
        iter->second = (log(iter->second)-densum);
        if (iter->second < min_lp || std::isinf(iter->second) || std::isnan(iter->second))
            iter->second = min_lp;
    }
}


int
Unigrams::cutoff(map<string, flt_type> &vocab,
                 flt_type limit,
                 const set<string> &stoplist,
                 unsigned int min_length)
{
    int nremovals = 0;
    auto iter = vocab.begin();
    while (iter != vocab.end()) {
        if (iter->second <= limit
            && get_factor_length(iter->first, this->utf8) >= min_length
            && stoplist.find(iter->first) == stoplist.end()) {
                vocab.erase(iter++);
                nremovals++;
        }
        else ++iter;
    }
    return nremovals;
}


int
Unigrams::cutoff(map<string, flt_type> &vocab,
                 flt_type limit,
                 unsigned int min_length)
{
    set<string> stoplist;
    return cutoff(vocab, limit, min_length);
}


// Select n_candidates number of subwords in the vocabulary as removal candidates
// running from the least common subword
int
Unigrams::init_candidates(const map<string, flt_type> &vocab,
                          set<string> &candidates,
                          unsigned int n_candidates,
                          const set<string> &stoplist,
                          unsigned int min_length)
{

    vector<pair<string, flt_type> > sorted_vocab;
    sort_vocab(vocab, sorted_vocab, false);

    int selected_candidates = 0;
    for (auto it = sorted_vocab.cbegin(); it != sorted_vocab.cend(); ++it) {
        const string &subword = it->first;
        if (get_factor_length(subword, this->utf8) < min_length) continue;
        if (candidates.find(subword) != candidates.end()) continue;
        if (stoplist.find(subword) != stoplist.end()) continue;
        candidates.insert(subword);
        selected_candidates++;
        if (candidates.size() >= n_candidates) break;
    }

    return selected_candidates;
}


int
Unigrams::init_candidates(const map<string, flt_type> &vocab,
                          set<string> &candidates,
                          unsigned int n_candidates,
                          unsigned int min_length)
{
    set<string> stoplist;
    return init_candidates(vocab, candidates, n_candidates, stoplist, min_length);
}


// Select n_candidates number of subwords in the vocabulary as removal candidates
// by random
int
Unigrams::init_candidates_by_random(const map<string, flt_type> &vocab,
                                    set<string> &candidates,
                                    unsigned int n_candidates,
                                    const set<string> &stoplist,
                                    unsigned int min_length)
{
    vector<string> shuffled_vocab;
    for (auto it = vocab.cbegin(); it != vocab.cend(); ++it)
        shuffled_vocab.push_back(it->first);
    random_shuffle(shuffled_vocab.begin(), shuffled_vocab.end());

    int selected_candidates = 0;
    for (auto it = shuffled_vocab.begin(); it != shuffled_vocab.end(); ++it) {
        const string &subword = *it;
        if (get_factor_length(subword, this->utf8) < min_length) continue;
        if (candidates.find(subword) != candidates.end()) continue;
        if (stoplist.find(subword) != stoplist.end()) continue;
        candidates.insert(subword);
        selected_candidates++;
        if (candidates.size() >= n_candidates) break;
    }

    return selected_candidates;
}


int
Unigrams::init_candidates_by_random(const map<string, flt_type> &vocab,
                                    set<string> &candidates,
                                    unsigned int n_candidates,
                                    unsigned int min_length)
{
    set<string> stoplist;
    return init_candidates_by_random(vocab, candidates, n_candidates, stoplist, min_length);
}


// Select n_candidates number of subwords in the vocabulary as removal candidates
// by the occupancy count in unweighted word segmentations
int
Unigrams::init_candidates_by_usage(const map<string, flt_type> &words,
                                   const map<string, flt_type> &vocab,
                                   set<string> &candidates,
                                   unsigned int n_candidates,
                                   const set<string> &stoplist,
                                   unsigned int min_length,
                                   flt_type max_usage)
{
    map<string, flt_type> words_type = words;
    for (auto it = words_type.begin(); it != words_type.end(); ++it)
        it->second = 1.0;

    map<string, flt_type> type_occ;
    resegment_words(words_type, vocab, type_occ);

    vector<pair<string, flt_type> > sorted_occ;
    sort_vocab(type_occ, sorted_occ, false);

    int selected_candidates = 0;
    for (auto it = sorted_occ.cbegin(); it != sorted_occ.cend(); ++it) {
        const string &subword = it->first;
        if (it->second > max_usage) break;
        if (get_factor_length(subword, this->utf8) < min_length) continue;
        if (candidates.find(subword) != candidates.end()) continue;
        if (stoplist.find(subword) != stoplist.end()) continue;
        candidates.insert(subword);
        selected_candidates++;
        if (candidates.size() >= n_candidates) break;
    }

    return selected_candidates;
}


int
Unigrams::init_candidates_by_usage(const map<string, flt_type> &words,
                                   const map<string, flt_type> &vocab,
                                   set<string> &candidates,
                                   unsigned int n_candidates,
                                   unsigned int min_length,
                                   flt_type max_usage)
{
    set<string> stoplist;
    return init_candidates_by_usage(words, vocab, candidates, n_candidates,
                                    stoplist, min_length, max_usage);
}


// Perform each of the removals (independent of others in the list) to get
// initial order for the removals
flt_type
Unigrams::rank_candidates(const map<string, flt_type> &words,
                          const map<string, flt_type> &vocab,
                          const set<string> &candidates,
                          map<string, flt_type> &new_freqs,
                          vector<pair<string, flt_type> > &removal_scores)
{
    new_freqs.clear();
    removal_scores.clear();

    StringSet ss_vocab(vocab);
    map<string, flt_type> ll_diffs;
    map<string, flt_type> token_diffs;

    flt_type curr_ll = 0.0;
    flt_type token_count = 0.0;

    for (auto worditer = words.cbegin(); worditer != words.cend(); ++worditer) {

        map<string, flt_type> stats;
        flt_type orig_score = segf(ss_vocab, worditer->first, stats, this->utf8);

        if (stats.size() == 0) {
            cerr << "warning, no segmentation for word: " << worditer->first << endl;
            continue;
        }

        curr_ll += worditer->second * orig_score;
        token_count += worditer->second * stats.size();

        // Update statistics
        for (auto it = stats.cbegin(); it != stats.cend(); ++it)
            new_freqs[it->first] += worditer->second * it->second;

        // Hypothesize what the segmentation would be if some string didn't exist
        for (auto hypoiter = stats.cbegin(); hypoiter != stats.cend(); ++hypoiter) {

            // If wanting to hypothesize removal of this string
            if (candidates.find(hypoiter->first) != candidates.end()) {

                flt_type stored_value = ss_vocab.remove(hypoiter->first);
                map<string, flt_type> hypo_stats;

                flt_type hypo_score = segf(ss_vocab, worditer->first, hypo_stats, this->utf8);

                if (hypo_stats.size() == 0) {
                    cerr << "warning, no hypo segmentation for word: " << worditer->first << endl;
                    exit(0);
                }

                ll_diffs[hypoiter->first] += worditer->second * (hypo_score-orig_score);
                token_diffs[hypoiter->first] += worditer->second * ((flt_type)(hypo_stats.size())-(flt_type)(stats.size()));

                ss_vocab.add(hypoiter->first, stored_value);
            }
        }
    }

    for (auto iter = ll_diffs.begin(); iter != ll_diffs.end(); ++iter) {
        flt_type renormalizer = sub_log_domain_probs(0, vocab.at(iter->first));
        flt_type hypo_token_count = (token_count + token_diffs[iter->first]);
        flt_type normalizer_ll_diff = hypo_token_count * -renormalizer;
        pair<string, flt_type> removal_score = make_pair(iter->first, iter->second + normalizer_ll_diff);
        removal_scores.push_back(removal_score);
    }

    sort(removal_scores.begin(), removal_scores.end(), descending_sort);

    return curr_ll;
}


// Perform each of the removals (independent of others in the list) to get
// initial order for the removals
flt_type
Unigrams::rank_candidates(std::vector<std::string> &sents,
                          const map<string, flt_type> &vocab,
                          const set<string> &candidates,
                          map<string, flt_type> &new_freqs,
                          vector<pair<string, flt_type> > &removal_scores)
{
    new_freqs.clear();
    removal_scores.clear();

    StringSet ss_vocab(vocab);
    map<string, flt_type> ll_diffs;
    map<string, flt_type> token_diffs;

    flt_type curr_ll = 0.0;
    flt_type token_count = 0.0;

    for (auto sentiter = sents.cbegin(); sentiter != sents.cend(); ++sentiter) {

        map<string, flt_type> stats;
        flt_type orig_score = segf(ss_vocab, *sentiter, stats, this->utf8);

        if (stats.size() == 0) {
            cerr << "warning, no segmentation for sentence: " << *sentiter << endl;
            continue;
        }

        curr_ll += orig_score;
        token_count += stats.size();

        // Update statistics
        for (auto it = stats.cbegin(); it != stats.cend(); ++it)
            new_freqs[it->first] += it->second;

        // Hypothesize what the segmentation would be if some string didn't exist
        for (auto hypoiter = stats.cbegin(); hypoiter != stats.cend(); ++hypoiter) {

            // If wanting to hypothesize removal of this string
            if (candidates.find(hypoiter->first) != candidates.end()) {

                flt_type stored_value = ss_vocab.remove(hypoiter->first);
                map<string, flt_type> hypo_stats;

                flt_type hypo_score = segf(ss_vocab, *sentiter, hypo_stats, this->utf8);

                if (hypo_stats.size() == 0) {
                    cerr << "warning, no hypo segmentation for sentence: " << *sentiter << endl;
                    exit(0);
                }

                ll_diffs[hypoiter->first] += (hypo_score-orig_score);
                token_diffs[hypoiter->first] += (flt_type)(hypo_stats.size())-(flt_type)(stats.size());

                ss_vocab.add(hypoiter->first, stored_value);
            }
        }
    }

    for (auto iter = ll_diffs.begin(); iter != ll_diffs.end(); ++iter) {
        flt_type renormalizer = sub_log_domain_probs(0, vocab.at(iter->first));
        flt_type hypo_token_count = (token_count + token_diffs[iter->first]);
        flt_type normalizer_ll_diff = hypo_token_count * -renormalizer;
        pair<string, flt_type> removal_score = make_pair(iter->first, iter->second + normalizer_ll_diff);
        removal_scores.push_back(removal_score);
    }

    sort(removal_scores.begin(), removal_scores.end(), descending_sort);

    return curr_ll;
}
