#ifndef FACTOR_ENCODER
#define FACTOR_ENCODER

#include <map>
#include <string>
#include <vector>


int read_vocab(const char* fname,
               std::map<std::string, double> &vocab,
               int &maxlen);


int viterbi(const std::map<std::string, double> &vocab,
            int maxlen,
            const std::string &sentence,
            std::vector<std::string> &best_path);

#endif
