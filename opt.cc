#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
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


void resegment_words(const std::map<std::string, long> &words,
                       const std::map<std::string, double> &vocab,
                       std::map<std::string, double> &new_freqs)
{
    int maxlen;
    for(std::map<std::string, long>::const_iterator iter = words.begin(); iter != words.end(); ++iter) {

        std::vector<std::string> best_path;
        viterbi(vocab, maxlen, iter->first, best_path);

        if (best_path.size() == 0) {
            std::cerr << "warning, no segmentation for word: " << iter->first << std::endl;
            exit(0);
        }

        // Update statistics
        for (int i=0; i<best_path.size(); i++)
            new_freqs[best_path[i]] += iter->second;

    }
}






int main(int argc, char* argv[]) {

    if (argc != 2) {
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



    while (true) {

        std::map<std::string, double> new_morph_freqs;
        resegment_words(words, vocab, new_morph_freqs);





    }

    exit(1);
}

