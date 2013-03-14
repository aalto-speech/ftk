#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "factorencoder.hh"


int read_words(const char* fname,
                 std::map<std::string, long> &words)
{
    std::ifstream vocabfile(fname);
    if (!vocabfile) return -1;

    std::string line, word;
    long count;
    while (getline(vocabfile, line)) {
        std::stringstream ss(line);
        ss >> count;
        ss >> word;
        words[word] = count;
    }
    vocabfile.close();

    return words.size();
}


void resegment_words(const std::map<std::string, long> &words,
                        const std::map<std::string, double> &vocab,
                        std::map<std::string, double> &new_freqs,
                        const int maxlen)
{
    for(std::map<std::string, long>::const_iterator worditer = words.begin(); worditer != words.end(); ++worditer) {

        std::vector<std::string> best_path;
        viterbi(vocab, maxlen, worditer->first, best_path, false);

        if (best_path.size() == 0) {
            std::cerr << "warning, no segmentation for word: " << worditer->first << std::endl;
            std::exit(0);
        }

        // Update statistics
        for (int i=0; i<best_path.size(); i++)
            new_freqs[best_path[i]] += double(worditer->second);
    }
}


void resegment_words_w_diff(const std::map<std::string, long> &words,
                                const std::map<std::string, double> &vocab,
                                std::map<std::string, double> &new_freqs,
                                std::map<std::string, std::map<std::string, double> > &diffs,
                                const int maxlen)
{
    std::map<std::string, double> hypo_vocab = vocab;
    new_freqs.clear();
    for (std::map<std::string, long>::const_iterator worditer = words.begin(); worditer != words.end(); ++worditer) {

        std::vector<std::string> best_path;
        viterbi(vocab, maxlen, worditer->first, best_path, false);

        if (best_path.size() == 0) {
            std::cerr << "warning, no segmentation for word: " << worditer->first << std::endl;
            std::exit(0);
        }

        // Update statistics
        std::map<std::string, double> best_path_types;
        for (int i=0; i<best_path.size(); i++) {
            new_freqs[best_path[i]] += double(worditer->second);
            best_path_types[best_path[i]] = 0.0;
        }

        // Hypothesize what the segmentation would be if some subword didn't exist
        for (std::map<std::string, double>::iterator hypoiter = best_path_types.begin(); hypoiter != best_path_types.end(); ++hypoiter) {
            if (diffs.find(hypoiter->first) != diffs.end()) {
                double stored_value = hypo_vocab.at(hypoiter->first);
                hypo_vocab.erase(hypoiter->first);
                std::vector<std::string> hypo_path;

                viterbi(hypo_vocab, maxlen, worditer->first, hypo_path, false);
                for (int ib=0; ib<best_path.size(); ib++)
                    diffs[hypoiter->first][best_path[ib]] -= double(worditer->second);
                for (int ih=0; ih<hypo_path.size(); ih++)
                    diffs[hypoiter->first][hypo_path[ih]] += double(worditer->second);

                diffs[hypoiter->first].erase(hypoiter->first);
                hypo_vocab[hypoiter->first] = stored_value;
            }
        }
    }
}


double get_sum(const std::map<std::string, double> &vocab)
{
    double total = 0.0;
    for(std::map<std::string, double>::const_iterator iter = vocab.begin(); iter != vocab.end(); ++iter) {
        total += iter->second;
    }
    return total;
}


double get_cost(const std::map<std::string, double> &vocab,
                  double densum)
{
    double total = 0.0;
    double tmp = 0.0;
    densum = log2(densum);
    for(std::map<std::string, double>::const_iterator iter = vocab.begin(); iter != vocab.end(); ++iter) {
        tmp = iter->second * (log2(iter->second)-densum);
        if (!isnan(tmp)) total += tmp;
    }
    return total;
}


void freqs_to_logprobs(std::map<std::string, double> &vocab,
                          double densum)
{
    densum = log2(densum);
    for(std::map<std::string, double>::iterator iter = vocab.begin(); iter != vocab.end(); ++iter)
        vocab[iter->first] = (log2(iter->second)-densum);
}


int cutoff(std::map<std::string, double> &vocab,
            double limit)
{
    int nremovals = 0;
    for(std::map<std::string, double>::iterator iter = vocab.begin(); iter != vocab.end(); ++iter) {
        if (vocab[iter->first] <= limit) {
            vocab.erase(iter->first);
            nremovals += 1;
        }
    }
    return nremovals;
}


