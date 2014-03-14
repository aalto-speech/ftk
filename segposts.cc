#include <cmath>
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
    bool unigram = true;

    conf::Config config;
    config("usage: segposts [OPTION...] INPUT SEGPROBS_OUTPUT\n")
      ('h', "help", "", "", "display help")
      ('v', "vocabulary=FILE", "arg", "", "Unigram model file")
      ('t', "transitions=FILE", "arg", "", "Bigram model file");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 2) config.print_help(stderr, 1);

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
        vocab_fname = config["transitions"].get_str();
        cerr << "Reading transitions " << trans_fname << endl;
        int retval = Bigrams::read_transitions(transitions, trans_fname);
        Bigrams::trans_to_vocab(transitions, vocab);
        ss_vocab = new StringSet(vocab);
        if (retval < 0) {
            cerr << "something went wrong reading transitions" << endl;
            exit(0);
        }
        cerr << "\t" << "vocabulary: " << transitions.size() << endl;
        cerr << "\t" << "transitions: " << retval << endl;
    }

    cerr << "Segmenting corpus" << endl;
    io::Stream infile, outfile;
    try {
        infile.open(in_fname, "r");
        outfile.open(out_fname, "w");
    }
    catch (io::Stream::OpenError oe) {
        cerr << "Something went wrong opening the files." << endl;
        exit(0);
    }

    char linebuffer[MAX_LINE_LEN];
    while (fgets(linebuffer, MAX_LINE_LEN, infile.file) != NULL) {

        linebuffer[strlen(linebuffer)-1] = '\0';
        string line(linebuffer);
        map<string, flt_type> ug_stats;
        transitions_t bg_stats;
        vector<flt_type> post_scores;

        if (unigram)
            forward_backward(*ss_vocab, line, ug_stats, post_scores);
        else {
            FactorGraph fg(line, start_end_symbol, *ss_vocab);
            forward_backward(transitions, fg, bg_stats, post_scores);
        }

        fprintf(outfile.file, "%s\t", line.c_str());
        for (unsigned int i=0; i<post_scores.size()-1; i++)
            fprintf(outfile.file, "%f ", post_scores[i]);
        fprintf(outfile.file, "\n");
    }

    if (ss_vocab != NULL) delete ss_vocab;

    exit(1);
}
