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


void viterbi(const std::map<std::string, double> &vocab,
               int maxlen,
               const std::string &sentence,
               std::vector<std::string> &best_path)
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

    std::reverse(best_path.begin(), best_path.end());
}
