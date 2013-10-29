#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <cstring>
#include <string>
#include <vector>

#include "defs.hh"
#include "conf.hh"
#include "io.hh"
#include "StringSet.hh"
#include "FactorGraph.hh"
#include "FactorEncoder.hh"
#include "Unigrams.hh"
#include "Bigrams.hh"

using namespace std;


int main(int argc, char* argv[]) {

    string vocab_fname;
    string trans_fname;
    int maxlen;
    map<string, flt_type> vocab;
    StringSet *ss_vocab = NULL;
    transitions_t transitions;
    flt_type one_char_min_lp = -25.0;
    bool unigram = true;

    conf::Config config;
    config("usage: fe [OPTION...] INPUT OUTPUT\n")
      ('h', "help", "", "", "display help")
      ('v', "vocabulary=FILE", "arg", "", "Unigram model file")
      ('t', "transitions=FILE", "arg", "", "Bigram model file")
      ('p', "posterior-decode", "", "", "Posterior decoding instead of Viterbi")
      ('s', "sentence-markers", "", "", "Print sentence begin and end symbols");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 2) config.print_help(stderr, 1);

    bool print_sentence_markers = config["sentence-markers"].specified;
    bool enable_posterior_decoding = config["posterior-decode"].specified;
    string in_fname = config.arguments[0];
    string out_fname = config.arguments[1];

    if (!config["vocabulary"].specified && !config["transitions"].specified) {
        cerr << "Please define vocabulary or transitions" << endl;
        exit(0);
    }

    if (config["vocabulary"].specified && config["transitions"].specified) {
        cerr << "Please don't define both vocabulary and transitions" << endl;
        exit(0);
    }

    if (config["vocabulary"].specified) {
        vocab_fname = config["vocabulary"].get_str();
        cerr << "Reading vocabulary " << vocab_fname << endl;
        int retval = Unigrams::read_vocab(vocab_fname, vocab, maxlen);
        if (retval < 0) {
            cerr << "something went wrong reading vocabulary" << endl;
            exit(0);
        }
        cerr << "\t" << "size: " << vocab.size() << endl;
        cerr << "\t" << "maximum string length: " << maxlen << endl;
        ss_vocab = new StringSet(vocab);
    }

    if (config["transitions"].specified) {
        unigram = false;
        trans_fname = config["vocabulary"].get_str();
        cerr << "Reading transitions " << trans_fname << endl;
        int retval = Bigrams::read_transitions(transitions, trans_fname);
        Bigrams::trans_to_vocab(transitions, vocab);
        ss_vocab = new StringSet(vocab);
        if (retval < 0) {
            cerr << "something went wrong reading transitions" << endl;
            exit(0);
        }
        cerr << "\t" << "vocabulary size: " << transitions.size() << endl;
        cerr << "\t" << "transitions: " << retval << endl;
    }

    cerr << "Segmenting corpus" << endl;
    io::Stream infile, outfile;
    try {
        infile.open(in_fname, "r");
        outfile.open(out_fname, "w");
    }
    catch (io::Stream::OpenError &oe) {
        cerr << "Something went wrong opening the files." << endl;
        exit(0);
    }

    char linebuffer[MAX_LINE_LEN];
    while (fgets(linebuffer, MAX_LINE_LEN , infile.file) != NULL) {

        linebuffer[strlen(linebuffer)-1] = '\0';
        string line(linebuffer);

        for (int i=0; i<line.size(); i++) {
            string currchr {line[i]};
            if (!ss_vocab->includes(currchr))
                ss_vocab->add(currchr, one_char_min_lp);
        }

        vector<string> best_path;
        if (unigram)
            if (enable_posterior_decoding) {
                cerr << "Posterior decoding not implemented yet for unigrams." << endl;
                exit(0);
            }
            else
                viterbi(*ss_vocab, line, best_path);
        else {
            FactorGraph fg(line, start_end_symbol, *ss_vocab);
            if (enable_posterior_decoding)
                posterior_decode(transitions, fg, best_path);
            else
                viterbi(transitions, fg, best_path);
            best_path.erase(best_path.begin());
            best_path.erase(best_path.end());
        }

        if (best_path.size() == 0) {
            cerr << "warning, no segmentation for line: " << line << endl;
            continue;
        }

        // Print out the best path
        if (print_sentence_markers) fwrite("<s> ", 1, 4, outfile.file);
        for (unsigned int i=0; i<best_path.size()-1; i++) {
            fwrite(best_path[i].c_str(), 1, best_path[i].length(), outfile.file);
            fwrite(" ", 1, 1, outfile.file);
        }
        fwrite(best_path[best_path.size()-1].c_str(), 1, best_path[best_path.size()-1].length(), outfile.file);
        if (print_sentence_markers) fwrite(" </s>", 1, 5, outfile.file);
        fwrite("\n", 1, 1, outfile.file);
    }

    if (ss_vocab != NULL) delete ss_vocab;

    exit(1);
}

