#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "io.hh"
#include "conf.hh"
#include "LM.hh"

using namespace std;
using namespace fsalm;


int main(int argc, char* argv[]) {

    conf::Config config;
    config("usage: strscore [OPTION...] ARPAFILE INPUT OUTPUT\n")
    ('h', "help", "", "", "display help");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 3) config.print_help(stderr, 1);

    string arpafname = config.arguments[0];
    string infname = config.arguments[1];
    string outfname = config.arguments[2];

    LM lm;
    lm.read_arpa(io::Stream(arpafname, "r").file, true);
    lm.trim();

    ifstream infile(infname);
    if (!infile) {
        cerr << "Something went wrong opening the input file." << endl;
        exit(0);
    }

    ofstream outfile(outfname);
    if (!outfile) {
        cerr << "Something went wrong opening the output file." << endl;
        exit(0);
    }

    int count;
    string line, lstr;
    while (getline(infile, line)) {
        stringstream ss(line);
        ss >> count;
        ss >> lstr;

        float total_prob = 0.0;
        int node_id = lm.initial_node_id();
        for (int i=0; i<lstr.length(); i++) {
            int sym = lm.symbol_map().index(lstr.substr(i, 1));
            float curr_prob = 0.0;
            node_id = lm.walk(node_id, sym, &curr_prob);
            total_prob += curr_prob;
        }

        outfile << total_prob << "\t" << lstr << endl;
    }

    infile.close();
    outfile.close();

    exit(1);
}

