#include <algorithm>
#include <stdexcept>

#include "EM.hh"

using namespace std;


flt_type viterbi(const map<string, flt_type> &vocab,
                 unsigned int maxlen,
                 const string &text,
                 vector<string> &best_path,
                 bool reverse,
                 bool utf8)
{
    best_path.clear();
    if (text.length() == 0) return MIN_FLOAT;
    vector<Token> search(text.length());

    vector<unsigned int> char_positions;
    get_character_positions(text, char_positions, utf8);

    for (unsigned int i=0; i<char_positions.size(); i++) {

        // Iterate all factors starting from this position
        unsigned int start_pos = char_positions[i];
        for (unsigned int j=i; j<char_positions.size() && (j-i < maxlen); j++) {

            unsigned int end_pos = text.size();
            if (j < (char_positions.size()-1)) end_pos = char_positions[j+1];
            auto vit = vocab.find(text.substr(start_pos, end_pos-start_pos));
            if (vit == vocab.end()) continue;

            flt_type cost = vit->second;
            if (start_pos > 0) cost += search[start_pos-1].cost;

            if (cost > search[end_pos-1].cost) {
                search[end_pos-1].cost = cost;
                search[end_pos-1].source = start_pos-1;
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
                 bool reverse,
                 bool utf8)
{
    best_path.clear();
    if (text.length() == 0) return MIN_FLOAT;
    vector<Token> search(text.length());

    vector<unsigned int> char_positions;
    get_character_positions(text, char_positions, utf8);

    for (unsigned int i=0; i<char_positions.size(); i++) {

        // Iterate all factors starting from this position
        unsigned int start_pos = char_positions[i];
        const StringSet::Node *node = &vocab.root_node;
        for (unsigned int j=start_pos; j<text.length(); j++) {

            StringSet::Arc *arc = vocab.find_arc(text[j], node);
            if (arc == nullptr) break;
            node = arc->target_node;

            // Factor associated with this node
            if (arc->factor.length() > 0) {
                flt_type cost = arc->cost;
                if (start_pos>0) cost += search[start_pos-1].cost;

                if (cost > search[j].cost) {
                    search[j].cost = cost;
                    search[j].source = start_pos-1;
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
                 map<string, flt_type> &stats,
                 bool utf8)
{
    stats.clear();
    vector<string> best_path;
    flt_type lp = viterbi(vocab, text, best_path, false, utf8);
    for (auto it = best_path.begin(); it != best_path.end(); ++it)
        stats[*it] += 1.0;
    return lp;
}


void forward(const StringSet &vocab,
             const string &text,
             vector<vector<Token> > &search,
             vector<flt_type> &fw,
             bool utf8)
{
    int len = text.length();

    vector<unsigned int> char_positions;
    get_character_positions(text, char_positions, utf8);

    for (unsigned int cpi=0; cpi<char_positions.size(); cpi++) {

        int i = char_positions[cpi];
        if (i>0 && search[i-1].size() == 0) continue;

        if (i>0) {
            fw[i-1] = search[i-1][0].cost;
            for (unsigned int t=1; t<search[i-1].size(); t++)
                fw[i-1] = add_log_domain_probs(fw[i-1], search[i-1][t].cost);
        }

        // Iterate all factors starting from this position
        const StringSet::Node *node = &vocab.root_node;
        for (unsigned int j=i; j<text.length(); j++) {

            StringSet::Arc *arc = vocab.find_arc(text[j], node);

            if (arc == nullptr) break;
            node = arc->target_node;

            // Factor associated with this node
            if (arc->factor.length() > 0) {
                flt_type cost = arc->cost;
                if (i>0) cost += fw[i-1];

                Token tok(i-1, cost);
                search[j].push_back(tok);
            }
        }
    }

    if (search[len-1].size() == 0) return;

    fw[len-1] = search[len-1][0].cost;
    for (unsigned int j=1; j<search[len-1].size(); j++)
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
        for (auto tok = search[i].cbegin(); tok != search[i].cend(); ++tok) {
            flt_type normalized = tok->cost - fw[i] + bw[i];
            if (bw[i] != SMALL_LP && fw[i] != SMALL_LP) {
                stats[text.substr(tok->source+1, i-tok->source)] += exp(normalized);
                if (tok->source == -1) continue;
                if (bw[tok->source] != SMALL_LP) bw[tok->source] = add_log_domain_probs(bw[tok->source], normalized);
                else bw[tok->source] = normalized;
            }
        }
    }
}


flt_type forward_backward(const StringSet &vocab,
                          const string &text,
                          map<string, flt_type> &stats,
                          bool utf8)
{
    int len = text.length();
    if (len == 0) return MIN_FLOAT;

    stats.clear();
    vector<vector<Token> > search(len);
    vector<flt_type> fw(len, SMALL_LP); fw[0] = 0.0;
    vector<flt_type> bw(len, SMALL_LP); bw.back() = 0.0;

    forward(vocab, text, search, fw, utf8);
    backward(vocab, text, search, fw, bw, stats);

    if (search[len-1].size() == 0) return MIN_FLOAT;
    return fw.back();
}


flt_type forward_backward(const StringSet &vocab,
                          const string &text,
                          map<string, flt_type> &stats,
                          vector<flt_type> &bw,
                          bool utf8)
{
    int len = text.length();
    if (len == 0) return MIN_FLOAT;

    stats.clear();
    vector<vector<Token> > search(len);
    vector<flt_type> fw(len, SMALL_LP); fw[0] = 0.0;
    bw.resize(len, SMALL_LP); bw.back() = 0.0;

    forward(vocab, text, search, fw, utf8);
    backward(vocab, text, search, fw, bw, stats);

    if (search[len-1].size() == 0) return MIN_FLOAT;
    return fw.back();
}


flt_type forward_backward(const map<string, flt_type> &vocab,
                          const string &text,
                          map<string, flt_type> &stats,
                          bool utf8)
{
    StringSet stringset_vocab(vocab);
    return forward_backward(stringset_vocab, text, stats, utf8);
}


flt_type viterbi(const StringSet &vocab,
                 const string &text,
                 transitions_t &stats,
                 const string &start_end_symbol)
{
    stats.clear();
    vector<string> best_path;
    flt_type lp = viterbi(vocab, text, best_path, true);
    if (best_path.size() == 0) return MIN_FLOAT;
    best_path.insert(best_path.begin(), start_end_symbol);
    best_path.push_back(start_end_symbol);
    for (unsigned int i=1; i<best_path.size(); i++)
        stats[best_path[i-1]][best_path[i]] += 1.0;
    return lp;
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
    for (unsigned int i=0; i<text.nodes.size(); i++) {

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
    int node = text.nodes.size()-1;
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
                 transitions_t &stats,
                 flt_type multiplier)
{
    vector<string> best_path;
    flt_type lp = viterbi(transitions, text, best_path, true);
    if (best_path.size() < 2) return MIN_FLOAT;
    for (unsigned int i=1; i<best_path.size(); i++)
        stats[best_path[i-1]][best_path[i]] += multiplier;
    return lp;
}


void forward(const transitions_t &transitions,
             FactorGraph &text,
             vector<flt_type> &fw)
{
    for (unsigned int i=0; i<text.nodes.size(); i++) {

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
    for (unsigned int i=1; i<text.nodes.size()-1; i++) {
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
    for (unsigned int i=0; i<text.nodes.size(); i++) {

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
    for (unsigned int i=0; i<text.nodes.size(); i++) {

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
    for (unsigned int i=0; i<text.nodes.size(); i++) {

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
    int node = text.nodes.size()-1;
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
    for (auto ndit = msfg.nodes.begin(); ndit != msfg.nodes.end(); ++ndit)
        for (auto arc = ndit->outgoing.begin(); arc != ndit->outgoing.end(); ++arc)
            (**arc).cost = nullptr;

    for (auto it = transitions.begin(); it != transitions.end(); ++it) {
        vector<msfg_node_idx_t> &nodes = msfg.factor_node_map.at(it->first);
        for (auto ndit = nodes.begin(); ndit != nodes.end(); ++ndit) {
            MultiStringFactorGraph::Node &node = msfg.nodes[*ndit];
            for (auto arc = node.outgoing.begin(); arc != node.outgoing.end(); ++arc) {
                string &tgt_str = msfg.nodes[(**arc).target_node].factor;
                if (it->second.find(tgt_str) != it->second.end())
                    (**arc).cost = &(it->second.at(tgt_str));
            }
        }
    }

    set<MultiStringFactorGraph::Arc*> to_remove;
    for (auto ndit = msfg.nodes.begin(); ndit != msfg.nodes.end(); ++ndit)
        for (auto arc = ndit->outgoing.begin(); arc != ndit->outgoing.end(); ++arc)
            if ((**arc).cost == nullptr) to_remove.insert(*arc);
    for (auto trit = to_remove.begin(); trit != to_remove.end(); ++trit)
        msfg.remove_arc(*trit);

    for (auto fnit=msfg.factor_node_map.begin(); fnit != msfg.factor_node_map.end(); ++fnit)
        if (transitions.find(fnit->first) == transitions.end())
            msfg.remove_arcs(fnit->first);
}


void
assign_scores(map<string, flt_type> &vocab,
              MultiStringFactorGraph &msfg)
{
    for (auto ndit = msfg.nodes.begin(); ndit != msfg.nodes.end(); ++ndit)
        for (auto arc = ndit->outgoing.begin(); arc != ndit->outgoing.end(); ++arc)
            (**arc).cost = nullptr;

    for (auto it = msfg.factor_node_map.begin(); it != msfg.factor_node_map.end(); ++it) {
        flt_type *curr_score = &(vocab.at(it->first));
        for (auto ndit = it->second.begin(); ndit != it->second.end(); ++ndit) {
            MultiStringFactorGraph::Node &node = msfg.nodes[*ndit];
            for (auto arc = node.incoming.begin(); arc != node.incoming.end(); ++arc)
                (**arc).cost = curr_score;
        }
    }

    set<MultiStringFactorGraph::Arc*> to_remove;
    for (auto ndit = msfg.nodes.begin(); ndit != msfg.nodes.end(); ++ndit)
        for (auto arc = ndit->outgoing.begin(); arc != ndit->outgoing.end(); ++arc)
            if ((**arc).cost == nullptr) to_remove.insert(*arc);
    for (auto trit = to_remove.begin(); trit != to_remove.end(); ++trit)
        msfg.remove_arc(*trit);

    for (auto fnit=msfg.factor_node_map.begin(); fnit != msfg.factor_node_map.end(); ++fnit)
        if (vocab.find(fnit->first) == vocab.end())
            msfg.remove_arcs(fnit->first);
}


void
forward(const MultiStringFactorGraph &msfg,
        vector<flt_type> &fw)
{
    for (unsigned int i=0; i<msfg.nodes.size(); i++) {

        if (fw[i] == MIN_FLOAT) continue;

        const MultiStringFactorGraph::Node &node = msfg.nodes[i];
        for (auto arc = node.outgoing.begin(); arc != node.outgoing.end(); ++arc) {
            int tgt_node = (**arc).target_node;
            flt_type cost = fw[i] + *(**arc).cost;
            if (fw[tgt_node] == MIN_FLOAT) fw[tgt_node] = cost;
            else fw[tgt_node] = add_log_domain_probs(fw[tgt_node], cost);
        }
    }
}


flt_type
forward(const string &text,
        const MultiStringFactorGraph &msfg,
        map<msfg_node_idx_t, flt_type> &fw)
{
    map<msfg_node_idx_t, vector<MultiStringFactorGraph::Arc*> > arcs;
    msfg.collect_arcs(text, arcs);

    for (auto ndit = arcs.begin(); ndit != arcs.end(); ++ndit) {

        const MultiStringFactorGraph::Node &node = msfg.nodes[ndit->first];
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
        const MultiStringFactorGraph &msfg,
        const set<string> &words_to_fb,
        bool full_forward_pass)
{
    flt_type total_lp = 0.0;

    if (full_forward_pass) {
        vector<flt_type> fw;
        forward(msfg, fw);
        for (auto wit = words_to_fb.cbegin(); wit != words_to_fb.cend(); ++wit)
            total_lp += words.at(*wit) * fw.at(msfg.string_end_nodes.at(*wit));
    }
    else {
        for (auto wit = words_to_fb.cbegin(); wit != words_to_fb.cend(); ++wit) {
            map<msfg_node_idx_t, flt_type> fw;
            total_lp += words.at(*wit) * forward(*wit, msfg, fw);
        }
    }

    return total_lp;
}


flt_type
likelihood_fb(const string &text,
              const MultiStringFactorGraph &msfg)
{
    map<msfg_node_idx_t, flt_type> fw;
    int text_end_node = msfg.string_end_nodes.at(text);
    set<int> nodes_to_process; nodes_to_process.insert(text_end_node);
    fw[text_end_node] = 0.0;

    while(nodes_to_process.size() > 0) {

        int i = *(nodes_to_process.rbegin());

        const MultiStringFactorGraph::Node &node = msfg.nodes[i];

        for (auto arc = node.incoming.begin(); arc != node.incoming.end(); ++arc) {
            int src_node = (**arc).source_node;
            flt_type cost = fw[i] + *(**arc).cost;
            if (fw.find(src_node) == fw.end()) fw[src_node] = cost;
            else fw[src_node] = add_log_domain_probs(fw[src_node], cost);
            nodes_to_process.insert(src_node);
        }

        nodes_to_process.erase(i);
    }

    return fw.at(0);
}


flt_type
likelihood_viterbi(const string &text,
                   const MultiStringFactorGraph &msfg)
{
    map<msfg_node_idx_t, flt_type> fw;
    int text_end_node = msfg.string_end_nodes.at(text);
    set<int> nodes_to_process; nodes_to_process.insert(text_end_node);
    fw[text_end_node] = 0.0;

    while(nodes_to_process.size() > 0) {

        int i = *(nodes_to_process.rbegin());

        const MultiStringFactorGraph::Node &node = msfg.nodes[i];

        for (auto arc = node.incoming.begin(); arc != node.incoming.end(); ++arc) {
            int src_node = (**arc).source_node;
            flt_type cost = fw[i] + *(**arc).cost;
            if (fw.find(src_node) == fw.end()) fw[src_node] = cost;
            else fw[src_node] = max(fw[src_node], cost);
            nodes_to_process.insert(src_node);
        }

        nodes_to_process.erase(i);
    }

    return fw.at(0);
}


flt_type
likelihood(const map<string, flt_type> &words,
           const set<string> &selected_words,
           const MultiStringFactorGraph &msfg,
           bool forward_backward)
{
    flt_type total_lp = 0.0;

    for (auto it = selected_words.cbegin(); it != selected_words.cend(); ++it)
        if (forward_backward)
            total_lp += words.at(*it) * likelihood_fb(*it, msfg);
        else
            total_lp += words.at(*it) * likelihood_viterbi(*it, msfg);

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
forward_backward(const MultiStringFactorGraph &msfg,
                 const map<string, flt_type> &word_freqs,
                 transitions_t &stats)
{
    if (msfg.nodes.size() == 0) return MIN_FLOAT;

    vector<flt_type> fw(msfg.nodes.size(), MIN_FLOAT);
    fw[0] = 0.0;

    forward(msfg, fw);
    flt_type total_lp = 0.0;
    for (auto it = msfg.string_end_nodes.begin(); it != msfg.string_end_nodes.end(); ++it) {
        flt_type lp = backward(msfg, it->first, fw, stats, word_freqs.at(it->first));
        total_lp += word_freqs.at(it->first) * lp;
    }

    return total_lp;
}


flt_type
forward_backward(const MultiStringFactorGraph &msfg,
                 const string &text,
                 transitions_t &stats)
{
    if (msfg.nodes.size() == 0) return MIN_FLOAT;

    map<msfg_node_idx_t, flt_type> fw;
    fw[0] = 0.0;

    forward(text, msfg, fw);
    backward(msfg, text, fw, stats);

    return fw[msfg.string_end_nodes.at(text)];
}


flt_type
viterbi(const MultiStringFactorGraph &msfg,
        const string &text,
        vector<string> &best_path)
{
    map<msfg_node_idx_t, flt_type> scores;
    map<msfg_node_idx_t, msfg_node_idx_t> sources;
    unsigned int text_end_node = msfg.string_end_nodes.at(text);
    set<unsigned int> nodes_to_process; nodes_to_process.insert(text_end_node);
    scores[text_end_node] = 0.0;

    while(nodes_to_process.size() > 0) {

        int i = *(nodes_to_process.rbegin());

        const MultiStringFactorGraph::Node &node = msfg.nodes[i];

        for (auto arc = node.incoming.begin(); arc != node.incoming.end(); ++arc) {
            int src_node = (**arc).source_node;
            flt_type cost = scores[i] + *(**arc).cost;
            if (scores.find(src_node) == scores.end() || (cost > scores[src_node])) {
                scores[src_node] = cost;
                sources[src_node] = i;
            }
            nodes_to_process.insert(src_node);
        }

        nodes_to_process.erase(i);
    }

    best_path.clear();

    msfg_node_idx_t curr_node = 0;
    while (true) {
        best_path.push_back(msfg.nodes[curr_node].factor);
        if (curr_node == text_end_node) break;
        curr_node = sources[curr_node];
    }

    return scores.at(0);
}


flt_type
viterbi(const MultiStringFactorGraph &msfg,
        const string &text,
        transitions_t &stats,
        flt_type multiplier)
{
    vector<string> best_path;
    flt_type lp = viterbi(msfg, text, best_path);
    for (unsigned int i=1; i<best_path.size(); i++)
        stats[best_path[i-1]][best_path[i]] += multiplier;
    return lp;
}


flt_type viterbi(const MultiStringFactorGraph &msfg,
                 const map<string, flt_type> &word_freqs,
                 transitions_t &stats)
{
    if (msfg.nodes.size() == 0) return MIN_FLOAT;

    vector<flt_type> fw(msfg.nodes.size(), MIN_FLOAT);
    vector<int> source_nodes(msfg.nodes.size(), -1);
    fw[0] = 0.0;

    for (unsigned int i=0; i<msfg.nodes.size(); i++) {
        if (fw[i] == MIN_FLOAT) continue;
        const MultiStringFactorGraph::Node &node = msfg.nodes[i];
        for (auto arc = node.outgoing.begin(); arc != node.outgoing.end(); ++arc) {
            int tgt_node = (**arc).target_node;
            flt_type cost = fw[i] + *(**arc).cost;
            if (cost > fw[tgt_node]) {
                fw[tgt_node] = cost;
                source_nodes[tgt_node] = i;
            }
        }
    }

    flt_type total_lp = 0.0;
    for (auto it = msfg.string_end_nodes.begin(); it != msfg.string_end_nodes.end(); ++it) {
        msfg_node_idx_t text_end_node = it->second;
        int curr_node = text_end_node;
        while (curr_node != 0) {
            if (source_nodes[curr_node] < 0) throw string("Problem in backtracking Viterbi path.");
            msfg_node_idx_t src_node = source_nodes[curr_node];
            stats[msfg.nodes.at(src_node).factor][msfg.nodes.at(curr_node).factor] += word_freqs.at(it->first);
            curr_node = src_node;
        }
        total_lp += word_freqs.at(it->first) * fw.at(text_end_node);
    }

    return total_lp;
}
