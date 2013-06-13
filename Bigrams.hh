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

static void collect_trans_stats(const transitions_t &transitions,
                                const std::map<std::string, flt_type> &words,
                                std::map<std::string, FactorGraph*> &fg_words,
                                transitions_t &trans_stats,
                                std::map<std::string, flt_type> &unigram_stats,
                                bool fb=true);

static flt_type collect_trans_stats(const transitions_t &transitions,
                                    const std::map<std::string, flt_type> &words,
                                    MultiStringFactorGraph &msfg,
                                    transitions_t &trans_stats,
                                    std::map<std::string, flt_type> &unigram_stats,
                                    bool fb=true,
                                    bool threaded=true);

static void threaded_backward(const MultiStringFactorGraph *msfg,
                              const std::vector<flt_type> *fw,
                              const std::map<std::string, flt_type> *words,
                              transitions_t *trans_stats,
                              int thread_index,
                              int thread_count,
                              flt_type *total_lp);

static flt_type collect_trans_stats(const std::map<std::string, flt_type> &vocab,
                                    const std::map<std::string, flt_type> &words,
                                    MultiStringFactorGraph &msfg,
                                    transitions_t &trans_stats,
                                    std::map<std::string, flt_type> &unigram_stats,
                                    bool fb=true);

static void normalize(transitions_t &trans_stats,
                      flt_type min_cost = SMALL_LP);

static void write_transitions(const transitions_t &transitions,
                              const std::string &filename);

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

static void remove_string(const transitions_t &reverse_transitions,
                          const std::string &text,
                          transitions_t &transitions);

static void trans_to_vocab(const transitions_t &transitions,
                           std::map<std::string, flt_type> &vocab);

static void get_backpointers(const MultiStringFactorGraph &msfg,
                             std::map<std::string, std::set<std::string> > &backpointers,
                             unsigned int minlen=2);

static void augment_affected_strings(const transitions_t &reverse_transitions,
                                     const std::map<std::string, std::set<std::string> > &backpointers,
                                     const std::string &factor,
                                     std::set<std::string> &strings);


private:

static void remove_transitions(std::vector<std::string> &to_remove,
                               transitions_t &transitions);

};

#endif /* BIGRAMS */
