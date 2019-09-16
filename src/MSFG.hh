#ifndef MSFG
#define MSFG

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <vector>

#include "defs.hh"
#include "FactorGraph.hh"


class MultiStringFactorGraph {
public:

    /** Arc of a multi string factor graph. */
    class Arc {
    public:
        Arc(msfg_node_idx_t source_node, msfg_node_idx_t target_node,
            flt_type *cost=NULL)
        : source_node(source_node), target_node(target_node), cost(cost) {}
        bool operator==(Arc& rhs) const {
            if (source_node != rhs.source_node) return false;
            if (target_node != rhs.target_node) return false;
            if (cost != rhs.cost) return false;
            return true;
        }
        bool operator!=(Arc& rhs) const {
            if (source_node != rhs.source_node) return true;
            if (target_node != rhs.target_node) return true;
            if (cost != rhs.cost) return true;
            return false;
        }
        msfg_node_idx_t source_node;
        msfg_node_idx_t target_node;
        flt_type *cost;
    };

    /** Node of a multi string factor graph. */
    class Node {
    public:
        Node() { }
        Node(const std::string &factor) { this->factor.assign(factor); }
        ~Node() { incoming.clear(); outgoing.clear(); }
        std::string factor;
        std::set<Arc*> incoming;
        std::set<Arc*> outgoing;
    };

    MultiStringFactorGraph(const std::string &start_end_symbol)
    : start_end_symbol(start_end_symbol) { nodes.push_back(Node(std::string(start_end_symbol))); };
    ~MultiStringFactorGraph();

    void add(const FactorGraph &text, bool lookahead=true);
    void get_factor(const Node &node, std::string &nstr) const
    { nstr.assign(node.factor); }
    void get_factor(msfg_node_idx_t node, std::string &nstr) const
    { nstr.assign(nodes[node].factor); }
    std::string get_factor(const Node &node) const
    { return node.factor; }
    std::string get_factor(msfg_node_idx_t node) const
    { return nodes[node].factor; }
    int num_paths(const std::string &text) const;
    void get_paths(const std::string &text, std::vector<std::vector<std::string> > &paths) const;
    void print_paths(const std::string &text) const;
    void create_arc(msfg_node_idx_t src_node, msfg_node_idx_t tgt_node);
    void find_or_create_arc(msfg_node_idx_t src_node, msfg_node_idx_t tgt_node);
    void remove_arcs(const std::string &factor);
    void remove_arc(Arc *arc);
    void collect_arcs(const std::string &text,
                      std::map<msfg_node_idx_t, std::vector<Arc*> > &arcs) const;
    void collect_factors(const std::string &text,
                         std::set<std::string> &factors) const;
    void prune_unreachable();
    void prune_unused(transitions_t &transitions);
    void write(const std::string &filename) const;
    void read(const std::string &filename);
    void update_factor_node_map();
    void print_dot_digraph(std::ostream &fstr = std::cout);

    std::string start_end_symbol;
    std::vector<Node> nodes;
    std::map<std::string, msfg_node_idx_t> string_end_nodes;
    std::map<msfg_node_idx_t, std::string> reverse_string_end_nodes;
    std::map<std::string, std::vector<msfg_node_idx_t> > factor_node_map;
    // Helper for constructing the graph
    std::map<msfg_node_idx_t, std::map<std::string, msfg_node_idx_t> > factor_lookahead;

private:

    // Helper for enumerating paths
    void advance(std::vector<std::vector<std::string> > &paths,
                 std::vector<std::string> &curr_string,
                 msfg_node_idx_t node) const;
    void collect_arcs(std::vector<Arc*> &arcs) const;
};


#endif /* MSFG */
