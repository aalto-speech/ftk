#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <queue>

using namespace std;

#include "FactorEncoder.hh"


class Token {
    public:
        int source;
        flt_type cost;
        Token(): source(-1), cost(-numeric_limits<flt_type>::max()) {};
        Token(int src, flt_type cst): source(src), cost(cst) {};
        Token(const Token& orig) { this->source=orig.source; this->cost=orig.cost; };
};


int read_vocab(string fname,
               map<string, flt_type> &vocab,
               int &maxlen)
{
    ifstream vocabfile(fname);
    if (!vocabfile) return -1;

    string line;
    flt_type count;
    maxlen = -1;
    while (getline(vocabfile, line)) {
        stringstream ss(line);
        string word;
        ss >> count;
        ss >> word;
        vocab[word] = count;
        maxlen = max(maxlen, int(word.length()));
    }
    vocabfile.close();

    return vocab.size();
}


int write_vocab(string fname,
                const map<string, flt_type> &vocab)
{
    ofstream vocabfile(fname);
    if (!vocabfile) return -1;

    vector<pair<string, flt_type> > sorted_vocab;
    sort_vocab(vocab, sorted_vocab);
    for (unsigned int i=0; i<sorted_vocab.size(); i++)
        vocabfile << sorted_vocab[i].second << " " << sorted_vocab[i].first << endl;
    vocabfile.close();

    return vocab.size();
}


bool descending_sort(pair<string, flt_type> i,pair<string, flt_type> j) { return (i.second > j.second); }
bool ascending_sort(pair<string, flt_type> i,pair<string, flt_type> j) { return (i.second < j.second); }

void sort_vocab(const map<string, flt_type> &vocab,
                vector<pair<string, flt_type> > &sorted_vocab,
                bool descending)
{
    sorted_vocab.clear();
    for (auto it = vocab.cbegin(); it != vocab.cend(); it++) {
        pair<string, flt_type> curr_pair(it->first, it->second);
        sorted_vocab.push_back(curr_pair);
    }
    if (descending)
        sort(sorted_vocab.begin(), sorted_vocab.end(), descending_sort);
    else
        sort(sorted_vocab.begin(), sorted_vocab.end(), ascending_sort);
}


