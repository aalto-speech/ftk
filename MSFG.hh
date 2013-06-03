#ifndef MSFG
#define MSFG

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
        Arc(unsigned int source_node, unsigned int target_node,
            double cost=MIN_FLOAT)
        : source_node(source_node), target_node(target_node), cost(0.0) {}
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
        flt_type cost;
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

    void add(const FactorGraph &text);
    void get_factor(const Node &node, std::string &nstr) const
    { nstr.assign(node.factor); }
    void get_factor(msfg_node_idx_t node, std::string &nstr) const
    { nstr.assign(nodes[node].factor); }
    std::string get_factor(const Node &node) const
    { return node.factor; }
    std::string get_factor(int node) const
    { return nodes[node].factor; }
    int num_paths(std::string &text) const;
    void get_paths(const std::string &text, std::vector<std::vector<std::string> > &paths) const;
    void create_arc(msfg_node_idx_t src_node, msfg_node_idx_t tgt_node);
    void remove_arcs(const std::string &factor);
    void prune_unreachable();
    void write(const std::string &filename) const;
    void read(const std::string &filename);

    std::string start_end_symbol;
    std::vector<Node> nodes;
    std::set<Arc*> arcs;
    std::map<std::string, int> string_end_nodes;
    std::map<std::string, std::vector<msfg_node_idx_t> > factor_node_map;

private:

    // Helper for enumerating paths
    void advance(std::vector<std::vector<std::string> > &paths,
                 std::vector<std::string> &curr_string,
                 msfg_node_idx_t node) const;
    // Helper for removing arcs
    void remove_arc(Arc *arc);
};


#endif /* MSFG */
