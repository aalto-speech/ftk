#ifndef UNIGRAMS
#define UNIGRAMS

#include <map>
#include <vector>

#include "defs.hh"
#include "StringSet.hh"
#include "FactorEncoder.hh"

class Unigrams {
public:

    Unigrams() { this->segf = viterbi; }
    Unigrams(flt_type (*segf)(const StringSet &vocab, const std::string &sentence, std::map<std::string, flt_type> &stats)) : segf(segf) {}

    void set_segmentation_method(flt_type (*segf)(const StringSet &vocab, const std::string &sentence, std::map<std::string, flt_type> &stats)) {
        this->segf = segf;
    }

    static int read_vocab(std::string fname,
                          std::map<std::string, flt_type> &vocab,
                          int &maxlen);

    static int write_vocab(std::string fname,
                           const std::map<std::string, flt_type> &vocab,
                           bool count_style=false);

    static void sort_vocab(const std::map<std::string, flt_type> &vocab,
                           std::vector<std::pair<std::string, flt_type> > &sorted_vocab,
                           bool descending=true);

    flt_type iterate(const std::map<std::string, flt_type> &words,
                     std::map<std::string, flt_type> &vocab,
                     unsigned int iterations = 1);

    flt_type resegment_words(const std::map<std::string, flt_type> &words,
                             const std::map<std::string, flt_type> &vocab,
                             std::map<std::string, flt_type> &new_freqs);

    flt_type resegment_words(const std::map<std::string, flt_type> &words,
                             const StringSet &vocab,
                             std::map<std::string, flt_type> &new_freqs);

    flt_type resegment_data(std::string fname,
                            const std::map<std::string, flt_type> &vocab,
                            std::map<std::string, flt_type> &new_freqs);

    flt_type resegment_data(std::string fname,
                            const StringSet &vocab,
                            std::map<std::string, flt_type> &new_freqs);

    static flt_type get_sum(const std::map<std::string, flt_type> &freqs);

    static flt_type get_cost(const std::map<std::string, flt_type> &freqs,
                             flt_type densum);

    static void freqs_to_logprobs(std::map<std::string, flt_type> &vocab,
                                  flt_type densum);

    static int cutoff(std::map<std::string, flt_type> &vocab,
                      flt_type limit);

    int init_removal_candidates(int n_candidates,
                                const std::map<std::string, flt_type> &words,
                                const std::map<std::string, flt_type> &vocab,
                                std::set<std::string> &candidates);

    void rank_removal_candidates(const std::map<std::string, flt_type> &words,
                                 const std::map<std::string, flt_type> &vocab,
                                 const std::set<std::string> &candidates,
                                 std::map<std::string, flt_type> &new_morph_freqs,
                                 std::vector<std::pair<std::string, flt_type> > &removal_scores);

private:
    flt_type (*segf)(const StringSet &vocab, const std::string &sentence, std::map<std::string, flt_type> &stats);
};

#endif /* UNIGRAMS */
