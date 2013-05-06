#ifndef FACTOR_ENCODER
#define FACTOR_ENCODER

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
        Arc(unsigned int source_node, unsigned int target_node, double cost=0.0) : source_node(source_node),
                                                                                   target_node(target_node),
                                                                                   cost(0.0) {}
        unsigned int source_node;
        unsigned int target_node;
        double cost;
    };

    /** Node of a factor graph. */
    class Node {
    public:
        Node(int start_pos, int len) : start_pos(start_pos),
                                       len(len) { }
        ~Node() { incoming.clear(); outgoing.clear(); }
        size_t start_pos; // text indices
        size_t len;
        std::vector<Arc*> incoming;
        std::vector<Arc*> outgoing;
    };

    FactorGraph(const std::string &text, const std::map<std::string, flt_type> &vocab, int maxlen);
    FactorGraph(const std::string &text, const StringSet<flt_type> &vocab);
    ~FactorGraph();
    void get_string(const Node &node, std::string &nstr) { nstr.assign(this->text, node.start_pos, node.len); }
    std::string text;
    std::vector<Node> nodes;
    std::vector<Arc*> arcs;
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
             int maxlen,
             const std::string &start_end_symbol,
             const std::string &text,
             std::vector<std::string> &best_path,
             bool reverse=true);

#endif /* FACTOR_ENCODER */
