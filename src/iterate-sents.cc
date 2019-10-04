#include <sstream>

#include "conf.hh"
#include "Unigrams.hh"

using namespace std;


int main(int argc, char* argv[]) {

    flt_type one_char_min_lp = -25.0;

    conf::Config config;
    config("usage: iterate-sents [OPTION...] CORPUS IN_VOCAB OUT_VOCAB\n")
      ('h', "help", "", "", "display help")
      ('i', "iterations=INT", "arg", "5", "Number of iterations")
      ('f', "forward-backward", "", "", "Use Forward-backward segmentation instead of Viterbi")
      ('t', "temp-vocabs", "", "", "Write out vocabulary after each iteration")
      ('8', "utf-8", "", "", "Utf-8 character encoding in use");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 3) config.print_help(stderr, 1);

    int num_iterations = config["iterations"].get_int();
    bool enable_forward_backward = config["forward-backward"].specified;
    bool write_temp_vocabs = config["temp-vocabs"].specified;
    bool utf8_encoding = config["utf-8"].specified;
    string training_fname = config.arguments[0];
    string vocab_in_fname = config.arguments[1];
    string vocab_out_fname = config.arguments[2];

    cerr << std::boolalpha;
    cerr << "parameters, training data: " << training_fname << endl;
    cerr << "parameters, initial vocabulary: " << vocab_in_fname << endl;
    cerr << "parameters, final vocabulary: " << vocab_out_fname << endl;
    cerr << "parameters, use forward-backward: " << enable_forward_backward << endl;
    cerr << "parameters, number of iterations: " << num_iterations << endl;
    cerr << "parameters, write vocabulary after each iteration: " << write_temp_vocabs << endl;
    cerr << "parameters, utf-8 encoding: " << utf8_encoding << endl;

    int maxlen;
    set<string> all_chars;
    map<string, flt_type> vocab;
    map<string, flt_type> freqs;
    vector<string> sents;

    cerr << "Reading vocabulary " << vocab_in_fname << endl;
    int retval = Unigrams::read_vocab(vocab_in_fname, vocab, maxlen, utf8_encoding);
    if (retval < 0) {
        cerr << "something went wrong reading vocabulary" << endl;
        exit(EXIT_FAILURE);
    }
    cerr << "\t" << "size: " << vocab.size() << endl;
    cerr << "\t" << "maximum string length: " << maxlen << endl;
    find_short_factors(vocab, all_chars, 2, utf8_encoding);

    Unigrams gg;
    if (enable_forward_backward)
        gg.set_segmentation_method(forward_backward);
    else
        gg.set_segmentation_method(viterbi);
    gg.set_utf8(utf8_encoding);

    cerr << "Reading training corpus " << training_fname << endl;
    Unigrams::read_sents(training_fname, sents);

    cerr << "iterating.." << endl;
    for (int i=0; i<num_iterations; i++) {
        flt_type cost = gg.resegment_sents(sents, vocab, freqs);
        cerr << "likelihood: " << cost << endl;
        vocab = freqs;
        Unigrams::freqs_to_logprobs(vocab);
        assert_factors(vocab, all_chars, one_char_min_lp);

        if (write_temp_vocabs) {
            ostringstream tempfname;
            tempfname << vocab_out_fname << ".iter" << i+1;
            Unigrams::write_vocab(tempfname.str(), vocab);
        }
    }

    Unigrams::write_vocab(vocab_out_fname, vocab);

    exit(EXIT_SUCCESS);
}

