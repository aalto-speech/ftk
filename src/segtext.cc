#include <cstring>

#include "conf.hh"
#include "io.hh"
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
    flt_type one_char_min_lp = -500.0;
    bool unigram = true;

    conf::Config config;
    config("usage: segtext [OPTION...] INPUT OUTPUT\n")
      ('h', "help", "", "", "display help")
      ('v', "vocabulary=FILE", "arg", "", "Unigram model file")
      ('t', "transitions=FILE", "arg", "", "Bigram model file")
      ('8', "utf-8", "", "", "Utf-8 character encoding in use");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 2) config.print_help(stderr, 1);

    bool word_boundaries = config["word-boundaries"].specified;
    bool utf8_encoding = config["utf-8"].specified;
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
        int retval = Unigrams::read_vocab(vocab_fname, vocab, maxlen, utf8_encoding);
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
        trans_fname = config["transitions"].get_str();
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
    SimpleFileInput infile(in_fname);
    SimpleFileOutput outfile(out_fname);

    string line;
    while (infile.getline(line)) {

        for (unsigned int i=0; i<line.size(); i++) {
            string currchr {line[i]};
            if (!ss_vocab->includes(currchr))
                ss_vocab->add(currchr, one_char_min_lp);
        }

        vector<string> best_path;
        if (unigram)
            viterbi(*ss_vocab, line, best_path, true, utf8_encoding);
        else {
            FactorGraph fg(line, start_end_symbol, *ss_vocab);
            viterbi(transitions, fg, best_path);
            best_path.erase(best_path.begin());
            best_path.erase(best_path.end());
        }

        if (best_path.size() == 0) {
            cerr << "warning, no segmentation for line: " << line << endl;
            continue;
        }

        for (unsigned int i=0; i<best_path.size(); i++) {
            if (best_path[i] == " ")
                outfile << "\t";
            else {
                if (i>0) outfile << " ";
                outfile << best_path[i];
            }
        }
        outfile << "\n";
    }

    outfile.close();
    if (ss_vocab != NULL) delete ss_vocab;
    exit(0);
}

