#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <cstring>
#include <string>
#include <vector>

#include <popt.h>

#include "defs.hh"
#include "io.hh"
#include "StringSet.hh"
#include "FactorGraph.hh"
#include "FactorEncoder.hh"
#include "Unigrams.hh"
#include "Bigrams.hh"

using namespace std;


int main(int argc, char* argv[]) {

    char *vocab_fname = NULL;
    char *trans_fname = NULL;
    string in_fname;
    string out_fname_1;
    string out_fname_2;
    int maxlen;
    map<string, flt_type> vocab;
    StringSet *ss_vocab = NULL;
    transitions_t transitions;
    flt_type one_char_min_lp = -25.0;
    bool enable_forward_backward = false;
    bool weights = false;

    poptContext pc;
    struct poptOption po[] = {
        {"vocabulary", 'v', POPT_ARG_STRING, &vocab_fname, 11001, NULL, "Vocabulary file name"},
        {"transitions", 't', POPT_ARG_STRING, &trans_fname, 11002, NULL, "Transition file name"},
        {"forward_backward", 'f', POPT_ARG_NONE, &enable_forward_backward, 11007, "Use Forward-backward segmentation instead of Viterbi", NULL},
        {"weights", 'w', POPT_ARG_NONE, &weights, 11007, "Training examples are weighted", NULL},
        POPT_AUTOHELP
        {NULL}
    };

    pc = poptGetContext(NULL, argc, (const char **)argv, po, 0);
    poptSetOtherOptionHelp(pc, "INPUT COUNTS1 COUNTS2");

    int val;
    while ((val = poptGetNextOpt(pc)) >= 0)
        continue;

    // poptGetNextOpt returns -1 when the final argument has been parsed
    // otherwise an error occured
    if (val != -1) {
        switch (val) {
        case POPT_ERROR_NOARG:
            cerr << "Argument missing for an option" << endl;
            exit(1);
        case POPT_ERROR_BADOPT:
            cerr << "Option's argument could not be parsed" << endl;
            exit(1);
        case POPT_ERROR_BADNUMBER:
        case POPT_ERROR_OVERFLOW:
            cerr << "Option could not be converted to number" << endl;
            exit(1);
        default:
            cerr << "Unknown error in option processing" << endl;
            exit(1);
        }
    }

    if (poptPeekArg(pc) != NULL)
        in_fname.assign((char*)poptGetArg(pc));
    else {
        cerr << "Input file not set" << endl;
        exit(1);
    }

    if (poptPeekArg(pc) != NULL)
        out_fname_1.assign((char*)poptGetArg(pc));
    else {
        cerr << "1-gram count file not set" << endl;
        exit(1);
    }

    if (poptPeekArg(pc) != NULL)
        out_fname_2.assign((char*)poptGetArg(pc));
    else {
        cerr << "2-gram count file not set" << endl;
        exit(1);
    }

    if (vocab_fname == NULL && trans_fname == NULL) {
        cerr << "Please define vocabulary or transitions" << endl;
        exit(0);
    }

    if (vocab_fname != NULL && trans_fname != NULL) {
        cerr << "Don't define both vocabulary and transitions" << endl;
        exit(0);
    }

    if (vocab_fname != NULL) {
        cerr << "Reading vocabulary " << vocab_fname << endl;
        int retval = Unigrams::read_vocab(vocab_fname, vocab, maxlen);
        vocab[start_end_symbol] = 0.0;
        if (retval < 0) {
            cerr << "something went wrong reading vocabulary" << endl;
            exit(0);
        }
        cerr << "\t" << "size: " << vocab.size() << endl;
        cerr << "\t" << "maximum string length: " << maxlen << endl;
        ss_vocab = new StringSet(vocab);
    }

    if (trans_fname != NULL) {
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

        for (int i=0; i<line.size(); i++) {
            string currchr {line[i]};
            if (!ss_vocab->includes(currchr))
                ss_vocab->add(currchr, one_char_min_lp);
        }

        transitions_t curr_stats;

        // Unigram model
        if (vocab_fname != NULL)
            if (enable_forward_backward) {
                FactorGraph fg(line, start_end_symbol, *ss_vocab);
                forward_backward(vocab, fg, curr_stats);
            }
            else {
                viterbi(vocab, line, curr_stats, start_end_symbol);
            }
        // Bigram model
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

    Unigrams::write_vocab(out_fname_1, unigram_stats, true);
    Bigrams::write_transitions(trans_stats, out_fname_2, true, 10);

    exit(1);
}

