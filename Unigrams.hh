#ifndef UNIGRAMS
#define UNIGRAMS

#include <map>
#include <vector>

#include "defs.hh"
#include "StringSet.hh"
#include "EM.hh"

class Unigrams {
public:

    Unigrams() { this->segf = viterbi; utf8=false; }
    Unigrams(flt_type (*segf)(const StringSet &vocab, const std::string &sentence, std::map<std::string, flt_type> &stats, bool utf8)) : segf(segf) {}

    void set_segmentation_method(flt_type (*segf)(const StringSet &vocab, const std::string &sentence, std::map<std::string, flt_type> &stats, bool utf8)) {
        this->segf = segf;
    }

    void set_utf8(bool utf8) { this->utf8 = utf8; }

    static int read_vocab(std::string fname,
                          std::map<std::string, flt_type> &vocab,
                          int &maxlen);

    static int write_vocab(std::string fname,
                           const std::map<std::string, flt_type> &vocab,
                           bool count_style=false,
                           int num_decimals=6);

    static int read_sents(std::string fname,
                          std::vector<std::string> &sents);

    static void sort_vocab(const std::map<std::string, flt_type> &vocab,
                           std::vector<std::pair<std::string, flt_type> > &sorted_vocab,
                           bool descending=true);

    flt_type iterate(const std::map<std::string, flt_type> &words,
                     std::map<std::string, flt_type> &vocab,
                     unsigned int iterations = 1);

    flt_type iterate(const std::vector<std::string> &sents,
                     std::map<std::string, flt_type> &vocab,
                     unsigned int iterations = 1);

    flt_type resegment_words(const std::map<std::string, flt_type> &words,
                             const std::map<std::string, flt_type> &vocab,
                             std::map<std::string, flt_type> &new_freqs);

    flt_type resegment_words(const std::map<std::string, flt_type> &words,
                             const StringSet &vocab,
                             std::map<std::string, flt_type> &new_freqs);

    flt_type resegment_sents(const std::vector<std::string> &sents,
                             const std::map<std::string, flt_type> &vocab,
                             std::map<std::string, flt_type> &new_freqs);

    flt_type resegment_sents(const std::vector<std::string> &sents,
                             const StringSet &vocab,
                             std::map<std::string, flt_type> &new_freqs);

    static flt_type get_sum(const std::map<std::string, flt_type> &freqs);

    static flt_type get_cost(const std::map<std::string, flt_type> &freqs,
                             flt_type densum);

    static void freqs_to_logprobs(std::map<std::string, flt_type> &vocab);

    static int cutoff(std::map<std::string, flt_type> &vocab,
                      flt_type limit,
                      unsigned int min_length=2);

    int init_candidates(const std::map<std::string, flt_type> &vocab,
                        std::set<std::string> &candidates,
                        unsigned int n_candidates,
                        unsigned int min_length=2);

    int init_candidates_by_random(const std::map<std::string, flt_type> &vocab,
                                  std::set<std::string> &candidates,
                                  unsigned int n_candidates,
                                  unsigned int min_length=2);

    int init_candidates_by_usage(const std::map<std::string, flt_type> &words,
                                 const std::map<std::string, flt_type> &vocab,
                                 std::set<std::string> &candidates,
                                 unsigned int n_candidates,
                                 unsigned int min_length=2,
                                 flt_type max_usage=std::numeric_limits<flt_type>::max());

    flt_type rank_candidates(const std::map<std::string, flt_type> &words,
                             const std::map<std::string, flt_type> &vocab,
                             const std::set<std::string> &candidates,
                             std::map<std::string, flt_type> &new_freqs,
                             std::vector<std::pair<std::string, flt_type> > &removal_scores);

    flt_type rank_candidates(std::vector<std::string> &sents,
                             const std::map<std::string, flt_type> &vocab,
                             const std::set<std::string> &candidates,
                             std::map<std::string, flt_type> &new_freqs,
                             std::vector<std::pair<std::string, flt_type> > &removal_scores);

private:
    flt_type (*segf)(const StringSet &vocab, const std::string &sentence, std::map<std::string, flt_type> &stats, bool utf8);
    bool utf8;
};

#endif /* UNIGRAMS */
