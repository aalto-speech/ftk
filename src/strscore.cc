#include <fstream>
#include <iostream>
#include <sstream>

#include "defs.hh"
#include "conf.hh"
#include "Ngram.hh"

using namespace std;


int main(int argc, char* argv[]) {

    conf::Config config;
    config("usage: strscore [OPTION...] ARPAFILE INPUT OUTPUT\n")
    ('8', "utf-8", "", "", "Utf-8 character encoding in use")
    ('h', "help", "", "", "display help");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 3) config.print_help(stderr, 1);

    string arpafname = config.arguments[0];
    string infname = config.arguments[1];
    string outfname = config.arguments[2];
    bool utf8_encoding = config["utf-8"].specified;

    Ngram lm;
    lm.read_arpa(arpafname);

    ifstream infile(infname);
    if (!infile) {
        cerr << "Something went wrong opening the input file." << endl;
        exit(1);
    }

    ofstream outfile(outfname);
    if (!outfile) {
        cerr << "Something went wrong opening the output file." << endl;
        exit(1);
    }

    float count;
    string line, lstr;
    vector<pair<string, float> > scores;
    float normalizer = SMALL_LP;
    while (getline(infile, line)) {
        stringstream ss(line);
        ss >> count;
        ss >> lstr;

        float total_prob = 0.0;
        int node_id = lm.root_node;

        vector<unsigned int> char_positions;
        get_character_positions(lstr, char_positions, utf8_encoding);
        for (unsigned int i=0; i<char_positions.size()-1; i++) {
            unsigned int start_pos = char_positions[i];
            unsigned int end_pos = char_positions[i+1];
            int sym = lm.vocabulary_lookup[lstr.substr(start_pos, end_pos-start_pos)];
            node_id = lm.score(node_id, sym, total_prob);
        }

        total_prob *= log(10.0); // Convert from log10 (ARPA default) to ln
        normalizer = add_log_domain_probs(normalizer, total_prob);
        scores.push_back(make_pair(lstr, total_prob));
    }

    for (auto it=scores.begin(); it != scores.end(); ++it)
        outfile << it->second-normalizer << "\t" << it->first << endl;

    infile.close();
    outfile.close();

    exit(0);
}

