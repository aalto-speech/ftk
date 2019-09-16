#ifndef NGRAM_HH
#define NGRAM_HH

#include <map>
#include <string>
#include <vector>


class Ngram {
public:

    class Node {
    public:
        Node() : prob(0.0), backoff_prob(0.0), backoff_node(-1),
            first_arc(-1), last_arc(-1) { }
        float prob;
        float backoff_prob;
        int backoff_node;
        int first_arc;
        int last_arc;
    };

    Ngram() : root_node(0),
        sentence_start_node(-1),
        sentence_start_symbol_idx(-1),
        max_order(-1)
    {
        sentence_start_symbol.assign("<s>");
    };
    ~Ngram() {};
    void read_arpa(std::string arpafname);
    int score(int node_idx, int word, double &score);
    int score(int node_idx, int word, float &score);
    int order() {
        return max_order;
    };

    int root_node;
    int sentence_start_node;
    int sentence_start_symbol_idx;
    std::string sentence_start_symbol;
    std::vector<std::string> vocabulary;
    std::map<std::string, int> vocabulary_lookup;

private:

    class NgramInfo {
    public:
        NgramInfo() : prob(0.0), backoff_prob(0.0) { }
        std::vector<int> ngram;
        double prob;
        double backoff_prob;
        bool operator<(const NgramInfo &ngri) const
        {
            if (ngram.size() != ngri.ngram.size())
                throw std::string("Comparing ngrams of different order");
            for (unsigned int i=0; i<ngram.size(); i++)
                if (ngram[i] < ngri.ngram[i]) return true;
                else if (ngri.ngram[i] < ngram[i]) return false;
            throw std::string("Comparing same ngrams");
        }
    };

    int find_node(int node_idx, int word);
    int read_arpa_read_order(std::ifstream &arpafile,
                             std::vector<NgramInfo> &order_ngrams,
                             std::string &line,
                             int curr_ngram_order,
                             int &linei);
    void read_arpa_insert_order_to_tree(std::vector<NgramInfo> &order_ngrams,
                                        int &curr_node_idx,
                                        int &curr_arc_idx,
                                        int curr_order);

    std::vector<Node> nodes;
    std::vector<int> arc_words;
    std::vector<int> arc_target_nodes;
    std::map<int, int> ngram_counts_per_order;
    int max_order;
};

#endif

