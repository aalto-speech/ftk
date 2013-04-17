#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

using namespace std;

#include "factorencoder.hh"
#include "io.hh"


int main(int argc, char* argv[]) {

    if (argc != 4) {
        cerr << "usage: " << argv[0] << " <vocabulary> <infile> <outfile>" << endl;
        exit(0);
    }

    int maxlen;
    map<string, double> vocab;
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
    io::Stream infile, outfile;
    try {
        infile.open(argv[2], "r");
        outfile.open(argv[3], "w");
    }
    catch (io::Stream::OpenError oe) {
        cerr << "Something went wrong opening the files." << endl;
        exit(0);
    }

    char linebuffer[8192];
    while (fgets(linebuffer, 8192 , infile.file) != NULL) {

        linebuffer[strlen(linebuffer)-1] = '\0';
        string line(linebuffer);
        vector<string> best_path;
        viterbi(vocab, maxlen, line, best_path);

        if (best_path.size() == 0) {
            cerr << "warning, no segmentation for line: " << line << endl;
            continue;
        }

        // Print out the best path
        for (int i=0; i<best_path.size()-1; i++) {
            fwrite(best_path[i].c_str(), 1, best_path[i].length(), outfile.file);
            fwrite(" ", 1, 1, outfile.file);
        }
        fwrite(best_path[best_path.size()-1].c_str(), 1, best_path[best_path.size()-1].length(), outfile.file);
        fwrite("\n", 1, 1, outfile.file);
    }

    infile.close();
    outfile.close();

    exit(1);
}
