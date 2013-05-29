#include <algorithm>
#include <iostream>
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
    map<int, int> created_nodes;
    vector<int> connect_to_end_node; // MSFG indices to connect to end node
    vector<pair<int, int> > nodes_to_process; // First node idx in text FG, second node idx in MSFG
    nodes_to_process.push_back(make_pair(0, 0));

    while (nodes_to_process.size() > 0) {

        int fg_src_node = nodes_to_process[0].first;
        int msfg_src_node = nodes_to_process[0].second;
        nodes_to_process.erase(nodes_to_process.begin());
        const FactorGraph::Node &fg_node = text.nodes[fg_src_node];

        for (auto fg_arcit = fg_node.outgoing.cbegin(); fg_arcit != fg_node.outgoing.cend(); ++fg_arcit) {

            int fg_target_node = (**fg_arcit).target_node;
            string target_node_factor = text.get_factor(fg_target_node);
            if (target_node_factor == start_end_symbol) {
                connect_to_end_node.push_back(msfg_src_node);
                continue;
            }
            int msfg_target_node = -1;

            // Check existing target nodes
            MultiStringFactorGraph::Node &msfg_node = nodes[msfg_src_node];
            for (auto msfg_arcit = msfg_node.outgoing.begin(); msfg_arcit != msfg_node.outgoing.end(); ++msfg_arcit) {
                if (get_factor((**msfg_arcit).target_node) == target_node_factor) {
                    msfg_target_node = (**msfg_arcit).target_node;
                    break;
                }
            }

            // Not found, check if node added but no arc yet
            if (msfg_target_node < 0) {
            if (created_nodes.find(fg_target_node) != created_nodes.end()) {
                msfg_target_node = created_nodes[fg_target_node];
                Arc *arc = new Arc(msfg_src_node, msfg_target_node, 0.0);
                arcs.push_back(arc);
                nodes[msfg_src_node].outgoing.push_back(arc);
                nodes[msfg_target_node].incoming.push_back(arc);
            }
            }

            // Create node and arc
            if (msfg_target_node < 0) {
                nodes.push_back(Node(target_node_factor));
                msfg_target_node = nodes.size()-1;
                created_nodes[fg_target_node] = msfg_target_node;
                Arc *arc = new Arc(msfg_src_node, msfg_target_node, 0.0);
                arcs.push_back(arc);
                nodes[msfg_src_node].outgoing.push_back(arc);
                nodes[msfg_target_node].incoming.push_back(arc);
            }

            nodes_to_process.push_back(make_pair(fg_target_node, msfg_target_node));
            sort(nodes_to_process.begin(), nodes_to_process.end(), ascending_by_first);
            auto uniq = unique(nodes_to_process.begin(), nodes_to_process.end());
            nodes_to_process.erase(uniq, nodes_to_process.end());
        }
    }

    // Create end node and connect to it
    nodes.push_back(Node(start_end_symbol));
    created_nodes[text.nodes.size()-1] = nodes.size()-1;
    for (auto it = connect_to_end_node.begin(); it != connect_to_end_node.end(); ++it) {
        Arc *arc = new Arc(*it, nodes.size()-1, 0.0);
        arcs.push_back(arc);
        nodes[*it].outgoing.push_back(arc);
        nodes[nodes.size()-1].incoming.push_back(arc);
    }
    string_end_nodes[text.text] = nodes.size()-1;
}


int
MultiStringFactorGraph::num_paths(std::string &text) const
{
    if (nodes.size() == 0) return 0;
    if (string_end_nodes.find(text) == string_end_nodes.end()) return 0;
    int end_node = string_end_nodes.at(text);
    map<int, int> path_counts;
    path_counts[end_node] = 1;

    // FIXME: inefficient, mainly for debug
    for (int i=end_node; i>=0; i--) {
        if (path_counts.find(i) == path_counts.end()) continue;
        const MultiStringFactorGraph::Node &node = nodes[i];
        for (auto arc = node.incoming.begin(); arc != node.incoming.end(); ++arc)
            path_counts[(**arc).source_node] += path_counts[i];
    }

    return path_counts[0];
}

