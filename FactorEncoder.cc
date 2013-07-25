#include <algorithm>
#include <cstdlib>
#include <set>
#include <stdexcept>
#include <queue>

#include "FactorEncoder.hh"

using namespace std;


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


flt_type viterbi(const StringSet &vocab,
                 const string &text,
                 vector<string> &best_path,
                 bool reverse)
{
    best_path.clear();
    if (text.length() == 0) return MIN_FLOAT;
    vector<Token> search(text.length());

    for (int i=0; i<text.length(); i++) {

        // Iterate all factors starting from this position
        const StringSet::Node *node = &vocab.root_node;
        for (int j=i; j<text.length(); j++) {

            StringSet::Arc *arc = vocab.find_arc(text[j], node);

            if (arc == NULL) break;
            node = arc->target_node;

            // Morph associated with this node
            if (arc->factor.length() > 0) {
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
    return search.back().cost;
}


flt_type viterbi(const StringSet &vocab,
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


void forward(const StringSet &vocab,
             const string &text,
             vector<vector<Token> > &search,
             vector<flt_type> &fw)
{
    int len = text.length();
    for (int i=0; i<len; i++) {

        if (i>0 && search[i-1].size() == 0) continue;

        if (i>0) {
            fw[i-1] = search[i-1][0].cost;
            for (unsigned int t=1; t<search[i-1].size(); t++)
                fw[i-1] = add_log_domain_probs(fw[i-1], search[i-1][t].cost);
        }

        // Iterate all factors starting from this position
        const StringSet::Node *node = &vocab.root_node;
        for (int j=i; j<text.length(); j++) {

            StringSet::Arc *arc = vocab.find_arc(text[j], node);

            if (arc == NULL) break;
            node = arc->target_node;

            // Morph associated with this node
            if (arc->factor.length() > 0) {
                flt_type cost = arc->cost;
                if (i>0) cost += fw[i-1];

                Token tok;
                tok.cost = cost;
                tok.source = i-1;
                search[j].push_back(tok);
            }
        }
    }

    if (search[len-1].size() == 0) return;

    fw[len-1] = search[len-1][0].cost;
    for (int j=1; j<search[len-1].size(); j++)
        fw[len-1] = add_log_domain_probs(fw[len-1], search[len-1][j].cost);
}

void backward(const StringSet &vocab,
              const string &text,
              const vector<vector<Token> > &search,
              const vector<flt_type> &fw,
              vector<flt_type> &bw,
              map<string, flt_type> &stats)
{
    int len = text.length();
    if (search[len-1].size() == 0) return;

    // Backward
    for (int i=len-1; i>=0; i--) {
        for (int toki=0; toki<search[i].size(); toki++) {
            Token tok = search[i][toki];
            flt_type normalized = tok.cost - fw[i] + bw[i];
            stats[text.substr(tok.source+1, i-tok.source)] += exp(normalized);
            if (tok.source == -1) continue;
            if (bw[tok.source] != 0.0) bw[tok.source] = add_log_domain_probs(bw[tok.source], normalized);
            else bw[tok.source] = normalized;
        }
    }
}


flt_type forward_backward(const StringSet &vocab,
                          const string &text,
                          map<string, flt_type> &stats)
{
    int len = text.length();
    if (len == 0) return MIN_FLOAT;

    stats.clear();
    vector<vector<Token> > search(len);
    vector<flt_type> fw(len);
    vector<flt_type> bw(len);

    forward(vocab, text, search, fw);
    backward(vocab, text, search, fw, bw, stats);

    if (search[len-1].size() == 0) return MIN_FLOAT;
    return fw.back();
}


flt_type forward_backward(const StringSet &vocab,
                          const string &text,
                          map<string, flt_type> &stats,
                          vector<flt_type> &bw)
{
    int len = text.length();
    if (len == 0) return MIN_FLOAT;

    stats.clear();
    vector<vector<Token> > search(len);
    vector<flt_type> fw(len);
    bw.resize(len);

    forward(vocab, text, search, fw);
    backward(vocab, text, search, fw, bw, stats);

    if (search[len-1].size() == 0) return MIN_FLOAT;
    return fw.back();
}


flt_type forward_backward(const map<string, flt_type> &vocab,
                          const string &text,
                          map<string, flt_type> &stats)
{
    StringSet stringset_vocab(vocab);
    return forward_backward(stringset_vocab, text, stats);
}


flt_type viterbi(const transitions_t &transitions,
                 FactorGraph &text,
                 vector<string> &best_path,
                 bool reverse)
{
    if (text.nodes.size() == 0) return MIN_FLOAT;
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

    return costs.back();
}


flt_type viterbi(const transitions_t &transitions,
                 FactorGraph &text,
                 transitions_t &stats)
{
    stats.clear();
    vector<string> best_path;
    flt_type lp = viterbi(transitions, text, best_path, true);
    if (best_path.size() < 2) return MIN_FLOAT;
    for (int i=1; i<best_path.size(); i++)
        stats[best_path[i-1]][best_path[i]] += 1.0;
    return lp;
}


void forward(const transitions_t &transitions,
             FactorGraph &text,
             std::vector<flt_type> &fw)
{
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
}


void backward(const FactorGraph &text,
              const vector<flt_type> &fw,
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


flt_type forward_backward(const transitions_t &transitions,
                          FactorGraph &text,
                          transitions_t &stats)
{
    if (text.nodes.size() == 0) return MIN_FLOAT;

    vector<flt_type> fw(text.nodes.size(), MIN_FLOAT);
    vector<flt_type> bw(text.nodes.size(), MIN_FLOAT);
    fw[0] = 0.0; bw[text.nodes.size()-1] = 0.0;

    forward(transitions, text, fw);
    backward(text, fw, bw, stats);

    return fw.back();
}


flt_type forward_backward(const transitions_t &transitions,
                          FactorGraph &text,
                          transitions_t &stats,
                          vector<flt_type> &post_scores)
{
    if (text.nodes.size() == 0) return MIN_FLOAT;
    stats.clear();

    vector<flt_type> fw(text.nodes.size(), MIN_FLOAT);
    vector<flt_type> bw(text.nodes.size(), MIN_FLOAT);
    fw[0] = 0.0; bw[text.nodes.size()-1] = 0.0;

    forward(transitions, text, fw);
    backward(text, fw, bw, stats);

    post_scores.clear();
    post_scores.resize(text.text.size());
    for (int i=1; i<text.nodes.size()-1; i++) {
        int idx = int(text.nodes[i].start_pos) + int(text.nodes[i].len) - 1;
        if (post_scores[idx] == 0.0) post_scores[idx] = bw[i];
        else post_scores[idx] = add_log_domain_probs(post_scores[idx], bw[i]);
    }

    return fw.back();
}


flt_type forward_backward(const transitions_t &transitions,
                          FactorGraph &text,
                          transitions_t &stats,
                          const string &block)
{
    if (text.nodes.size() == 0) return MIN_FLOAT;
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

    return fw.back();
}


flt_type forward_backward(const map<string, flt_type> &vocab,
                          FactorGraph &text,
                          transitions_t &stats)
{
    if (text.nodes.size() == 0) return MIN_FLOAT;
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

    return fw.back();
}


flt_type posterior_decode(const transitions_t &transitions,
                          FactorGraph &text,
                          vector<string> &path,
                          bool reverse)
{
    if (text.nodes.size() == 0) return MIN_FLOAT;
    path.clear();

    transitions_t stats;
    vector<flt_type> fw(text.nodes.size(), MIN_FLOAT);
    vector<flt_type> bw(text.nodes.size(), MIN_FLOAT);
    fw[0] = 0.0; bw[text.nodes.size()-1] = 0.0;

    forward(transitions, text, fw);
    backward(text, fw, bw, stats);

    // Initialize node scores
    vector<flt_type> costs(text.nodes.size(), MIN_FLOAT);
    vector<int> source_nodes(text.nodes.size());
    costs[0] = 0.0; source_nodes[0] = -1;

    // Traverse paths
    for (int i=0; i<text.nodes.size(); i++) {

        FactorGraph::Node &node = text.nodes[i];
        for (auto arc = node.outgoing.begin(); arc != node.outgoing.end(); ++arc) {

            int tgt_node = (**arc).target_node;
            flt_type curr_cost = costs[tgt_node];
            flt_type new_cost = costs[i] + bw[tgt_node];
            if (new_cost > curr_cost) {
                costs[tgt_node] = new_cost;
                source_nodes[tgt_node] = i;
            }
        }
    }

    // Find best path
    unsigned int node = text.nodes.size()-1;
    path.push_back(text.get_factor(node));
    while (true) {
        node = source_nodes[node];
        if (node == -1) break;
        path.push_back(text.get_factor(node));
    }
    if (reverse) std::reverse(path.begin(), path.end());

    return costs.back();
}


void
assign_scores(transitions_t &transitions,
              MultiStringFactorGraph &msfg)
{
    for (auto it = msfg.factor_node_map.begin(); it != msfg.factor_node_map.end(); ++it) {

        std::map<std::string, flt_type> &curr_transitions = transitions.at(it->first);

        for (auto ndit = it->second.begin(); ndit != it->second.end(); ++ndit) {
            MultiStringFactorGraph::Node &node = msfg.nodes[*ndit];
            for (auto arc = node.outgoing.begin(); arc != node.outgoing.end(); ++arc)
                (**arc).cost = &curr_transitions.at(msfg.nodes[(**arc).target_node].factor);
        }
    }
}


void
assign_scores(map<string, flt_type> &vocab,
              MultiStringFactorGraph &msfg)
{
    for (auto it = msfg.factor_node_map.begin(); it != msfg.factor_node_map.end(); ++it) {

        flt_type *curr_score = &(vocab.at(it->first));

        for (auto ndit = it->second.begin(); ndit != it->second.end(); ++ndit) {
            MultiStringFactorGraph::Node &node = msfg.nodes[*ndit];
            for (auto arc = node.incoming.begin(); arc != node.incoming.end(); ++arc)
                (**arc).cost = curr_score;
        }
    }
}


void
forward(MultiStringFactorGraph &msfg,
        std::vector<flt_type> &fw)
{
    for (int i=0; i<msfg.nodes.size(); i++) {

        if (fw[i] == MIN_FLOAT) continue;

        MultiStringFactorGraph::Node &node = msfg.nodes[i];
        for (auto arc = node.outgoing.begin(); arc != node.outgoing.end(); ++arc) {
            int tgt_node = (**arc).target_node;
            flt_type cost = fw[i] + *(**arc).cost;
            if (fw[tgt_node] == MIN_FLOAT) fw[tgt_node] = cost;
            else fw[tgt_node] = add_log_domain_probs(fw[tgt_node], cost);
        }
    }
}


flt_type
forward(const std::string &text,
        MultiStringFactorGraph &msfg,
        std::map<msfg_node_idx_t, flt_type> &fw)
{
    map<msfg_node_idx_t, vector<MultiStringFactorGraph::Arc*> > arcs;
    msfg.collect_arcs(text, arcs);

    for (auto ndit = arcs.begin(); ndit != arcs.end(); ++ndit) {

        MultiStringFactorGraph::Node &node = msfg.nodes[ndit->first];
        for (auto arc = node.outgoing.begin(); arc != node.outgoing.end(); ++arc) {
            int tgt_node = (**arc).target_node;
            flt_type cost = fw[ndit->first] + *(**arc).cost;
            if (fw.find(tgt_node) == fw.end()) fw[tgt_node] = cost;
            else fw[tgt_node] = add_log_domain_probs(fw[tgt_node], cost);
        }
    }

    return fw.at(msfg.string_end_nodes.at(text));
}


flt_type
forward(const map<string, flt_type> &words,
        MultiStringFactorGraph &msfg,
        const std::set<std::string> &words_to_fb,
        bool full_forward_pass)
{
    flt_type total_lp = 0.0;

    if (full_forward_pass) {
        std::vector<flt_type> fw;
        forward(msfg, fw);
        for (auto wit = words_to_fb.cbegin(); wit != words_to_fb.cend(); ++wit)
            total_lp += words.at(*wit) * fw.at(msfg.string_end_nodes.at(*wit));
    }
    else {
        for (auto wit = words_to_fb.cbegin(); wit != words_to_fb.cend(); ++wit) {
            std::map<msfg_node_idx_t, flt_type> fw;
            total_lp += words.at(*wit) * forward(*wit, msfg, fw);
        }
    }

    return total_lp;
}


flt_type
backward(const MultiStringFactorGraph &msfg,
         const string &text,
         const vector<flt_type> &fw,
         transitions_t &stats,
         flt_type text_weight)
{

    int text_end_node = msfg.string_end_nodes.at(text);
    map<int, flt_type> bw; bw[text_end_node] = 0.0;
    set<int> nodes_to_process; nodes_to_process.insert(text_end_node);

    while(nodes_to_process.size() > 0) {

        int i = *(nodes_to_process.rbegin());

        const MultiStringFactorGraph::Node &node = msfg.nodes[i];

        for (auto arc = node.incoming.begin(); arc != node.incoming.end(); ++arc) {
            int src_node = (**arc).source_node;
            if (fw[src_node] == MIN_FLOAT) continue;
            flt_type curr_cost = *(**arc).cost + fw[src_node] - fw[i] + bw[i];
            stats[msfg.nodes.at(src_node).factor][node.factor] += text_weight * exp(curr_cost);
            if (bw.find(src_node) == bw.end()) {
                bw[src_node] = curr_cost;
                nodes_to_process.insert(src_node);
            }
            else bw[src_node] = add_log_domain_probs(bw[src_node], curr_cost);
        }

        nodes_to_process.erase(i);
    }

    return fw.at(msfg.string_end_nodes.at(text));
}


flt_type
backward(const MultiStringFactorGraph &msfg,
         const string &text,
         const map<msfg_node_idx_t, flt_type> &fw,
         transitions_t &stats,
         flt_type text_weight)
{

    int text_end_node = msfg.string_end_nodes.at(text);
    map<int, flt_type> bw; bw[text_end_node] = 0.0;
    set<int> nodes_to_process; nodes_to_process.insert(text_end_node);

    while(nodes_to_process.size() > 0) {

        int i = *(nodes_to_process.rbegin());

        const MultiStringFactorGraph::Node &node = msfg.nodes[i];

        for (auto arc = node.incoming.begin(); arc != node.incoming.end(); ++arc) {
            int src_node = (**arc).source_node;
            flt_type curr_cost = *(**arc).cost + fw.at(src_node) - fw.at(i) + bw[i];
            stats[msfg.nodes.at(src_node).factor][node.factor] += text_weight * exp(curr_cost);
            if (bw.find(src_node) == bw.end()) {
                bw[src_node] = curr_cost;
                nodes_to_process.insert(src_node);
            }
            else bw[src_node] = add_log_domain_probs(bw[src_node], curr_cost);
        }

        nodes_to_process.erase(i);
    }

    return fw.at(msfg.string_end_nodes.at(text));
}



flt_type
forward_backward(MultiStringFactorGraph &msfg,
                 transitions_t &stats,
                 map<string, flt_type> &word_freqs)
{
    if (msfg.nodes.size() == 0) return MIN_FLOAT;

    vector<flt_type> fw(msfg.nodes.size(), MIN_FLOAT);
    fw[0] = 0.0;

    forward(msfg, fw);
    flt_type total_lp = 0.0;
    for (auto it = msfg.string_end_nodes.begin(); it != msfg.string_end_nodes.end(); ++it) {
        flt_type lp = backward(msfg, it->first, fw, stats);
        total_lp += word_freqs[it->first] * lp;
    }

    return total_lp;
}


flt_type
forward_backward(MultiStringFactorGraph &msfg,
                 const string &text,
                 transitions_t &stats)
{
    if (msfg.nodes.size() == 0) return MIN_FLOAT;

    map<msfg_node_idx_t, flt_type> fw;
    fw[0] = 0.0;

    forward(text, msfg, fw);
    backward(msfg, text, fw, stats);

    return fw[msfg.string_end_nodes[text]];
}