// Select n_candidates number of subwords in the vocabulary as removal candidates
// running from the least common subword
void init_removal_candidates(int &n_candidates,
                                 const int maxlen,
                                 const std::map<std::string, long> &words,
                                 const std::map<std::string, double> &vocab,
                                 std::map<std::string, std::map<std::string, double> > &diffs)
{
    std::map<std::string, double> new_morph_freqs;
    resegment_words(words, vocab, new_morph_freqs, maxlen);

    std::vector<std::pair<std::string, double> > sorted_vocab;
    sort_vocab(new_morph_freqs, sorted_vocab, false);

    n_candidates = std::min(n_candidates, (int)vocab.size());
    for (int i=0; i<n_candidates; i++) {
        std::pair<std::string, double> &subword = sorted_vocab[i];
        std::map<std::string, double> emptymap;
        diffs[subword.first] = emptymap;
    }
}


bool rank_desc_sort(std::pair<std::string, double> i,std::pair<std::string, double> j) { return (i.second > j.second); }

// Perform each of the removals (independent of others in the list) to get
// initial order for the removals
void rank_removal_candidates(const std::map<std::string, long> &words,
                                 const std::map<std::string, double> &vocab,
                                 std::map<std::string, std::map<std::string, double> > &diffs,
                                 std::map<std::string, double> &new_morph_freqs,
                                 const int maxlen,
                                 std::vector<std::pair<std::string, double> > &removal_scores)
{
    resegment_words_w_diff(words, vocab, new_morph_freqs, diffs, maxlen);
    double densum = get_sum(new_morph_freqs);
    double cost = get_cost(new_morph_freqs, densum);

    for (std::map<std::string, std::map<std::string, double> >::iterator iter = diffs.begin(); iter != diffs.end(); ++iter) {
        double stored_value = new_morph_freqs.at(iter->first);
        for (std::map<std::string, double>::iterator diffiter = iter->second.begin(); diffiter != iter->second.end(); ++diffiter)
            new_morph_freqs[diffiter->first] += diffiter->second;
        new_morph_freqs.erase(iter->first);

        double hypo_densum = get_sum(new_morph_freqs);
        double hypo_cost = get_cost(new_morph_freqs, hypo_densum);
        std::pair<std::string, double> removal_score = std::make_pair(iter->first, hypo_cost-cost);
        removal_scores.push_back(removal_score);

        for (std::map<std::string, double>::iterator diffiter = iter->second.begin(); diffiter != iter->second.end(); ++diffiter)
            new_morph_freqs[diffiter->first] -= diffiter->second;
        new_morph_freqs[iter->first] = stored_value;
    }

    std::sort(removal_scores.begin(), removal_scores.end(), rank_desc_sort);
}


// Really performs the removal and gives out updated freqs
void remove_subword(const std::map<std::string, long> &words,
                       const std::map<std::string, double> &vocab,
                       const int maxlen,
                       const std::string &subword,
                       std::map<std::string, double> &new_freqs)
{
    std::map<std::string, double> hypo_vocab = vocab;
    hypo_vocab.erase(subword);

    for(std::map<std::string, long>::const_iterator worditer = words.begin(); worditer != words.end(); ++worditer) {

        // FIXME: Is this too slow?
        if (worditer->first.find(subword) != std::string::npos) {

            std::vector<std::string> best_path;
            viterbi(vocab, maxlen, worditer->first, best_path, false);
            std::vector<std::string> hypo_path;
            viterbi(hypo_vocab, maxlen, worditer->first, hypo_path, false);

            if (best_path.size() == 0) {
                std::cerr << "warning, no segmentation for word: " << worditer->first << std::endl;
                std::exit(0);
            }

            if (hypo_path.size() == 0) {
                std::cerr << "warning, no hypo segmentation for word: " << worditer->first << std::endl;
                std::exit(0);
            }

            // Update statistics
            for (int i=0; i<best_path.size(); i++)
                new_freqs[best_path[i]] -= double(worditer->second);
            for (int i=0; i<hypo_path.size(); i++)
                new_freqs[hypo_path[i]] += double(worditer->second);

        }
    }

    new_freqs.erase(subword);
    for(std::map<std::string, double>::iterator iter = new_freqs.begin(); iter != new_freqs.end(); ++iter)
        if (iter->second <= 0.0) new_freqs.erase(iter->first);
}


