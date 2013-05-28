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
#include "FactorEncoder.hh"
#include "Bigrams.hh"

using namespace std;


int main(int argc, char* argv[]) {

    char *vocab_fname;
    char *trans_fname;
    string in_fname;
    string out_fname;
    int maxlen;
    map<string, flt_type> vocab;
    StringSet<flt_type> *ss_vocab = NULL;
    transitions_t transitions;
    string start_end_symbol("*");
    bool enable_posterior_decoding = false;

    poptContext pc;
    struct poptOption po[] = {
        {"vocabulary", 'v', POPT_ARG_STRING, &vocab_fname, 11001, NULL, "Vocabulary file name"},
        {"transitions", 't', POPT_ARG_STRING, &trans_fname, 11002, NULL, "Transition file name"},
        {"posterior-decode", 'p', POPT_ARG_NONE, &enable_posterior_decoding, 11002, NULL, "Posterior decoding instead of Viterbi"},
        POPT_AUTOHELP
        {NULL}
    };

    pc = poptGetContext(NULL, argc, (const char **)argv, po, 0);
    poptSetOtherOptionHelp(pc, "INPUT SEGMENTED_OUTPUT");

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
        out_fname.assign((char*)poptGetArg(pc));
    else {
        cerr << "Output file not set" << endl;
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
        int retval = read_vocab(vocab_fname, vocab, maxlen);
        if (retval < 0) {
            cerr << "something went wrong reading vocabulary" << endl;
            exit(0);
        }
        cerr << "\t" << "size: " << vocab.size() << endl;
        cerr << "\t" << "maximum string length: " << maxlen << endl;
        ss_vocab = new StringSet<flt_type>(vocab);
    }

    if (trans_fname != NULL) {
        cerr << "Reading transitions " << trans_fname << endl;
        int retval = Bigrams::read_transitions(transitions, trans_fname);
        Bigrams::trans_to_vocab(transitions, vocab);
        ss_vocab = new StringSet<flt_type>(vocab);
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

    char linebuffer[8192];
    while (fgets(linebuffer, 8192 , infile.file) != NULL) {

        linebuffer[strlen(linebuffer)-1] = '\0';
        string line(linebuffer);
        vector<string> best_path;
        if (vocab_fname != NULL)
            if (enable_posterior_decoding)
                cerr << "Posterior decoding not implemented yet for unigrams." << endl;
            else
                viterbi(*ss_vocab, line, best_path);
        else {
            FactorGraph fg(line, start_end_symbol, vocab);
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
        for (unsigned int i=0; i<best_path.size()-1; i++) {
            fwrite(best_path[i].c_str(), 1, best_path[i].length(), outfile.file);
            fwrite(" ", 1, 1, outfile.file);
        }
        fwrite(best_path[best_path.size()-1].c_str(), 1, best_path[best_path.size()-1].length(), outfile.file);
        fwrite("\n", 1, 1, outfile.file);
    }

    if (ss_vocab != NULL) delete ss_vocab;

    exit(1);
}

