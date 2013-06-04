#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "MSFG.hh"

using namespace std;


MultiStringFactorGraph::~MultiStringFactorGraph() {
    for (auto it = arcs.begin(); it != arcs.end(); ++it)
        delete *it;
}


bool ascending_by_first(pair<int, int> i,pair<int, int> j) { return (i.first < j.first); }

void
MultiStringFactorGraph::add(const FactorGraph &text)
{
    map<fg_node_idx_t, msfg_node_idx_t> created_nodes;
    vector<msfg_node_idx_t> connect_to_end_node; // MSFG indices to connect to end node
    map<fg_node_idx_t, msfg_node_idx_t> nodes_to_process; // Key is node idx in text FG, value idx in MSFG
    nodes_to_process[0] = 0;

    while (nodes_to_process.size() > 0) {

        fg_node_idx_t fg_src_node = nodes_to_process.begin()->first;
        msfg_node_idx_t msfg_src_node = nodes_to_process.begin()->second;
        nodes_to_process.erase(nodes_to_process.begin());

        const FactorGraph::Node &fg_node = text.nodes[fg_src_node];
        for (auto fg_arcit = fg_node.outgoing.cbegin(); fg_arcit != fg_node.outgoing.cend(); ++fg_arcit) {

            fg_node_idx_t fg_target_node = (**fg_arcit).target_node;
            string target_node_factor = text.get_factor(fg_target_node);
            if (target_node_factor == start_end_symbol) {
                connect_to_end_node.push_back(msfg_src_node);
                continue;
            }
            msfg_node_idx_t msfg_target_node = 0;

            // Check existing target nodes
            MultiStringFactorGraph::Node &msfg_node = nodes[msfg_src_node];
            for (auto msfg_arcit = msfg_node.outgoing.begin(); msfg_arcit != msfg_node.outgoing.end(); ++msfg_arcit) {
                if (get_factor((**msfg_arcit).target_node) == target_node_factor) {
                    msfg_target_node = (**msfg_arcit).target_node;
                    break;
                }
            }

            // Not found, check if node added but no arc yet
            if (msfg_target_node == 0)
            if (created_nodes.find(fg_target_node) != created_nodes.end()) {
                msfg_target_node = created_nodes[fg_target_node];
                create_arc(msfg_src_node, msfg_target_node);
            }

            // Create node and arc
            if (msfg_target_node == 0) {
                nodes.push_back(Node(target_node_factor));
                msfg_target_node = nodes.size()-1;
                created_nodes[fg_target_node] = msfg_target_node;
                create_arc(msfg_src_node, msfg_target_node);
            }

            nodes_to_process[fg_target_node] = msfg_target_node;
        }
    }

    // Create end node and connect to it
    nodes.push_back(Node(start_end_symbol));
    created_nodes[text.nodes.size()-1] = nodes.size()-1;
    for (auto it = connect_to_end_node.begin(); it != connect_to_end_node.end(); ++it)
        create_arc((*it), (nodes.size()-1));
    string_end_nodes[text.text] = nodes.size()-1;
}


int
MultiStringFactorGraph::num_paths(const std::string &text) const
{
    if (nodes.size() == 0) return 0;
    if (string_end_nodes.find(text) == string_end_nodes.end()) return 0;
    msfg_node_idx_t end_node = string_end_nodes.at(text);
    map<msfg_node_idx_t, int> path_counts; path_counts[end_node] = 1;
    set<msfg_node_idx_t> nodes_to_process; nodes_to_process.insert(end_node);

    while(nodes_to_process.size() > 0) {

        msfg_node_idx_t i = *(nodes_to_process.rbegin());

        const Node &node = nodes[i];
        for (auto arc = node.incoming.begin(); arc != node.incoming.end(); ++arc) {
            path_counts[(**arc).source_node] += path_counts[i];
            nodes_to_process.insert((**arc).source_node);
        }

        nodes_to_process.erase(i);
    }

    return path_counts[0];
}


void
MultiStringFactorGraph::get_paths(const string &text,
                                  vector<vector<string> > &paths) const
{
    if (string_end_nodes.find(text) == string_end_nodes.end()) return;
    msfg_node_idx_t end_node = string_end_nodes.at(text);
    vector<string> curr_string;
    advance(paths, curr_string, end_node);
    for (auto it = paths.begin(); it != paths.end(); ++it)
        std::reverse(it->begin(), it->end());
}


void
MultiStringFactorGraph::print_paths(const string &text) const
{
    vector<vector<string> > paths;
    get_paths(text, paths);

    for (auto pathit = paths.begin(); pathit != paths.end(); ++pathit) {
        for (auto it = pathit->begin(); it != pathit->end(); ++it)
            cout << *it << " ";
        cout << endl;
    }
}


