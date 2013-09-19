#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <popt.h>

#include "defs.hh"
#include "StringSet.hh"
#include "FactorEncoder.hh"
#include "Unigrams.hh"

using namespace std;


void assert_single_chars(map<string, flt_type> &vocab,
                         const map<string, flt_type> &chars,
                         flt_type val)
{
    for (auto it = chars.cbegin(); it != chars.cend(); ++it)
        if (vocab.find(it->first) == vocab.end())
            vocab[it->first] = val;
}


int main(int argc, char* argv[]) {

    bool enable_forward_backward = false;
    bool write_temp_vocabs = false;
    int num_iterations = 5;
    flt_type one_char_min_lp = -25.0;
    string vocab_in_fname;
    string vocab_out_fname;
    string training_fname;

    // Popt documentation:
    // http://linux.die.net/man/3/popt
    // http://privatemisc.blogspot.fi/2012/12/popt-basic-example.html
    poptContext pc;
    struct poptOption po[] = {
        {"iterations", 'i', POPT_ARG_INT, &num_iterations, 11003, NULL, "Number of iterations"},
        {"forward_backward", 'f', POPT_ARG_NONE, &enable_forward_backward, 11007, "Use Forward-backward segmentation instead of Viterbi", NULL},
        {"temp_vocabs", 't', POPT_ARG_NONE, &write_temp_vocabs, 11007, "Write out vocabulary after each iteration", NULL},
        POPT_AUTOHELP
        {NULL}
    };

    pc = poptGetContext(NULL, argc, (const char **)argv, po, 0);
    poptSetOtherOptionHelp(pc, "[INITIAL_VOCABULARY] [WORDLIST] [FINAL_VOCAB]");

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

    // Handle ARG part of command line
    if (poptPeekArg(pc) != NULL)
        vocab_in_fname.assign((char*)poptGetArg(pc));
    else {
        cerr << "Initial vocabulary file not set" << endl;
        exit(1);
    }

    if (poptPeekArg(pc) != NULL)
        training_fname.assign((char*)poptGetArg(pc));
    else {
        cerr << "Wordlist file not set" << endl;
        exit(1);
    }

    if (poptPeekArg(pc) != NULL)
        vocab_out_fname.assign((char*)poptGetArg(pc));
    else {
        cerr << "Output vocabulary file not set" << endl;
        exit(1);
    }

    cerr << "parameters, initial vocabulary: " << vocab_in_fname << endl;
    cerr << "parameters, training data: " << training_fname << endl;
    cerr << "parameters, final vocabulary: " << vocab_out_fname << endl;
    cerr << "parameters, use forward-backward: " << enable_forward_backward << endl;
    cerr << "parameters, number of iterations: " << num_iterations << endl;
    cerr << "parameters, write vocabulary after each iteration: " << write_temp_vocabs << endl;

    int maxlen;
    map<string, flt_type> all_chars;
    map<string, flt_type> vocab;
    map<string, flt_type> freqs;
    vector<string> sents;

    cerr << "Reading vocabulary " << vocab_in_fname << endl;
    int retval = Unigrams::read_vocab(vocab_in_fname, vocab, maxlen);
    if (retval < 0) {
        cerr << "something went wrong reading vocabulary" << endl;
        exit(0);
    }
    cerr << "\t" << "size: " << vocab.size() << endl;
    cerr << "\t" << "maximum string length: " << maxlen << endl;
    for (auto it = vocab.cbegin(); it != vocab.end(); ++it)
        if (it->first.length() == 1)
            all_chars[it->first] = 0.0;

    Unigrams gg;
    if (enable_forward_backward)
        gg.set_segmentation_method(forward_backward);
    else
        gg.set_segmentation_method(viterbi);

    cerr << "Reading training corpus " << training_fname << endl;
    Unigrams::read_sents(training_fname, sents);

    cerr << "iterating.." << endl;
    for (int i=0; i<num_iterations; i++) {
        flt_type cost = gg.resegment_sents(sents, vocab, freqs);
        cerr << "cost: " << cost << endl;
        vocab = freqs;
        Unigrams::freqs_to_logprobs(vocab);
        assert_single_chars(vocab, all_chars, one_char_min_lp);

        if (write_temp_vocabs) {
            ostringstream tempfname;
            tempfname << vocab_out_fname << ".iter" << i+1;
            Unigrams::write_vocab(tempfname.str(), vocab);
        }
    }

    Unigrams::write_vocab(vocab_out_fname, vocab);

    exit(1);
}

