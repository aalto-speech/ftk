#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "conf.hh"
#include "defs.hh"

using namespace std;


bool desc_sort(pair<string, int> i, pair<string, int> j) { return (i.second > j.second); }


int get_substrings(const string &infname,
                   unsigned int maxlen,
                   map<string,int> &substrs,
                   bool utf8) {

    ifstream infile(infname);
    if (!infile) {
        cerr << "Something went wrong opening the input file." << endl;
        exit(0);
    }

    int count;
    string line, word;

    while (getline(infile, line)) {
        stringstream ss(line);
        ss >> count;
        ss >> word;

        vector<unsigned int> char_positions;
        get_character_positions(word, char_positions, utf8);

        for (unsigned int i=0; i<char_positions.size()-1; i++)
            for (unsigned int j=i+1; j<char_positions.size() && j-i <= maxlen; j++) {
                unsigned int start_pos = char_positions[i];
                unsigned int end_pos = char_positions[j];
                substrs[word.substr(start_pos, end_pos-start_pos)] += count;
            }
    }

    infile.close();
    return substrs.size();
}


void write_substrings(const string &outfname,
                      vector<pair<string, int> > &sorted_substrs)
{
    ofstream outfile(outfname);
    if (!outfile) {
        cerr << "Something went wrong opening the output file." << endl;
        exit(0);
    }

    for (auto it = sorted_substrs.cbegin(); it != sorted_substrs.cend(); ++it)
        outfile << it->second << "\t" << it->first << endl;

    outfile.close();
}


int main(int argc, char* argv[]) {

    conf::Config config;
    config("usage: substrings [OPTION...] INPUT OUTPUT\n")
      ('h', "help", "", "", "display help")
      ('8', "utf-8", "", "", "Utf-8 character encoding in use")
      ('l', "max-length=INT", "arg must", "100", "Maximum length of the substrings");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 2) config.print_help(stderr, 1);

    string infname = config.arguments[0];
    string outfname = config.arguments[1];
    unsigned int maxlen = config["max-length"].get_int();
    bool utf8_encoding = config["utf-8"].specified;

    map<string, int> substrs;
    int substr_count = get_substrings(infname, maxlen, substrs, utf8_encoding);

    vector<pair<string, int> > sorted_substrs;
    for (auto it = substrs.cbegin(); it != substrs.cend(); ++it)
        sorted_substrs.push_back(make_pair(it->first, it->second));
    sort(sorted_substrs.begin(), sorted_substrs.end(), desc_sort);

    cerr << substr_count << " substrings in total up to length of " << maxlen << endl;
    write_substrings(outfname, sorted_substrs);

    exit(0);
}

