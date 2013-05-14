#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

using namespace std;


bool desc_sort(pair<string, int> i, pair<string, int> j) { return (i.second > j.second); }


int get_substrings(const string &infname,
                   int maxlen,
                   map<string,int> &substrs) {

    ifstream infile(infname);
    if (!infile)
        return -1;

    int count;
    string line, word;

    while (getline(infile, line)) {
        stringstream ss(line);
        ss >> count;
        ss >> word;

        for (int i=0; i<word.length(); i++)
            for (int j=i; j<word.length() && j-i+1 <= maxlen; j++)
                substrs[word.substr(i, j-i+1)] += count;
    }

    infile.close();
    return substrs.size();
}


int main(int argc, char* argv[]) {

    if (argc < 2) {
        cerr << "usage: " << argv[0] << " <wordlist> <maxlen>" << endl;
        exit(0);
    }

    map<string, int> substrs;
    string fname(argv[1]);
    int maxlen = 100;
    if (argc > 2) maxlen = atoi(argv[2]);
    int substr_count = get_substrings(fname, maxlen, substrs);

    vector<pair<string, int> > sorted_substrs;
    for (auto it = substrs.cbegin(); it != substrs.cend(); ++it)
        sorted_substrs.push_back(make_pair(it->first, it->second));
    sort(sorted_substrs.begin(), sorted_substrs.end(), desc_sort);

    cerr << substr_count << " substrings in total up to length of " << maxlen << endl;
    for (auto it = sorted_substrs.cbegin(); it != sorted_substrs.cend(); ++it)
        cout << it->second << "\t" << it->first << endl;

    exit(1);
}

