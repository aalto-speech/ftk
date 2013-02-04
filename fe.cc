#include <algorithm>
#include <limits>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "fe.hh"


int read_vocab(const char* fname,
               std::map<std::string, double> &vocab,
               int &maxlen)
{
    std::ifstream vocabfile(fname);
    if (!vocabfile) return -1;

    std::string line, word;
    double count;
    while (getline(vocabfile, line)) {
        std::stringstream ss(line);
        ss >> count;
        ss >> word;
        vocab[word] = count;
        maxlen = std::max(maxlen, int(word.length()));
    }
    vocabfile.close();

    return vocab.size();
}



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
    std::vector<std::string> best_path;
    while (getline(std::cin, line)) {

        std::vector<Node> search(line.length());
        int start_pos = 0;
        int end_pos = 0;
        int len = 0;

        for (int i=0; i<line.length(); i++) {

            // Iterate all factors ending in this position
            for (int j=std::max(0, i-maxlen); j<=i; j++) {
                start_pos = j;
                end_pos = i+1;
                len = end_pos-start_pos;

                if (vocab.find(line.substr(start_pos, len)) != vocab.end()) {
                    Token tok(j-1, vocab[line.substr(start_pos, len)]);
                    if (i > 0 && search[j].size() > 0) {
                        Token source_top = search[j].top();
                        tok.cost += source_top.cost;
                    }
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
            if (source < 0) break; 
            target = source;
            Token top = search[target].top();
            source = top.source;
        }

        // Print out the best path
        for (int i=best_path.size()-1; i>0; i--)
            std::cout << best_path[i] << " ";
        std::cout << best_path[0] << std::endl;


        best_path.clear();
    }

    exit(1);
}

