#ifndef FACTOR_ENCODER
#define FACTOR_ENCODER

#include <map>
#include <string>
#include <vector>


int read_vocab(const char* fname,
               std::map<std::string, double> &vocab,
               int &maxlen);

int write_vocab(const char* fname,
                  const std::map<std::string, double> &vocab);

void sort_vocab(const std::map<std::string, double> &vocab,
                  std::vector<std::pair<std::string, double> > &sorted_vocab,
                  bool descending=true);

void viterbi(const std::map<std::string, double> &vocab,
             int maxlen,
             const std::string &sentence,
             std::vector<std::string> &best_path,
             bool reverse=true);

#endif
