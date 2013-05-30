#ifndef MSFG
#define MSFG

#include <map>
#include <vector>

#include "defs.hh"
#include "FactorGraph.hh"


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
        unsigned int source_node;
        unsigned int target_node;
        flt_type cost;
    };

    /** Node of a multi string factor graph. */
    class Node {
    public:
        Node() { }
        Node(const std::string &factor) { this->factor.assign(factor); }
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
    int num_paths(std::string &text) const;
    void create_arc(unsigned int src_node, unsigned int tgt_node);
    void remove_arcs(const std::string &source, const std::string &target);
    void remove_arcs(const std::string &remstr);
    void write(const std::string &filename) const;
    void read(const std::string &filename);

    std::string start_end_symbol;
    std::vector<Node> nodes;
    std::vector<Arc*> arcs;
    std::map<std::string, int> string_end_nodes;

private:

    // Helper for removing arcs
    void remove_arc(Arc *arc);
};


#endif /* MSFG */
