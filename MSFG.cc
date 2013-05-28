#include <iostream>

#include "MSFG.hh"

using namespace std;



void
MultiStringFactorGraph::expand(const FactorGraph &text,
                               int fg_src_node,
                               int msfg_src_node,
                               map<int, int> &created_nodes) // Key is node idx in text FG, value node idx in MSFG
{
    const FactorGraph::Node &fg_node = text.nodes[fg_src_node];
    MultiStringFactorGraph::Node &msfg_node = nodes[msfg_src_node];

    for (auto fg_arcit = fg_node.outgoing.begin(); fg_arcit != fg_node.outgoing.end(); ++fg_arcit) {

        int fg_target_node = (**fg_arcit).target_node;
        string target_node_factor = text.get_factor(fg_target_node);
        int msfg_target_node = -1;

        // Check existing target nodes
        for (auto msfg_arcit = msfg_node.outgoing.begin(); msfg_arcit != msfg_node.outgoing.end(); ++msfg_arcit)
            if (get_factor((**msfg_arcit).target_node) == target_node_factor) {
                msfg_target_node = (**msfg_arcit).target_node;
                break;
            }

        // Not found, check if added but no arc yet
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

        expand(text, fg_target_node, msfg_target_node, created_nodes);
    }
}


void
MultiStringFactorGraph::add(const FactorGraph &text)
{
    map<int, int> new_nodes;
    expand(text, 0, 0, new_nodes);

    if (new_nodes.find(text.nodes.size()-1) == new_nodes.end()) {
        cerr << "MSFG: end node was not set properly for " << text.text << endl;
        exit(0);
    }

    string_end_nodes[text.text] = new_nodes[text.nodes.size()-1];
}
