#include <iomanip>
#include <sstream>

#include "conf.hh"
#include "Unigrams.hh"
#include "Bigrams.hh"

using namespace std;


int main(int argc, char* argv[]) {

    conf::Config config;
    config("usage: g2gkn [OPTION...] WORDLIST TRANSITIONS_INIT MSFG_IN TRANSITIONS_OUT\n")
      ('h', "help", "", "", "display help")
      ('c', "candidates=INT", "arg", "5000", "Number of candidate subwords to try to remove per iteration")
      ('r', "removals=INT", "arg", "500", "Number of removals per iteration")
      ('v', "vocab-size=INT", "arg must", "", "Target vocabulary size (stopping criterion)")
      ('d', "discount=FLOAT", "arg", "0.1", "Kneser-Ney discount parameter")
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
    flt_type discount = config["discount"].get_float();
    bool enable_fb = config["forward-backward"].specified;
    bool utf8_encoding = config["utf-8"].specified;

    cerr << "parameters, wordlist: " << wordlist_fname << endl;
    cerr << "parameters, initial transitions: " << initial_transitions_fname << endl;
    cerr << "parameters, msfg: " << msfg_fname << endl;
    cerr << "parameters, transitions out: " << transition_fname << endl;
    cerr << "parameters, candidates per iteration: " << n_candidates_per_iter << endl;
    cerr << "parameters, removals per iteration: " << removals_per_iter << endl;
    cerr << "parameters, target vocab size: " << target_vocab_size << endl;
    cerr << "parameters, discount: " << discount << endl;
    cerr << "parameters, floor lp: " << FLOOR_LP << endl;
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
    while (true) {

        cerr << "Iteration " << iteration << endl;

        assign_scores(transitions, msfg);
        flt_type lp = Bigrams::collect_trans_stats(words, msfg, trans_stats, unigram_stats, enable_fb);

        Bigrams::kn_smooth(trans_stats, transitions, discount);
        trans_stats.clear();

        cerr << "\tbigram likelihood: " << lp << endl;
        cerr << "\tnumber of transitions: " << Bigrams::transition_count(transitions) << endl;
        cerr << "\tvocabulary size: " << transitions.size() << endl;

        // Write temp transitions
        ostringstream transitions_temp;
        transitions_temp << "transitions.iter" << iteration << ".bz2";
        cerr << "\twriting to: " << transitions_temp.str() << endl;
        Bigrams::write_transitions(transitions, transitions_temp.str());

        // Get removal candidates based on unigram stats
        cerr << "\tinitializing removals .." << endl;
        map<string, flt_type> candidates;
        Bigrams::init_candidate_subwords(n_candidates_per_iter, unigram_stats, candidates);

        // Score all candidates
        cerr << "\tranking removals .." << endl;
        Bigrams::rank_candidate_subwords(words, msfg, unigram_stats, transitions, candidates, enable_fb);

        // Remove least significant subwords
        vector<pair<string, flt_type> > sorted_scores;
        Unigrams::sort_vocab(candidates, sorted_scores, true);
        vector<string> to_remove;
        for (auto it = sorted_scores.begin(); it != sorted_scores.end(); ++it) {
            to_remove.push_back(it->first);
            if (iteration == 1 && transitions.size() % 1000 != 0) {
                if ((to_remove.size() >= transitions.size() % 1000) && (to_remove.size() > 0)) break;
            }
            else if (to_remove.size() >= removals_per_iter) break;
        }
        Bigrams::remove_transitions(to_remove, transitions);
        for (auto it = to_remove.begin(); it != to_remove.end(); ++it)
            msfg.remove_arcs(*it);

        if  (transitions.size() <= target_vocab_size) break;
        iteration++;
    }

    // Write transitions
    Bigrams::write_transitions(transitions, transition_fname);

    exit(0);
}