void viterbi(const map<string, flt_type> &vocab,
             int maxlen,
             const string &text,
             vector<string> &best_path,
             bool reverse)
{
    best_path.clear();
    if (text.length() == 0) return;
    vector<Token> search(text.length());
    int start_pos = 0;
    int end_pos = 0;
    int len = 0;

    for (int i=0; i<text.length(); i++) {

        // Iterate all factors ending in this position
        for (int j=max(0, i-maxlen); j<=i; j++) {

            start_pos = j;
            end_pos = i+1;
            len = end_pos-start_pos;

            if (vocab.find(text.substr(start_pos, len)) != vocab.end()) {
                flt_type cost = vocab.at(text.substr(start_pos, len));
                if (j-1 >= 0) {
                    if (search[j-1].cost == -numeric_limits<flt_type>::max()) break;
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
    if (search[target].cost == -numeric_limits<flt_type>::max()) return;

    int source = search[target].source;
    while (true) {
        best_path.push_back(text.substr(source+1, target-source));
        if (source == -1) break;
        target = source;
        source = search[target].source;
    }

    if (reverse) std::reverse(best_path.begin(), best_path.end());
}


void viterbi(const StringSet<flt_type> &vocab,
             const string &text,
             vector<string> &best_path,
             bool reverse)
{
    best_path.clear();
    if (text.length() == 0) return;
    vector<Token> search(text.length());

    for (int i=0; i<text.length(); i++) {

        // Iterate all factors starting from this position
        const StringSet<flt_type>::Node *node = &vocab.root_node;
        for (int j=i; j<text.length(); j++) {

            StringSet<flt_type>::Arc *arc = vocab.find_arc(text[j], node);

            if (arc == NULL) break;
            node = arc->target_node;

            // Morph associated with this node
            if (arc->morph.length() > 0) {
                flt_type cost = arc->cost;
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
    if (search[target].cost == -numeric_limits<flt_type>::max()) return;

    int source = search[target].source;
    while (true) {
        best_path.push_back(text.substr(source+1, target-source));
        if (source == -1) break;
        target = source;
        source = search[target].source;
    }

    if (reverse) std::reverse(best_path.begin(), best_path.end());
}


void viterbi(const StringSet<flt_type> &vocab,
             const string &text,
             map<string, flt_type> &stats)
{
    if (text.length() == 0) return;
    vector<Token> search(text.length());

    for (int i=0; i<text.length(); i++) {

        // Iterate all factors starting from this position
        const StringSet<flt_type>::Node *node = &vocab.root_node;
        for (int j=i; j<text.length(); j++) {

            StringSet<flt_type>::Arc *arc = vocab.find_arc(text[j], node);

            if (arc == NULL) break;
            node = arc->target_node;

            // Morph associated with this node
            if (arc->morph.length() > 0) {
                flt_type cost = arc->cost;
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
    if (search[target].cost == -numeric_limits<flt_type>::max()) return;

    int source = search[target].source;
    while (true) {
        stats[text.substr(source+1, target-source)] += 1;
        if (source == -1) break;
        target = source;
        source = search[target].source;
    }
}


flt_type add_log_domain_probs(flt_type a, flt_type b) {

    if (b>a) {
        flt_type tmp = b;
        b = a;
        a = tmp;
    }

    return a + log(1 + exp(b - a));
}


void forward_backward(const StringSet<flt_type> &vocab,
                      const string &text,
                      map<string, flt_type> &stats)
{
    int len = text.length();
    if (len == 0) return;

    stats.clear();
    vector<vector<Token> > search(len);
    vector<flt_type> normalizers(len); // Normalizers (total forward score) for each position
    vector<flt_type> bw(len); // Backward propagated scores

    // Forward pass
    for (int i=0; i<len; i++) {

        if (i>0 && search[i-1].size() == 0) continue;

        if (i>0) {
            normalizers[i-1] = search[i-1][0].cost;
            for (unsigned int t=1; t<search[i-1].size(); t++)
                normalizers[i-1] = add_log_domain_probs(normalizers[i-1], search[i-1][t].cost);
        }

        // Iterate all factors starting from this position
        const StringSet<flt_type>::Node *node = &vocab.root_node;
        for (int j=i; j<text.length(); j++) {

            StringSet<flt_type>::Arc *arc = vocab.find_arc(text[j], node);

            if (arc == NULL) break;
            node = arc->target_node;

            // Morph associated with this node
            if (arc->morph.length() > 0) {
                flt_type cost = arc->cost;
                if (i>0) cost += normalizers[i-1];

                Token tok;
                tok.cost = cost;
                tok.source = i-1;
                search[j].push_back(tok);
            }
        }
    }

    if (search[len-1].size() == 0) return;

    normalizers[len-1] = search[len-1][0].cost;
    for (int j=1; j<search[len-1].size(); j++)
        normalizers[len-1] = add_log_domain_probs(normalizers[len-1], search[len-1][j].cost);

    // Backward
    for (int i=len-1; i>=0; i--) {
        for (int toki=0; toki<search[i].size(); toki++) {
            Token tok = search[i][toki];
            flt_type normalized = tok.cost - normalizers[i] + bw[i];
            stats[text.substr(tok.source+1, i-tok.source)] += exp(normalized);
            if (tok.source == -1) continue;
            if (bw[tok.source] != 0.0) bw[tok.source] = add_log_domain_probs(bw[tok.source], normalized);
            else bw[tok.source] = normalized;
        }
    }
}


void forward_backward(const map<string, flt_type> &vocab,
                      const string &text,
                      map<string, flt_type> &stats)
{
    StringSet<flt_type> stringset_vocab(vocab);
    forward_backward(stringset_vocab, text, stats);
}


void viterbi(const map<pair<string,string>, flt_type> &transitions,
             int maxlen,
             const string &start_end_symbol,
             const string &text,
             vector<string> &best_path,
             bool reverse)
{
    int len = text.length();
    if (len == 0) return;

    vector<vector<Token> > search(len);

    for (int i=0; i<len; i++) {

        if (i>0 && search[i-1].size() == 0) continue;

        // Iterate all factors starting from this position
        for (int j=i; j<text.length(); j++) {

        }
    }

    if (search[len-1].size() == 0) return;

}
