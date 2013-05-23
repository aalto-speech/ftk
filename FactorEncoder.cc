#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <queue>

#include "FactorEncoder.hh"

using namespace std;

flt_type SMALL_LP = -200.0;
flt_type MIN_FLOAT = -std::numeric_limits<flt_type>::max();


void
FactorGraph::create_nodes(const string &text, const map<string, flt_type> &vocab,
                          int maxlen, vector<unordered_set<unsigned int> > &incoming)
{
    nodes.push_back(Node(0,0));
    for (unsigned int i=0; i<text.length(); i++) {
        if (incoming[i].size() == 0) continue;
        for (unsigned int j=i+1; j<=text.size(); j++) {
            unsigned int len = j-i;
            if (len>maxlen) break;
            if (vocab.find(text.substr(i, len)) != vocab.end()) {
                nodes.push_back(Node(i, len));
                incoming[j].insert(i);
            }
        }
    }
}


void
FactorGraph::create_nodes(const string &text, const StringSet<flt_type> &vocab,
                          vector<unordered_set<unsigned int> > &incoming)
{
    nodes.push_back(Node(0,0));
    for (unsigned int i=0; i<text.length(); i++) {
        if (incoming[i].size() == 0) continue;

        const StringSet<flt_type>::Node *node = &vocab.root_node;
        for (unsigned int j=i; j<text.length(); j++) {

            StringSet<flt_type>::Arc *arc = vocab.find_arc(text[j], node);

            if (arc == NULL) break;
            node = arc->target_node;

            // String associated with this node
            if (arc->morph.length() > 0) {
                nodes.push_back(Node(i, j+1-i));
                incoming[j+1].insert(i);
            }
        }
    }
}


void
FactorGraph::prune_and_create_arcs(vector<unordered_set<unsigned int> > &incoming)
{
    // Find all possible node start positions
    unordered_set<int> possible_node_starts;
    possible_node_starts.insert(text.size());
    for (int i=incoming.size()-1; i>= 0; i--) {
        if (possible_node_starts.find(i) == possible_node_starts.end()) continue;
        for (auto it = incoming[i].cbegin(); it != incoming[i].cend(); ++it)
            possible_node_starts.insert(*it);
    }

    // Prune non-reachable nodes
    for (auto it = nodes.begin(); it != nodes.end(); ) {
        if (possible_node_starts.find(it->start_pos) == possible_node_starts.end() ||
            possible_node_starts.find(it->start_pos+it->len) == possible_node_starts.end())
            it = nodes.erase(it);
        else
            ++it;
    }

    // Add end node
    nodes.push_back(Node(text.size(),0));

    // Collect nodes by start position
    vector<vector<int> > nodes_by_start_pos(text.size()+1);
    for (int i=1; i<nodes.size(); i++)
        nodes_by_start_pos[nodes[i].start_pos].push_back(i);

    // Set arcs
    for (int i=0; i<nodes.size()-1; i++) {
        int end_pos = nodes[i].start_pos + nodes[i].len;
        for (int j=0; j<nodes_by_start_pos[end_pos].size(); j++) {
            int nodei = nodes_by_start_pos[end_pos][j];
            Arc *arc = new Arc(i, nodei, 0.0);
            arcs.push_back(arc);
            nodes[i].outgoing.push_back(arc);
            nodes[nodei].incoming.push_back(arc);
        }
    }
}

void
FactorGraph::set_text(const string &text,
                      const string &start_end_symbol,
                      const map<string, flt_type> &vocab,
                      int maxlen)
{
    this->text.assign(text);
    this->start_end_symbol.assign(start_end_symbol);
    if (text.length() == 0) return;

    vector<unordered_set<unsigned int> > incoming(text.size()+1); // (pos in text, source pos)

    // Create all nodes
    incoming[0].insert(0);
    create_nodes(text, vocab, maxlen, incoming);

    // No possible segmentations
    if (incoming[text.size()].size() == 0) {
        nodes.clear();
        return;
    }

    prune_and_create_arcs(incoming);
}


void
FactorGraph::set_text(const string &text,
                      const string &start_end_symbol,
                      const StringSet<flt_type> &vocab)
{
    this->text.assign(text);
    this->start_end_symbol.assign(start_end_symbol);
    if (text.length() == 0) return;

    vector<unordered_set<unsigned int> > incoming(text.size()+1); // (pos in text, source pos)

    // Create all nodes
    incoming[0].insert(0);
    create_nodes(text, vocab, incoming);

    // No possible segmentations
    if (incoming[text.size()].size() == 0) {
        nodes.clear();
        return;
    }

    prune_and_create_arcs(incoming);
}


