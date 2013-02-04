#include <algorithm>
#include <limits>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <queue>


class Token {
    public:
        int source;
        double cost;
        Token(): source(-1), cost(std::numeric_limits<double>::max()) {};
        Token(int src, double cst): source(src), cost(cst) {};
        Token(const Token& orig) { this->source=orig.source; this->cost=orig.cost; };
};

bool operator< (const Token& token1, const Token &token2)
{
    return token1.cost > token2.cost;
}

bool operator> (const Token& token1, const Token &token2)
{
    return token1.cost < token2.cost;
}

typedef std::priority_queue<Token> Node;


int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <vocabulary>" << std::endl;
        exit(0);
    }

    std::ifstream vocabfile(argv[1]);
    if (!vocabfile) exit(1);

    std::cerr << "Reading vocabulary " << argv[1] << std::endl;
    double count;
    int maxlen = 0;
    std::string line, word;
    std::map<std::string, double> vocab;
    while (getline(vocabfile, line)) {
        std::stringstream ss(line);
        ss >> count;
        ss >> word;
        vocab[word] = count;
        maxlen = std::max(maxlen, int(word.length()));
    }
    vocabfile.close();
    std::cerr << "\t" << "size: " << vocab.size() << std::endl;
    std::cerr << "\t" << "maximum string length: " << maxlen << std::endl;

    std::cerr << "Segmenting corpus" << std::endl;
    std::vector<std::string> best_path;
    while (getline(std::cin, line)) {
        std::cerr << "Segmenting sentence: " << line << std::endl;

        std::vector<Node> search(line.length());
        int start_pos = 0;
        int end_pos = 0;
        int len = 0;

        for (int i=0; i<line.length(); i++) {

//            std::cerr << "end index: " << i << std::endl;

            // Iterate all factor ending in this position
            for (int j=std::max(0, i-maxlen); j<=i; j++) {
                start_pos = j;
                end_pos = i+1;
                len = end_pos-start_pos;

//                std::cerr << "trying indices: " << start_pos << " " << end_pos << std::endl;
//                std::cerr << "trying factor: " << line.substr(start_pos, len) << std::endl;

                if (vocab.find(line.substr(start_pos, len)) != vocab.end()) {
                    std::cerr << "factor: " << line.substr(start_pos, len) << " cost: " << vocab[line.substr(start_pos, len)] << std::endl;
                    Token tok(j, vocab[line.substr(start_pos, len)]);
                    if (i > 0) {
                        if (search[j].size() > 0) {
                            Token source_top = search[j].top();
                            tok.cost += source_top.cost;
                        }
                    }
                    else {
                        tok.source = -1;
                    }
                    search[i].push(tok);
                }
            }
            std::cerr << std::endl;
        }

        // Look up the best path
        int target = search.size()-1;
        Token top = search[target].top();
        int source = top.source;
        while (true) {
            best_path.push_back(line.substr(source+1, target-source));
            if (source == -1) break;
            target = source;
            Token top = search[target].top();
            source = top.source;
        }

        // Print out the best path
        for (int i=best_path.size()-1; i<0; i--)
            std::cout << best_path[i] << " ";
        std::cout << best_path[0] << std::endl;

        best_path.clear();
    }

    exit(1);
}