void
MultiStringFactorGraph::advance(vector<vector<string> > &paths,
                                vector<string> &curr_string,
                                msfg_node_idx_t node_idx) const
{
    const MultiStringFactorGraph::Node &node = nodes[node_idx];
    curr_string.push_back(node.factor);

    if (node_idx == 0) {
        paths.push_back(curr_string);
        return;
    }

    for (auto arc = node.incoming.begin(); arc != node.incoming.end(); ++arc) {
        vector<string> curr_copy(curr_string);
        advance(paths, curr_copy, (**arc).source_node);
    }
}


void
MultiStringFactorGraph::create_arc(msfg_node_idx_t src_node,
                                   msfg_node_idx_t tgt_node)
{
    Arc *arc = new Arc(src_node, tgt_node, 0.0);
    arcs.insert(arc);
    nodes[src_node].outgoing.insert(arc);
    nodes[tgt_node].incoming.insert(arc);
}


void
MultiStringFactorGraph::remove_arc(Arc *arc)
{
    //auto ait = find(arcs.begin(), arcs.end(), arc);
    //arcs.erase(ait);
    arcs.erase(arc);

    Node &src_node = nodes[(*arc).source_node];
    //auto sit = find(src_node.outgoing.begin(), src_node.outgoing.end(), arc);
    //src_node.outgoing.erase(sit);
    src_node.outgoing.erase(arc);

    Node &tgt_node = nodes[(*arc).target_node];
    //auto tit = find(tgt_node.incoming.begin(), tgt_node.incoming.end(), arc);
    //tgt_node.incoming.erase(tit);
    tgt_node.incoming.erase(arc);

    delete arc;
}


void
MultiStringFactorGraph::remove_arcs(const std::string &factor)
{
    if (factor.length() < 2) {
        cerr << "Trying to remove factor of length 1: " << factor << endl;
        exit(0);
    }

    for (auto ndit = factor_node_map[factor].begin(); ndit != factor_node_map[factor].end(); ++ndit) {
        Node &node = nodes[*ndit];
        while (node.incoming.size() > 0)
            remove_arc(*(node.incoming.begin()));
        while (node.outgoing.size() > 0)
            remove_arc(*(node.outgoing.begin()));
    }
}


void
MultiStringFactorGraph::prune_unreachable()
{
    for (auto it = nodes.begin(); it != nodes.end(); it++) {

        if (it->factor == start_end_symbol) continue;

        if (it->incoming.size() == 0)
            while (it->outgoing.size() > 0)
                remove_arc(*(it->outgoing.begin()));

        if (it->outgoing.size() == 0)
            while (it->incoming.size() > 0)
                remove_arc(*(it->incoming.begin()));
    }
}


void
MultiStringFactorGraph::write(const std::string &filename) const
{
    ofstream outfile(filename);
    if (!outfile) return;

    outfile << nodes.size() << " " << arcs.size() << " " << string_end_nodes.size() << endl;
    for (int i=0; i<nodes.size(); i++)
        outfile << "n " << i << " " << nodes[i].factor << endl;
    for (auto it = arcs.begin(); it != arcs.end(); ++it)
        outfile << "a " << (**it).source_node << " " << (**it).target_node << endl;
    for (auto it = string_end_nodes.cbegin(); it != string_end_nodes.cend(); ++it)
        outfile << "e " << it->first << " " << it->second << endl;
    outfile.close();
}


void
MultiStringFactorGraph::read(const std::string &filename)
{
    ifstream infile(filename);
    if (!infile) return;

    int node_count, arc_count, end_node_count;
    char type;
    string line;
    getline(infile, line);
    stringstream ss(line);
    ss >> node_count >> arc_count >> end_node_count;

    nodes.clear(); arcs.clear(); string_end_nodes.clear();
    nodes.resize(node_count);

    msfg_node_idx_t node_idx;
    string factor;
    for (int i=0; i<node_count; i++) {
        getline(infile, line);
        stringstream nodess(line);
        nodess >> type;
        if (type != 'n') {
            cerr << "Some problem reading MSFG file" << endl;
            exit(0);
        }
        nodess >> node_idx >> factor;
        nodes[node_idx].factor.assign(factor);
        factor_node_map[factor].push_back(node_idx);
    }

    msfg_node_idx_t src_node, tgt_node;
    for (int i=0; i<arc_count; i++) {
        getline(infile, line);
        stringstream arcss(line);
        arcss >> type;
        if (type != 'a') {
            cerr << "Some problem reading MSFG file" << endl;
            exit(0);
        }
        arcss >> src_node >> tgt_node;
        create_arc(src_node, tgt_node);
    }

    msfg_node_idx_t end_node_idx;
    string curr_string;
    for (int i=0; i<arc_count; i++) {
        getline(infile, line);
        stringstream endnss(line);
        endnss >> type;
        if (type != 'e') {
            cerr << "Some problem reading MSFG file" << endl;
            exit(0);
        }
        endnss >> curr_string >> end_node_idx;
        string_end_nodes[curr_string] = end_node_idx;
    }

    infile.close();
}

