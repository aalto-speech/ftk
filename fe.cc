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

    double count;
    std::string line, word;
    std::map<std::string, double> vocab;
    while (getline(vocabfile, line)) {
        std::stringstream ss(line);
        ss >> count;
        ss >> word;
        vocab[word] = count;
    }
    vocabfile.close();

    std::vector<std::string> best_path;
    while (getline(std::cin, line)) {
        std::vector<Node> search(line.length());

        for (int i=0; i<line.length(); i++) {

            // Possible factors starting from the beginning of the line
            if (vocab.find(line.substr(0, i+1)) != vocab.end()) {
                Token tok;
                tok.cost = vocab[line.substr(0, i+1)];
                search[i].push(tok);
            }

            // Factors originating from other positions
            for (int j=0; j<i; j++) {
                if (vocab.find(line.substr(j+1, i-j)) != vocab.end()) {
                    Token tok;
                    tok.source  = j;
                    tok.cost = vocab[line.substr(j+1, i-j)];
                    search[i].push(tok);
                }
            }
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

    exit(0);
}

