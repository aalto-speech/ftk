#ifndef BIGRAMS
#define BIGRAMS

#include <map>
#include <string>

#include "defs.hh"
#include "FactorGraph.hh"
#include "MSFG.hh"


class Bigrams {
public:

Bigrams() {}

static void update_trans_stats(const transitions_t &collected_stats,
                               flt_type weight,
                               transitions_t &trans_stats,
                               std::map<std::string, flt_type> &unigram_stats);

static void collect_trans_stats(transitions_t &transitions,
                                const std::map<std::string, flt_type> &words,
                                std::map<std::string, FactorGraph*> &fg_words,
                                transitions_t &trans_stats,
                                std::map<std::string, flt_type> &unigram_stats,
                                bool fb=true);

static flt_type collect_trans_stats(const std::map<std::string, flt_type> &words,
                                    MultiStringFactorGraph &msfg,
                                    transitions_t &trans_stats,
                                    std::map<std::string, flt_type> &unigram_stats,
                                    bool fb=true);

static void normalize(transitions_t &trans_stats,
                      flt_type min_cost = FLOOR_LP);

static void copy_transitions(transitions_t &src,
                             transitions_t &tgt);

static void write_transitions(const transitions_t &transitions,
                              const std::string &filename,
                              bool count_style=false);

static int read_transitions(transitions_t &transitions,
                            const std::string &filename);

static flt_type bigram_cost(const transitions_t &transitions,
                            const transitions_t &trans_stats);

static int cutoff(const std::map<std::string, flt_type> &unigram_stats,
                  flt_type cutoff,
                  transitions_t &transitions,
                  std::map<std::string, FactorGraph*> &fg_words);

static int cutoff(const std::map<std::string, flt_type> &unigram_stats,
                  flt_type cutoff,
                  transitions_t &transitions,
                  MultiStringFactorGraph &msfg);

static int remove_least_common(const std::map<std::string, flt_type> &unigram_stats,
                               int num_removals,
                               transitions_t &transitions,
                               std::map<std::string, FactorGraph*> &fg_words);

static int remove_least_common(const std::map<std::string, flt_type> &unigram_stats,
                               int num_removals,
                               transitions_t &transitions,
                               MultiStringFactorGraph &msfg);

static int transition_count(const transitions_t &transitions);

static void reverse_transitions(const transitions_t &transitions,
                                transitions_t &reverse_transitions);

static flt_type disable_string(const transitions_t &reverse_transitions,
                               const std::string &text,
                               const std::map<std::string, flt_type> &unigram_stats,
                               transitions_t &transitions,
                               transitions_t &changes);

static void restore_string(transitions_t &transitions,
                           const transitions_t &changes);

static flt_type disable_transition(const std::map<std::string, flt_type> &unigram_stats,
                                   const transitions_t &to_remove,
                                   transitions_t &transitions,
                                   transitions_t &changes);

static void trans_to_vocab(const transitions_t &transitions,
                           std::map<std::string, flt_type> &vocab);

static flt_type score(const transitions_t &transitions,
                      std::vector<std::string> &path);

static void get_backpointers(const MultiStringFactorGraph &msfg,
                             std::map<std::string, std::set<std::string> > &backpointers,
                             unsigned int minlen=2);

static void remove_transitions(std::vector<std::string> &to_remove,
                               transitions_t &transitions);

static int init_removal_candidates(int n_candidates,
                                   const std::map<std::string, flt_type> &unigram_stats,
                                   std::map<std::string, flt_type> &candidates);

static void rank_removal_candidates(const std::map<std::string, flt_type> &words,
                                    const MultiStringFactorGraph &msfg,
                                    const std::map<std::string, flt_type> &unigram_stats,
                                    transitions_t &transitions,
                                    std::map<std::string, flt_type> &candidates);

private:

};

#endif /* BIGRAMS */
