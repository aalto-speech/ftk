#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>

#include "RevMSFG.hh"

using namespace std;


ReverseMultiStringFactorGraph::~ReverseMultiStringFactorGraph() {
    vector<Arc*> arcs;
    collect_arcs(arcs);
    for (auto it = arcs.begin(); it != arcs.end(); ++it)
        delete *it;
}


bool ascending_by_first(pair<int, int> i,pair<int, int> j) {
    return (i.first < j.first);
}

void
ReverseMultiStringFactorGraph::add(const FactorGraph &text)
{
    map<fg_node_idx_t, msfg_node_idx_t> visited_nodes;
    multimap<fg_node_idx_t, pair<msfg_node_idx_t, FactorGraph::Arc*> > arcs_to_process;
    // multimap key is source index in fg, value pair source index in msfg, arc in fg
    // ordering arcs by target index ensures topological order for text nodes in msfg

    const FactorGraph::Node &fg_node = text.nodes.back(); // FIXME??
    for (auto fg_arcit = fg_node.incoming.cbegin(); fg_arcit != fg_node.incoming.cend(); ++fg_arcit)
        arcs_to_process.insert(make_pair((**fg_arcit).source_node, make_pair(0, *fg_arcit)));

    while (arcs_to_process.size() > 0) {

        auto arc_to_process = arcs_to_process.rbegin();
        FactorGraph::Arc *arc = (*arc_to_process).second.second;
        fg_node_idx_t fg_source_node = arc->source_node;
        msfg_node_idx_t msfg_source_node = (*arc_to_process).second.first;
        arcs_to_process.erase(arc_to_process);

        string target_node_factor = text.get_factor(fg_source_node);
        msfg_node_idx_t msfg_target_node = 0;

        // Check if have connected to this node already from some other node
        // Just find or add arc and continue
        if (visited_nodes.find(fg_source_node) != visited_nodes.end()) {
            msfg_target_node = visited_nodes[fg_source_node];
            find_or_create_arc(msfg_source_node, msfg_target_node);
            continue;
        }

        // Check if node exists, just not visited yet
        ReverseMultiStringFactorGraph::Node &msfg_node = nodes[msfg_source_node];
        for (auto msfg_arcit = msfg_node.outgoing.begin(); msfg_arcit != msfg_node.outgoing.end(); ++msfg_arcit) {
            if (nodes[(**msfg_arcit).target_node].factor == target_node_factor) {
                msfg_target_node = (**msfg_arcit).target_node;
                break;
            }
        }
        if (msfg_target_node != 0) {
            visited_nodes[fg_source_node] = msfg_target_node;
            const FactorGraph::Node &fg_node = text.nodes[fg_source_node];
            for (auto fg_arcit = fg_node.outgoing.cbegin(); fg_arcit != fg_node.outgoing.cend(); ++fg_arcit)
                arcs_to_process.insert(make_pair((**fg_arcit).source_node, make_pair(msfg_target_node, *fg_arcit)));
            continue;
        }

        // Create new node and arc
        nodes.push_back(Node(target_node_factor));
        msfg_target_node = nodes.size()-1;
        visited_nodes[fg_source_node] = msfg_target_node;
        const FactorGraph::Node &fg_node = text.nodes[fg_source_node];
        for (auto fg_arcit = fg_node.incoming.cbegin(); fg_arcit != fg_node.incoming.cend(); ++fg_arcit)
            arcs_to_process.insert(make_pair((**fg_arcit).source_node, make_pair(msfg_target_node, *fg_arcit)));
        create_arc(msfg_source_node, msfg_target_node);
    }

    string_end_nodes[text.text] = visited_nodes[text.nodes.size()-1];
    reverse_string_end_nodes[visited_nodes[text.nodes.size()-1]] = text.text;
}