FactorGraph::FactorGraph(const string &text,
                         const string &start_end_symbol,
                         const map<string, flt_type> &vocab,
                         int maxlen)
{
    set_text(text, start_end_symbol, vocab, maxlen);
}


FactorGraph::FactorGraph(const string &text,
                         const string &start_end_symbol,
                         const StringSet<flt_type> &vocab)
{
    set_text(text, start_end_symbol, vocab);
}


bool
FactorGraph::assert_equal(const FactorGraph &other) const
{
    if (nodes.size() != other.nodes.size()) return false;

    auto it = nodes.begin();
    auto it2 = other.nodes.begin();
    for (; it != nodes.end(); ) {
        if (it->start_pos != it2->start_pos) return false;
        if (it->len != it2->len) return false;
        if (it->incoming.size() != it2->incoming.size()) return false;
        if (it->outgoing.size() != it2->outgoing.size()) return false;
        for (int i=0; i<it->incoming.size(); i++)
            if (*(it->incoming[i]) != *(it2->incoming[i])) return false;
        for (int i=0; i<it->outgoing.size(); i++)
            if (*(it->outgoing[i]) != *(it2->outgoing[i])) return false;
        it++;
        it2++;
    }

    return true;
}


int
FactorGraph::num_paths() const
{

    if (nodes.size() == 0) return 0;
    vector<int> path_counts(nodes.size());
    path_counts[0] = 1;

    for (int i=0; i<nodes.size(); i++) {
        const FactorGraph::Node &node = nodes[i];
        for (auto arc = node.outgoing.begin(); arc != node.outgoing.end(); ++arc)
            path_counts[(**arc).target_node] += path_counts[i];
    }

    return path_counts.back();
}


void
FactorGraph::get_paths(vector<vector<string> > &paths) const
{

    vector<string> curr_string;
    advance(paths, curr_string, 0);
}


void
FactorGraph::advance(vector<vector<string> > &paths,
                     vector<string> &curr_string,
                     unsigned int node_idx) const
{
    const FactorGraph::Node &node = nodes[node_idx];

    std::string tmp;
    get_factor(node, tmp);
    curr_string.push_back(tmp);

    if (node_idx == nodes.size()-1) {
        paths.push_back(curr_string);
        return;
    }

    for (auto arc  = node.outgoing.begin(); arc != node.outgoing.end(); ++arc) {
        vector<string> curr_copy(curr_string);
        advance(paths, curr_copy, (**arc).target_node);
    }
}


void
FactorGraph::remove_arc(Arc *arc)
{
    auto ait = find(arcs.begin(), arcs.end(), arc);
    arcs.erase(ait);

    Node &src_node = nodes[(*arc).source_node];
    auto sit = find(src_node.outgoing.begin(), src_node.outgoing.end(), arc);
    src_node.outgoing.erase(sit);

    Node &tgt_node = nodes[(*arc).target_node];
    auto tit = find(tgt_node.incoming.begin(), tgt_node.incoming.end(), arc);
    tgt_node.incoming.erase(tit);

    delete arc;
}


void
FactorGraph::remove_arcs(const std::string &source,
                         const std::string &target)
{
    for (auto node = nodes.begin(); node != nodes.end(); ++node) {
        if (source != this->get_factor(*node)) continue;
        for (int i=0; i<node->outgoing.size(); i++) {
            FactorGraph::Arc *arc = node->outgoing[i];
            if (target != this->get_factor(arc->target_node)) continue;
            this->remove_arc(arc);
            break;
        }
    }

    // Prune all transitions from non-reachable nodes
    for (auto it = nodes.begin(); it != nodes.end(); it++) {
        if (it->incoming.size() == 0 && it->len > 0) {
            while (it->outgoing.size() > 0)
                remove_arc(it->outgoing[0]);
            continue;
        }
        if (it->outgoing.size() == 0 && it->len > 0) {
            while (it->incoming.size() > 0)
                remove_arc(it->incoming[0]);
        }
    }
}


void
FactorGraph::remove_arcs(const std::string &remstr)
{
    for (auto node = nodes.begin(); node != nodes.end(); ++node) {
        if (this->get_factor(*node) != remstr) continue;
        while (node->incoming.size() > 0)
            remove_arc(node->incoming[0]);
        while (node->outgoing.size() > 0)
            remove_arc(node->outgoing[0]);
    }

    // Prune all transitions from non-reachable nodes
    for (auto it = nodes.begin(); it != nodes.end(); it++) {
        if (it->incoming.size() == 0 && it->len > 0) {
            while (it->outgoing.size() > 0)
                remove_arc(it->outgoing[0]);
            continue;
        }
        if (it->outgoing.size() == 0 && it->len > 0) {
            while (it->incoming.size() > 0)
                remove_arc(it->incoming[0]);
        }
    }
}


