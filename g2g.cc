#include <iomanip>
#include <sstream>

#include "conf.hh"
#include "Unigrams.hh"
#include "Bigrams.hh"

using namespace std;


int main(int argc, char* argv[]) {

    conf::Config config;
    config("usage: g2g [OPTION...] WORDLIST TRANSITIONS_INIT MSFG_IN TRANSITIONS_OUT\n")
      ('h', "help", "", "", "display help")
      ('c', "candidates=INT", "arg", "5000", "Number of candidate subwords to try to remove per iteration")
      ('r', "removals=INT", "arg", "500", "Number of removals per iteration")
      ('v', "vocab-size=INT", "arg must", "", "Target vocabulary size (stopping criterion)")
      ('m', "temp-models=INT", "arg", "0", "Write out intermediate models for #V mod INT == 0")
      ('f', "forward-backward", "", "", "Use Forward-backward segmentation instead of Viterbi")
      ('8', "utf-8", "", "", "Utf-8 character encoding in use");
    config.default_parse(argc, argv);
    if (config.arguments.size() != 4) config.print_help(stderr, 1);

    unsigned int n_candidates_per_iter = config["candidates"].get_int();
    unsigned int removals_per_iter = config["removals"].get_int();
    unsigned int target_vocab_size = config["vocab-size"].get_int();
    string wordlist_fname = config.arguments[0];
    string initial_transitions_fname = config.arguments[1];
    string msfg_fname = config.arguments[2];
    string transition_fname = config.arguments[3];
    unsigned int temp_vocab_interval = config["temp-models"].get_int();
    bool enable_fb = config["forward-backward"].specified;
    bool utf8_encoding = config["utf-8"].specified;

    cerr << "parameters, wordlist: " << wordlist_fname << endl;
    cerr << "parameters, initial transitions: " << initial_transitions_fname << endl;
    cerr << "parameters, msfg: " << msfg_fname << endl;
    cerr << "parameters, transitions out: " << transition_fname << endl;
    cerr << "parameters, candidates per iteration: " << n_candidates_per_iter << endl;
    cerr << "parameters, removals per iteration: " << removals_per_iter << endl;
    cerr << "parameters, target vocab size: " << target_vocab_size << endl;
    cerr << "parameters, floor lp: " << FLOOR_LP << endl;
    if (temp_vocab_interval > 0)
        cerr << "parameters, write temp models whenever #V modulo " << temp_vocab_interval << " == 0" << endl;
    else
        cerr << "parameters, write temp models: NO" << endl;
    cerr << "parameters, use forward-backward: " << enable_fb << endl;
    cerr << "parameters, utf-8 encoding: " << utf8_encoding << endl;

    int word_maxlen;
    map<string, flt_type> all_chars;
    map<string, flt_type> freqs;
    map<string, flt_type> words;
    MultiStringFactorGraph msfg(start_end_symbol);
    transitions_t transitions;
    transitions_t trans_stats;
    map<string, flt_type> unigram_stats;

    cerr << "Reading initial transitions " << initial_transitions_fname << endl;
    int retval = Bigrams::read_transitions(transitions, initial_transitions_fname);
    if (retval < 0) {
        cerr << "something went wrong reading transitions" << endl;
        exit(0);
    }
    cerr << "\tnumber of transitions: " << Bigrams::transition_count(transitions) << endl;
    cerr << "\tvocabulary size: " << transitions.size() << endl;

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

    if (transitions.size() < msfg.factor_node_map.size()) {
        vector<string> to_remove;
        cerr << "Pruning " << msfg.factor_node_map.size()-transitions.size() << " unused transitions from msfg." << endl;
        for (auto it = msfg.factor_node_map.begin(); it != msfg.factor_node_map.end(); ++it)
            if (transitions.find(it->first) == transitions.end())
                to_remove.push_back(it->first);
        for (auto it = to_remove.begin(); it != to_remove.end(); ++it)
            msfg.remove_arcs(*it);
    }

    std::cerr << std::setprecision(15);
    int iteration = 1;
    unsigned int next_out_vocab_size=0;
    while (true) {

        cerr << "Iteration " << iteration << endl;

        assign_scores(transitions, msfg);
        flt_type lp = Bigrams::collect_trans_stats(words, msfg, trans_stats, unigram_stats, enable_fb);
        transitions.swap(trans_stats);
        Bigrams::freqs_to_logprobs(transitions);
        trans_stats.clear();

        cerr << "\tbigram likelihood: " << lp << endl;
        cerr << "\tnumber of transitions: " << Bigrams::transition_count(transitions) << endl;
        cerr << "\tvocabulary size: " << transitions.size() << endl;

        if (iteration==1) next_out_vocab_size = transitions.size()/temp_vocab_interval * temp_vocab_interval;

        // Get candidate subwords
        cerr << "\tinitializing removals .." << endl;
        map<string, flt_type> candidates;
        Bigrams::init_candidates_freq(n_candidates_per_iter/2, unigram_stats, candidates);
        Bigrams::init_candidates_num_contexts(n_candidates_per_iter, transitions, unigram_stats, candidates);

        // Score all candidates
        cerr << "\tranking removals .." << endl;
        Bigrams::rank_candidate_subwords(words, msfg, unigram_stats, transitions, candidates, enable_fb);

        // Remove subwords
        vector<pair<string, flt_type> > sorted_scores;
        Unigrams::sort_vocab(candidates, sorted_scores, true);
        vector<string> to_remove;
        for (auto it = sorted_scores.begin(); it != sorted_scores.end(); ++it) {
            to_remove.push_back(it->first);
            if (((transitions.size()-to_remove.size()) % removals_per_iter == 0)
                  && to_remove.size() >= (removals_per_iter/2))
                    break;
        }
        Bigrams::remove_transitions(to_remove, transitions);
        for (auto it = to_remove.begin(); it != to_remove.end(); ++it)
            msfg.remove_arcs(*it);

        Bigrams::iterate(words, msfg, transitions, enable_fb, 1);

        // Write intermediate model
        if (temp_vocab_interval > 0
            && transitions.size() <= next_out_vocab_size
            && transitions.size() > target_vocab_size)
        {
            ostringstream transitions_temp;
            transitions_temp << "transitions." << transitions.size() << ".bz2";
            cerr << "\twriting to: " << transitions_temp.str() << endl;
            Bigrams::write_transitions(transitions, transitions_temp.str());
            next_out_vocab_size -= temp_vocab_interval;
        }

        if  (transitions.size() <= target_vocab_size) break;
        iteration++;
    }

    // Write the final model
    Bigrams::write_transitions(transitions, transition_fname);

    exit(0);
}
