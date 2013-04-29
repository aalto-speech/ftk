#ifndef FACTOR_ENCODER
#define FACTOR_ENCODER

#include <map>
#include <string>
#include <vector>

#include "MorphSet.hh"

using namespace std;

int read_vocab(string fname,
               map<string, double> &vocab,
               int &maxlen);

int write_vocab(const char* fname,
                const map<string, double> &vocab);

void sort_vocab(const map<string, double> &vocab,
                vector<pair<string, double> > &sorted_vocab,
                bool descending=true);

void viterbi(const map<string, double> &vocab,
             int maxlen,
             const string &sentence,
             vector<string> &best_path,
             bool reverse=true);

void viterbi(MorphSet &vocab,
             const string &sentence,
             vector<string> &best_path,
             bool reverse=true);

void viterbi(MorphSet &vocab,
             const string &sentence,
             map<string, double> &stats);

double add_log_domain_probs(double a,
                            double b);

void forward_backward(MorphSet &vocab,
                      const string &sentence,
                      map<string, double> &stats);

void forward_backward(const map<string, double> &vocab,
                      const string &sentence,
                      map<string, double> &stats);


#endif
