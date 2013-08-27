#ifndef FACTOR_GRAPH
#define FACTOR_GRAPH

#include <string>
#include <unordered_set>

#include "defs.hh"
#include "StringSet.hh"


class FactorGraph {
public:

    /** Arc of a factor graph. */
    class Arc {
    public:
        Arc(fg_node_idx_t source_node, fg_node_idx_t target_node,
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
        fg_node_idx_t source_node;
        fg_node_idx_t target_node;
        flt_type cost;
    };

    /** Node of a factor graph. */
    class Node {
    public:
        Node(int start_pos, int len)
        : start_pos(start_pos), len(len) { }
        ~Node() { incoming.clear(); outgoing.clear(); }
        factor_pos_t start_pos; // text indices
        factor_len_t len;
        std::vector<Arc*> incoming;
        std::vector<Arc*> outgoing;
    };

    FactorGraph() {};
    FactorGraph(const std::string &text, const std::string &start_end_symbol,
                const std::map<std::string, flt_type> &vocab, int maxlen);
    FactorGraph(const std::string &text, const std::string &start_end_symbol,
                const StringSet &vocab);
    ~FactorGraph();

    void set_text(const std::string &text, const std::string &start_end_symbol,
                  const std::map<std::string, flt_type> &vocab, int maxlen);
    void set_text(const std::string &text, const std::string &start_end_symbol,
                  const StringSet &vocab);
    void get_factor(const Node &node, std::string &nstr) const
    { if (node.len == 0) nstr.assign(start_end_symbol);
      else nstr.assign(this->text, node.start_pos, node.len); }
    void get_factor(int node, std::string &nstr) const
    { if (nodes[node].len == 0) nstr.assign(start_end_symbol);
      else nstr.assign(this->text, nodes[node].start_pos, nodes[node].len); }
    std::string get_factor(const Node &node) const
    { if (node.len == 0) return start_end_symbol;
      else return this->text.substr(node.start_pos, node.len); }
    std::string get_factor(int node) const
    { if (nodes[node].len == 0) return start_end_symbol;
      else return this->text.substr(nodes[node].start_pos, nodes[node].len); }
    bool assert_equal(const FactorGraph &other) const;
    int num_paths() const;
    void get_paths(std::vector<std::vector<std::string> > &paths) const;
    void remove_arcs(const std::string &source, const std::string &target);
    void remove_arcs(const std::string &remstr);

    std::string text;
    std::string start_end_symbol;
    std::vector<Node> nodes;
    std::vector<Arc*> arcs;

private:
    // Constructor helpers
    void create_nodes(const std::string &text, const std::map<std::string, flt_type> &vocab,
                      int maxlen, std::vector<std::unordered_set<fg_node_idx_t> > &incoming);
    void create_nodes(const std::string &text, const StringSet &vocab,
                      std::vector<std::unordered_set<fg_node_idx_t> > &incoming);
    void prune_and_create_arcs(std::vector<std::unordered_set<fg_node_idx_t> > &incoming);
    // Helper for enumerating paths
    void advance(std::vector<std::vector<std::string> > &paths,
                 std::vector<std::string> &curr_string, fg_node_idx_t node) const;
    // Helper for removing arcs
    void remove_arc(Arc *arc);
};


#endif /* FACTOR_GRAPH */

