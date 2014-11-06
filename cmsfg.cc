#include <sstream>

#include "conf.hh"
#include "Unigrams.hh"

using namespace std;


int main(int argc, char* argv[]) {

    conf::Config config;
    config("usage: cmsfg [OPTION...] WORDLIST VOCAB MSFG\n")
      ('h', "help", "", "", "display help")
      ('n', "no-lookahead", "", "", "don't use lookahead, uses less memory but much slower")
      ('t', "temp-graphs=INT", "arg", "0", "Write out intermediate graphs for #G mod INT == 0")
      ('8', "utf-8", "", "", "Utf-8 character encoding in use");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 3) config.print_help(stderr, 1);

    string wordlist_fname = config.arguments[0];
    string vocab_fname = config.arguments[1];
    string msfg_fname = config.arguments[2];
    unsigned int temp_graph_interval = config["temp-graphs"].get_int();
    bool lookahead = !config["no-lookahead"].specified;
    bool utf8_encoding = config["utf-8"].specified;

    cerr << std::boolalpha;
    cerr << "parameters, wordlist: " << wordlist_fname << endl;
    cerr << "parameters, initial vocabulary: " << vocab_fname << endl;
    cerr << "parameters, msfg to write: " << msfg_fname << endl;
    cerr << "parameters, lookahead: " << lookahead << endl;
    if (temp_graph_interval > 0)
        cerr << "parameters, write intermediate graphs whenever #V modulo " << temp_graph_interval << " == 0" << endl;
    else
        cerr << "parameters, write intermediate graphs: NO" << endl;
    cerr << "parameters, utf-8 encoding: " << utf8_encoding << endl;

    int maxlen, word_maxlen;
    map<string, flt_type> vocab;
    map<string, flt_type> words;

    cerr << "Reading vocabulary " << vocab_fname << endl;
    int retval = Unigrams::read_vocab(vocab_fname, vocab, maxlen, utf8_encoding);
    if (retval < 0) {
        cerr << "something went wrong reading vocabulary" << endl;
        exit(0);
    }
    cerr << "\t" << "vocabulary size: " << vocab.size() << endl;
    cerr << "\t" << "maximum string length: " << maxlen << endl;

    cerr << "Reading word list " << wordlist_fname << endl;
    retval = Unigrams::read_vocab(wordlist_fname, words, word_maxlen, utf8_encoding);
    if (retval < 0) {
        cerr << "something went wrong reading word list" << endl;
        exit(0);
    }
    cerr << "\t" << "wordlist size: " << words.size() << endl;
    cerr << "\t" << "maximum word length: " << word_maxlen << endl;

    StringSet ss_vocab(vocab);
    MultiStringFactorGraph msfg(start_end_symbol);
    vocab[start_end_symbol] = 0.0;

    unsigned int num_separate_nodes = 0;
    unsigned int num_separate_arcs = 0;
    unsigned int curr_word_idx = 0;
    for (auto it = words.cbegin(); it != words.cend(); ++it) {
        FactorGraph fg(it->first, start_end_symbol, ss_vocab);
        msfg.add(fg, lookahead);
        num_separate_nodes += fg.nodes.size();
        num_separate_arcs += fg.arcs.size();
        curr_word_idx++;
        if (curr_word_idx % 10000 == 0) cerr << "... processing word " << curr_word_idx << endl;
        if (temp_graph_interval > 0 && curr_word_idx % temp_graph_interval == 0) {
            stringstream tempfname;
            tempfname << msfg_fname << "." << curr_word_idx;
            cerr << "... writing intermediate graph to file " << tempfname.str() << endl;
            msfg.write(tempfname.str());
        }
    }

    msfg.write(msfg_fname);

    cerr << "factor graph strings: " << msfg.string_end_nodes.size() << endl;
    cerr << "factor graph nodes: " << msfg.nodes.size() << endl;
    cerr << "nodes for separate fgs: " << num_separate_nodes << endl;
    cerr << "arcs for separate fgs: " << num_separate_arcs << endl;

    exit(0);
}
