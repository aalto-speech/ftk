#ifndef FACTOR_ENCODER
#define FACTOR_ENCODER

#include <limits>
#include <map>
#include <string>
#include <vector>

#include "defs.hh"
#include "StringSet.hh"


class FactorGraph {
public:

    /** Arc of a factor graph. */
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

    /** Node of a factor graph. */
    class Node {
    public:
        Node(int start_pos, int len)
        : start_pos(start_pos), len(len) { }
        ~Node() { incoming.clear(); outgoing.clear(); }
        size_t start_pos; // text indices
        size_t len;
        std::vector<Arc*> incoming;
        std::vector<Arc*> outgoing;
    };

    FactorGraph() {};
    FactorGraph(const std::string &text, const std::string &start_end_symbol,
                const std::map<std::string, flt_type> &vocab, int maxlen);
    FactorGraph(const std::string &text, const std::string &start_end_symbol,
                const StringSet<flt_type> &vocab);
    ~FactorGraph();

    void set_text(const std::string &text, const std::string &start_end_symbol,
                  const std::map<std::string, flt_type> &vocab, int maxlen);
    void set_text(const std::string &text, const std::string &start_end_symbol,
                  const StringSet<flt_type> &vocab);
    void get_factor(const Node &node, std::string &nstr) const
    { if (node.len == 0) nstr.assign(start_end_symbol);
      else nstr.assign(this->text, node.start_pos, node.len); }
    void get_factor(int node, std::string &nstr) const
    { if (nodes[node].len == 0) nstr.assign(start_end_symbol);
      else nstr.assign(this->text, nodes[node].start_pos, nodes[node].len); }
    bool assert_equal(const FactorGraph &other) const;
    void get_paths(std::vector<std::vector<std::string> > &paths) const;

    std::string text;
    std::string start_end_symbol;
    std::vector<Node> nodes;
    std::vector<Arc*> arcs;

private:
    // Constructor helpers
    void create_nodes(const std::string &text, const std::map<std::string, flt_type> &vocab,
                      int maxlen, std::vector<std::map<unsigned int, bool> > &incoming);
    void create_nodes(const std::string &text, const StringSet<flt_type> &vocab,
                      std::vector<std::map<unsigned int, bool> > &incoming);
    void prune_and_create_arcs(std::vector<std::map<unsigned int, bool> > &incoming);
    // Helper for enumerating paths
    void advance(std::vector<std::vector<std::string> > &paths,
                 std::vector<std::string> &curr_string, unsigned int node) const;
};


int read_vocab(std::string fname,
               std::map<std::string, flt_type> &vocab,
               int &maxlen);

int write_vocab(std::string fname,
                const std::map<std::string, flt_type> &vocab);

void sort_vocab(const std::map<std::string, flt_type> &vocab,
                std::vector<std::pair<std::string, flt_type> > &sorted_vocab,
                bool descending=true);

void viterbi(const std::map<std::string, flt_type> &vocab,
             int maxlen,
             const std::string &text,
             std::vector<std::string> &best_path,
             bool reverse=true);

void viterbi(const StringSet<flt_type> &vocab,
             const std::string &text,
             std::vector<std::string> &best_path,
             bool reverse=true);

void viterbi(const StringSet<flt_type> &vocab,
             const std::string &text,
             std::map<std::string, flt_type> &stats);

flt_type add_log_domain_probs(flt_type a,
                              flt_type b);

void forward_backward(const StringSet<flt_type> &vocab,
                      const std::string &text,
                      std::map<std::string, flt_type> &stats);

void forward_backward(const std::map<std::string, flt_type> &vocab,
                      const std::string &text,
                      std::map<std::string, flt_type> &stats);

void viterbi(const std::map<std::pair<std::string,std::string>, flt_type> &transitions,
             FactorGraph &text,
             std::vector<std::string> &best_path,
             bool reverse=true);

void viterbi(const std::map<std::pair<std::string,std::string>, flt_type> &transitions,
             FactorGraph &text,
             std::map<std::pair<std::string,std::string>, flt_type> &stats);

void forward_backward(const std::map<std::pair<std::string,std::string>, flt_type> &transitions,
                      FactorGraph &text,
                      std::map<std::pair<std::string,std::string>, flt_type> &stats);

void forward_backward(const std::map<std::string, flt_type> &vocab,
                      FactorGraph &text,
                      std::map<std::pair<std::string,std::string>, flt_type> &stats);

#endif /* FACTOR_ENCODER */
