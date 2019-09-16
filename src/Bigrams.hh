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

static flt_type iterate(const std::map<std::string, flt_type> &words,
                        MultiStringFactorGraph &msfg,
                        transitions_t &transitions,
                        bool forward_backward=false,
                        unsigned int iterations=1);

static flt_type iterate_kn(const std::map<std::string, flt_type> &words,
                           MultiStringFactorGraph &msfg,
                           transitions_t &transitions,
                           bool forward_backward=false,
                           flt_type D=0.1,
                           unsigned int iterations=1);

static void update_trans_stats(const transitions_t &collected_stats,
                               flt_type weight,
                               transitions_t &trans_stats,
                               std::map<std::string, flt_type> &unigram_stats);

static void update_trans_stats(const transitions_t &collected_stats,
                               flt_type weight,
                               transitions_t &trans_stats);

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

static void get_unigram_stats(const transitions_t &trans_stats,
                              std::map<std::string, flt_type> &unigram_stats);

static void finalize_viterbi_stats(const MultiStringFactorGraph &msfg,
                                   transitions_t &trans_stats);

static void freqs_to_logprobs(transitions_t &trans_stats,
                              flt_type min_lp = FLOOR_LP);

static void normalize(transitions_t &trans_stats);

static void write_transitions(const transitions_t &transitions,
                              const std::string &filename,
                              bool count_style=false,
                              int num_decimals=6);

static int read_transitions(transitions_t &transitions,
                            const std::string &filename);

static int cutoff(const std::map<std::string, flt_type> &unigram_stats,
                  flt_type cutoff,
                  transitions_t &transitions,
                  std::map<std::string, FactorGraph*> &fg_words);

static int cutoff(const std::map<std::string, flt_type> &unigram_stats,
                  flt_type cutoff,
                  transitions_t &transitions,
                  MultiStringFactorGraph &msfg);

static int remove_least_common(const std::map<std::string, flt_type> &unigram_stats,
                               unsigned int num_removals,
                               transitions_t &transitions,
                               std::map<std::string, FactorGraph*> &fg_words);

static int remove_least_common(const std::map<std::string, flt_type> &unigram_stats,
                               unsigned int num_removals,
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
                                   std::string src,
                                   std::string tgt,
                                   transitions_t &transitions,
                                   transitions_t &changes);

static void trans_to_vocab(const transitions_t &transitions,
                           std::map<std::string, flt_type> &vocab);

static void get_backpointers(const MultiStringFactorGraph &msfg,
                             std::map<std::string, std::set<std::string> > &backpointers,
                             unsigned int minlen=2);

static void remove_transitions(std::vector<std::string> &to_remove,
                               transitions_t &transitions);

static int init_candidates_freq(unsigned int n_candidates,
                                const std::map<std::string, flt_type> &unigram_stats,
                                std::map<std::string, flt_type> &candidates,
                                const std::set<std::string> &stoplist);

static int init_candidates_num_contexts(unsigned int n_candidates,
                                        const transitions_t &transitions,
                                        const std::map<std::string, flt_type> &unigram_stats,
                                        std::map<std::string, flt_type> &candidates,
                                        const std::set<std::string> &stoplist);

static void rank_candidate_subwords(const std::map<std::string, flt_type> &words,
                                    const MultiStringFactorGraph &msfg,
                                    const std::map<std::string, flt_type> &unigram_stats,
                                    transitions_t &transitions,
                                    std::map<std::string, flt_type> &candidates,
                                    bool forward_backward=true,
                                    bool normalize_by_bigram_count=false);

static void kn_smooth(const transitions_t &counts,
                      transitions_t &kn,
                      double D=0.1,
                      double min_lp=FLOOR_LP);


private:

};

#endif /* BIGRAMS */
