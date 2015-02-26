#include <iomanip>
#include <sstream>

#include "conf.hh"
#include "Unigrams.hh"
#include "Bigrams.hh"

using namespace std;


void floor_values(map<string, flt_type> &vocab,
                  flt_type floor_val)
{
    for (auto it = vocab.begin(); it != vocab.end(); ++it)
        if (it->second < floor_val) it->second = floor_val;
}


int main(int argc, char* argv[]) {

    conf::Config config;
    config("usage: iterate12 [OPTION...] WORDLIST VOCAB_INIT MSFG_IN TRANSITIONS_OUT\n")
      ('h', "help", "", "", "display help")
      ('i', "iterations=INT", "arg", "5", "Number of iterations")
      ('f', "forward-backward", "", "", "Use Forward-backward segmentation instead of Viterbi")
      ('8', "utf-8", "", "", "Utf-8 character encoding in use");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 4) config.print_help(stderr, 1);

    int num_iterations = config["iterations"].get_int();
    bool enable_forward_backward = config["forward-backward"].specified;
    bool utf8_encoding = config["utf-8"].specified;
    string wordlist_fname = config.arguments[0];
    string vocab_in_fname = config.arguments[1];
    string msfg_fname = config.arguments[2];
    string transitions_out_fname = config.arguments[3];

    cerr << std::boolalpha;
    cerr << "parameters, wordlist: " << wordlist_fname << endl;
    cerr << "parameters, initial vocabulary: " << vocab_in_fname << endl;
    cerr << "parameters, msfg: " << msfg_fname << endl;
    cerr << "parameters, final model: " << transitions_out_fname << endl;
    cerr << "parameters, use forward-backward: " << enable_forward_backward << endl;
    cerr << "parameters, number of iterations: " << num_iterations << endl;
    cerr << "parameters, utf-8 encoding: " << utf8_encoding << endl;

    int word_maxlen, subword_maxlen;
    set<string> all_chars;
    map<string, flt_type> vocab;
    map<string, flt_type> words;
    MultiStringFactorGraph msfg(start_end_symbol);
    transitions_t transitions;
    transitions_t trans_stats;
    map<string, flt_type> unigram_stats;

    cerr << "Reading initial vocabulary " << vocab_in_fname << endl;
    int retval = Unigrams::read_vocab(vocab_in_fname, vocab, subword_maxlen, utf8_encoding);
    if (retval < 0) {
        cerr << "something went wrong reading initial vocabulary" << endl;
        exit(0);
    }
    cerr << "\t" << "size: " << vocab.size() << endl;
    cerr << "\t" << "maximum string length: " << subword_maxlen << endl;
    find_short_factors(vocab, all_chars, 2, utf8_encoding);

    cerr << "Reading word list " << wordlist_fname << endl;
    retval = Unigrams::read_vocab(wordlist_fname, words, word_maxlen, utf8_encoding);
    if (retval < 0) {
        cerr << "something went wrong reading word list" << endl;
        exit(0);
    }
    cerr << "\t" << "wordlist size: " << words.size() << endl;
    cerr << "\t" << "maximum word length: " << word_maxlen << endl;

    cerr << "Reading msfg " << msfg_fname << endl;
    msfg.read(msfg_fname);

    if (vocab.size() < msfg.factor_node_map.size()) {
        vector<string> to_remove;
        cerr << "Pruning " << msfg.factor_node_map.size()-transitions.size() << " unused transitions from msfg." << endl;
        for (auto it = msfg.factor_node_map.begin(); it != msfg.factor_node_map.end(); ++it)
            if (transitions.find(it->first) == transitions.end())
                to_remove.push_back(it->first);
        for (auto it = to_remove.begin(); it != to_remove.end(); ++it)
            msfg.remove_arcs(*it);
    }

    assign_scores(transitions, msfg);

    std::cerr << std::setprecision(15);
    int iteration = 1;
    while (true) {

        cerr << "Iteration " << iteration << endl;

        flt_type lp = Bigrams::collect_trans_stats(words, msfg, trans_stats, unigram_stats);
        Bigrams::copy_transitions(trans_stats, transitions);
        Bigrams::normalize(transitions);

        cerr << "\tbigram cost: " << lp << endl;
        cerr << "\tamount of transitions: " << Bigrams::transition_count(transitions) << endl;
        cerr << "\tvocab size: " << transitions.size() << endl;

        // Write temp transitions
        ostringstream transitions_temp;
        transitions_temp << "transitions.iter" << iteration << ".bz2";
        cerr << "\twriting to: " << transitions_temp.str() << endl;
        Bigrams::write_transitions(transitions, transitions_temp.str());


    }

    // Write transitions
    Bigrams::write_transitions(transitions, transitions_out_fname);

    exit(0);
}
