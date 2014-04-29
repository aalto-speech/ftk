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
#include "EM.hh"
#include "Unigrams.hh"
#include "Bigrams.hh"

using namespace std;


int main(int argc, char* argv[]) {

    conf::Config config;
    config("usage: counts [OPTION...] INPUT COUNTS1 COUNTS2\n")
      ('h', "help", "", "", "display help")
      ('v', "vocabulary=FILE", "arg", "", "Unigram model file")
      ('t', "transitions=FILE", "arg", "", "Bigram model file")
      ('f', "forward-backward", "", "", "Use Forward-backward segmentation instead of Viterbi")
      ('w', "weights", "", "", "Training examples are weighted")
      ('8', "utf-8", "", "", "Utf-8 character encoding in use");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 3) config.print_help(stderr, 1);

    if (!config["vocabulary"].specified && !config["transitions"].specified) {
        cerr << "Please define vocabulary or transitions" << endl;
        exit(0);
    }

    if (config["vocabulary"].specified && config["transitions"].specified) {
        cerr << "Please don't define both vocabulary and transitions" << endl;
        exit(0);
    }

    string vocab_fname;
    string trans_fname;
    int maxlen;
    map<string, flt_type> vocab;
    StringSet *ss_vocab = NULL;
    transitions_t transitions;
    bool unigram = true;
    flt_type one_char_min_lp = -25.0;
    string in_fname = config.arguments[0];
    string out_fname_1 = config.arguments[1];
    string out_fname_2 = config.arguments[2];
    bool enable_forward_backward = config["forward-backward"].specified;
    bool weights = config["weights"].specified;
    bool utf8_encoding = config["utf-8"].specified;

    if (config["vocabulary"].specified) {
        vocab_fname = config["vocabulary"].get_str();
        cerr << "Reading vocabulary " << vocab_fname << endl;
        int retval = Unigrams::read_vocab(vocab_fname, vocab, maxlen, utf8_encoding);
        vocab[start_end_symbol] = 0.0;
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
        cerr << "\t" << "vocabulary: " << transitions.size() << endl;
        cerr << "\t" << "transitions: " << retval << endl;
    }

    cerr << "Segmenting corpus" << endl;
    io::Stream infile;
    try {
        infile.open(in_fname, "r");
    }
    catch (io::Stream::OpenError oe) {
        cerr << "Something went wrong opening the input." << endl;
        exit(0);
    }

    map<string, flt_type> unigram_stats;
    transitions_t trans_stats;
    char linebuffer[MAX_LINE_LEN];
    int li = 1;
    while (fgets(linebuffer, MAX_LINE_LEN, infile.file) != NULL) {

        if (li % 100000 == 0) cerr << "processing line number: " << li << endl;

        linebuffer[strlen(linebuffer)-1] = '\0';
        string line(linebuffer);
        trim(line, '\n');

        flt_type curr_weight = 1.0;
        if (weights) {
            string remainder;
            std::istringstream iss(line);
            iss >> std::skipws >> curr_weight;
            getline(iss, remainder);
            trim(remainder, '\t'); trim(remainder,' ');
            line.assign(remainder);
        }

        for (unsigned int i=0; i<line.size(); i++) {
            string currchr {line[i]};
            if (!ss_vocab->includes(currchr))
                ss_vocab->add(currchr, one_char_min_lp);
        }

        transitions_t curr_stats;

        if (unigram)
            if (enable_forward_backward) {
                FactorGraph fg(line, start_end_symbol, *ss_vocab);
                forward_backward(vocab, fg, curr_stats);
            }
            else {
                viterbi(vocab, line, curr_stats, start_end_symbol);
            }
        else {
            FactorGraph fg(line, start_end_symbol, *ss_vocab);
            if (enable_forward_backward)
                forward_backward(transitions, fg, curr_stats);
            else
                viterbi(transitions, fg, curr_stats);
        }

        Bigrams::update_trans_stats(curr_stats, curr_weight,
                                    trans_stats, unigram_stats);

        li++;
    }

    if (ss_vocab != NULL) delete ss_vocab;

    Unigrams::write_vocab(out_fname_1, unigram_stats, true, 10);
    Bigrams::write_transitions(trans_stats, out_fname_2, true, 10);

    exit(1);
}

