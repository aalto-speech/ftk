#include <algorithm>

#include "FactorGraph.hh"

using namespace std;


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
            if (arc->factor.length() > 0) {
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
