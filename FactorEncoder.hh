#ifndef FACTOR_ENCODER
#define FACTOR_ENCODER

#include <map>
#include <string>
#include <vector>

#include "defs.hh"
#include "StringSet.hh"
#include "FactorGraph.hh"
#include "MSFG.hh"


// 1-GRAM
flt_type viterbi(const std::map<std::string, flt_type> &vocab,
                 int maxlen,
                 const std::string &text,
                 std::vector<std::string> &best_path,
                 bool reverse=true);

flt_type viterbi(const StringSet &vocab,
                 const std::string &text,
                 std::vector<std::string> &best_path,
                 bool reverse=true);

flt_type viterbi(const StringSet &vocab,
                 const std::string &text,
                 std::map<std::string, flt_type> &stats);


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
             std::vector<flt_type> &fw);

void backward(const StringSet &vocab,
              const std::string &text,
              const std::vector<std::vector<Token> > &search,
              const std::vector<flt_type> &fw,
              std::vector<flt_type> &bw,
              std::map<std::string, flt_type> &stats);

flt_type forward_backward(const StringSet &vocab,
                          const std::string &text,
                          std::map<std::string, flt_type> &stats);

flt_type forward_backward(const StringSet &vocab,
                          const std::string &text,
                          std::map<std::string, flt_type> &stats,
                          std::vector<flt_type> &post_scores);

flt_type forward_backward(const std::map<std::string, flt_type> &vocab,
                          const std::string &text,
                          std::map<std::string, flt_type> &stats);

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
void score_arcs(const transitions_t &transitions,
                MultiStringFactorGraph &msfg);

// Scores each arc in the MSFG with unigram scores
void score_arcs(const std::map<std::string, flt_type> &vocab,
                MultiStringFactorGraph &msfg);

// Basic forward pass, assumes prescored arcs in msfg
void forward(MultiStringFactorGraph &msfg,
             std::vector<flt_type> &fw);

// Normal forward pass for all paths with bigram scores
void forward(const transitions_t &transitions,
             MultiStringFactorGraph &msfg,
             std::vector<flt_type> &fw);

// Forward pass for all paths using unigram stats
void forward(const std::map<std::string, flt_type> &vocab,
             MultiStringFactorGraph &msfg,
             std::vector<flt_type> &fw);

// Forward pass for only one string
void forward(const transitions_t &transitions,
             const std::string &text,
             MultiStringFactorGraph &msfg,
             std::map<msfg_node_idx_t, flt_type> &fw);

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
flt_type forward_backward(const transitions_t &transitions,
                          MultiStringFactorGraph &msfg,
                          transitions_t &stats,
                          std::map<std::string, flt_type> &word_freqs);

// Forward-backward for one string
flt_type forward_backward(const transitions_t &transitions,
                          MultiStringFactorGraph &msfg,
                          const std::string &text,
                          transitions_t &stats);

// Forward-backward for selected strings, don't collect stats
flt_type forward_backward(const transitions_t &transitions,
                          const std::map<std::string, flt_type> &words,
                          MultiStringFactorGraph &msfg,
                          const std::set<std::string> &selected_words);


#endif /* FACTOR_ENCODER */
