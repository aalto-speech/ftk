#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <utility>

#include "defs.hh"
#include "io.hh"
#include "conf.hh"
#include "LM.hh"

using namespace std;
using namespace fsalm;


int main(int argc, char* argv[]) {

    conf::Config config;
    config("usage: strscore [OPTION...] ARPAFILE INPUT OUTPUT\n")
    ('h', "help", "", "", "display help")
    ('8', "utf-8", "", "", "Utf-8 character encoding in use");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 3) config.print_help(stderr, 1);

    string arpafname = config.arguments[0];
    string infname = config.arguments[1];
    string outfname = config.arguments[2];
    bool utf8_encoding = config["utf-8"].specified;

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
    vector<pair<string, float> > scores;
    float normalizer = SMALL_LP;
    while (getline(infile, line)) {
        stringstream ss(line);
        ss >> count;
        ss >> lstr;

        float total_prob = 0.0;
        int node_id = lm.empty_node_id();

        vector<unsigned int> char_positions;
        get_character_positions(lstr, char_positions, utf8_encoding);
        for (unsigned int i=0; i<char_positions.size()-1; i++) {
            unsigned int start_pos = char_positions[i];
            unsigned int len = char_positions[i+1] - start_pos;
            int sym = lm.symbol_map().index(lstr.substr(start_pos, len));
            float curr_prob = 0.0;
            node_id = lm.walk(node_id, sym, &curr_prob);
            total_prob += curr_prob;
        }

        total_prob *= log(10.0); // Convert from log10 (ARPA default) to ln
        normalizer = add_log_domain_probs(normalizer, total_prob);
        scores.push_back(make_pair(lstr, total_prob));
    }

    for (auto it=scores.begin(); it != scores.end(); ++it)
        outfile << it->second-normalizer << "\t" << it->first << endl;

    infile.close();
    outfile.close();

    exit(1);
}

