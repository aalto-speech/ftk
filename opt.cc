#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#include "factorencoder.hh"


int read_words(const char* fname,
               map<string, double> &words)
{
    ifstream vocabfile(fname);
    if (!vocabfile) return -1;

    string line, word;
    double count;
    while (getline(vocabfile, line)) {
        stringstream ss(line);
        ss >> count;
        ss >> word;
        words[word] = count;
    }
    vocabfile.close();

    return words.size();
}


void resegment_words(const map<string, double> &words,
                     const map<string, double> &vocab,
                     map<string, double> &new_freqs)
{
    new_freqs.clear();
    MorphSet morphset_vocab(vocab);
    for (auto worditer = words.cbegin(); worditer != words.cend(); ++worditer) {

        vector<string> best_path;
        viterbi(morphset_vocab, worditer->first, best_path, false);

        if (best_path.size() == 0) {
            cerr << "warning, no segmentation for word: " << worditer->first << endl;
            exit(0);
        }

        // Update statistics
        for (unsigned int i=0; i<best_path.size(); i++)
            new_freqs[best_path[i]] += worditer->second;
    }
}


void resegment_words_w_diff(const map<string, double> &words,
                            const map<string, double> &vocab,
                            map<string, double> &new_freqs,
                            map<string, map<string, double> > &diffs)
{
    new_freqs.clear();
    MorphSet morphset_vocab(vocab);
    MorphSet hypo_vocab(vocab);
    for (auto worditer = words.cbegin(); worditer != words.cend(); ++worditer) {

        vector<string> best_path;
        viterbi(morphset_vocab, worditer->first, best_path, false);

        if (best_path.size() == 0) {
            cerr << "warning, no segmentation for word: " << worditer->first << endl;
            exit(0);
        }

        // Update statistics
        map<string, double> best_path_types;
        for (unsigned int i=0; i<best_path.size(); i++) {
            new_freqs[best_path[i]] += worditer->second;
            best_path_types[best_path[i]] = 0.0;
        }

        // Hypothesize what the segmentation would be if some subword didn't exist
        for (auto hypoiter = best_path_types.begin(); hypoiter != best_path_types.end(); ++hypoiter) {
            if (diffs.find(hypoiter->first) != diffs.end()) {
                double stored_value = hypo_vocab.remove(hypoiter->first);
                vector<string> hypo_path;

                viterbi(hypo_vocab, worditer->first, hypo_path, false);

                if (hypo_path.size() == 0) {
                    cerr << "warning, no hypo segmentation for word: " << worditer->first << endl;
                    exit(0);
                }

                for (unsigned int ib=0; ib<best_path.size(); ib++)
                    diffs[hypoiter->first][best_path[ib]] -= worditer->second;
                for (unsigned int ih=0; ih<hypo_path.size(); ih++)
                    diffs[hypoiter->first][hypo_path[ih]] += worditer->second;

//                diffs[hypoiter->first].erase(hypoiter->first);
                hypo_vocab.add(hypoiter->first, stored_value);
            }
        }
    }
}


double get_sum(const map<string, double> &freqs)
{
    double total = 0.0;
    for (auto iter = freqs.cbegin(); iter != freqs.cend(); ++iter) {
        total += iter->second;
    }
    return total;
}


double get_sum(const map<string, double> &freqs,
               const map<string, double> &freq_diffs)
{
    double total = 0.0;
    for (auto iter = freqs.cbegin(); iter != freqs.cend(); ++iter)
        total += iter->second;
    for (auto iter = freq_diffs.cbegin(); iter != freq_diffs.cend(); ++iter)
        total += iter->second;
    return total;
}