FactorGraph::~FactorGraph() {
    for (auto it = arcs.begin(); it != arcs.end(); ++it)
        delete *it;
}


class Token {
    public:
        int source;
        flt_type cost;
        Token(): source(-1), cost(MIN_FLOAT) {};
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


flt_type viterbi(const map<string, flt_type> &vocab,
                 int maxlen,
                 const string &text,
                 vector<string> &best_path,
                 bool reverse)
{
    best_path.clear();
    if (text.length() == 0) return MIN_FLOAT;
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
                    if (search[j-1].cost == MIN_FLOAT) break;
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
    if (search[target].cost == MIN_FLOAT) return MIN_FLOAT;

    int source = search[target].source;
    while (true) {
        best_path.push_back(text.substr(source+1, target-source));
        if (source == -1) break;
        target = source;
        source = search[target].source;
    }

    if (reverse) std::reverse(best_path.begin(), best_path.end());
    return search.back().cost;
}


flt_type viterbi(const StringSet<flt_type> &vocab,
                 const string &text,
                 vector<string> &best_path,
                 bool reverse)
{
    best_path.clear();
    if (text.length() == 0) return MIN_FLOAT;
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
    if (search[target].cost == MIN_FLOAT) return MIN_FLOAT;

    int source = search[target].source;
    while (true) {
        best_path.push_back(text.substr(source+1, target-source));
        if (source == -1) break;
        target = source;
        source = search[target].source;
    }

    if (reverse) std::reverse(best_path.begin(), best_path.end());
    return MIN_FLOAT;
}


flt_type viterbi(const StringSet<flt_type> &vocab,
                 const string &text,
                 map<string, flt_type> &stats)
{
    stats.clear();
    vector<string> best_path;
    flt_type lp = viterbi(vocab, text, best_path, false);
    for (auto it = best_path.begin(); it != best_path.end(); ++it)
        stats[*it] += 1.0;
    return lp;
}


flt_type add_log_domain_probs(flt_type a, flt_type b) {

    if (b>a) {
        flt_type tmp = b;
        b = a;
        a = tmp;
    }

    return a + log(1 + exp(b - a));
}


flt_type forward_backward(const StringSet<flt_type> &vocab,
                          const string &text,
                          map<string, flt_type> &stats)
{
    int len = text.length();
    if (len == 0) return MIN_FLOAT;

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

    if (search[len-1].size() == 0) return MIN_FLOAT;

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

    return normalizers[len-1];
}


flt_type forward_backward(const map<string, flt_type> &vocab,
                          const string &text,
                          map<string, flt_type> &stats)
{
    StringSet<flt_type> stringset_vocab(vocab);
    return forward_backward(stringset_vocab, text, stats);
}


void viterbi(const transitions_t &transitions,
             FactorGraph &text,
             vector<string> &best_path,
             bool reverse)
{
    if (text.nodes.size() == 0) return;
    best_path.clear();

    // Initialize node scores
    vector<flt_type> costs(text.nodes.size(), MIN_FLOAT);
    vector<int> source_nodes(text.nodes.size());
    costs[0] = 0.0; source_nodes[0] = -1;

    // Traverse paths
    for (int i=0; i<text.nodes.size(); i++) {

        FactorGraph::Node &node = text.nodes[i];
        for (auto arc = node.outgoing.begin(); arc != node.outgoing.end(); ++arc) {

            int src_node = (**arc).source_node;
            int tgt_node = (**arc).target_node;

            try {
                (**arc).cost = transitions.at(text.get_factor(src_node)).at(text.get_factor(tgt_node));
            }
            catch (std::out_of_range &oor) {
                (**arc).cost = SMALL_LP;
            }

            flt_type curr_cost = costs[tgt_node];
            flt_type new_cost = costs[i] + (**arc).cost;
            if (new_cost > curr_cost) {
                costs[tgt_node] = new_cost;
                source_nodes[tgt_node] = i;
            }
        }
    }

    // Find best path
    unsigned int node = text.nodes.size()-1;
    best_path.push_back(text.get_factor(node));
    while (true) {
        node = source_nodes[node];
        if (node == -1) break;
        best_path.push_back(text.get_factor(node));
    }
    if (reverse) std::reverse(best_path.begin(), best_path.end());
}


void viterbi(const transitions_t &transitions,
             FactorGraph &text,
             transitions_t &stats)
{
    stats.clear();
    vector<string> best_path;
    viterbi(transitions, text, best_path, true);
    if (best_path.size() < 2) return;
    for (int i=1; i<best_path.size(); i++)
        stats[best_path[i-1]][best_path[i]] += 1.0;
}


void backward(const FactorGraph &text,
              vector<flt_type> &fw,
              vector<flt_type> &bw,
              transitions_t &stats)
{
    for (int i=text.nodes.size()-1; i>0; i--) {

        if (bw[i] == MIN_FLOAT) continue;

        const FactorGraph::Node &node = text.nodes[i];
        string target_node_str = text.get_factor(node);

        for (auto arc = node.incoming.begin(); arc != node.incoming.end(); ++arc) {
            int src_node = (**arc).source_node;
            if (fw[src_node] == MIN_FLOAT) continue;
            flt_type curr_cost = (**arc).cost + fw[src_node] - fw[i] + bw[i];
            stats[text.get_factor(src_node)][target_node_str] += exp(curr_cost);
            if (bw[src_node] == MIN_FLOAT) bw[src_node] = curr_cost;
            else bw[src_node] = add_log_domain_probs(bw[src_node], curr_cost);
        }
    }
}


void forward_backward(const transitions_t &transitions,
                      FactorGraph &text,
                      transitions_t &stats)
{
    if (text.nodes.size() == 0) return;
    stats.clear();

    vector<flt_type> fw(text.nodes.size(), MIN_FLOAT);
    vector<flt_type> bw(text.nodes.size(), MIN_FLOAT);
    fw[0] = 0.0; bw[text.nodes.size()-1] = 0.0;

    // Forward
    for (int i=0; i<text.nodes.size(); i++) {

        if (fw[i] == MIN_FLOAT) continue;

        FactorGraph::Node &node = text.nodes[i];
        for (auto arc = node.outgoing.begin(); arc != node.outgoing.end(); ++arc) {

            int tgt_node = (**arc).target_node;
            try {
                (**arc).cost = transitions.at(text.get_factor(node)).at(text.get_factor(tgt_node));
            }
            catch (std::out_of_range &oor) {
                (**arc).cost = SMALL_LP;
            }

            flt_type cost = fw[i] + (**arc).cost;
            if (fw[tgt_node] == MIN_FLOAT) fw[tgt_node] = cost;
            else fw[tgt_node] = add_log_domain_probs(fw[tgt_node], cost);
        }
    }

    backward(text, fw, bw, stats);
}


void forward_backward(const transitions_t &transitions,
                      FactorGraph &text,
                      transitions_t &stats,
                      const string &block)
{
    if (text.nodes.size() == 0) return;
    stats.clear();

    vector<flt_type> fw(text.nodes.size(), MIN_FLOAT);
    vector<flt_type> bw(text.nodes.size(), MIN_FLOAT);
    fw[0] = 0.0; bw[text.nodes.size()-1] = 0.0;

    string source_node_str, target_node_str;

    // Forward
    for (int i=0; i<text.nodes.size(); i++) {

        if (fw[i] == MIN_FLOAT) continue;

        FactorGraph::Node &node = text.nodes[i];
        source_node_str = text.get_factor(node);
        if (source_node_str == block) continue;

        for (auto arc = node.outgoing.begin(); arc != node.outgoing.end(); ++arc) {

            int tgt_node = (**arc).target_node;
            target_node_str = text.get_factor(tgt_node);
            if (target_node_str == block) continue;
            try {
                (**arc).cost = transitions.at(source_node_str).at(target_node_str);
            }
            catch (std::out_of_range &oor) {
                (**arc).cost = SMALL_LP;
            }

            flt_type cost = fw[i] + (**arc).cost;
            if (fw[tgt_node] == MIN_FLOAT) fw[tgt_node] = cost;
            else fw[tgt_node] = add_log_domain_probs(fw[tgt_node], cost);
        }
    }

    backward(text, fw, bw, stats);
}


void forward_backward(const map<string, flt_type> &vocab,
                      FactorGraph &text,
                      transitions_t &stats)
{
    if (text.nodes.size() == 0) return;
    stats.clear();

    vector<flt_type> fw(text.nodes.size(), MIN_FLOAT);
    vector<flt_type> bw(text.nodes.size(), MIN_FLOAT);
    fw[0] = 0.0; bw[text.nodes.size()-1] = 0.0;

    // Forward
    for (int i=0; i<text.nodes.size(); i++) {

        if (fw[i] == MIN_FLOAT) continue;

        FactorGraph::Node &node = text.nodes[i];
        for (auto arc = node.outgoing.begin(); arc != node.outgoing.end(); ++arc) {
            int tgt_node = (**arc).target_node;
            (**arc).cost = vocab.at(text.get_factor(tgt_node));
            flt_type cost = fw[i] + (**arc).cost;
            if (fw[tgt_node] == MIN_FLOAT) fw[tgt_node] = cost;
            else fw[tgt_node] = add_log_domain_probs(fw[tgt_node], cost);
        }
    }

    backward(text, fw, bw, stats);
}