int
ReverseMultiStringFactorGraph::num_paths(const std::string &text) const
{
    if (nodes.size() == 0) return 0;
    if (string_end_nodes.find(text) == string_end_nodes.end()) return 0;
    msfg_node_idx_t end_node = string_end_nodes.at(text);
    map<msfg_node_idx_t, int> path_counts;
    path_counts[end_node] = 1;
    set<msfg_node_idx_t> nodes_to_process;
    nodes_to_process.insert(end_node);

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
ReverseMultiStringFactorGraph::get_paths(const string &text,
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
ReverseMultiStringFactorGraph::print_paths(const string &text) const
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
ReverseMultiStringFactorGraph::advance(vector<vector<string> > &paths,
                                       vector<string> &curr_string,
                                       msfg_node_idx_t node_idx) const
{
    const ReverseMultiStringFactorGraph::Node &node = nodes[node_idx];
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
ReverseMultiStringFactorGraph::create_arc(msfg_node_idx_t src_node,
                                          msfg_node_idx_t tgt_node)
{
    Arc *arc = new Arc(src_node, tgt_node, NULL);
    nodes[src_node].outgoing.insert(arc);
    nodes[tgt_node].incoming.insert(arc);
}


void
ReverseMultiStringFactorGraph::find_or_create_arc(msfg_node_idx_t src_node,
                                                  msfg_node_idx_t tgt_node)
{
    for (auto ait = nodes[tgt_node].incoming.begin(); ait != nodes[tgt_node].incoming.end(); ++ait)
        if ((**ait).source_node == src_node) return;
    create_arc(src_node, tgt_node);
}


void
ReverseMultiStringFactorGraph::remove_arc(Arc *arc)
{
    Node &src_node = nodes[(*arc).source_node];
    src_node.outgoing.erase(arc);

    Node &tgt_node = nodes[(*arc).target_node];
    tgt_node.incoming.erase(arc);

    delete arc;
}


void
ReverseMultiStringFactorGraph::remove_arcs(const std::string &factor)
{
    if (factor.length() < 2) {
        cerr << "Trying to remove factor of length 1: " << factor << endl;
        exit(EXIT_FAILURE);
    }

    for (auto ndit = factor_node_map[factor].begin(); ndit != factor_node_map[factor].end(); ++ndit) {
        Node &node = nodes[*ndit];
        while (node.incoming.size() > 0)
            remove_arc(*(node.incoming.begin()));
        while (node.outgoing.size() > 0)
            remove_arc(*(node.outgoing.begin()));
    }

    factor_node_map.erase(factor);
}


void
ReverseMultiStringFactorGraph::prune_unreachable()
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
ReverseMultiStringFactorGraph::write(const std::string &filename) const
{
    ofstream outfile(filename);
    if (!outfile) return;

    vector<Arc*> arcs;
    collect_arcs(arcs);

    outfile << nodes.size() << " " << arcs.size() << " " << string_end_nodes.size() << endl;
    for (unsigned int i=0; i<nodes.size(); i++)
        outfile << "n " << i << " " << nodes[i].factor << endl;
    for (auto it = arcs.begin(); it != arcs.end(); ++it)
        outfile << "a " << (**it).source_node << " " << (**it).target_node << endl;
    for (auto it = string_end_nodes.cbegin(); it != string_end_nodes.cend(); ++it)
        outfile << "e " << it->first << " " << it->second << endl;
    outfile.close();
}


void
ReverseMultiStringFactorGraph::read(const std::string &filename)
{
    ifstream infile(filename);
    if (!infile) return;

    int node_count, arc_count, end_node_count;
    char type;
    string line;
    getline(infile, line);
    stringstream ss(line);
    ss >> node_count >> arc_count >> end_node_count;

    nodes.clear();
    string_end_nodes.clear();
    reverse_string_end_nodes.clear();
    nodes.resize(node_count);

    msfg_node_idx_t node_idx;
    string factor;
    for (int i=0; i<node_count; i++) {
        getline(infile, line);
        stringstream nodess(line);
        nodess >> type;
        if (type != 'n') {
            cerr << "Some problem reading MSFG file" << endl;
            exit(EXIT_FAILURE);
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
            exit(EXIT_FAILURE);
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
            exit(EXIT_FAILURE);
        }
        endnss >> curr_string >> end_node_idx;
        string_end_nodes[curr_string] = end_node_idx;
        reverse_string_end_nodes[end_node_idx] = curr_string;
    }

    infile.close();
}


void
ReverseMultiStringFactorGraph::collect_arcs(vector<Arc*> &arcs) const
{
    for (auto ndit = nodes.begin(); ndit != nodes.end(); ++ndit)
        for (auto arcit = ndit->outgoing.begin(); arcit != ndit->outgoing.end(); ++arcit)
            arcs.push_back(*arcit);
}


void
ReverseMultiStringFactorGraph::collect_arcs(const string &text,
        map<msfg_node_idx_t,
        vector<Arc*> > &arcs) const
{
    msfg_node_idx_t end_node = string_end_nodes.at(text);
    set<msfg_node_idx_t> nodes_to_process;
    nodes_to_process.insert(end_node);

    while(nodes_to_process.size() > 0) {

        msfg_node_idx_t i = *(nodes_to_process.rbegin());

        const Node &node = nodes[i];
        for (auto arc = node.incoming.begin(); arc != node.incoming.end(); ++arc) {
            arcs[(**arc).source_node].push_back(*arc);
            nodes_to_process.insert((**arc).source_node);
        }

        nodes_to_process.erase(i);
    }
}


void
ReverseMultiStringFactorGraph::collect_factors(const string &text, set<string> &factors) const
{
    msfg_node_idx_t end_node = string_end_nodes.at(text);
    set<msfg_node_idx_t> nodes_to_process;
    nodes_to_process.insert(end_node);

    while(nodes_to_process.size() > 0) {

        msfg_node_idx_t i = *(nodes_to_process.rbegin());

        const Node &node = nodes[i];
        factors.insert(node.factor);
        for (auto arc = node.incoming.begin(); arc != node.incoming.end(); ++arc)
            nodes_to_process.insert((**arc).source_node);

        nodes_to_process.erase(i);
    }
}


void ReverseMultiStringFactorGraph::print_dot_digraph(ostream &fstr)
{
    fstr << "digraph {" << endl << endl;
    fstr << "\tnode [shape=ellipse,fontsize=30,fixedsize=false,width=0.95];" << endl;
    fstr << "\tedge [fontsize=12];" << endl;
    fstr << "\trankdir=LR;" << endl << endl;

    for (msfg_node_idx_t ni=0; ni<nodes.size(); ++ni) {
        fstr << "\t" << ni;
        string label = nodes[ni].factor;
        if (label == start_end_symbol && ni > 0) {
            label += " / " + reverse_string_end_nodes[ni];
            fstr << " [label=\"" << label << "\", style=filled, fillcolor=grey]" << endl;
        }
        else
            fstr << " [label=\"" << label << "\"]" << endl;
    }
    fstr << endl;

    for (msfg_node_idx_t ni=0; ni<nodes.size(); ++ni) {
        Node &nd = nodes[ni];
        for (auto ait = nd.outgoing.begin(); ait != nd.outgoing.end(); ++ait) {
            fstr << "\t" << (*ait)->source_node << " -> " << (*ait)->target_node;
            flt_type *cost = (*(*ait)).cost;
            if (cost != nullptr)
                fstr << "[label=\"" << *cost << "\"];";
            fstr << endl;
        }
    }
    fstr << "}" << endl;
}
