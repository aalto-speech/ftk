#ifndef UNIGRAMS
#define UNIGRAMS

#include <map>
#include <vector>

#include "defs.hh"
#include "StringSet.hh"

class Unigrams {
public:

    Unigrams() {}
    Unigrams(flt_type (*segf)(const StringSet<flt_type> &vocab, const std::string &sentence, std::map<std::string, flt_type> &stats)) : segf(segf) {}

    void set_segmentation_method(flt_type (*segf)(const StringSet<flt_type> &vocab, const std::string &sentence, std::map<std::string, flt_type> &stats)) {
        this->segf = segf;
    }

    void resegment_words(const std::map<std::string, flt_type> &words,
                         const std::map<std::string, flt_type> &vocab,
                         std::map<std::string, flt_type> &new_freqs);

    void resegment_words_w_diff(const std::map<std::string, flt_type> &words,
                                const std::map<std::string, flt_type> &vocab,
                                std::map<std::string, flt_type> &new_freqs,
                                std::map<std::string, std::map<std::string, flt_type> > &diffs);

    flt_type get_sum(const std::map<std::string, flt_type> &freqs);

    flt_type get_sum(const std::map<std::string, flt_type> &freqs,
                     const std::map<std::string, flt_type> &freq_diffs);

    flt_type get_cost(const std::map<std::string, flt_type> &freqs,
                      flt_type densum);

    flt_type get_cost(const std::map<std::string, flt_type> &freqs,
                      const std::map<std::string, flt_type> &freq_diffs,
                      flt_type densum);

    void apply_freq_diffs(std::map<std::string, flt_type> &freqs,
                          const std::map<std::string, flt_type> &freq_diffs);

    void apply_backpointer_changes(std::map<std::string, std::map<std::string, flt_type> > &backpointers,
                                   const std::map<std::string, std::map<std::string, flt_type> > &bps_to_remove,
                                   const std::map<std::string, std::map<std::string, flt_type> > &bps_to_add);

    void freqs_to_logprobs(std::map<std::string, flt_type> &vocab,
                           flt_type densum);

    int cutoff(std::map<std::string, flt_type> &vocab,
               flt_type limit);

    void init_removal_candidates(int n_candidates,
                                 const std::map<std::string, flt_type> &words,
                                 const std::map<std::string, flt_type> &vocab,
                                 std::map<std::string, std::map<std::string, flt_type> > &diffs);

    void rank_removal_candidates(const std::map<std::string, flt_type> &words,
                                 const std::map<std::string, flt_type> &vocab,
                                 std::map<std::string, std::map<std::string, flt_type> > &diffs,
                                 std::map<std::string, flt_type> &new_morph_freqs,
                                 std::vector<std::pair<std::string, flt_type> > &removal_scores);

    void get_backpointers(const std::map<std::string, flt_type> &words,
                          const std::map<std::string, flt_type> &vocab,
                          std::map<std::string, std::map<std::string, flt_type> > &backpointers);

    void hypo_removal(StringSet<flt_type> &vocab,
                      const std::string &subword,
                      const std::map<std::string, std::map<std::string, flt_type> > &backpointers,
                      std::map<std::string, std::map<std::string, flt_type> > &backpointers_to_remove,
                      std::map<std::string, std::map<std::string, flt_type> > &backpointers_to_add,
                      std::map<std::string, flt_type> &freq_diffs);

private:
    flt_type (*segf)(const StringSet<flt_type> &vocab, const std::string &sentence, std::map<std::string, flt_type> &stats);
};

#endif /* UNIGRAMS */
