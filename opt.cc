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
    for(std::map<std::string, long>::const_iterator iter = words.begin(); iter != words.end(); ++iter) {

        std::vector<std::string> best_path;
        viterbi(vocab, maxlen, iter->first, best_path);

        if (best_path.size() == 0) {
            std::cerr << "warning, no segmentation for word: " << iter->first << std::endl;
            exit(0);
        }

        // Update statistics
        for (int i=0; i<best_path.size(); i++)
            new_freqs[best_path[i]] += double(iter->second);
    }
}


void resegment_words_w_diff(const std::map<std::string, long> &words,
                                const std::map<std::string, double> &vocab,
                                std::map<std::string, double> &new_freqs,
                                std::map<std::string, std::map<std::string, double> > &diffs,
                                const int maxlen)
{
    std::map<std::string, double> hypo_vocab = vocab;
    for (std::map<std::string, long>::const_iterator iter = words.begin(); iter != words.end(); ++iter) {

        std::vector<std::string> best_path;
        viterbi(vocab, maxlen, iter->first, best_path);

        if (best_path.size() == 0) {
            std::cerr << "warning, no segmentation for word: " << iter->first << std::endl;
            exit(0);
        }

        // Update statistics
        std::map<std::string, double> best_path_types;
        for (int i=0; i<best_path.size(); i++) {
            new_freqs[best_path[i]] += double(iter->second);
            best_path_types[best_path[i]] = 0.0;
        }

        // Hypothesize what the segmentation would be if some subword didn't exist
        for (std::map<std::string, double>::iterator hypoiter = best_path_types.begin(); hypoiter != best_path_types.end(); ++hypoiter) {
            if (diffs.find(hypoiter->first) != diffs.end()) {
                double stored_value = hypo_vocab.at(hypoiter->first);
                hypo_vocab.erase(hypoiter->first);
                std::vector<std::string> hypo_path;
                viterbi(hypo_vocab, maxlen, hypoiter->first, hypo_path);
                for (int ib=0; ib<best_path.size(); ib++)
                    diffs[hypoiter->first][best_path[ib]] -= 1.0;
                for (int ih=0; ih<hypo_path.size(); ih++)
                    diffs[hypoiter->first][hypo_path[ih]] += 1.0;
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
    for(std::map<std::string, double>::iterator iter = vocab.begin(); iter != vocab.end(); ++iter)
        vocab[iter->first] = (log2(iter->second)-densum);
}


void cutoff(std::map<std::string, double> &vocab,
             double limit)
{
    for(std::map<std::string, double>::iterator iter = vocab.begin(); iter != vocab.end(); ++iter)
        if (vocab[iter->first] <= limit) vocab.erase(iter->first);
}


bool ascending_sort(std::pair<std::string, double> i,std::pair<std::string, double> j) { return (i.second < j.second); }
bool descending_sort(std::pair<std::string, double> i,std::pair<std::string, double> j) { return (i.second > j.second); }

void sort_vocab(std::map<std::string, double> &vocab,
                  std::vector<std::pair<std::string, double> > &sorted_vocab)
{
    sorted_vocab.clear();
    for (std::map<std::string,double>::const_iterator it = vocab.begin(); it != vocab.end(); it++) {
        std::pair<std::string, double> curr_pair(it->first, it->second);
        sorted_vocab.push_back(curr_pair);
    }
    std::sort(sorted_vocab.begin(), sorted_vocab.end(), ascending_sort);
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
    sort_vocab(new_morph_freqs, sorted_vocab);

    n_candidates = std::min(n_candidates, (int)vocab.size());
    for (int i=0; i<n_candidates; i++) {
        std::pair<std::string, double> &subword = sorted_vocab[i];
        std::map<std::string, double> emptymap;
        diffs[subword.first] = emptymap;
    }
}


// Perform each of the removals (independent of others in the list) to get
// initial order for the removals
void rank_removal_candidates(const std::map<std::string, long> &words,
                                 const std::map<std::string, double> &vocab,
                                 std::map<std::string, std::map<std::string, double> > &diffs,
                                 const int maxlen,
                                 std::vector<std::pair<std::string, double> > &removal_scores)
{
    std::map<std::string, double> new_morph_freqs;
    resegment_words_w_diff(words, vocab, new_morph_freqs, diffs, maxlen);
    double densum = get_sum(new_morph_freqs);
    double cost = get_cost(new_morph_freqs, densum);

    for (std::map<std::string, std::map<std::string, double> >::iterator iter = diffs.begin(); iter != diffs.end(); ++iter) {
        for (std::map<std::string, double>::iterator diffiter = iter->second.begin(); diffiter != iter->second.end(); ++diffiter)
            new_morph_freqs[diffiter->first] += diffiter->second;

        double hypo_densum = get_sum(new_morph_freqs);
        double hypo_cost = get_cost(new_morph_freqs, hypo_densum);
        std::pair<std::string, double> removal_score = std::make_pair(iter->first, hypo_cost-cost);
        removal_scores.push_back(removal_score);

        for (std::map<std::string, double>::iterator diffiter = iter->second.begin(); diffiter != iter->second.end(); ++diffiter)
            new_morph_freqs[diffiter->first] -= diffiter->second;
    }
    std::sort(removal_scores.begin(), removal_scores.end(), descending_sort);
}


int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " <vocabulary> <words>" << std::endl;
        exit(0);
    }

    int maxlen;
    std::map<std::string, double> vocab;
    std::map<std::string, double> new_morph_freqs;
    std::map<std::string, long> words;

    std::cerr << "Reading vocabulary " << argv[1] << std::endl;
    int retval = read_vocab(argv[1], vocab, maxlen);
    if (retval < 0) {
        std::cerr << "something went wrong reading vocabulary" << std::endl;
        exit(0);
    }
    std::cerr << "\t" << "size: " << vocab.size() << std::endl;
    std::cerr << "\t" << "maximum string length: " << maxlen << std::endl;

    std::cerr << "Reading word list" << std::endl;

    read_words(argv[2], words);


    std::cerr << "\t\t\t" << "vocabulary size: " << vocab.size() << std::endl;

    std::cerr << "Iterating" << std::endl;
    const int n_cutoff_iters = 1;
    int cutoffs[n_cutoff_iters] = { 50 };
    for (int i=0; i<n_cutoff_iters; i++) {
        resegment_words(words, vocab, new_morph_freqs, maxlen);
        double densum = get_sum(new_morph_freqs);
        double cost = get_cost(new_morph_freqs, densum);
        std::cerr << "cost: " << cost << std::endl;
        vocab.swap(new_morph_freqs);
        new_morph_freqs.clear();

        cutoff(vocab, cutoffs[i]);
        std::cerr << "\tcutoff: " << cutoffs[i] << "\t" << "vocabulary size: " << vocab.size() << std::endl;
        densum = get_sum(vocab);
        freqs_to_logprobs(vocab, densum);
    }


    int n_candidates = 100;
    std::map<std::string, std::map<std::string, double> > diffs;
    init_removal_candidates(n_candidates, maxlen, words, vocab, diffs);

    std::vector<std::pair<std::string, double> > removal_scores;
    rank_removal_candidates(words, vocab, diffs, maxlen, removal_scores);

    for (int i=0; i<removal_scores.size(); i++)
        std::cout << "word: " << removal_scores[i].first << "\t" << "change in cost after removal: " << removal_scores[i].second << std::endl;

    // Really perform removals one by one


    exit(1);
}

