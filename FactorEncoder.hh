#ifndef FACTOR_ENCODER
#define FACTOR_ENCODER

#include <limits>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>

#include "defs.hh"
#include "StringSet.hh"

typedef std::map<std::string, std::map<std::string, flt_type> > transitions_t;
flt_type SMALL_LP = -200.0;
flt_type MIN_FLOAT = -std::numeric_limits<flt_type>::max();


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
                      int maxlen, std::vector<std::unordered_set<unsigned int> > &incoming);
    void create_nodes(const std::string &text, const StringSet<flt_type> &vocab,
                      std::vector<std::unordered_set<unsigned int> > &incoming);
    void prune_and_create_arcs(std::vector<std::unordered_set<unsigned int> > &incoming);
    // Helper for enumerating paths
    void advance(std::vector<std::vector<std::string> > &paths,
                 std::vector<std::string> &curr_string, unsigned int node) const;
    // Helper for removing arcs
    void remove_arc(Arc *arc);
};

// 1-GRAM
int read_vocab(std::string fname,
               std::map<std::string, flt_type> &vocab,
               int &maxlen);

int write_vocab(std::string fname,
                const std::map<std::string, flt_type> &vocab);

void sort_vocab(const std::map<std::string, flt_type> &vocab,
                std::vector<std::pair<std::string, flt_type> > &sorted_vocab,
                bool descending=true);

flt_type viterbi(const std::map<std::string, flt_type> &vocab,
                 int maxlen,
                 const std::string &text,
                 std::vector<std::string> &best_path,
                 bool reverse=true);

flt_type viterbi(const StringSet<flt_type> &vocab,
                 const std::string &text,
                 std::vector<std::string> &best_path,
                 bool reverse=true);

flt_type viterbi(const StringSet<flt_type> &vocab,
                 const std::string &text,
                 std::map<std::string, flt_type> &stats);

flt_type add_log_domain_probs(flt_type a,
                              flt_type b);


class Token {
    public:
        int source;
        flt_type cost;
        Token(): source(-1), cost(MIN_FLOAT) {};
        Token(int src, flt_type cst): source(src), cost(cst) {};
        Token(const Token& orig) { this->source=orig.source; this->cost=orig.cost; };
};

void forward(const StringSet<flt_type> &vocab,
             const std::string &text,
             std::vector<std::vector<Token> > &search,
             std::vector<flt_type> &fw);

void backward(const StringSet<flt_type> &vocab,
              const std::string &text,
              const std::vector<std::vector<Token> > &search,
              const std::vector<flt_type> &fw,
              std::vector<flt_type> &bw,
              std::map<std::string, flt_type> &stats);

flt_type forward_backward(const StringSet<flt_type> &vocab,
                          const std::string &text,
                          std::map<std::string, flt_type> &stats);

flt_type forward_backward(const StringSet<flt_type> &vocab,
                          const std::string &text,
                          std::map<std::string, flt_type> &stats,
                          std::vector<flt_type> &bw);

flt_type forward_backward(const std::map<std::string, flt_type> &vocab,
                          const std::string &text,
                          std::map<std::string, flt_type> &stats);

// 2-GRAM
flt_type viterbi(const transitions_t &transitions,
                 FactorGraph &text,
                 std::vector<std::string> &best_path,
                 bool reverse=true);

flt_type viterbi(const transitions_t &transitions,
                 FactorGraph &text,
                 transitions_t &stats);

flt_type forward_backward(const transitions_t &transitions,
                          FactorGraph &text,
                          transitions_t &stats);

flt_type forward_backward(const transitions_t &transitions,
                          FactorGraph &text,
                          transitions_t &stats,
                          const std::string &block);

flt_type forward_backward(const std::map<std::string, flt_type> &vocab,
                          FactorGraph &text,
                          transitions_t &stats);

void backward(const FactorGraph &text,
              std::vector<flt_type> &fw,
              std::vector<flt_type> &bw,
              transitions_t &stats);

#endif /* FACTOR_ENCODER */
