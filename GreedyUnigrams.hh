#include <map>
#include <vector>

#include "MorphSet.hh"


void resegment_words(const std::map<std::string, double> &words,
                     const std::map<std::string, double> &vocab,
                     std::map<std::string, double> &new_freqs);

void resegment_words_w_diff(const std::map<std::string, double> &words,
                            const std::map<std::string, double> &vocab,
                            std::map<std::string, double> &new_freqs,
                            std::map<std::string, std::map<std::string, double> > &diffs);

double get_sum(const std::map<std::string, double> &freqs);

double get_sum(const std::map<std::string, double> &freqs,
               const std::map<std::string, double> &freq_diffs);

double get_cost(const std::map<std::string, double> &freqs,
                double densum);

double get_cost(const std::map<std::string, double> &freqs,
                const std::map<std::string, double> &freq_diffs,
                double densum);

void apply_freq_diffs(std::map<std::string, double> &freqs,
                      const std::map<std::string, double> &freq_diffs);

void apply_backpointer_changes(std::map<std::string, std::map<std::string, double> > &backpointers,
                               const std::map<std::string, std::map<std::string, double> > &bps_to_remove,
                               const std::map<std::string, std::map<std::string, double> > &bps_to_add);

void freqs_to_logprobs(std::map<std::string, double> &vocab,
                       double densum);

int cutoff(std::map<std::string, double> &vocab,
           double limit);

void init_removal_candidates(int n_candidates,
                             const std::map<std::string, double> &words,
                             const std::map<std::string, double> &vocab,
                             std::map<std::string, std::map<std::string, double> > &diffs);

void rank_removal_candidates(const std::map<std::string, double> &words,
                             const std::map<std::string, double> &vocab,
                             std::map<std::string, std::map<std::string, double> > &diffs,
                             std::map<std::string, double> &new_morph_freqs,
                             vector<pair<std::string, double> > &removal_scores);

void get_backpointers(const std::map<std::string, double> &words,
                      const std::map<std::string, double> &vocab,
                      std::map<std::string, std::map<std::string, double> > &backpointers);

void hypo_removal(MorphSet &vocab,
                  const std::string &subword,
                  const std::map<std::string, std::map<std::string, double> > &backpointers,
                  std::map<std::string, std::map<std::string, double> > &backpointers_to_remove,
                  std::map<std::string, std::map<std::string, double> > &backpointers_to_add,
                  std::map<std::string, double> &freq_diffs);
