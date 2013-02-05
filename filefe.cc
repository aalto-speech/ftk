#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "factorencoder.hh"


int main(int argc, char* argv[]) {

    if (argc != 4) {
        std::cerr << "usage: " << argv[0] << " <vocabulary> <infile> <outfile>" << std::endl;
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
    std::ifstream infile(argv[2]);
    if (!infile) exit(0);
    std::ofstream outfile(argv[3]);
    if (!outfile) exit(0);
    while (getline(infile, line)) {

        std::vector<std::string> best_path;
        viterbi(vocab, maxlen, line, best_path);

        if (best_path.size() == 0) {
            std::cerr << "warning, no segmentation for line: " << line << std::endl;
            continue;
        }

        // Print out the best path
        for (int i=0; i<best_path.size()-1; i++)
            outfile << best_path[i] << " ";
        outfile << best_path[best_path.size()-1] << std::endl;
    }

    exit(1);
}

