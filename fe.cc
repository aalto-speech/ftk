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

    std::vector<Node> search;
    while (getline(std::cin, line)) {
        search.resize(line.length());

    }

    exit(0);
}

