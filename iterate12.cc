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


void prune_msfg(const map<string, flt_type> &vocab,
                MultiStringFactorGraph &msfg)
{
    if (vocab.size() < msfg.factor_node_map.size()) {
        vector<string> to_remove;
        cerr << "Pruning " << msfg.factor_node_map.size()-vocab.size() << " unused transitions from msfg." << endl;
        for (auto it = msfg.factor_node_map.begin(); it != msfg.factor_node_map.end(); ++it)
            if (vocab.find(it->first) == vocab.end())
                to_remove.push_back(it->first);
        for (auto it = to_remove.begin(); it != to_remove.end(); ++it)
            msfg.remove_arcs(*it);
        cerr << "Removed " << to_remove.size() << " subwords" << endl;
    }
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
    MultiStringFactorGraph msfg(start_end_symbol);
    msfg.read(msfg_fname);

    if (vocab.find(start_end_symbol) == vocab.end()) vocab[start_end_symbol] = log(0.5);
    prune_msfg(vocab, msfg);

    std::cerr << std::setprecision(15);
    transitions_t transitions;
    transitions_t trans_stats;
    map<string, flt_type> unigram_stats;

    for (int i=0; i<3; i++) {
        cerr << "Unigram iteration " << i << endl;
        assign_scores(vocab, msfg);
        flt_type lp = Bigrams::collect_trans_stats(words, msfg, trans_stats, unigram_stats, true);
        vocab.swap(unigram_stats);
        Unigrams::freqs_to_logprobs(vocab);
        prune_msfg(vocab, msfg);
        if (i>0) {
            cerr << "\tlikelihood: " << lp << endl;
            cerr << "\tvocabulary size: " << vocab.size() << endl;
        }
    }

    transitions = trans_stats;
    Bigrams::normalize(transitions);
    assign_scores(transitions, msfg);
    for (int i=0; i<num_iterations; i++) {
        cerr << "Bigram iteration " << i+1 << endl;
        flt_type lp = Bigrams::collect_trans_stats(words, msfg, trans_stats, unigram_stats, enable_forward_backward);
        Bigrams::copy_transitions(trans_stats, transitions);
        Bigrams::normalize(transitions);
        cerr << "\tlikelihood: " << lp << endl;
        cerr << "\tnumber of transitions: " << Bigrams::transition_count(transitions) << endl;
        cerr << "\tvocabulary size: " << transitions.size() << endl;
    }

    // Write transitions
    Bigrams::write_transitions(transitions, transitions_out_fname);

    exit(0);
}
