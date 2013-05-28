#ifndef MSFG
#define MSFG

#include <vector>

#include "FactorEncoder.hh"


class MultiStringFactorGraph {
public:

    /** Arc of a multi string factor graph. */
    class Arc {
    public:
        Arc(unsigned int source_node, unsigned int target_node,
            double cost=-std::numeric_limits<flt_type>::max())
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
        fg_node_idx_t source_node;
        fg_node_idx_t target_node;
        flt_type cost;
    };

    /** Node of a multi string factor graph. */
    class Node {
    public:
        Node(const std::string factor)
        : factor(factor) { }
        ~Node() { incoming.clear(); outgoing.clear(); }
        std::string factor;
        std::vector<Arc*> incoming;
        std::vector<Arc*> outgoing;
    };

    MultiStringFactorGraph(const std::string &start_end_symbol)
    : start_end_symbol(start_end_symbol) { nodes.push_back(Node(std::string(start_end_symbol))); };
    ~MultiStringFactorGraph();

    void add(const FactorGraph &text);
    void get_factor(const Node &node, std::string &nstr) const
    { nstr.assign(node.factor); }
    void get_factor(int node, std::string &nstr) const
    { nstr.assign(nodes[node].factor); }
    std::string get_factor(const Node &node) const
    { return node.factor; }
    std::string get_factor(int node) const
    { return nodes[node].factor; }
    //int num_paths(std::string &text) const;
    //void remove_arcs(const std::string &source, const std::string &target);
    //void remove_arcs(const std::string &remstr);

    std::string start_end_symbol;
    std::vector<Node> nodes;
    std::vector<Arc*> arcs;
    std::map<std::string, int> string_end_nodes;

private:

    // Helper for removing arcs
    //void remove_arc(Arc *arc);

    // Helper for adding new text recursively to the graph
    void expand(const FactorGraph &text,
                int curr_node_in_fg,
                int curr_node_in_msfg,
                std::map<int, int> &created_nodes); // Key is node idx in text FG, value node idx in MSFG
};


#endif /* MSFG */