double get_cost(const map<string, double> &freqs,
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


double get_cost(const map<string, double> &freqs,
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


void apply_freq_diffs(map<string, double> &freqs,
                      const map<string, double> &freq_diffs)
{
    for(auto iter = freq_diffs.cbegin(); iter != freq_diffs.cend(); ++iter)
        freqs[iter->first] += iter->second;

    // http://stackoverflow.com/questions/8234779/how-to-remove-from-a-map-while-iterating-it
    auto iter = freqs.begin();
    while (iter != freqs.end()) {
        if (iter->second <= 0.0) freqs.erase(iter++);
        else ++iter;
    }
}


void apply_backpointer_changes(map<string, map<string, double> > &backpointers,
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


void freqs_to_logprobs(map<string, double> &vocab,
                       double densum)
{
    densum = log2(densum);
    for (auto iter = vocab.begin(); iter != vocab.end(); ++iter)
        iter->second = (log2(iter->second)-densum);
}


int cutoff(map<string, double> &vocab,
           double limit)
{
    // http://stackoverflow.com/questions/8234779/how-to-remove-from-a-map-while-iterating-it
    int nremovals = 0;
    auto iter = vocab.begin();
    while (iter != vocab.end()) {
        if (iter->second <= limit && iter->first.length() > 1) {
            vocab.erase(iter++);
            nremovals += 1;
        }
        else ++iter;
    }
    return nremovals;
}


// Select n_candidates number of subwords in the vocabulary as removal candidates
// running from the least common subword
void init_removal_candidates(int n_candidates,
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
void rank_removal_candidates(const map<string, double> &words,
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


void get_backpointers(const map<string, double> &words,
                      const map<string, double> &vocab,
                      map<string, map<string, double> > &backpointers)
{
    backpointers.clear();
    MorphSet morphset_vocab(vocab);

    for (auto worditer = words.cbegin(); worditer != words.cend(); ++worditer) {

        vector<string> best_path;
        viterbi(morphset_vocab, worditer->first, best_path, false);

        if (best_path.size() == 0) {
            cerr << "warning, no segmentation for word: " << worditer->first << endl;
            exit(0);
        }

        // Store backpointers
        for (unsigned int i=0; i<best_path.size(); i++)
            backpointers[best_path[i]][worditer->first] = worditer->second;
    }
}


// Gives out updated freqs for removal of this subword
void hypo_removal(MorphSet &vocab,
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

        vector<string> best_path;
        viterbi(vocab, worditer->first, best_path, false);

        vector<string> hypo_path;
        double stored_value = vocab.remove(subword);
        viterbi(vocab, worditer->first, hypo_path, false);
        vocab.add(subword, stored_value);

        if (best_path.size() == 0) {
            cerr << "warning, no segmentation for word: " << worditer->first << endl;
            exit(0);
        }

        if (hypo_path.size() == 0) {
            cerr << "warning, no hypo segmentation for word: " << worditer->first << endl;
            exit(0);
        }

        // Collect frequency differences
        // Collect backpointer changes
        for (unsigned int i=0; i<best_path.size(); i++) {
            freq_diffs[best_path[i]] -= worditer->second;
            backpointers_to_remove[best_path[i]][worditer->first] += worditer->second;
        }
        for (unsigned int i=0; i<hypo_path.size(); i++) {
            freq_diffs[hypo_path[i]] += worditer->second;
            backpointers_to_add[hypo_path[i]][worditer->first] += worditer->second;
        }
    }
}


int main(int argc, char* argv[]) {

    if (argc < 10) {
        cerr << "usage: " << argv[0] << " <vocabulary> <words> <cutoff> <#candidates> <#removals> <threshold> <threshold_decrease> <minremovals> <minvocabsize>" << endl;
        exit(0);
    }

    int cutoff_value = atoi(argv[3]);
    unsigned int n_candidates_per_iter = atoi(argv[4]);
    int n_removals_per_iter = atoi(argv[5]);
    double threshold = atof(argv[6]);
    double threshold_decrease = atof(argv[7]);
    int min_removals_per_iter = atoi(argv[8]);
    unsigned int min_vocab_size = atoi(argv[9]);

    cerr << "parameters, initial vocabulary: " << argv[1] << endl;
    cerr << "parameters, wordlist: " << argv[2] << endl;
    cerr << "parameters, cutoff: " << cutoff_value << endl;
    cerr << "parameters, candidates per iteration: " << n_candidates_per_iter << endl;
    cerr << "parameters, removals per iteration: " << n_removals_per_iter << endl;
    cerr << "parameters, threshold: " << threshold << endl;
    cerr << "parameters, threshold decrease per iteration: " << threshold_decrease << endl;
    cerr << "parameters, min removals per iteration: " << min_removals_per_iter << endl;
    cerr << "parameters, target vocab size: " << min_vocab_size << endl;

    int maxlen;
    map<string, double> vocab;
    map<string, double> freqs;
    map<string, double> words;

    cerr << "Reading vocabulary " << argv[1] << endl;
    int retval = read_vocab(argv[1], vocab, maxlen);
    if (retval < 0) {
        cerr << "something went wrong reading vocabulary" << endl;
        exit(0);
    }
    cerr << "\t" << "size: " << vocab.size() << endl;
    cerr << "\t" << "maximum string length: " << maxlen << endl;

    cerr << "Reading word list" << endl;

    read_words(argv[2], words);


    cerr << "\t\t\t" << "vocabulary size: " << vocab.size() << endl;

    cerr << "Initial cutoff" << endl;
    resegment_words(words, vocab, freqs);
    double densum = get_sum(freqs);
    double cost = get_cost(freqs, densum);
    cerr << "cost: " << cost << endl;


    cutoff(freqs, cutoff_value);
    cerr << "\tcutoff: " << cutoff_value << "\t" << "vocabulary size: " << freqs.size() << endl;
    vocab = freqs;
    densum = get_sum(vocab);
    freqs_to_logprobs(vocab, densum);


    cerr << "Removing subwords one by one" << endl;
    int itern = 1;
    while (true) {

        cerr << "iteration " << itern << endl;

        cerr << "collecting candidate subwords for removal" << endl;
        map<string, map<string, double> > diffs;
        if (vocab.size()-n_candidates_per_iter < min_vocab_size) n_candidates_per_iter = vocab.size()-min_vocab_size;
        init_removal_candidates(n_candidates_per_iter, words, vocab, diffs);

        cerr << "ranking candidate subwords" << endl;
        vector<pair<string, double> > removal_scores;
        rank_removal_candidates(words, vocab, diffs, freqs, removal_scores);

        // Perform removals one by one if likelihood change above threshold
        double curr_densum = get_sum(freqs);
        double curr_cost = get_cost(freqs, curr_densum);
        map<string, map<string, double> > backpointers;
        get_backpointers(words, vocab, backpointers);

        cerr << "starting cost before removing subwords one by one: " << curr_cost << endl;

        int n_removals = 0;
        for (unsigned int i=0; i<removal_scores.size(); i++) {

            if (removal_scores[i].first.length() == 1) continue;

            cout << removal_scores[i].first << "\t" << "expected ll diff: " << removal_scores[i].second << endl;

            map<string, double> freq_diffs;
            map<string, map<string, double> > backpointers_to_remove;
            map<string, map<string, double> > backpointers_to_add;
            MorphSet morphset_vocab(vocab);
            hypo_removal(morphset_vocab, removal_scores[i].first, backpointers,
                         backpointers_to_remove, backpointers_to_add, freq_diffs);

            double hypo_densum = get_sum(freqs, freq_diffs);
            double hypo_cost = get_cost(freqs, freq_diffs, hypo_densum);

            cout << removal_scores[i].first << "\t" << "change in likelihood: " << hypo_cost-curr_cost;
            if (hypo_cost-curr_cost < threshold) {
                cout << " was below threshold " << threshold << endl;
                continue;
            }
            cout << " removed, was above threshold " << threshold << endl;

            apply_freq_diffs(freqs, freq_diffs);
            freqs.erase(removal_scores[i].first);
            apply_backpointer_changes(backpointers, backpointers_to_remove, backpointers_to_add);
            backpointers.erase(removal_scores[i].first);

            curr_densum = hypo_densum;
            curr_cost = hypo_cost;
            vocab = freqs;
            freqs_to_logprobs(vocab, hypo_densum);

            n_removals++;

            if (vocab.size() % 5000 == 0) {
                ostringstream vocabfname;
                vocabfname << "iter" << itern << "_" << vocab.size() << ".vocab";
                write_vocab(vocabfname.str().c_str(), vocab);

                ostringstream freqsfname;
                freqsfname << "iter" << itern << "_" << vocab.size() << ".freqs";
                write_vocab(freqsfname.str().c_str(), freqs);
            }

            if (n_removals >= n_removals_per_iter) break;
            if (vocab.size() <= min_vocab_size) break;
        }

        int n_cutoff = cutoff(freqs, cutoff_value);
        double co_densum = get_sum(freqs);
        vocab = freqs;
        freqs_to_logprobs(vocab, co_densum);
        resegment_words(words, vocab, freqs);
        curr_densum = get_sum(freqs);
        curr_cost = get_cost(freqs, densum);

        cerr << "subwords removed in this iteration: " << n_removals << endl;
        cerr << "subwords removed with cutoff this iteration: " << n_cutoff << endl;
        cerr << "current vocabulary size: " << vocab.size() << endl;
        cerr << "likelihood after the removals: " << curr_cost << endl;

        ostringstream vocabfname;
        vocabfname << "iter" << itern << ".vocab";
        write_vocab(vocabfname.str().c_str(), vocab);

        ostringstream freqsfname;
        freqsfname << "iter" << itern << ".freqs";
        write_vocab(freqsfname.str().c_str(), freqs);

        itern++;
        threshold -= threshold_decrease;

        if (n_removals < min_removals_per_iter) {
            cerr << "stopping by min_removals_per_iter." << endl;
            break;
        }

        if (vocab.size() <= min_vocab_size) {
            cerr << "stopping by min_vocab_size." << endl;
            break;
        }
    }

    exit(1);
}

