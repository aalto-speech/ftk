#ifndef FACTOR_ENCODER
#define FACTOR_ENCODER

#include <limits>
#include <map>
#include <string>
#include <vector>

#include "defs.hh"
#include "StringSet.hh"
#include "FactorGraph.hh"
#include "MSFG.hh"

typedef std::map<std::string, std::map<std::string, flt_type> > transitions_t;


// 1-GRAM
int read_vocab(std::string fname,
               std::map<std::string, flt_type> &vocab,
               int &maxlen);

int write_vocab(std::string fname,
                const std::map<std::string, flt_type> &vocab);

void sort_vocab(const std::map<std::string, flt_type> &vocab,
                std::vector<std::pair<std::string, flt_type> > &sorted_vocab,
                bool descending=true);

flt_type viterbi(const std::map<std::string, flt_type> &vocab,
                 int maxlen,
                 const std::string &text,
                 std::vector<std::string> &best_path,
                 bool reverse=true);

flt_type viterbi(const StringSet<flt_type> &vocab,
                 const std::string &text,
                 std::vector<std::string> &best_path,
                 bool reverse=true);

flt_type viterbi(const StringSet<flt_type> &vocab,
                 const std::string &text,
                 std::map<std::string, flt_type> &stats);

flt_type add_log_domain_probs(flt_type a,
                              flt_type b);


class Token {
    public:
        int source;
        flt_type cost;
        Token(): source(-1), cost(-std::numeric_limits<flt_type>::max()) {};
        Token(int src, flt_type cst): source(src), cost(cst) {};
        Token(const Token& orig) { this->source=orig.source; this->cost=orig.cost; };
};

void forward(const StringSet<flt_type> &vocab,
             const std::string &text,
             std::vector<std::vector<Token> > &search,
             std::vector<flt_type> &fw);

void backward(const StringSet<flt_type> &vocab,
              const std::string &text,
              const std::vector<std::vector<Token> > &search,
              const std::vector<flt_type> &fw,
              std::vector<flt_type> &bw,
              std::map<std::string, flt_type> &stats);

flt_type forward_backward(const StringSet<flt_type> &vocab,
                          const std::string &text,
                          std::map<std::string, flt_type> &stats);

flt_type forward_backward(const StringSet<flt_type> &vocab,
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


// 2-GRAM
// MultiStringFactorGraph implementations
void forward(const transitions_t &transitions,
             MultiStringFactorGraph &msfg,
             std::vector<flt_type> &fw);

void backward(const MultiStringFactorGraph &msfg,
              const std::string &text,
              const std::vector<flt_type> &fw,
              std::vector<flt_type> &bw,
              transitions_t &stats);

// Forward-backward for all strings
void forward_backward(const transitions_t &transitions,
                      MultiStringFactorGraph &msfg,
                      transitions_t &stats);

// Forward-backward for one string
void forward_backward(const transitions_t &transitions,
                      MultiStringFactorGraph &msfg,
                      const std::string &text,
                      transitions_t &stats);


#endif /* FACTOR_ENCODER */
