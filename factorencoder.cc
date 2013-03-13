#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <queue>

#include "factorencoder.hh"


class Token {
    public:
        int source;
        double cost;
        Token(): source(-1), cost(std::numeric_limits<double>::max()) {};
        Token(int src, double cst): source(src), cost(cst) {};
        Token(const Token& orig) { this->source=orig.source; this->cost=orig.cost; };
};

// Note just changed > to < ..
bool operator< (const Token& token1, const Token &token2)
{
    return token1.cost < token2.cost;
}

typedef std::priority_queue<Token> Node;


int read_vocab(const char* fname,
                 std::map<std::string, double> &vocab,
                 int &maxlen)
{
    std::ifstream vocabfile(fname);
    if (!vocabfile) return -1;

    std::string line, word;
    double count;
    maxlen = -1;
    while (getline(vocabfile, line)) {
        std::stringstream ss(line);
        ss >> count;
        ss >> word;
        vocab[word] = count;
        maxlen = std::max(maxlen, int(word.length()));
    }
    vocabfile.close();

    return vocab.size();
}


int write_vocab(const char* fname,
                  const std::map<std::string, double> &vocab)
{
    std::ofstream vocabfile(fname);
    if (!vocabfile) return -1;

    std::vector<std::pair<std::string, double> > sorted_vocab;
    sort_vocab(vocab, sorted_vocab);
    for (int i=0; i<sorted_vocab.size(); i++)
        vocabfile << sorted_vocab[i].second << " " << sorted_vocab[i].first << std::endl;
    vocabfile.close();

    return vocab.size();
}


bool descending_sort(std::pair<std::string, double> i,std::pair<std::string, double> j) { return (i.second > j.second); }
bool ascending_sort(std::pair<std::string, double> i,std::pair<std::string, double> j) { return (i.second < j.second); }

void sort_vocab(const std::map<std::string, double> &vocab,
                  std::vector<std::pair<std::string, double> > &sorted_vocab,
                  bool descending)
{
    sorted_vocab.clear();
    for (std::map<std::string,double>::const_iterator it = vocab.begin(); it != vocab.end(); it++) {
        std::pair<std::string, double> curr_pair(it->first, it->second);
        sorted_vocab.push_back(curr_pair);
    }
    if (descending)
        std::sort(sorted_vocab.begin(), sorted_vocab.end(), descending_sort);
    else
        std::sort(sorted_vocab.begin(), sorted_vocab.end(), ascending_sort);
}


void viterbi(const std::map<std::string, double> &vocab,
               int maxlen,
               const std::string &sentence,
               std::vector<std::string> &best_path,
               bool reverse)
{
    if (sentence.length() == 0) return;
    std::vector<Node> search(sentence.length());
    int start_pos = 0;
    int end_pos = 0;
    int len = 0;

    for (int i=0; i<sentence.length(); i++) {

        // Iterate all factors ending in this position
        for (int j=std::max(0, i-maxlen); j<=i; j++) {

            start_pos = j;
            end_pos = i+1;
            len = end_pos-start_pos;

            if (vocab.find(sentence.substr(start_pos, len)) != vocab.end()) {
                Token tok(j-1, vocab.at(sentence.substr(start_pos, len)));
                if (j-1 >= 0 && search[j-1].size() > 0) {
                    Token source_top = search[j-1].top();
                    tok.cost += source_top.cost;
                }
                search[i].push(tok);
            }
        }
    }

    // Look up the best path
    int target = search.size()-1;
    if (search[target].size() == 0) return;
    Token top = search[target].top();
    int source = top.source;

    while (true) {
        best_path.push_back(sentence.substr(source+1, target-source));
        if (source == -1) break;
        target = source;
        Token top = search[target].top();
        source = top.source;
    }

    if (reverse) std::reverse(best_path.begin(), best_path.end());
}