int main(int argc, char* argv[]) {

    if (argc < 9) {
        std::cerr << "usage: " << argv[0] << " <vocabulary> <words> <cutoff> <#candidates> <threshold> <threshold_decrease> <minremovals> <minvocabsize>" << std::endl;
        std::exit(0);
    }

    int cutoff_value = std::atoi(argv[3]);
    int n_candidates_per_iter = std::atoi(argv[4]);
    double threshold = std::atof(argv[5]);
    double threshold_decrease = std::atof(argv[6]);
    int min_removals_per_iter = std::atoi(argv[7]);
    int min_vocab_size = std::atoi(argv[8]);

    std::cerr << "parameters, initial vocabulary: " << argv[1] << std::endl;
    std::cerr << "parameters, wordlist: " << argv[2] << std::endl;
    std::cerr << "parameters, cutoff: " << cutoff_value << std::endl;
    std::cerr << "parameters, candidates per iteration: " << n_candidates_per_iter << std::endl;
    std::cerr << "parameters, threshold: " << threshold << std::endl;
    std::cerr << "parameters, threshold decrease per iteration: " << threshold_decrease << std::endl;
    std::cerr << "parameters, min removals per iteration: " << min_removals_per_iter << std::endl;
    std::cerr << "parameters, target vocab size: " << min_vocab_size << std::endl;

    int maxlen;
    std::map<std::string, double> vocab;
    std::map<std::string, double> new_morph_freqs;
    std::map<std::string, long> words;

    std::cerr << "Reading vocabulary " << argv[1] << std::endl;
    int retval = read_vocab(argv[1], vocab, maxlen);
    if (retval < 0) {
        std::cerr << "something went wrong reading vocabulary" << std::endl;
        std::exit(0);
    }
    std::cerr << "\t" << "size: " << vocab.size() << std::endl;
    std::cerr << "\t" << "maximum string length: " << maxlen << std::endl;

    std::cerr << "Reading word list" << std::endl;

    read_words(argv[2], words);


    std::cerr << "\t\t\t" << "vocabulary size: " << vocab.size() << std::endl;

    std::cerr << "Initial cutoff" << std::endl;
    resegment_words(words, vocab, new_morph_freqs, maxlen);
    double densum = get_sum(new_morph_freqs);
    double cost = get_cost(new_morph_freqs, densum);
    std::cerr << "cost: " << cost << std::endl;
    vocab.swap(new_morph_freqs);
    new_morph_freqs.clear();

    cutoff(vocab, cutoff_value);
    std::cerr << "\tcutoff: " << cutoff_value << "\t" << "vocabulary size: " << vocab.size() << std::endl;
    densum = get_sum(vocab);
    freqs_to_logprobs(vocab, densum);


    std::cerr << "Removing subwords one by one" << std::endl;
    int itern = 1;
    while (true) {

        std::cerr << "iteration " << itern << std::endl;

        std::cerr << "collecting candidate subwords for removal" << std::endl;
        std::map<std::string, std::map<std::string, double> > diffs;
        init_removal_candidates(n_candidates_per_iter, maxlen, words, vocab, diffs);

        std::cerr << "ranking candidate subwords" << std::endl;
        std::map<std::string, double> freqs;
        std::vector<std::pair<std::string, double> > removal_scores;
        rank_removal_candidates(words, vocab, diffs, freqs, maxlen, removal_scores);

        // Perform removals one by one if likelihood change below threshold
        double curr_densum = get_sum(freqs);
        double curr_cost = get_cost(freqs, curr_densum);
        std::cerr << "starting cost before removing subwords one by one: " << curr_cost << std::endl;

        int n_removals = 0;
        for (int i=0; i<removal_scores.size(); i++) {
            std::cout << "try removing subword: " << removal_scores[i].first << "\t" << "expected ll diff: " << removal_scores[i].second << std::endl;
//            if (removal_scores[i].second < threshold) break;

            std::map<std::string, double> hypo_freqs = freqs;
            remove_subword(words, vocab, maxlen, removal_scores[i].first, hypo_freqs);
            double hypo_densum = get_sum(hypo_freqs);
            double hypo_cost = get_cost(hypo_freqs, hypo_densum);
            if (hypo_cost-curr_cost > threshold) {
                std::cout << "removed subword: " << removal_scores[i].first << "\t" << "change in likelihood: " << hypo_cost-curr_cost << std::endl;
                curr_densum = hypo_densum;
                curr_cost = hypo_cost;
                freqs = hypo_freqs;
                vocab = freqs;
                freqs_to_logprobs(vocab, hypo_densum);
                n_removals++;
            }
        }

        int n_cutoff = cutoff(freqs, cutoff_value);
        curr_densum = get_sum(freqs);
        curr_cost = get_cost(freqs, curr_densum);
        vocab = freqs;
        freqs_to_logprobs(vocab, curr_densum);

        std::cerr << "subwords removed in this iteration: " << n_removals << std::endl;
        std::cerr << "subwords removed with cutoff this iteration: " << n_cutoff << std::endl;
        std::cerr << "current vocabulary size: " << vocab.size() << std::endl;
        std::cerr << "likelihood after the removals: " << curr_cost << std::endl;

        std::ostringstream vocabfname;
        vocabfname << "vocab.iter" << itern;
        write_vocab(vocabfname.str().c_str(), vocab);

        std::ostringstream freqsfname;
        freqsfname << "freqs.iter" << itern;
        write_vocab(freqsfname.str().c_str(), freqs);

        itern++;
        threshold -= threshold_decrease;

        if (n_removals < min_removals_per_iter) {
            std::cerr << "stopping by min_removals_per_iter." << std::endl;
            break;
        }

        if (vocab.size() <= min_vocab_size) {
            std::cerr << "stopping by min_vocab_size." << std::endl;
            break;
        }
    }

    std::exit(1);
}

