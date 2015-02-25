#ifndef EM
#define EM

#include <map>
#include <string>
#include <vector>

#include "defs.hh"
#include "StringSet.hh"
#include "FactorGraph.hh"
#include "MSFG.hh"


// 1-GRAM
flt_type viterbi(const std::map<std::string, flt_type> &vocab,
                 unsigned int maxlen,
                 const std::string &text,
                 std::vector<std::string> &best_path,
                 bool reverse=true,
                 bool utf8=false);

flt_type viterbi(const StringSet &vocab,
                 const std::string &text,
                 std::vector<std::string> &best_path,
                 bool reverse=true,
                 bool utf8=false);

flt_type viterbi(const StringSet &vocab,
                 const std::string &text,
                 std::map<std::string, flt_type> &stats,
                 bool utf8=false);

// 1-GRAM model, 2-GRAM stats
flt_type viterbi(const StringSet &vocab,
                 const std::string &text,
                 transitions_t &stats,
                 const std::string &start_end_symbol);

class Token {
    public:
        int source;
        flt_type cost;
        Token(): source(-1), cost(MIN_FLOAT) {};
        Token(int src, flt_type cst): source(src), cost(cst) {};
        Token(const Token& orig) { this->source=orig.source; this->cost=orig.cost; };
};

void forward(const StringSet &vocab,
             const std::string &text,
             std::vector<std::vector<Token> > &search,
             std::vector<flt_type> &fw,
             bool utf8=false);

void backward(const StringSet &vocab,
              const std::string &text,
              const std::vector<std::vector<Token> > &search,
              const std::vector<flt_type> &fw,
              std::vector<flt_type> &bw,
              std::map<std::string, flt_type> &stats);

flt_type forward_backward(const StringSet &vocab,
                          const std::string &text,
                          std::map<std::string, flt_type> &stats,
                          bool utf8=false);

flt_type forward_backward(const StringSet &vocab,
                          const std::string &text,
                          std::map<std::string, flt_type> &stats,
                          std::vector<flt_type> &post_scores,
                          bool utf8=false);

flt_type forward_backward(const std::map<std::string, flt_type> &vocab,
                          const std::string &text,
                          std::map<std::string, flt_type> &stats,
                          bool utf8=false);

// 2-GRAM

flt_type viterbi(const transitions_t &transitions,
                 FactorGraph &text,
                 std::vector<std::string> &best_path,
                 bool reverse=true);

flt_type viterbi(const transitions_t &transitions,
                 FactorGraph &text,
                 transitions_t &stats);

void forward(const transitions_t &transitions,
             FactorGraph &text,
             std::vector<flt_type> &fw);

void backward(const FactorGraph &text,
              const std::vector<flt_type> &fw,
              std::vector<flt_type> &bw,
              transitions_t &stats);

// Normal case
flt_type forward_backward(const transitions_t &transitions,
                          FactorGraph &text,
                          transitions_t &stats);

// Get out the final posterior scores for each character position
flt_type forward_backward(const transitions_t &transitions,
                          FactorGraph &text,
                          transitions_t &stats,
                          std::vector<flt_type> &post_scores);

// Allows blocking one factor
flt_type forward_backward(const transitions_t &transitions,
                          FactorGraph &text,
                          transitions_t &stats,
                          const std::string &block);

// Initialization with unigram scores, bigram stats out
flt_type forward_backward(const std::map<std::string, flt_type> &vocab,
                          FactorGraph &text,
                          transitions_t &stats);

flt_type posterior_decode(const transitions_t &transitions,
                          FactorGraph &text,
                          std::vector<std::string> &path,
                          bool reverse=true);


// MultiStringFactorGraph implementations

// Scores each arc in the MSFG with bigram scores
void assign_scores(transitions_t &transitions,
                   MultiStringFactorGraph &msfg);

// Scores each arc in the MSFG with unigram scores
void assign_scores(std::map<std::string, flt_type> &vocab,
                   MultiStringFactorGraph &msfg);

// Basic forward pass for all strings
void forward(const MultiStringFactorGraph &msfg,
             std::vector<flt_type> &fw);

// Forward pass for only one string
flt_type forward(const std::string &text,
                 const MultiStringFactorGraph &msfg,
                 std::map<msfg_node_idx_t, flt_type> &fw);

// Forward for selected strings, don't collect stats
flt_type forward(const std::map<std::string, flt_type> &words,
                 const MultiStringFactorGraph &msfg,
                 const std::set<std::string> &selected_words,
                 bool full_forward_pass = false);

// Compute likelihood of one string
// Same result as for forward pass, but done backwards for efficiency
flt_type likelihood(const std::string &text,
                    const MultiStringFactorGraph &msfg);

// Compute likelihood for given strings
// Same results as for forward pass, but done backwards for efficiency
flt_type likelihood(const std::map<std::string, flt_type> &words,
                    const std::set<std::string> &selected_words,
                    const MultiStringFactorGraph &msfg);

// Compute likelihood for all strings
// Same results as for forward pass, but done backwards for efficiency
flt_type likelihood(const std::map<std::string, flt_type> &words,
                    const MultiStringFactorGraph &msfg);

// Backward pass for one string given forward scores
flt_type backward(const MultiStringFactorGraph &msfg,
                  const std::string &text,
                  const std::vector<flt_type> &fw,
                  transitions_t &stats,
                  flt_type text_weight = 1.0);

// Backward pass for one string given forward scores
// Map container for forward scores
flt_type backward(const MultiStringFactorGraph &msfg,
                  const std::string &text,
                  const std::map<msfg_node_idx_t, flt_type> &fw,
                  transitions_t &stats,
                  flt_type text_weight = 1.0);

// Forward-backward for all strings
flt_type forward_backward(const MultiStringFactorGraph &msfg,
                          transitions_t &stats,
                          std::map<std::string, flt_type> &word_freqs);

// Forward-backward for one string
flt_type forward_backward(const MultiStringFactorGraph &msfg,
                          const std::string &text,
                          transitions_t &stats);

// Viterbi for one string
flt_type viterbi(const MultiStringFactorGraph &msfg,
                 const std::string &text,
                 std::vector<std::string> &best_path);

// Viterbi stats for one string
flt_type viterbi(const MultiStringFactorGraph &msfg,
                 const std::string &text,
                 transitions_t &stats);

#endif /* EM */
