#include <queue>
#include <map>


class Token {
    public:
        int source;
        double cost;
        Token(): source(-1), cost(std::numeric_limits<double>::max()) {};
        Token(int src, double cst): source(src), cost(cst) {};
        Token(const Token& orig) { this->source=orig.source; this->cost=orig.cost; };
};

// Note just changed > to < ..
bool operator< (const Token& token1, const Token &token2)
{
    return token1.cost < token2.cost;
}

typedef std::priority_queue<Token> Node;


int read_vocab(const char* fname,
               std::map<std::string, double> &vocab,
               int &maxlen);


int viterbi(const std::map<std::string, double> &vocab,
            int maxlen,
            const std::string &sentence,
            std::vector<std::string> &best_path);

