#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

#include "defs.hh"
#include "FactorEncoder.hh"


int main(int argc, char* argv[]) {

    if (argc != 2) {
        cerr << "usage: " << argv[0] << " <vocabulary>" << endl;
        exit(0);
    }

    int maxlen;
    map<string, flt_type> vocab;
    cerr << "Reading vocabulary " << argv[1] << endl;
    int retval = read_vocab(argv[1], vocab, maxlen);
    if (retval < 0) {
        cerr << "something went wrong reading vocabulary" << endl;
        exit(0);
    }
    cerr << "\t" << "size: " << vocab.size() << endl;
    cerr << "\t" << "maximum string length: " << maxlen << endl;

    cerr << "Segmenting corpus" << endl;
    string line;
    while (getline(cin, line)) {

        vector<string> best_path;
        viterbi(vocab, maxlen, line, best_path);

        if (best_path.size() == 0) {
            cerr << "warning, no segmentation for line: " << line << endl;
            continue;
        }

        // Print out the best path
        for (unsigned int i=0; i<best_path.size()-1; i++)
            cout << best_path[i] << " ";
        cout << best_path[best_path.size()-1] << endl;
    }

    exit(1);
}

