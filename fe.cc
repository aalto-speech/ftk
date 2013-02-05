#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "factorencoder.hh"


int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <vocabulary>" << std::endl;
        exit(0);
    }

    int maxlen;
    std::map<std::string, double> vocab;
    std::cerr << "Reading vocabulary " << argv[1] << std::endl;
    int retval = read_vocab(argv[1], vocab, maxlen);
    if (retval < 0) {
        std::cerr << "something went wrong reading vocabulary" << std::endl;
        exit(0);
    }
    std::cerr << "\t" << "size: " << vocab.size() << std::endl;
    std::cerr << "\t" << "maximum string length: " << maxlen << std::endl;

    std::cerr << "Segmenting corpus" << std::endl;
    std::string line;
    while (getline(std::cin, line)) {

        std::vector<std::string> best_path;
        viterbi(vocab, maxlen, line, best_path);

        if (best_path.size() == 0) {
            std::cerr << "warning, no segmentation for line: " << line << std::endl;
            continue;
        }

        // Print out the best path
        for (int i=0; i<best_path.size()-1; i++)
            std::cout << best_path[i] << " ";
        std::cout << best_path[best_path.size()-1] << std::endl;
    }

    exit(1);
}

