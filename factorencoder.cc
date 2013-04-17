#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <queue>

using namespace std;

#include "factorencoder.hh"


class Token {
    public:
        int source;
        double cost;
        Token(): source(-1), cost(-numeric_limits<double>::max()) {};
        Token(int src, double cst): source(src), cost(cst) {};
        Token(const Token& orig) { this->source=orig.source; this->cost=orig.cost; };
};


int read_vocab(const char* fname,
               map<string, double> &vocab,
               int &maxlen)
{
    ifstream vocabfile(fname);
    if (!vocabfile) return -1;

    string line, word;
    double count;
    maxlen = -1;
    while (getline(vocabfile, line)) {
        stringstream ss(line);
        ss >> count;
        ss >> word;
        vocab[word] = count;
        maxlen = max(maxlen, int(word.length()));
    }
    vocabfile.close();

    return vocab.size();
}


int write_vocab(const char* fname,
                const map<string, double> &vocab)
{
    ofstream vocabfile(fname);
    if (!vocabfile) return -1;

    vector<pair<string, double> > sorted_vocab;
    sort_vocab(vocab, sorted_vocab);
    for (int i=0; i<sorted_vocab.size(); i++)
        vocabfile << sorted_vocab[i].second << " " << sorted_vocab[i].first << endl;
    vocabfile.close();

    return vocab.size();
}


bool descending_sort(pair<string, double> i,pair<string, double> j) { return (i.second > j.second); }
bool ascending_sort(pair<string, double> i,pair<string, double> j) { return (i.second < j.second); }

void sort_vocab(const map<string, double> &vocab,
                vector<pair<string, double> > &sorted_vocab,
                bool descending)
{
    sorted_vocab.clear();
    for (auto it = vocab.cbegin(); it != vocab.cend(); it++) {
        pair<string, double> curr_pair(it->first, it->second);
        sorted_vocab.push_back(curr_pair);
    }
    if (descending)
        sort(sorted_vocab.begin(), sorted_vocab.end(), descending_sort);
    else
        sort(sorted_vocab.begin(), sorted_vocab.end(), ascending_sort);
}


void viterbi(const map<string, double> &vocab,
             int maxlen,
             const string &sentence,
             vector<string> &best_path,
             bool reverse)
{
    if (sentence.length() == 0) return;
    vector<Token> search(sentence.length());
    int start_pos = 0;
    int end_pos = 0;
    int len = 0;

    for (int i=0; i<sentence.length(); i++) {

        // Iterate all factors ending in this position
        for (int j=max(0, i-maxlen); j<=i; j++) {

            start_pos = j;
            end_pos = i+1;
            len = end_pos-start_pos;

            if (vocab.find(sentence.substr(start_pos, len)) != vocab.end()) {
                double cost = vocab.at(sentence.substr(start_pos, len));
                if (j-1 >= 0) {
                    if (search[j-1].cost == -numeric_limits<double>::max()) break;
                    cost += search[j-1].cost;
                }
                if (cost > search[i].cost) {
                    search[i].cost = cost;
                    search[i].source = j-1;
                }
            }
        }
    }

    // Look up the best path
    int target = search.size()-1;
    if (search[target].cost == -numeric_limits<double>::max()) return;

    int source = search[target].source;
    while (true) {
        best_path.push_back(sentence.substr(source+1, target-source));
        if (source == -1) break;
        target = source;
        source = search[target].source;
    }

    if (reverse) std::reverse(best_path.begin(), best_path.end());
}


void forward_backward(const map<string, double> &vocab,
                         int maxlen,
                         const string &sentence,
                         map<string, double> &stats)
{
    if (sentence.length() == 0) return;
    vector<Token> search(sentence.length());
    int start_pos = 0;
    int end_pos = 0;
    int len = 0;

    for (int i=0; i<sentence.length(); i++) {

        // Iterate all factors ending in this position
        for (int j=max(0, i-maxlen); j<=i; j++) {

            start_pos = j;
            end_pos = i+1;
            len = end_pos-start_pos;

            if (vocab.find(sentence.substr(start_pos, len)) != vocab.end()) {
                double cost = vocab.at(sentence.substr(start_pos, len));
                if (j-1 >= 0) {
                    if (search[j-1].cost == -numeric_limits<double>::max()) break;
                    cost += search[j-1].cost;
                }
                if (cost > search[i].cost) {
                    search[i].cost = cost;
                    search[i].source = j-1;
                }
            }
        }
    }

}
