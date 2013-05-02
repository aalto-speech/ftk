#ifndef FACTOR_ENCODER
#define FACTOR_ENCODER

#include <map>
#include <string>
#include <vector>

#include "defs.hh"
#include "StringSet.hh"

using namespace std;

int read_vocab(string fname,
               map<string, flt_type> &vocab,
               int &maxlen);

int write_vocab(string fname,
                const map<string, flt_type> &vocab);

void sort_vocab(const map<string, flt_type> &vocab,
                vector<pair<string, flt_type> > &sorted_vocab,
                bool descending=true);

void viterbi(const map<string, flt_type> &vocab,
             int maxlen,
             const string &text,
             vector<string> &best_path,
             bool reverse=true);

void viterbi(const StringSet<flt_type> &vocab,
             const string &text,
             vector<string> &best_path,
             bool reverse=true);

void viterbi(const StringSet<flt_type> &vocab,
             const string &text,
             map<string, flt_type> &stats);

flt_type add_log_domain_probs(flt_type a,
                              flt_type b);

void forward_backward(const StringSet<flt_type> &vocab,
                      const string &text,
                      map<string, flt_type> &stats);

void forward_backward(const map<string, flt_type> &vocab,
                      const string &text,
                      map<string, flt_type> &stats);

#endif /* FACTOR_ENCODER */
