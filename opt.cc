#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "factorencoder.hh"


int read_words(const char* fname,
                 std::map<std::string, long> &words)
{
    std::ifstream vocabfile(fname);
    if (!vocabfile) return -1;

    std::string line, word;
    long count;
    while (getline(vocabfile, line)) {
        std::stringstream ss(line);
        ss >> count;
        ss >> word;
        words[word] = count;
    }
    vocabfile.close();

    return words.size();
}


double resegment_words(const std::map<std::string, long> &words,
                          const std::map<std::string, double> &vocab,
                          std::map<std::string, double> &new_freqs,
                          const int maxlen)
{
    for(std::map<std::string, long>::const_iterator iter = words.begin(); iter != words.end(); ++iter) {

        std::vector<std::string> best_path;
        viterbi(vocab, maxlen, iter->first, best_path);

        if (best_path.size() == 0) {
            std::cerr << "warning, no segmentation for word: " << iter->first << std::endl;
            exit(0);
        }

        // Update statistics
        for (int i=0; i<best_path.size(); i++)
            new_freqs[best_path[i]] += double(iter->second);
    }

    double total = 0.0;
    for(std::map<std::string, double>::const_iterator iter = new_freqs.begin(); iter != new_freqs.end(); ++iter) {
        total += iter->second;
    }

    return total;
}


double get_cost(const std::map<std::string, double> &vocab,
                  double densum)
{
    double total = 0.0;
    densum = log2(densum);
    for(std::map<std::string, double>::const_iterator iter = vocab.begin(); iter != vocab.end(); ++iter) {
        total += iter->second * (log2(iter->second)-densum);
    }
    return total;
}


int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " <vocabulary> <words>" << std::endl;
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

    std::cerr << "Reading word list" << std::endl;
    std::map<std::string, long> words;
    read_words(argv[2], words);

    std::cerr << "Segmenting words" << std::endl;
    while (true) {
        std::map<std::string, double> new_morph_freqs;
        double densum = resegment_words(words, vocab, new_morph_freqs, maxlen);
        double cost = get_cost(new_morph_freqs, densum);
        std::cout << "initial cost: " << cost << std::endl;
        break;
    }

    exit(1);
}

