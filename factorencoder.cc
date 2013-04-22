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


void viterbi(MorphSet &vocab,
             const string &sentence,
             vector<string> &best_path,
             bool reverse)
{
    if (sentence.length() == 0) return;
    vector<Token> search(sentence.length());

    for (int i=0; i<sentence.length(); i++) {

        // Iterate all factors starting from this position
        MorphSet::Node *node = &vocab.root_node;
        for (int j=i; j<sentence.length(); j++) {

            MorphSet::Arc *arc = vocab.find_arc(sentence[j], node);

            if (arc == NULL) break;
            node = arc->target_node;

            // Morph associated with this node
            if (arc->morph.length() > 0) {
                double cost = arc->cost;
                if (i>0) cost += search[i-1].cost;

                if (cost > search[j].cost) {
                    search[j].cost = cost;
                    search[j].source = i-1;
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


double add_log_domain_probs(double a, double b) {

    if (b>a) {
        double tmp = b;
        b = a;
        a = tmp;
    }

    return a + log(1 + exp(b - a));
}


void forward_backward(MorphSet &vocab,
                      const string &sentence,
                      map<string, double> &stats)
{
    if (sentence.length() == 0) return;
    vector<vector<Token> > search(sentence.length());
    double len = sentence.length();

    // Forward pass
    for (int i=0; i<len; i++) {

        if (i>0 && search[i-1].size() == 0) continue;

        // Iterate all factors starting from this position
        MorphSet::Node *node = &vocab.root_node;
        for (int j=i; j<sentence.length(); j++) {

            MorphSet::Arc *arc = vocab.find_arc(sentence[j], node);

            if (arc == NULL) break;
            node = arc->target_node;

            // Morph associated with this node
            if (arc->morph.length() > 0) {
                double cost = arc->cost;
//                if (i>0) cost += search[i-1].cost;

                Token tok;
                tok.cost = cost;
                tok.source = i-1;
                search[j].push_back(tok);
            }
        }
    }

    if (search[len-1].size() == 0) return;

    //cout << endl << "starting backward pass" << endl;

    vector<double> normalizers;
    normalizers.resize(len);
    for (int i=0; i<len; i++)
        for (int j=0; j<search[i].size(); j++)
            normalizers[i] += search[i][j].cost;

    // Backward
    for (int i=(len-1); i>=0; i--) {
        for (int toki=0; toki<search[i].size(); toki++) {
            Token tok = search[i][toki];
            //cout << "tok source: " << tok.source << endl;
            double normalized = tok.cost - normalizers[i];
            //cout << "current substr: " << sentence.substr(tok.source+1, i+1) << endl;
            //cout << "tok cost: " << tok.cost << endl;
            //cout << "normalizer: " << normalizers[i] << endl;
            stats[sentence.substr(tok.source+1, i+1)] += exp(normalized);
            if (tok.source == -1) break;
            //normalizers[tok.source] = add_log_domain_probs(normalizers[tok.source], normalized);
        }
    }
}


void forward_backward(const map<string, double> &vocab,
                      const string &sentence,
                      map<string, double> &stats)
{
    MorphSet morphset_vocab(vocab);
    forward_backward(morphset_vocab, sentence, stats);
}
