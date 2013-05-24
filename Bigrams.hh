#ifndef BIGRAMS
#define BIGRAMS

#include <map>
#include <string>

#include "defs.hh"
#include "FactorEncoder.hh"

class Bigrams {
public:

Bigrams() {}

static void update_trans_stats(const transitions_t &collected_stats,
                               flt_type weight,
                               transitions_t &trans_stats,
                               std::map<std::string, flt_type> &trans_normalizers,
                               std::map<std::string, flt_type> &unigram_stats);

static void collect_trans_stats(const transitions_t &transitions,
                                const std::map<std::string, flt_type> &words,
                                std::map<std::string, FactorGraph*> &fg_words,
                                transitions_t &trans_stats,
                                std::map<std::string, flt_type> &trans_normalizers,
                                std::map<std::string, flt_type> &unigram_stats,
                                bool fb=true);

static void normalize(transitions_t &trans_stats,
                      std::map<std::string, flt_type> &trans_normalizers,
                      flt_type min_cost = -200.0);

static void write_transitions(const transitions_t &transitions,
                              const std::string &filename);

static flt_type bigram_cost(const transitions_t &transitions,
                            const transitions_t &trans_stats);

static int cutoff(const std::map<std::string, flt_type> &unigram_stats,
                  flt_type cutoff,
                  transitions_t &transitions,
                  std::map<std::string, FactorGraph*> &fg_words);

static int remove_least_common(const std::map<std::string, flt_type> &unigram_stats,
                               int num_removals,
                               transitions_t &transitions,
                               std::map<std::string, FactorGraph*> &fg_words);

static int transition_count(const transitions_t &transitions);

};

#endif /* BIGRAMS */